cmake_minimum_required(VERSION 2.8)
project(FrameStoreClient)

set(SOURCES include/FrameStoreClient/Frame.h
            include/FrameStoreClient/RecorderURLS.h
            include/FrameStoreClient/ResultParser.h
            include/FrameStoreClient/Video.h
            source/Frame.cpp
            source/RecorderURLS.cpp
            source/ResultParser.cpp
            source/Video.cpp)

set(WINDOWS_LIBS XSDK ws2_32 Webby AVKit)
set(LINUX_LIBS XSDK Webby AVKit)

include(common.cmake NO_POLICY_SCOPE)
include("${devel_artifacts_path}/build/base_lib.cmake" NO_POLICY_SCOPE)

