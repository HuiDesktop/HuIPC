project(huMessageQueue)

add_library(tools STATIC src/tools.c include/tools.h)
target_include_directories(tools PUBLIC include)

add_library(sharedMemory STATIC src/win/sharedMemory.c include/sharedMemory.h include/dll.h)
target_include_directories(sharedMemory PUBLIC include)
target_link_libraries(sharedMemory PUBLIC tools)

add_library(event STATIC src/win/event.c include/event.h include/dll.h)
target_include_directories(event PUBLIC include)
target_link_libraries(event PUBLIC tools)

add_library(huMessageQueue SHARED src/huMessageQueue.c include/huMessageQueue.h include/dll.h)
