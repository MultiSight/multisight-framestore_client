
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// XSDK
// Copyright (c) 2015 Schneider Electric
//
// Use, modification, and distribution is subject to the Boost Software License,
// Version 1.0 (See accompanying file LICENSE).
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#ifndef __FRAME_STORE_CLIENT__ResultParser_h
#define __FRAME_STORE_CLIENT__ResultParser_h

#include "XSDK/XBaseObject.h"
#include "XSDK/XMemory.h"
#include "Webby/ClientSideResponse.h"
#include "AVKit/Packet.h"
#include "AVKit/PacketFactory.h"
#include "MediaParser/Defines.h"

namespace FRAME_STORE_CLIENT
{

struct ResultStatistics
{
    uint32_t averageBitRate;
    double frameRate;
    uint16_t gopSize;
};

class ResultParser : public XSDK::XBaseObject
{
public:
    X_API ResultParser();
    X_API virtual ~ResultParser() throw();

    X_API void Parse( XIRef<XSDK::XMemory> response );

    X_API void Parse( XIRef<WEBBY::ClientSideResponse> response );

    X_API struct ResultStatistics GetStatistics();

    X_API void Run( float rate );

    X_API void Flush();

    X_API bool StartOfSegment() const;
    X_API bool EndOfSegment() const;
    X_API bool Live() const;

    X_API XSDK::XString GetSDP() const;
    X_API XSDK::XString GetSDPFrameRate() const;
    X_API XIRef<XSDK::XMemory> GetSPS() const;
    X_API XIRef<XSDK::XMemory> GetPPS() const;
    X_API MEDIA_PARSER::MEDIA_TYPE GetCodecID() const;
    X_API uint32_t GetSourceClockRate() const;

    X_API int GetVideoStreamIndex() const;
    X_API int GetPrimaryAudioStreamIndex() const;
    X_API int GetMetaDataStreamIndex() const;

    // Iteration...
    X_API bool ReadFrame( int& streamIndex );
    X_API bool EndOfFile() const;
    X_API size_t GetFrameSize() const;
    X_API XIRef<AVKit::Packet> Get();

    X_API uint8_t* GetFramePointer() const;
    X_API int64_t GetFrameTS() const;
    X_API uint32_t GetSegmentID() const;
    X_API uint32_t GetFrameFlags() const;
    X_API bool IsKey() const;

    X_API void Reset();

    X_API float FractionConsumed() const;

    X_API int64_t GetLastFrameTS() const;
    X_API int64_t GetFirstFrameTS() const;

private:
    void _BuildIndex();
    void _ParseSDP( const XSDK::XString& sdp );

    uint32_t _GetFrameIndex( uint32_t frameNumber ) const;

    inline bool _ValidIterator() const
    {
        if( _numFrames > 0 )
        {
            if( (_currentFrameNumber >= 0) && (_currentFrameNumber < (int32_t)_numFrames) )
                return true;
        }

        return false;
    }

    XIRef<XSDK::XMemory> _response;
    XIRef<XSDK::XMemory> _index;
    XSDK::XString _encodedSPS;
    XSDK::XString _encodedPPS;
    XSDK::XString _currentSDP;
    MEDIA_PARSER::MEDIA_TYPE _codecId;
    uint32_t _sourceClockRate;
    int32_t _currentFrameNumber;
    uint32_t _numFrames;
    float _rate;
    XSDK::XString _SDPFrameRate;
    XIRef<AVKit::PacketFactory> _pf;
};

}

#endif
