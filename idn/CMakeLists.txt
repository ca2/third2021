cmake_minimum_required(VERSION 3.16)
project(zlib C)

#set(VERSION "1.2.11")

#option(ASM686 "Enable building i686 assembly implementation")
#option(AMD64 "Enable building amd64 assembly implementation")

#set(INSTALL_BIN_DIR "${CMAKE_INSTALL_PREFIX}/bin" CACHE PATH "Installation directory for executables")
#set(INSTALL_LIB_DIR "${CMAKE_INSTALL_PREFIX}/lib" CACHE PATH "Installation directory for libraries")
#set(INSTALL_INC_DIR "${CMAKE_INSTALL_PREFIX}/include" CACHE PATH "Installation directory for headers")
#set(INSTALL_MAN_DIR "${CMAKE_INSTALL_PREFIX}/share/man" CACHE PATH "Installation directory for manual pages")
#set(INSTALL_PKGCONFIG_DIR "${CMAKE_INSTALL_PREFIX}/share/pkgconfig" CACHE PATH "Installation directory for pkgconfig (.pc) files")

#include(CheckTypeSize)
#include(CheckFunctionExists)
#include(CheckIncludeFile)
#include(CheckCSourceCompiles)
#enable_testing()

if(MSVC)
    set(CMAKE_DEBUG_POSTFIX "d")
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
    add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR})
endif()


#set(ZLIB_PC ${CMAKE_CURRENT_BINARY_DIR}/zlib.pc)
#configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/zlib.pc.cmakein
#		${ZLIB_PC} @ONLY)
#configure_file(	${CMAKE_CURRENT_SOURCE_DIR}/zconf.h.cmakein
#		${CMAKE_CURRENT_BINARY_DIR}/zconf.h @ONLY)
#include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR})


#set(IDN_PUBLIC_HDRS
 #   idn/idna.h
#)

#target_include_directories(idn PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../..)
#target_include_directories(idn PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)



set(IDN_PRIVATE_HDRS
        config.h
        lib/tld.h
)
set(IDN_SRCS
        lib/idn-free.c
        lib/idna.c
        lib/nfkc.c
        lib/pr29.c
        lib/profiles.c
        lib/punycode.c
        lib/rfc3454.c
        lib/strerror-idna.c
        lib/strerror-pr29.c
        lib/strerror-punycode.c
        lib/strerror-stringprep.c
        lib/strerror-tld.c
        lib/stringprep.c
        lib/tld.c
        lib/tlds.c
        lib/toutf8.c
        #lib/version.c
)




add_library(idn SHARED ${IDN_SRCS} ${IDN_PUBLIC_HDRS} ${IDN_PRIVATE_HDRS})
target_compile_definitions(idn PRIVATE HAVE_CONFIG_H)
target_include_directories(idn PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(idn PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../../operating-system/third-${OPERATING_SYSTEM_NAME}/third/include/idn)
#add_library(zlibstatic STATIC ${ZLIB_SRCS} ${ZLIB_ASMS} ${ZLIB_PUBLIC_HDRS} ${ZLIB_PRIVATE_HDRS})
set_target_properties(idn PROPERTIES DEFINE_SYMBOL IDN_DLL)
#set_target_properties(idn PROPERTIES SOVERSION 1)


