
if (cpu STREQUAL arm)
    SET(CMAKE_C_COMPILER aarch64-poky-linux-g++)
    LINK_DIRECTORIES(../lib/arm64)
    #设置输出路径
    SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib/arm64)
else ()
    SET(CMAKE_C_COMPILER gcc)
    LINK_DIRECTORIES(../lib/x86)
    SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib/x86)
endif()



INCLUDE_DIRECTORIES(../include)
INCLUDE_DIRECTORIES(include)


AUX_SOURCE_DIRECTORY(src DIR_SRCS)

if (so STREQUAL share)
    ADD_LIBRARY(voice SHARED ${DIR_SRCS})
else()
    ADD_LIBRARY(voice STATIC ${DIR_SRCS})
endif ()


INCLUDE_DIRECTORIES(../include)
INCLUDE_DIRECTORIES(include)
ADD_EXECUTABLE(voicon VoiceConnectIf.cpp VoiceConnectService.c)
TARGET_LINK_LIBRARIES(voicon aawant voice)
