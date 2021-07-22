cmake_minimum_required(VERSION 3.17)
project(audio CXX)


list(APPEND library_references
   acme
   apex
   aqua
   id3lib
   )


list(APPEND library_source
    audio/_.h
    audio/_const.h
    audio/audio.cpp
    audio/audio.h
   audio/id3.cpp
    audio/id3.h
    audio/interprocess_communication.cpp
    audio/interprocess_communication.h
    audio/ms_translator_speaker.cpp
    audio/ms_translator_speaker.h
    audio/sound_track_player.cpp
    audio/sound_track_player.h
    audio/sound_track_player_container.cpp
    audio/sound_track_player_container.h
    decode/_.h
    decode/decoder.cpp
    decode/decoder.h
    decode/decoder_plugin.cpp
    decode/decoder_plugin.h
    decode/decoder_plugin_set.cpp
    decode/decoder_plugin_set.h
    decode/encoder.cpp
    decode/encoder.h
    decode/encoder_plugin.cpp
    decode/encoder_plugin.h
    decode/encoder_plugin_set.cpp
    decode/encoder_plugin_set.h
   factory_exchange.cpp
    iaudio/_.h
    iaudio/wave_stream_effect.cpp
    iaudio/wave_stream_effect.h
    iaudio/wave_stream_effect_chain.cpp
    iaudio/wave_stream_effect_chain.h
    iaudio/wave_stream_effect_event_listener.h
    iaudio/wave_stream_effect_listener.cpp
    iaudio/wave_stream_effect_order.cpp
    iaudio/wave_stream_effect_order.h
    imedia/_.h
    imedia/position.cpp
    imedia/position.h
    imedia/position_array.cpp
    imedia/position_array.h
    imedia/time.cpp
    imedia/time.h
    imedia/time_array.cpp
    imedia/time_array.h
    #ortp/_.h
    #ortp/aaa_want_to_remove_ortp.cpp
    #ortp/avprofile.cpp
    #ortp/b64.cpp
    #ortp/b64.h
    #ortp/event.cpp
    #ortp/event.h
    #ortp/jitterctl.cpp
    #ortp/jitterctl.h
    #ortp/ortp-config-linux.h
    #ortp/ortp-config-win32.h
    #ortp/ortp.cpp
    #ortp/ortp_utils.h
    #ortp/payloadtype.cpp
    #ortp/payloadtype.h
    #ortp/port.cpp
    #ortp/port.h
    #ortp/posixtimer.cpp
    #ortp/rtcp.cpp
    #ortp/rtcp.h
    #ortp/rtcpparse.cpp
    #ortp/rtp.h
    #ortp/rtpfile.cpp
    #ortp/rtpfile.h
    #ortp/rtpparse.cpp
    #ortp/rtpsession.cpp
    #ortp/rtpsession.h
    #ortp/rtpsession_inet.cpp
    #ortp/rtpsession_priv.h
    #ortp/rtpsignaltable.cpp
    #ortp/rtpsignaltable.h
    #ortp/rtptimer.cpp
    #ortp/rtptimer.h
    #ortp/scheduler.cpp
    #ortp/scheduler.h
    #ortp/sessionset.cpp
    #ortp/sessionset.h
    #ortp/srtp.cpp
    #ortp/srtp.h
    #ortp/str_utils.cpp
    #ortp/str_utils.h
    #ortp/stun.cpp
    #ortp/stun.h
    #ortp/stun_udp.cpp
    #ortp/stun_udp.h
    #ortp/telephonyevents.cpp
    #ortp/telephonyevents.h
    #ortp/utils.cpp
    #ortp/utils.h
    plugin/_.h
    plugin/mixer.cpp
    plugin/mixer.h
    plugin/playground.cpp
    plugin/playground.h
    plugin/plugin.cpp
    plugin/plugin.h
    plugin/resampler.cpp
    plugin/resampler.h
    plugin/sampler.cpp
    plugin/sampler.h
    plugin/source.cpp
    plugin/source.h
    plugin/track.cpp
    plugin/track.h
    #srtp/aes_tables.cpp
    #srtp/config.h
    #srtp/config2.h
    #srtp/config_in.h
    #srtp/getopt_s.h
    #srtp/rtp.h
    #srtp/rtp_priv.h
    #srtp/srtp.cpp
    #srtp/srtp.h
    #srtp/srtp_priv.h
    wave/_.h
    wave/base.cpp
    wave/base.h
    wave/buffer.cpp
    wave/buffer.h
    wave/command.h
    wave/device_player.cpp
    wave/device_player.h
    wave/effect_crescendo.cpp
    wave/effect_crescendo.h
    wave/file.cpp
    wave/file.h
    wave/format.cpp
    wave/format.h
    wave/global.h
    wave/in.cpp
    wave/in.h
    wave/in_listener.cpp
    wave/in_listener.h
    wave/in_listener_set.cpp
    wave/in_listener_set.h
    wave/logical_player.cpp
    wave/logical_player.h
    wave/out.cpp
    wave/out.h
    wave/player.cpp
    wave/player.h
    wave/player_command.cpp
    wave/player_command.h
    wave/player_container.cpp
    wave/player_container.h
    wave/pre_buffer.cpp
    wave/pre_buffer.h
    wave/recorder.cpp
    wave/recorder.h
    wave/recorder_command.h
    wave/recorder_container.cpp
    wave/recorder_container.h
    wave/wave.cpp
    wave/wave.h
    _.cpp
    _.h
    audio_wave_base.cpp
    framework.cpp
    framework.h
    )


if(WIN32)

    list(APPEND audio_source
       #audio/windows/tts_speaker.cpp
       #audio/windows/tts_speaker.h
       )

endif()



add_library(audio SHARED ${library_source})

target_precompile_headers(audio PRIVATE framework.h)

set_target_properties(audio PROPERTIES LINKER_LANGUAGE CXX)

if (MSVC)
    target_compile_options(audio PRIVATE "/MD$<$<CONFIG:Debug>:d>")
    target_link_options(audio PRIVATE "/DEFAULTLIB:msvcrt$<$<CONFIG:Debug>:d>")
    target_link_options(audio PRIVATE "/SUBSYSTEM:WINDOWS")
    target_link_libraries(audio PRIVATE ${library_references})
    target_link_libraries(audio PRIVATE ws2_32 winmm shlwapi)
elseif(LINUX)
    target_compile_options(audio PRIVATE -fpermissive)
    #target_link_options(audio PRIVATE -Wl,-rpath=$ORIGIN)
    target_link_libraries(audio PRIVATE pthread)
    target_link_libraries(audio PRIVATE ${library_references})
    target_link_options(audio PRIVATE -Wl,-z,defs)
endif()




target_include_directories(audio PRIVATE .)
target_include_directories(audio PRIVATE $ENV{HOME}/sensitive/sensitive/include)
target_include_directories(audio PRIVATE $ENV{__sensitive}/sensitive/include)
target_include_directories(audio PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_include_directories(audio PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_include_directories(audio PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})


target_compile_definitions(audio PRIVATE _APP_CORE_AUDIO_LIBRARY)



