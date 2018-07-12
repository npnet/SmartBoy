SET(CMAKE_C_COMPILER g++)


if (cpu STREQUAL arm)
    LINK_DIRECTORIES(../lib/arm64)
else ()
    LINK_DIRECTORIES(../lib/x86)
endif()

INCLUDE_DIRECTORIES(../include)
INCLUDE_DIRECTORIES(include)

AUX_SOURCE_DIRECTORY(src DIR_SRCS)
#ADD_EXECUTABLE(upgraded ${DIR_SRCS})


ADD_EXECUTABLE(upgraded src/AawantUpgrade.c
        src/upg_control.c
        src/upg_control_cli.c
        src/upg_download.c
        src/upg_utility.c)
TARGET_LINK_LIBRARIES(upgraded aawant curl pthread)


#AUX_SOURCE_DIRECTORY()
ADD_EXECUTABLE(upgrade_test test/update_test.cpp
        src/upg_control.c
        src/upg_control_cli.c
        src/upg_download.c
        src/upg_utility.c)

TARGET_LINK_LIBRARIES(upgrade_test  curl pthread)
