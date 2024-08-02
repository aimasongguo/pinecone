//
// Created by 28192 on 2024/8/2.
//
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdatomic.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/uio.h>
#include <sys/syscall.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/io_uring.h>

#define URING_ENTRIES 1024

struct user_sq {
  unsigned *head;
  unsigned *tail;
  unsigned *ring_mask;
  unsigned *ring_entries;
  unsigned *flags;
  unsigned *dropped;
  unsigned *array;
  struct io_uring_sqe *sqes;
};

struct user_cq {
  unsigned *head;
  unsigned *tail;
  unsigned *ring_mask;
  unsigned *ring_entries;
  unsigned *overflow;
  struct io_uring_cqe *cqes;
};

struct user_ring {
  unsigned int fd;
  struct user_sq sq;
  struct user_cq cq;
};

static struct io_uring_params uring_params = {0};
static struct user_ring user_ring = {0};

static inline int io_uring_setup(__u32 entries, struct io_uring_params *p) {
  return syscall(__NR_io_uring_setup, entries, p);
}

static inline int io_uring_enter(unsigned int fd, __u32 to_submit, __u32 min_complete, __u32 flags, const sigset_t *sig, size_t sigsz) {
  return syscall(__NR_io_uring_enter, fd, to_submit, min_complete, flags, sig, sigsz);
}

static inline int io_uring_register(unsigned int fd, unsigned int opcode, void *arg, unsigned int nr_args) {
  return syscall(__NR_io_uring_register, fd, opcode, arg, nr_args);
}

void munmap_user_ring() {
  if (user_ring.sq.sqes != NULL) {
    munmap(user_ring.sq.sqes, *user_ring.sq.ring_entries * sizeof(struct io_uring_sqe));
  }
  if (user_ring.sq.head != NULL) {
    size_t sq_size = uring_params.sq_off.array + uring_params.sq_entries * sizeof(__u32);
    void *sq_ring_ptr = (void *) user_ring.sq.head - uring_params.sq_off.head;
    munmap(sq_ring_ptr, sq_size);

    if (user_ring.cq.head != NULL) {
      void *cq_ring_ptr = (void *) user_ring.cq.head - uring_params.cq_off.head;
      if (cq_ring_ptr != sq_ring_ptr) {
        size_t cq_size = uring_params.cq_off.cqes + uring_params.cq_entries * sizeof(struct io_uring_cqe);
        munmap(cq_ring_ptr, cq_size);
      }
    }
  }
}

int mmap_user_ring() {
  struct io_sqring_offsets *sq_off = &uring_params.sq_off;
  struct io_cqring_offsets *cq_off = &uring_params.cq_off;

  size_t sq_size = sq_off->array + uring_params.sq_entries * sizeof(__u32);
  size_t cq_size = cq_off->cqes + uring_params.cq_entries * sizeof(struct io_uring_cqe);

  if (uring_params.features & IORING_FEAT_SINGLE_MMAP) {
    if (cq_size > sq_size) {
      sq_size = cq_size;
    }
  }

  void *sq_ptr = mmap(NULL,
                      sq_size,
                      PROT_READ | PROT_WRITE,
                      MAP_SHARED | MAP_POPULATE,
                      user_ring.fd,
                      IORING_OFF_SQ_RING);
  if (sq_ptr == MAP_FAILED) {
    return 1;
  }

  void *cq_ptr = NULL;
  if (uring_params.features & IORING_FEAT_SINGLE_MMAP) {
    cq_ptr = sq_ptr;
  } else {
    cq_ptr = mmap(NULL,
                  cq_size,
                  PROT_READ | PROT_WRITE,
                  MAP_SHARED | MAP_POPULATE,
                  user_ring.fd,
                  IORING_OFF_CQ_RING);
    if (cq_ptr == MAP_FAILED) {
      return 1;
    }
  }

  struct user_sq *sq = &user_ring.sq;
  sq->head = sq_ptr + sq_off->head;
  sq->tail = sq_ptr + sq_off->tail;
  sq->ring_mask = sq_ptr + sq_off->ring_mask;
  sq->ring_entries = sq_ptr + sq_off->ring_entries;
  sq->flags = sq_ptr + sq_off->flags;
  sq->dropped = sq_ptr + sq_off->dropped;
  sq->array = sq_ptr + sq_off->array;

  size_t sqes_size = uring_params.sq_entries * sizeof(struct io_uring_sqe);
  sq->sqes = mmap(NULL,
                  sqes_size,
                  PROT_READ | PROT_WRITE,
                  MAP_SHARED | MAP_POPULATE,
                  user_ring.fd,
                  IORING_OFF_SQES);
  if (sq->sqes == MAP_FAILED) {
    return 1;
  }

  struct user_cq *cq = &user_ring.cq;
  cq->head = cq_ptr + cq_off->head;
  cq->tail = cq_ptr + cq_off->tail;
  cq->ring_mask = cq_ptr + cq_off->ring_mask;
  cq->ring_entries = cq_ptr + cq_off->ring_entries;
  cq->overflow = cq_ptr + cq_off->overflow;
  cq->cqes = cq_ptr + cq_off->cqes;

  return 0;
}

