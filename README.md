# pinecone
pinecone

add clion config
debug
```config
-DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=./conan_provider.cmake
build\x64_Debug
--config Debug -- -j 16
```

release
```config
-DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=./conan_provider.cmake
build\x64_Release
--config Release -- -j 16```
```


```shell
cmake -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=./conan_provider.cmake -DCMAKE_BUILD_TYPE=Release -B build/x64_Release
cmake -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=./conan_provider.cmake -DCMAKE_BUILD_TYPE=Debug -B build/x64_Debug
cmake --build build/x64_Release -- -j16
cmake --build build/x64_Debug -- -j16
```