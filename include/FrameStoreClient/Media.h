
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// XSDK
// Copyright (c) 2015 Schneider Electric
//
// Use, modification, and distribution is subject to the Boost Software License,
// Version 1.0 (See accompanying file LICENSE).
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#ifndef __FRAME_STORE_CLIENT_Media_h
#define __FRAME_STORE_CLIENT_Media_h

#include "XSDK/XMemory.h"

namespace FRAME_STORE_CLIENT
{

X_API XIRef<XSDK::XMemory> FetchMedia( const XSDK::XString& recorderIP,
                                       int recorderPort,
                                       const XSDK::XString& url,
                                       uint32_t recvTimeout = 10000 );

X_API XIRef<XSDK::XMemory> FetchMedia( const XSDK::XString& recorderIP,
                                       int recorderPort,
                                       const XSDK::XString& dataSourceID,
                                       const XSDK::XString& startTime,
                                       const XSDK::XString& endTime,
                                       const XSDK::XString& type = "video",
                                       bool previousPlayable = true,
                                       bool keyFrameOnly = false,
                                       uint32_t recvTimeout = 10000 );

}

#endif
