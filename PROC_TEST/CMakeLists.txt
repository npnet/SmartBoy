
if (cpu STREQUAL arm)
    SET(CMAKE_C_COMPILER aarch64-poky-linux-g++)
    LINK_DIRECTORIES(../lib/arm64)
else ()
    SET(CMAKE_C_COMPILER g++)
    LINK_DIRECTORIES(../lib/x86)
endif()

INCLUDE_DIRECTORIES(../include)
INCLUDE_DIRECTORIES(include)

#AUX_SOURCE_DIRECTORY(src DIR_SRCS)
ADD_EXECUTABLE(mytest ./src/test.c)


TARGET_LINK_LIBRARIES(mytest aawant  pthread curl)


