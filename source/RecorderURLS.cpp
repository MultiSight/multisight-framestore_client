
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
        _requestSize = 12000;
        _keyFrameOnly = true;
    }
    else if( speed < 10.000 )
    {
        _requestSize = 20000;
        _keyFrameOnly = true;
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

    int64_t requestStart = _requestStart;
    int64_t requestEnd = 0;

    if( (_requestStart + _requestSize) >=
        XTime::FromISOExtString( _endTime ).ToUnixTimeAsMSecs() )
    {
        _done = true;

        requestEnd = XTime::FromISOExtString( _endTime ).ToUnixTimeAsMSecs();

        // no reason to update _requestStart here because we're done after this request.
    }
    else
    {
        requestEnd = _requestStart + _requestSize;

        _requestStart = requestEnd;
    }

    url = XString::Format( "/recorder/media?data_source_id=%s&start_time=%s&end_time=%s&key_frame_only=%s&previous_playable=%s",
                           _dataSourceID.c_str(),
                           XString::FromInt64( requestStart ).c_str(),
                           XString::FromInt64( requestEnd ).c_str(),
                           (_keyFrameOnly)?"true":"false",
                           (_firstRequest)?"true":"false" );

    _firstRequest = false;

    return true;
}
