cmake_minimum_required(VERSION 3.16)
project(id3lib CXX)


list(APPEND library_references
   acme
   )


list(APPEND library_source
   ca2_readers.cpp
    #src/c_wrapper.cpp
   src/field.cpp
   src/field_binary.cpp
   src/field_def.h
   src/field_impl.h
   src/field_integer.cpp
   src/field_string_ascii.cpp
   src/field_string_unicode.cpp
   src/flags.h
   src/frame.cpp
   src/frame_def.h
   src/frame_impl.cpp
   src/frame_impl.h
   src/frame_parse.cpp
   src/frame_render.cpp
   src/globals.cpp
   src/header.cpp
   src/header.h
   src/header_frame.cpp
   src/header_frame.h
   src/header_tag.cpp
   src/header_tag.h
   src/helpers.cpp
   src/io.cpp
   src/io_decorators.cpp
   src/io_helpers.cpp
   src/misc_support.cpp
   src/mp3_header.h
   src/mp3_parse.cpp
   src/readers.cpp
   src/spec.cpp
   src/spec.h
   src/tag.cpp
   src/tag_file.cpp
   src/tag_find.cpp
   src/tag_impl.cpp
   src/tag_impl.h
   src/tag_parse.cpp
   src/tag_parse_lyrics3.cpp
   src/tag_parse_musicmatch.cpp
   src/tag_parse_v1.cpp
   src/tag_render.cpp
   src/utils.cpp
   src/writers.cpp


   )


if(WIN32)

    #list(APPEND audio_source
       #id3lib/windows/tts_speaker.cpp
       #id3lib/windows/tts_speaker.h
     #  )

endif()



add_library(id3lib SHARED ${library_source})

target_precompile_headers(id3lib PRIVATE framework.h)

set_target_properties(id3lib PROPERTIES LINKER_LANGUAGE CXX)

target_compile_definitions(id3lib PRIVATE HAVE_CONFIG_H)

if (MSVC)
    target_compile_options(id3lib PRIVATE "/MD$<$<CONFIG:Debug>:d>")
    target_link_options(id3lib PRIVATE "/DEFAULTLIB:msvcrt$<$<CONFIG:Debug>:d>")
    target_link_options(id3lib PRIVATE "/SUBSYSTEM:WINDOWS")
    target_link_libraries(id3lib PRIVATE ${library_references})
    target_link_libraries(id3lib PRIVATE ws2_32 winmm shlwapi)
elseif(LINUX OR FREEBSD)
    target_compile_options(id3lib PRIVATE -fpermissive)
    #target_link_options(id3lib PRIVATE -Wl,-rpath=$ORIGIN)
    target_link_libraries(id3lib PRIVATE pthread z)
    target_link_libraries(id3lib PRIVATE ${library_references})
    target_link_options(id3lib PRIVATE -Wl,-z,defs)
endif()




target_include_directories(id3lib PRIVATE .)
target_include_directories(id3lib PRIVATE $ENV{HOME}/sensitive/sensitive/include)
target_include_directories(id3lib PRIVATE $ENV{__sensitive}/sensitive/include)
list(APPEND library_include_directories ${CMAKE_CURRENT_SOURCE_DIR}/../..)
list(APPEND library_include_directories ${CMAKE_CURRENT_SOURCE_DIR}/..)
list(APPEND library_include_directories ${CMAKE_CURRENT_SOURCE_DIR})


target_include_directories(${PROJECT_NAME} PRIVATE ${library_include_directories})
target_include_directories(id3lib PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)


target_compile_definitions(id3lib PRIVATE _APP_CORE_AUDIO_LIBRARY)