int setup_ring() {
  int fd = io_uring_setup(URING_ENTRIES, &uring_params);
  if (fd == -1) {
    perror("io_uring_setup");
    return 1;
  }
  user_ring.fd = fd;
  if (mmap_user_ring()) {
    perror("mmap");
    return 1;
  }
  return 0;
}

int cleanup_ring() {
  munmap_user_ring();
  if (user_ring.fd != 0) {
    close(user_ring.fd);
  }
  return 0;
}

#define BUFFER_GROUP_ID 1
#define BUFFER_LEN 1024
#define BUFFER_NUM 1024
#define BUFFER_PTR(_p, _bid) (_p + BUFFER_LEN * (uintptr_t) _bid)

#define IS_CQ_READY(head, tail) ((head) != atomic_load_explicit((tail), memory_order_acquire))

#define SET_SQE_FIELDS(_sqe, _op, _flags, _fd, _off, _addr, _len, _user_data) { \
    _sqe->opcode = _op; \
    _sqe->flags = _flags; \
    _sqe->ioprio = 0; \
    _sqe->fd = _fd; \
    _sqe->off = (__u64) (_off); \
    _sqe->addr = (__u64) (_addr); \
    _sqe->len = _len; \
    _sqe->rw_flags = 0; \
    _sqe->user_data = (__u64) (_user_data); \
    _sqe->__pad2[0] = _sqe->__pad2[1] = _sqe->__pad2[2] = 0; \
}

enum user_op {
  user_op_nop = 0,
  user_op_provide_buffers = 1,
  user_op_accept = 2,
  user_op_recv = 3,
  user_op_send = 4,
  user_op_close = 5,
};

struct local_ring {
  unsigned head;
  unsigned tail;
};

struct user_event {
  __u32 fd;
  __u8 op;
  __u8 retry;
  __u16 bid;
};

static void *buffers;
static int listen_fd = 0;
static struct local_ring cached_sq = {0};
static bool deferred_accept = 0;

struct io_uring_sqe *acquire_sqe() {
  unsigned head = atomic_load_explicit(user_ring.sq.head, memory_order_acquire);
  unsigned tail = cached_sq.tail;
  if (tail - head >= *user_ring.sq.ring_entries) {
    return NULL;
  }
  cached_sq.tail = tail + 1;
  return &user_ring.sq.sqes[tail & *user_ring.sq.ring_mask];
}

unsigned flush_cached_sqes() {
  struct user_sq *sq = &user_ring.sq;
  unsigned tail = *sq->tail;
  unsigned ring_mask = *sq->ring_mask;
  unsigned to_submit = cached_sq.tail - cached_sq.head;
  while (cached_sq.head != cached_sq.tail) {
    sq->array[tail & ring_mask] = cached_sq.head & ring_mask;
    cached_sq.head++;
    tail++;
  }
  if (to_submit) {
    atomic_store_explicit(sq->tail, tail, memory_order_release);
  }
  return to_submit;
}

bool add_provide_buffer_op(unsigned bgid, void *bufs, size_t buflen, size_t nbuf, unsigned bid) {
  struct io_uring_sqe *sqe = acquire_sqe();
  if (sqe == NULL) {
    return 0;
  }

  _Static_assert(sizeof(struct user_event) == 8, "unexpect user_data size");

  struct user_event event = {
      .op = user_op_provide_buffers,
  };
  SET_SQE_FIELDS(sqe, IORING_OP_PROVIDE_BUFFERS, 0, nbuf, bid, bufs, buflen, *(__u64 *) &event);
  sqe->buf_group = bgid;
  return 1;
}

