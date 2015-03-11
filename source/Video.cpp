
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// XSDK
// Copyright (c) 2015 Schneider Electric
//
// Use, modification, and distribution is subject to the Boost Software License,
// Version 1.0 (See accompanying file LICENSE).
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#include "FrameStoreClient/Video.h"
#include "Webby/WebbyException.h"
#include "Webby/ClientSideRequest.h"
#include "Webby/ClientSideResponse.h"
#include "XSDK/XRef.h"
#include "XSDK/XSocket.h"
#include "XSDK/XTime.h"

using namespace FRAME_STORE_CLIENT;
using namespace XSDK;
using namespace WEBBY;

namespace FRAME_STORE_CLIENT
{

XIRef<XMemory> FetchVideo( const XString& recorderIP,
                           int recorderPort,
                           const XString& url )
{
    XRef<XSocket> sok = new XSocket;

    sok->Connect( recorderIP, recorderPort );

    ClientSideRequest request;

    request.SetRequestType( kWebbyGETRequest );

    request.SetURI( url );

    request.WriteRequest( sok );

    ClientSideResponse response;
    response.ReadResponse( sok );

    sok->Close();

    if( response.GetBodySize() == 0 )
        X_STHROW( HTTP500Exception, ( "Zero length Recorder response to URI." ) );

    return response.GetBodyAsXMemory();
}

XIRef<XMemory> FetchVideo( const XString& recorderIP,
                           int recorderPort,
                           const XString& dataSourceID,
                           const XString& startTime,
                           const XString& endTime,
                           bool previousPlayable,
                           bool keyFrameOnly )
{
    XRef<XSocket> sok = new XSocket;

    sok->Connect( recorderIP, recorderPort );

    ClientSideRequest request;

    request.SetRequestType( kWebbyGETRequest );

    request.SetURI( XString::Format( "/recorder/media?data_source_id=%s&start_time=%s&end_time=%s&previous_playable=%s&key_frame_only=%s",
                                     dataSourceID.c_str(),
                                     XString::FromInt64( XTime::FromISOExtString( startTime ).ToUnixTimeAsMSecs() ).c_str(),
                                     XString::FromInt64( XTime::FromISOExtString( endTime ).ToUnixTimeAsMSecs() ).c_str(),
                                     (previousPlayable) ? "true" : "false",
                                     (keyFrameOnly) ? "true" : "false" ) );

    request.WriteRequest( sok );

    ClientSideResponse response;
    response.ReadResponse( sok );

    sok->Close();

    if( response.GetBodySize() == 0 )
        X_STHROW( HTTP500Exception, ( "Zero length Recorder response to URI." ) );

    return response.GetBodyAsXMemory();
}

}
