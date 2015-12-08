
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// XSDK
// Copyright (c) 2015 Schneider Electric
//
// Use, modification, and distribution is subject to the Boost Software License,
// Version 1.0 (See accompanying file LICENSE).
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#include "FrameStoreClient/RecorderURLS.h"

#include "XSDK/XTime.h"

using namespace FRAME_STORE_CLIENT;
using namespace XSDK;

RecorderURLS::RecorderURLS( const XSDK::XString& dataSourceID,
                            const XSDK::XString& startTime,
                            const XSDK::XString& endTime,
                            int64_t requestSize,
                            bool keyFrameOnly ) :
    _dataSourceID( dataSourceID ),
    _startTime( startTime ),
    _endTime( endTime ),
    _requestStart( 0 ),
    _done( false ),
    _firstRequest( true ),
    _requestSize( requestSize ),
    _keyFrameOnly( keyFrameOnly )
{
}

RecorderURLS::RecorderURLS( const XSDK::XString& dataSourceID,
                            const XSDK::XString& startTime,
                            const XSDK::XString& endTime,
                            double speed ) :
    _dataSourceID( dataSourceID ),
    _startTime( startTime ),
    _endTime( endTime ),
    _requestStart( 0 ),
    _done( false ),
    _firstRequest( true )
{
    if( speed < 4.000 )
    {
        _requestSize = 6000;
        _keyFrameOnly = false;
    }
    else if( speed < 6.000 )
    {
        _requestSize = 24000;
        _keyFrameOnly = false;
    }
    else if( speed < 10.000 )
    {
        _requestSize = 30000;
        _keyFrameOnly = false;
    }
    else if( speed < 30.000 )
    {
        _requestSize = 40000;
        _keyFrameOnly = true;
    }
    else X_THROW(("Invalid speed."));
}

RecorderURLS::~RecorderURLS() throw()
{
}

bool RecorderURLS::GetNextURL( XString& url )
{
    if( _done )
        return false;

    if( _firstRequest )
        _requestStart = XTime::FromISOExtString( _startTime ).ToUnixTimeAsMSecs();

    int64_t exportEnd = XTime::FromISOExtString( _endTime ).ToUnixTimeAsMSecs();
    int64_t requestStart = _requestStart;
    int64_t requestEnd = _requestStart + _requestSize;

    // We never want to make a recorder request for a time window that is too small to return any frames (e.g. asking
    // for all the video between 2 consecutive frames, I.E. NONE!). If the overall export happens to be just a few
    // milliseconds over a multiple of _requestSize, then that exact thing can happen. To address this, we compare our
    // computed requestEnd with our overall exportEnd... If it is bigger, then it should obviously be adjusted and set
    // to our exportEnd. But even it is a bit smaller, it should be set to our exportEnd. If our requestEnd is 1.1
    // seconds before our export end, then that is fine... because a request of 1.1 seconds should return frames.
    if( requestEnd > (exportEnd - 1000) )
    {
        _done = true;
        requestEnd = exportEnd;
    }
    else _requestStart = requestEnd;

    url = XString::Format( "/recorder/media?data_source_id=%s&start_time=%s&end_time=%s&key_frame_only=%s&previous_playable=%s",
                           _dataSourceID.c_str(),
                           XString::FromInt64( requestStart ).c_str(),
                           XString::FromInt64( requestEnd ).c_str(),
                           (_keyFrameOnly)?"true":"false",
                           (_firstRequest)?"true":"false" );

    _firstRequest = false; // note: this needs to be after the Format() because its used there!

    return true;
}
