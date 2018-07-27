
if (cpu STREQUAL arm)
    SET(CMAKE_C_COMPILER aarch64-poky-linux-g++)
    LINK_DIRECTORIES(../lib/arm64)
    SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/arm-bin)

else ()
    SET(CMAKE_C_COMPILER g++)
    LINK_DIRECTORIES(../lib/x86)
    SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
endif ()


INCLUDE_DIRECTORIES(../include)
INCLUDE_DIRECTORIES(include)

AUX_SOURCE_DIRECTORY(src DIR_SRCS)
#ADD_EXECUTABLE(upgraded ${DIR_SRCS})


ADD_EXECUTABLE(upgraded src/AawantUpgrade.c
        src/upg_control.c
        src/upg_download.c
        src/upg_utility.c)
TARGET_LINK_LIBRARIES(upgraded aawant pthread curl)

#if(t STREQUAL test)
#AUX_SOURCE_DIRECTORY()
ADD_EXECUTABLE(upgrade_test test/update_test.cpp
        src/upg_control.c
        src/upg_download.c
        src/upg_utility.c
        src/HttpClient.cpp)

TARGET_LINK_LIBRARIES(upgrade_test aawant curl pthread)
#endif()