

#include(CheckCXXCompilerFlag)
#CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
#CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
#if(COMPILER_SUPPORTS_CXX11)
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
#elseif(COMPILER_SUPPORTS_CXX0X)
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
#else()
#    message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
#endif()




if (cpu STREQUAL arm)
    SET(CMAKE_C_COMPILER aarch64-poky-linux-gcc)
    LINK_DIRECTORIES(../lib/arm64)
    #设置输出路径
    SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib/arm64)
    SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/arm-bin)
else ()
    SET(CMAKE_C_COMPILER gcc)
    LINK_DIRECTORIES(../lib/x86)
    SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib/x86)
    SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
endif()

#ADD_DEFINITIONS( "-Winline")

#ADD_DEFINITIONS("-fgnu89-inline")
INCLUDE_DIRECTORIES(../include)
INCLUDE_DIRECTORIES(include)


AUX_SOURCE_DIRECTORY(src DIR_SRCS)

if (so STREQUAL share)
    ADD_LIBRARY(voiceRecog SHARED ${DIR_SRCS})
else()
    ADD_LIBRARY(voiceRecog STATIC ${DIR_SRCS})
endif ()


INCLUDE_DIRECTORIES(../include)
INCLUDE_DIRECTORIES(include)
ADD_EXECUTABLE(voicon  VoiceConnectService.c )
TARGET_LINK_LIBRARIES(voicon aawant voiceRecog pthread m asound)


#ADD_EXECUTABLE(rec test/AudioQueue.c test/record.c test/cae_thread.c)
#TARGET_LINK_LIBRARIES(rec asound pthread)

