#cmake_minimum_required(VERSION 3.10)
cmake_minimum_required(VERSION 3.5.1)
project(smartboy)



#if($(var) STREQUAL "ON")

#else()
#SET(CMAKE_CXX_STANDARD 11)
#SET(CMAKE_C_COMPILER c++)
#endif()


include_directories(include)
#include_directories(PROC_ALARM/include)
#include_directories(PROC_IOT/include)
#include_directories(PROC_IOT/src)



ADD_SUBDIRECTORY(aawant)
ADD_SUBDIRECTORY(PROC_AAWANT)
ADD_SUBDIRECTORY(PROC_UPGRADE)
ADD_SUBDIRECTORY(PROC_TEST)
#ADD_SUBDIRECTORY(PROC_KEYEVENT)
#ADD_SUBDIRECTORY(PROC_VOICE)
#ADD_SUBDIRECTORY(PROC_BT)

#ADD_SUBDIRECTORY(PROC_AAWANT)


