cmake_minimum_required(VERSION 2.8)
project(ut)

include_directories("../include")

set(SOURCES source/TestDataSources.cpp 
    source/framework.cpp)

set(COMMON_LIBS FrameStoreClient XSDK Webby )
set(WINDOWS_LIBS XSDK DbgHelp ws2_32 iphlpapi)

include(common.cmake NO_POLICY_SCOPE)
include("${devel_artifacts_path}/build/base_ut.cmake" NO_POLICY_SCOPE)
