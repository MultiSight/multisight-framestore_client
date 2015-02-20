
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// XSDK
// Copyright (c) 2015 Schneider Electric
//
// Use, modification, and distribution is subject to the Boost Software License,
// Version 1.0 (See accompanying file LICENSE).
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#ifndef __FRAME_STORE_CLIENT_RecorderURLS_h
#define __FRAME_STORE_CLIENT_RecorderURLS_h

#include "XSDK/XString.h"
#include "XSDK/XSharedLib.h"

namespace FRAME_STORE_CLIENT
{

class RecorderURLS
{
public:
    X_API RecorderURLS( const XSDK::XString& dataSourceID,
                        const XSDK::XString& startTime,
                        const XSDK::XString& endTime,
                        int64_t requestSize,
                        bool keyFrameOnly = false );

    X_API RecorderURLS( const XSDK::XString& dataSourceID,
                        const XSDK::XString& startTime,
                        const XSDK::XString& endTime,
                        double speed );

    X_API ~RecorderURLS() throw();

    X_API bool GetNextURL( XSDK::XString& url );

    X_API bool KeyFrameOnly() const { return _keyFrameOnly; }

private:
    RecorderURLS( const RecorderURLS& obj );
    RecorderURLS& operator = ( const RecorderURLS& obj );

    XSDK::XString _dataSourceID;
    XSDK::XString _startTime;
    XSDK::XString _endTime;
    int64_t _requestStart;
    bool _done;
    bool _firstRequest;
    int64_t _requestSize;
    bool _keyFrameOnly;
};

}

#endif
