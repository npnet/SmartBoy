
if (cpu STREQUAL arm)
    SET(CMAKE_C_COMPILER aarch64-poky-linux-g++)
    LINK_DIRECTORIES(../lib/arm64)
    #设置输出路径
    SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib/arm64)
    SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/arm-bin)
else ()
    SET(CMAKE_C_COMPILER g++)
    LINK_DIRECTORIES(../lib/x86)
    SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib/x86)
    SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
endif ()
INCLUDE_DIRECTORIES(../include)
INCLUDE_DIRECTORIES(include)


AUX_SOURCE_DIRECTORY(src DIR_SRCS)

if (so STREQUAL share)
    ADD_LIBRARY(voiceRecog SHARED ${DIR_SRCS})
else()
    ADD_LIBRARY(voiceRecog STATIC ${DIR_SRCS})
endif ()



#ADD_EXECUTABLE(voice ${DIR_SRCS})
#TARGET_LINK_LIBRARIES(voice pthread m asound)


ADD_EXECUTABLE(ProcNetConfigure  VoiceConnectService.c md5.c )
TARGET_LINK_LIBRARIES(ProcNetConfigure aawant voiceRecog pthread m asound curl)

ADD_EXECUTABLE(main  main.c md5.c)
TARGET_LINK_LIBRARIES(main aawant voiceRecog pthread m asound)

#ADD_EXECUTABLE(rec test/AudioQueue.c test/record.c test/cae_thread.c)
#TARGET_LINK_LIBRARIES(rec asound pthread)
#