bool add_accept_op(int fd, struct sockaddr_in *addr, size_t *addr_len) {
  struct io_uring_sqe *sqe = acquire_sqe();
  if (sqe == NULL) {
    return false;
  }

  struct user_event event = {
      .op = user_op_accept,
  };
  SET_SQE_FIELDS(sqe, IORING_OP_ACCEPT, 0, fd, addr_len, addr, 0, *(__u64 *) &event);
  sqe->accept_flags = 0;
  return true;
}

bool add_recv_buf_op(int fd, unsigned gid, size_t len, int retry) {
  struct io_uring_sqe *sqe = acquire_sqe();
  if (sqe == NULL) {
    return false;
  }

  struct user_event event = {
      .fd = fd,
      .op = user_op_recv,
      .retry = retry,
  };
  SET_SQE_FIELDS(sqe, IORING_OP_RECV, IOSQE_BUFFER_SELECT, fd, 0, NULL, len, *(__u64 *) &event);
  sqe->buf_group = gid;
  return true;
}

bool add_send_buf_op(int fd, __u16 bid, size_t len) {
  struct io_uring_sqe *sqe = acquire_sqe();
  if (sqe == NULL) {
    return false;
  }

  struct user_event event = {
      .fd = fd,
      .op = user_op_send,
      .bid = bid,
  };
  SET_SQE_FIELDS(sqe, IORING_OP_SEND, 0, fd, 0, BUFFER_PTR(buffers, bid), len, *(__u64 *) &event);
  return true;
}

bool add_close_op(int fd) {
  struct io_uring_sqe *sqe = acquire_sqe();
  if (sqe == NULL) {
    return false;
  }

  struct user_event event = {
      .fd = fd,
      .op = user_op_close,
  };
  SET_SQE_FIELDS(sqe, IORING_OP_CLOSE, 0, fd, 0, NULL, 0, *(__u64 *) &event);
  return true;
}


int close_fd(int fd) {
  if (add_close_op(fd)) {
    return 0;
  }
  return close(fd);
}

int handle_cqe(struct io_uring_cqe *cqe) {
  unsigned bid;
  int res = cqe->res;
  struct user_event ev = *(struct user_event *) &cqe->user_data;
  // fprintf(stderr, "handle_cq: op=%d res=%d\n", ev.op, res);

  switch (ev.op) {
    case user_op_provide_buffers:
      if (res < 0) {
        fprintf(stderr, "op_provide_buffers result: %s\n", strerror(-res));
        exit(1);
      }
      break;
    case user_op_accept:
      if (res < 0) {
        // kernel 不支持 opcode 返回 -EINVAL 或者 -EMFILE 等 fd 不足会导致 accept 失败
        // 目前直接退出
        fprintf(stderr, "op_accept result: %s\n", strerror(-res));
        exit(1);
      }
      int fd = res;
      if (!add_recv_buf_op(fd, BUFFER_GROUP_ID, BUFFER_LEN, 0)) {
        // 分配 SQE 失败直接关闭
        close_fd(fd);
      }
      if (!add_accept_op(listen_fd, NULL, NULL)) {
        deferred_accept = true;
      }
      break;
    case user_op_recv:
      bid = cqe->flags >> IORING_CQE_BUFFER_SHIFT;
      if (res <= 0) {
        if ((cqe->flags & IORING_CQE_F_BUFFER) && !add_provide_buffer_op(BUFFER_GROUP_ID, BUFFER_PTR(buffers, bid), BUFFER_LEN, 1, bid)) {
          return 0;
        }
        if (res == -ENOBUFS && !ev.retry && add_recv_buf_op(ev.fd, BUFFER_GROUP_ID, BUFFER_LEN, 1)) {
          break;
        }
        fprintf(stderr, "op_recv %d result: %s\n", ev.fd, strerror(-res));
        close_fd(ev.fd);
        break;
      }
      if (!add_send_buf_op(ev.fd, bid, res)) {
        close(ev.fd);
      }
      break;
    case user_op_send:
      if (res < 0) {
        fprintf(stderr, "op_send %d result: %s\n", ev.fd, strerror(-res));
      }
      if (!add_provide_buffer_op(BUFFER_GROUP_ID, BUFFER_PTR(buffers, ev.bid), BUFFER_LEN, 1, ev.bid)) {
        return 0;
      }
      if (!add_recv_buf_op(ev.fd, BUFFER_GROUP_ID, BUFFER_LEN, 0)) {
        close(ev.fd);
      }
      break;
    case user_op_close:
      if (res < 0) {
        fprintf(stderr, "op_close %d result: %s\n", ev.fd, strerror(-res));
      }
      break;
    case user_op_nop:
      break;
    default:
      fprintf(stderr, "unrecognized op: %d\n", ev.op);
      break;
  }
  return 1;
}

