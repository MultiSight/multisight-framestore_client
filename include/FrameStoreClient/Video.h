
#ifndef __FRAME_STORE_CLIENT_Video_h
#define __FRAME_STORE_CLIENT_Video_h

#include "XSDK/XMemory.h"

namespace FRAME_STORE_CLIENT
{

X_API XIRef<XSDK::XMemory> FetchVideo( const XSDK::XString& recorderIP,
                                       int recorderPort,
                                       const XSDK::XString& url );

X_API XIRef<XSDK::XMemory> FetchVideo( const XSDK::XString& recorderIP,
                                       int recorderPort,
                                       const XSDK::XString& dataSourceID,
                                       const XSDK::XString& startTime,
                                       const XSDK::XString& endTime,
                                       bool previousPlayable = true,
                                       bool keyFrameOnly = false );

}

#endif
