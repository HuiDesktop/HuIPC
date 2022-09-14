project(huMessageQueue)

set(CMAKE_C_STANDARD 11)

add_library(tools STATIC src/tools.c "include/huIPC/tools.h")
target_include_directories(tools PUBLIC include/huIPC)

add_library(sharedMemory STATIC src/win/sharedMemory.c "include/huIPC/sharedMemory.h" "include/huIPC/dll.h")
target_include_directories(sharedMemory PUBLIC include/huIPC)
target_link_libraries(sharedMemory PUBLIC tools)

add_library(event STATIC src/win/event.c "include/huIPC/event.h" "include/huIPC/dll.h")
target_include_directories(event PUBLIC include/huIPC)
target_link_libraries(event PUBLIC tools)

add_library(huMessageQueue SHARED src/huMessageQueue.c "include/huIPC/huMessageQueue.h" "include/huIPC/dll.h")
target_include_directories(huMessageQueue PUBLIC include/huIPC)
target_link_libraries(huMessageQueue PUBLIC sharedMemory)
target_link_libraries(huMessageQueue PUBLIC event)

add_executable(tests src/test_cir.c)
target_link_libraries(tests PRIVATE huMessageQueue)
