if (cpu STREQUAL arm)
	SET(CMAKE_C_COMPILER aarch64-poky-linux-g++)
	LINK_DIRECTORIES(../lib/arm64)
	SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/arm-bin)

else ()
	SET(CMAKE_C_COMPILER g++)
	LINK_DIRECTORIES(../lib/x86)
	SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
endif()

INCLUDE_DIRECTORIES(../include)
INCLUDE_DIRECTORIES(include)

#AUX_SOURCE_DIRECTORY(src DIR_SRCS)
#ADD_EXECUTABLE(perip ${DIR_SRCS})
ADD_EXECUTABLE(ProcPerip src/perip.c src/ringbuffer.c)
TARGET_LINK_LIBRARIES(ProcPerip aawant pthread )

#INCLUDE_DIRECTORIES(../include)
#INCLUDE_DIRECTORIES(include)

ADD_EXECUTABLE(keytest test/test.cpp)
#ADD_EXECUTABLE(testd test/test.c src/user_interface_key.c)
TARGET_LINK_LIBRARIES(keytest pthread)


