cmake_minimum_required(VERSION 3.17)
project(sn C)

set(CMAKE_C_STANDARD 11)


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


#target_link_options(sn PRIVATE -Wl,-rpath=$ORIGIN)
target_link_libraries(sn PRIVATE X11-xcb xcb xcb-util)
target_link_options(sn PRIVATE -Wl,-z,defs)
