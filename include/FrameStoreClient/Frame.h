
#ifndef __FRAME_STORE_CLIENT_Frame_h
#define __FRAME_STORE_CLIENT_Frame_h

#include "XSDK/XMemory.h"
#include "XSDK/XString.h"
#include "XSDK/XSharedLib.h"

namespace FRAME_STORE_CLIENT
{

X_API XIRef<XSDK::XMemory> FetchFrame( const XSDK::XString& recorderIP,
                                       int recorderPort,
                                       const XSDK::XString& dataSourceID,
                                       const XSDK::XString& time );

}

#endif
