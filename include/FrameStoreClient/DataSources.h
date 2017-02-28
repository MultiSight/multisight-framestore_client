
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// XSDK
// Copyright (c) 2015 Schneider Electric
//
// Use, modification, and distribution is subject to the Boost Software License,
// Version 1.0 (See accompanying file LICENSE).
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#ifndef __FRAME_STORE_CLIENT_DataSources_h
#define __FRAME_STORE_CLIENT_DataSources_h

#include "XSDK/XMemory.h"
#include "XSDK/XString.h"
#include "XSDK/XSharedLib.h"

namespace FRAME_STORE_CLIENT
{
    typedef enum _SourceType_
    {
        Video, Audio, MetaData
    }SourceType;

    typedef struct _DataSource_
    {
        XSDK::XString _id;
        SourceType _type;
        XSDK::XString _sourceUrl;
        bool _recordingVideo;
        bool _recordingAudio;
        bool _recordingMetaData;
        bool _videoHealty;
        bool _audioHealthy;
        bool _metaDataHealthy;
    }DataSource;

X_API std::vector<DataSource> FetchDataSources( const XSDK::XString& recorderIP, int recorderPort );

}

#endif
