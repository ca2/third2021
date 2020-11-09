cmake_minimum_required(VERSION 3.17)
project(sn C)

set(CMAKE_C_STANDARD 11)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(LINUX TRUE)
    set(PLATFORM_NAME "linux")
    message("System is Linux")
    include(FindPkgConfig)
elseif(WIN32)
    set(PLATFORM_NAME "windows")
elseif(UWP)
    set(PLATFORM_NAME "uwp")
    set(WINDOWS_TSF TRUE)
endif()


set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../../time-${PLATFORM_NAME}/cmake)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../../time-${PLATFORM_NAME}/cmake)


add_library(sn SHARED
        config.h
        sn-common.c
        sn-common.h
        sn-internals.c
        sn-internals.h
        sn-launchee.c
        sn-launchee.h
        sn-launcher.c
        sn-launcher.h
        sn-list.c
        sn-list.h
        sn-monitor.c
        sn-monitor.h
        sn-util.c
        sn-util.h
        sn-xmessages.c
        sn-xmessages.h
        sn-xutils.c
        sn-xutils.h
        sn.h)


target_link_options(sn PRIVATE -Wl,-rpath=\$ORIGIN)
target_link_libraries(sn PRIVATE X11-xcb xcb xcb-util)
target_link_options(sn PRIVATE -Wl,-z,defs)
