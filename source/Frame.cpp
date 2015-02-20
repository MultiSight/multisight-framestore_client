
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// XSDK
// Copyright (c) 2015 Schneider Electric
//
// Use, modification, and distribution is subject to the Boost Software License,
// Version 1.0 (See accompanying file LICENSE).
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#include "FrameStoreClient/Frame.h"
#include "XSDK/XSocket.h"
#include "Webby/ClientSideRequest.h"
#include "Webby/ClientSideResponse.h"
#include "Webby/WebbyException.h"

using namespace FRAME_STORE_CLIENT;
using namespace XSDK;
using namespace WEBBY;

namespace FRAME_STORE_CLIENT
{

XIRef<XMemory> FetchFrame( const XString& recorderIP,
                           int recorderPort,
                           const XString& dataSourceID,
                           const XString& time )
{
    ClientSideResponse frameResponse;

    {
        XRef<XSocket> sok = new XSocket;

        int connectTries = 16;
        while( connectTries > 0 && !sok->QueryConnect( recorderIP, recorderPort ) )
            connectTries--;

        if( connectTries <= 0 )
            X_STHROW( HTTP500Exception, ( "Unable to connect to recorder!" ) );

        ClientSideRequest request;

        request.SetRequestType( kWebbyGETRequest );

        request.SetURI( XString::Format( "/frame?data_source_id=%s&time=%s&closest=true",
                                         dataSourceID.c_str(),
                                         time.c_str() ) );

        request.WriteRequest( sok );


        frameResponse.ReadResponse( sok );

        sok->Close();
    }

    if( frameResponse.GetBodySize() == 0 )
        X_STHROW( HTTP500Exception, ( "Zero length Recorder response to URI." ) );

    return frameResponse.GetBodyAsXMemory();
}

}