void server_loop() {
  buffers = malloc(BUFFER_LEN * BUFFER_NUM);
  if (!add_provide_buffer_op(BUFFER_GROUP_ID, buffers, BUFFER_LEN, BUFFER_NUM, 0)) {
    fprintf(stderr, "add_provide_buffer_op failed\n");
    return;
  }

  if (!add_accept_op(listen_fd, NULL, NULL)) {
    fprintf(stderr, "add_accept_op failed\n");
    return;
  }

  struct user_cq *cq = &user_ring.cq;
  while (1) {
    if (deferred_accept && add_accept_op(listen_fd, NULL, NULL)) {
      deferred_accept = false;
    }

    unsigned cq_head = *cq->head;
    bool consume = false;
    while (IS_CQ_READY(cq_head, cq->tail)) {
      struct io_uring_cqe *cqe = &cq->cqes[cq_head & *cq->ring_mask];
      if (!handle_cqe(cqe)) {
        break;
      }
      cq_head++;
      consume = true;
    }
    if (consume) {
      atomic_store_explicit(cq->head, cq_head, memory_order_release);
    }

    unsigned to_submit = flush_cached_sqes();
    unsigned min_complete = 1;
    if (IS_CQ_READY(cq_head, cq->tail)) {
      min_complete = 0;
    }
    int res = io_uring_enter(user_ring.fd, to_submit, min_complete, IORING_ENTER_GETEVENTS, NULL, 0);
    if (res < 0) {
      // NOTE: 可能存在死锁情况
      //  在 acquire_sqe 在 SQ 已满时会失败，handle_cqe 返回 0，CQ head 不会被消费
      //  以及 FEATURE_NODROP，CQ 最终堆积满，产生 EBUSY 错误
      //  把 EBUSY 且 CQ 不消费看作致命错误，后续考虑加入跳过或者本地 cache 的设计
      if (errno == EBUSY && consume) {
        continue;
      }
      perror("io_uring_enter");
      return;
    }
  }
}

int listen_tcp(int port) {
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0) {
    perror("socket");
    return -1;
  }

  int enable = 1;
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
    perror("setsockopt");
    goto fail;
  }

  struct sockaddr_in la = {
      .sin_family = AF_INET,
      .sin_port = htons(port),
      .sin_addr = { .s_addr = htonl(INADDR_ANY) },
  };
  if (bind(fd, (const struct sockaddr *) &la, sizeof(struct sockaddr_in)) < 0) {
    perror("bind");
    goto fail;
  }

  if (listen(fd, SOMAXCONN) < 0) {
    perror("listen");
    goto fail;
  }
  return fd;

  fail:
  close(fd);
  return -1;
}

bool is_all_ops_supported(__u8 *ops, size_t opnum) {
  struct io_uring_probe probe = {0};
  if (io_uring_register(user_ring.fd, IORING_REGISTER_PROBE, &probe, 0) < 0) {
    perror("io_uring_register");
    return false;
  }
  for (size_t i = 0; i < opnum; i++) {
    if (ops[i] > probe.last_op) {
      return false;
    }
  }
  return true;
}

int main() {
  if (setup_ring()) {
    return 1;
  }

  const __u32 required_features = IORING_FEAT_NODROP | IORING_FEAT_FAST_POLL;
  if ((uring_params.features & required_features) != required_features) {
    fprintf(stderr, "required features %x, supported: %x\n", required_features, uring_params.features);
    return 1;
  }
#define REQUIRED_OPS_NUM 5
  static __u8 required_ops[REQUIRED_OPS_NUM] = {
      IORING_OP_PROVIDE_BUFFERS,
      IORING_OP_ACCEPT,
      IORING_OP_SEND,
      IORING_OP_RECV,
      IORING_OP_CLOSE,
  };
  if (!is_all_ops_supported(required_ops, REQUIRED_OPS_NUM)) {
    fprintf(stderr, "some op not supported\n");
    return 1;
  }

  int fd = listen_tcp(3246);
  if (fd < 0) {
    return 1;
  }
  listen_fd = fd;

  server_loop();

  cleanup_ring();
  return 0;
}