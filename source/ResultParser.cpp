
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// XSDK
// Copyright (c) 2015 Schneider Electric
//
// Use, modification, and distribution is subject to the Boost Software License,
// Version 1.0 (See accompanying file LICENSE).
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#include "FrameStoreClient/ResultParser.h"
#include "XSDK/XBytePtr.h"
#include "XSDK/XSocket.h"
#include "XSDK/XStatistics.h"
#include <vector>
#include <cmath>

using namespace XSDK;
using namespace std;
using namespace FRAME_STORE_CLIENT;
using namespace AVKit;

static const int INDEX_ENTRY_SIZE = 48;

// This code will currently have an issue if the result set contains two
// segments and the SDP changes between those segments.
// The solution is probably to store the sourceClockRate and the codec id
// in the index per frame. Then GetCodecID() would return the codec id of
// the curent frame.

ResultParser::ResultParser() :
    XBaseObject(),
    _response(),
    _index( new XMemory ),
    _encodedSPS(),
    _encodedPPS(),
    _currentSDP(),
    _codecId( MEDIA_PARSER::MEDIA_TYPE_UNKNOWN ),
    _sourceClockRate( 0 ),
    _currentFrameNumber( -1 ),
    _numFrames( 0 ),
    _rate( 0.0 ),
    _SDPFrameRate(),
    _pf( new PacketFactoryDefault() )
{
}

ResultParser::~ResultParser() throw()
{
}

void ResultParser::Parse( XIRef<XMemory> response )
{
    _response = response;

    _BuildIndex();

    _numFrames = _index->GetDataSize() / INDEX_ENTRY_SIZE;

    _currentFrameNumber = -1;
}

void ResultParser::Parse( XIRef<WEBBY::ClientSideResponse> response )
{
    _response = response->GetBodyAsXMemory();

    _BuildIndex();

    _numFrames = _index->GetDataSize() / INDEX_ENTRY_SIZE;

    _currentFrameNumber = -1;
}

template<class T>
static T _round( T d, int place )
{
    return std::floor(d)+std::floor((d-std::floor(d))*((double)place)+0.5)/((double)place);
}

struct ResultStatistics ResultParser::GetStatistics()
{
    struct ResultStatistics result;

    XStatistics<Average,size_t> avgFrameSize;
    XStatistics<Average,double> avgTimeDelta;

    int64_t lastTS = 0;

    uint16_t indexFirstKey = 0;
    bool foundFirstKey = false;
    bool foundGopSize = false;
    uint16_t currentIndex = 0;

    Reset();

    int videoStreamIndex = GetVideoStreamIndex();
    int streamIndex = 0;
    while( ReadFrame( streamIndex ) )
    {
        if( streamIndex != videoStreamIndex ) // We gather stats on only video frames.
            continue;

        // First, see if we have seen enough key's to determine our gop size...

        if( IsKey() )
        {
            if( !foundFirstKey )
            {
                indexFirstKey = currentIndex;
                foundFirstKey = true;
            }
            else
            {
                if( !foundGopSize )
                {
                    result.gopSize = currentIndex - indexFirstKey;
                    foundGopSize = true;
                }
            }
        }

        // Now, lots build our average frame durations...

        int64_t ts = GetFrameTS();

        if( lastTS != 0 )
            avgTimeDelta.AddSample( ts - lastTS );
        lastTS = ts;

        // and our average frame size...

        avgFrameSize.AddSample( GetFrameSize() );

        currentIndex++;
    }

    double avgDelta = 0;
    avgTimeDelta.GetResult( avgDelta );

    size_t avgSize = 0;
    avgFrameSize.GetResult( avgSize );

    result.averageBitRate = (uint32_t)(((1000 / (double)avgDelta) * (double)avgSize) * 8);
    result.frameRate = _round( 1000 / avgDelta, 10 );

    Reset();

    return result;
}

void ResultParser::Run( float rate )
{
    _rate = rate;
}

void ResultParser::Flush()
{
    _response.Clear();
    _index = new XMemory;
    _encodedSPS.clear();
    _encodedPPS.clear();
    _currentSDP.clear();
    _codecId = MEDIA_PARSER::MEDIA_TYPE_UNKNOWN;
    _sourceClockRate = 0;
    _currentFrameNumber = -1;
    _numFrames = 0;
    _rate = 0.0;
    _SDPFrameRate = XString();
}

bool ResultParser::StartOfSegment() const
{
    uint32_t indexSize = _index->GetDataSize();
    const uint8_t* index = _index->Map();
    const uint8_t* lastIndex = ((index + indexSize)-INDEX_ENTRY_SIZE);

    uint32_t frameIndex = _GetFrameIndex( 0 );

    uint32_t indexOffset = (frameIndex * INDEX_ENTRY_SIZE);

    if( (index+indexOffset) > lastIndex )
        X_THROW(( "Cannot access index out of range." ));

    uint32_t flags = *((uint32_t*)(index+indexOffset+24));

    if( flags & 0x80000000 )
        return true;

    return false;
}

bool ResultParser::EndOfSegment() const
{
    uint32_t indexSize = _index->GetDataSize();
    const uint8_t* index = _index->Map();
    const uint8_t* lastIndex = ((index + indexSize)-INDEX_ENTRY_SIZE);

    uint32_t frameIndex = _GetFrameIndex( 0 );

    uint32_t indexOffset = (frameIndex * INDEX_ENTRY_SIZE);

    if( (index+indexOffset) > lastIndex )
        X_THROW(( "Cannot access index out of range." ));

    uint32_t flags = *((uint32_t*)(index+indexOffset+24));

    if( flags & 0x40000000 )
        return true;
    return false;
}

bool ResultParser::Live() const
{
    uint32_t indexSize = _index->GetDataSize();
    const uint8_t* index = _index->Map();
    const uint8_t* lastIndex = ((index + indexSize)-INDEX_ENTRY_SIZE);

    uint32_t frameIndex = _GetFrameIndex( 0 );

    uint32_t indexOffset = (frameIndex * INDEX_ENTRY_SIZE);

    if( (index+indexOffset) > lastIndex )
        X_THROW(( "Cannot access index out of range." ));

    uint32_t flags = *((uint32_t*)(index+indexOffset+28));

    if( flags & 0x20000000 )
        return true;

    return false;
}

XString ResultParser::GetSDP() const
{
    return _currentSDP;
}

XString ResultParser::GetSDPFrameRate() const
{
    return _SDPFrameRate;
}

XIRef<XMemory> ResultParser::GetSPS() const
{
    if( _encodedSPS.length() > 0 )
        return _encodedSPS.FromBase64();
    else return new XMemory();
}

XIRef<XMemory> ResultParser::GetPPS() const
{
    if( _encodedPPS.length() > 0 )
        return _encodedPPS.FromBase64();
    else return new XMemory();
}

MEDIA_PARSER::MEDIA_TYPE ResultParser::GetCodecID() const
{
    uint32_t indexSize = _index->GetDataSize();
    const uint8_t* index = _index->Map();
    const uint8_t* lastIndex = ((index + indexSize)-INDEX_ENTRY_SIZE);

    uint32_t frameIndex = _GetFrameIndex( 0 );

    uint32_t indexOffset = (frameIndex * INDEX_ENTRY_SIZE);

    if( (index+indexOffset) > lastIndex )
        X_THROW(( "Cannot access index out of range." ));

    uint32_t mediaType = *((uint32_t*)(index+indexOffset+36));

    return (MEDIA_PARSER::MEDIA_TYPE)mediaType;
}

uint32_t ResultParser::GetSourceClockRate() const
{
    uint32_t indexSize = _index->GetDataSize();
    const uint8_t* index = _index->Map();
    const uint8_t* lastIndex = ((index + indexSize)-INDEX_ENTRY_SIZE);

    uint32_t frameIndex = _GetFrameIndex( 0 );

    uint32_t indexOffset = (frameIndex * INDEX_ENTRY_SIZE);

    if( (index+indexOffset) > lastIndex )
        X_THROW(( "Cannot access index out of range." ));

    uint32_t sourceClockRate = *((uint32_t*)(index+indexOffset+32));

    return sourceClockRate;
}

int ResultParser::GetMetaDataStreamIndex() const
{
    return 0;
}

int ResultParser::GetVideoStreamIndex() const
{
    // For now, our video stream index is a fixed number... Eventually, we may support muxed content in a
    // recorder result and we will use this to distinguish between streams.
    return 0;
}

int ResultParser::GetPrimaryAudioStreamIndex() const
{
    X_THROW(("ResultParser does not current support audio."));
}

size_t ResultParser::GetFrameSize() const
{
    if( !_ValidIterator() )
        X_THROW(("Invalid Iterator."));

    uint32_t indexSize = _index->GetDataSize();
    const uint8_t* index = _index->Map();
    const uint8_t* lastIndex = ((index + indexSize)-INDEX_ENTRY_SIZE);

    uint32_t frameIndex = _GetFrameIndex( _currentFrameNumber );

    uint32_t indexOffset = (frameIndex * INDEX_ENTRY_SIZE);

    if( (index+indexOffset) > lastIndex )
        X_THROW(( "Cannot access index out of range." ));

    int64_t frameSize = *((int64_t*)(index+indexOffset+16));

    return (size_t)frameSize;
}

XIRef<Packet> ResultParser::Get()
{
    if( !_ValidIterator() )
        X_THROW(("Invalid Iterator."));

    uint32_t indexSize = _index->GetDataSize();
    const uint8_t* index = _index->Map();
    const uint8_t* lastIndex = ((index + indexSize)-INDEX_ENTRY_SIZE);

    uint32_t frameIndex = _GetFrameIndex( _currentFrameNumber );

    uint32_t indexOffset = (frameIndex * INDEX_ENTRY_SIZE);

    if( (index+indexOffset) > lastIndex )
        X_THROW(( "Cannot access index out of range." ));

    int64_t offset = *((int64_t*)(index+indexOffset+8));
    int64_t frameSize = *((int64_t*)(index+indexOffset+16));

    size_t packetBufferSize = (frameSize % 8)?frameSize+(8-(frameSize%8)):frameSize;

    XIRef<Packet> pkt = _pf->Get( packetBufferSize );

    uint8_t* source = (uint8_t*)_response->Map();

    memcpy( pkt->Map(),
            (source+offset),
            (size_t)frameSize );

    pkt->SetDataSize( frameSize );

    return pkt;
}

uint8_t* ResultParser::GetFramePointer() const
{
    if( !_ValidIterator() )
        X_THROW(("Invalid Iterator."));

    uint32_t indexSize = _index->GetDataSize();
    const uint8_t* index = _index->Map();
    const uint8_t* lastIndex = ((index + indexSize)-INDEX_ENTRY_SIZE);

    uint32_t frameIndex = _GetFrameIndex( _currentFrameNumber );

    uint32_t indexOffset = (frameIndex * INDEX_ENTRY_SIZE);

    if( (index+indexOffset) > lastIndex )
        X_THROW(( "Cannot access index out of range." ));

    int64_t offset = *((int64_t*)(index+indexOffset+8));

    return ((uint8_t*)_response->Map()) + offset;
}

int64_t ResultParser::GetFrameTS() const
{
    if( !_ValidIterator() )
        X_THROW(("Invalid Iterator."));

    uint32_t indexSize = _index->GetDataSize();
    const uint8_t* index = _index->Map();
    const uint8_t* lastIndex = ((index + indexSize)-INDEX_ENTRY_SIZE);

    uint32_t frameIndex = _GetFrameIndex( _currentFrameNumber );

    uint32_t indexOffset = (frameIndex * INDEX_ENTRY_SIZE);

    if( (index+indexOffset) > lastIndex )
        X_THROW(( "Cannot access index out of range." ));

    return *((int64_t*)(index+indexOffset));
}

uint32_t ResultParser::GetSegmentID() const
{
    uint32_t indexSize = _index->GetDataSize();
    const uint8_t* index = _index->Map();
    const uint8_t* lastIndex = ((index + indexSize)-INDEX_ENTRY_SIZE);

    uint32_t frameIndex = _GetFrameIndex( 0 );

    uint32_t indexOffset = (frameIndex * INDEX_ENTRY_SIZE);

    if( (index+indexOffset) > lastIndex )
        X_THROW(( "Cannot access index out of range." ));

    return *((uint32_t*)(index+indexOffset+44));
}

uint32_t ResultParser::GetFrameFlags() const
{
    if( !_ValidIterator() )
        X_THROW(("Invalid Iterator."));

    uint32_t indexSize = _index->GetDataSize();
    const uint8_t* index = _index->Map();
    const uint8_t* lastIndex = ((index + indexSize)-INDEX_ENTRY_SIZE);

    uint32_t frameIndex = _GetFrameIndex( _currentFrameNumber );

    uint32_t indexOffset = (frameIndex * INDEX_ENTRY_SIZE);

    if( (index+indexOffset) > lastIndex )
        X_THROW(("Cannot access index out of range." ));

    return *((uint32_t*)(index+indexOffset+24));
}

bool ResultParser::IsKey() const
{
    // The REAL FF_KEY_FRAME constant is defined in FrameStore's "File.h"... Until I have a good way of getting that
    // constant here, we'll use this:

    const uint32_t FF_KEY_FRAME = 0x01;

    return (GetFrameFlags() & FF_KEY_FRAME) ? true : false;
}

bool ResultParser::ReadFrame( int& streamIndex )
{
    if( _numFrames <= 0 )
        X_THROW(("Unable to iterate over result with no frames."));

    _currentFrameNumber++;

    if( _currentFrameNumber < _numFrames )
    {
        streamIndex = 0; // Currently, we only support video. At some point, this will change to support
                         // muxed audio and video.

        return true;
    }

    return false;
}

bool ResultParser::EndOfFile() const
{
    if( _currentFrameNumber >= (int32_t)_numFrames )
        return true;

    return false;
}

void ResultParser::Reset()
{
    _currentFrameNumber = -1;
}

float ResultParser::FractionConsumed() const
{
    if( !_ValidIterator() )
        X_THROW(("Invalid Iterator."));

    if( _numFrames > 0 )
        return ((float)(_currentFrameNumber+1)) / (float)_numFrames;
    return 0.0f;
}

int64_t ResultParser::GetFirstFrameTS() const
{
    if ( _numFrames < 1)
        X_THROW(( "No Frames to get Time from." ));
        
    uint32_t indexSize = _index->GetDataSize();
    const uint8_t* index = _index->Map();
    const uint8_t* lastIndex = ((index + indexSize)-INDEX_ENTRY_SIZE);

    uint32_t frameIndex = _GetFrameIndex( 0 );

    uint32_t indexOffset = (frameIndex * INDEX_ENTRY_SIZE);

    if( (index+indexOffset) > lastIndex )
        X_THROW(( "Cannot access index out of range." ));

    return *((int64_t*)(index+indexOffset));
}

int64_t ResultParser::GetLastFrameTS() const
{
    uint32_t lastFrameNumber = (_numFrames - 1);

    uint32_t indexSize = _index->GetDataSize();
    const uint8_t* index = _index->Map();
    const uint8_t* lastIndex = ((index + indexSize)-INDEX_ENTRY_SIZE);

    uint32_t frameIndex = _GetFrameIndex( lastFrameNumber );

    uint32_t indexOffset = (frameIndex * INDEX_ENTRY_SIZE);

    if( (index+indexOffset) > lastIndex )
        X_THROW(( "Cannot access index out of range." ));

    return *((int64_t*)(index+indexOffset));
}

void ResultParser::_BuildIndex()
{
    _index->ResizeData( 0 );

    if( _response->GetDataSize() )
    {
        XBytePtr p( (uint8_t*)_response->Map(), _response->GetDataSize() );

        uint32_t resultSegmentIndex = 0;

        while( p.InBounds() )
        {
            int64_t doubleWord = 0;
            uint32_t word = 0;
            uint8_t sig[4] = { 0, 0, 0, 0 };

            doubleWord = p.Consume<int64_t>();

            memcpy( sig, p.GetPtr(), 4 );
            p += 4;

            // Read the segments id.
            word = p.Consume<uint32_t>();
            uint32_t segmentID = ntohl( word );

            // Read the segments codec type.
            word = p.Consume<uint32_t>();
            MEDIA_PARSER::MEDIA_TYPE codecId = (MEDIA_PARSER::MEDIA_TYPE) ntohl( word );

            // Read the segments media clockRate
            word = p.Consume<uint32_t>();
            uint32_t sourceClockRate = ntohl( word );

            // Read the segments fmtp sdp track (media description) value.
            word = p.Consume<uint32_t>();
            uint32_t fmtpLength = ntohl( word );

            XString fmtp;
            if( fmtpLength > 0 )
            {
                fmtp = XString( (char*)p.GetPtr(), (size_t)fmtpLength );
                p += (size_t)fmtpLength;
            }

            uint32_t flags = 0;
            int64_t framesStartOffset = 0;
            int64_t framesEndOffset = 0;

            for( int i = 0; i < 4; i++ )
            {
                doubleWord = p.Consume<int64_t>();
                int64_t payloadSize = x_ntohll( doubleWord );

                memcpy( sig, p.GetPtr(), 4 );
                p += 4;

                if( sig[0] == 'S' && sig[1] == 'D' && sig[2] == 'P' && sig[3] == ' ' )
                {
                    _currentSDP = XString( (char*)p.GetPtr(), (size_t)payloadSize );

                    // TODO(frank.lamar): With the new segment values (codecId, sourceClockRate, and fmtp)
                    // there doesn't really seem to be a reason to actually store the whole sdp.  This
                    // method should also be updated to just parse the fmtp string for the sps, and pps
                    // values.
                    _ParseSDP( _currentSDP );

                    p += (size_t)payloadSize;
                }
                else if( sig[0] == 'F' && sig[1] == 'R' && sig[2] == 'A' && sig[3] == 'M' )
                {
                    framesStartOffset = p.GetPtr() - (uint8_t*)_response->Map();

                    framesEndOffset = (p.GetPtr() + payloadSize) - (uint8_t*)_response->Map();

                    p += (size_t)payloadSize;
                }
                else if( sig[0] == 'F' && sig[1] == 'L' && sig[2] == 'A' && sig[3] == 'G' )
                {
                    word = p.Consume<uint32_t>();
                    flags = ntohl( word );
                }
                else if( sig[0] == 'I' && sig[1] == 'N' && sig[2] == 'D' && sig[3] == 'X' )
                {
                    uint32_t numIndexes = (size_t)payloadSize / 20;

                    for( uint32_t ii = 0; ii < numIndexes; ii++ )
                    {
                        doubleWord = p.Consume<int64_t>();
                        int64_t time = x_ntohll( doubleWord );

                        doubleWord = p.Consume<int64_t>();
                        int64_t offset = x_ntohll( doubleWord ) + framesStartOffset;

                        word = p.Consume<uint32_t>();
                        uint32_t frameFlags = ntohl( word );

                        int64_t frameSize = 0;

                        if( (ii + 1) < numIndexes )
                        {
                            doubleWord = *((int64_t*)(p.GetPtr() + 8));
                            int64_t nextOffset = x_ntohll( doubleWord );
                            frameSize = (framesStartOffset + nextOffset) - offset;
                        }
                        else frameSize = framesEndOffset - offset;

                        _index->Append<int64_t>( time );
                        _index->Append<int64_t>( offset );
                        _index->Append<int64_t>( frameSize );
                        _index->Append<uint32_t>( frameFlags );
                        _index->Append<uint32_t>( flags );
                        _index->Append<uint32_t>( sourceClockRate );
                        _index->Append<uint32_t>( codecId );
                        _index->Append<uint32_t>( resultSegmentIndex );
                        _index->Append<uint32_t>( segmentID );
                    }

                    resultSegmentIndex++;
                }

            }
        }
    }
}

void ResultParser::_ParseSDP( const XString& sdp )
{
    if( sdp.Contains( "video" ) )
    {
        vector<XString> sdpLines;
        sdp.Split( "\r\n", sdpLines );

        for( unsigned int i = 0; i < sdpLines.size(); i++ )
        {
            if( sdpLines[i].StartsWith( "a=" ) )
            {
                XString rhs = sdpLines[i].substr( sdpLines[i].find( '=' )+1 );

                if( rhs.StartsWith( "framerate" ) )
                {
                    vector<XString> rightOfEqualsParts;
                    rhs.Split( ":", rightOfEqualsParts );
                    if( rightOfEqualsParts.size() < 2 )
                        X_THROW(( "Detected malformed SDP\n (%s).",
                                  sdp.c_str() ));

                    _SDPFrameRate = rightOfEqualsParts[1];
                }
                else if( rhs.Contains( "sprop-parameter-sets" ) )
                {
                    XString sprop = rhs.substr( rhs.find( "sprop-parameter-sets=" ) + 21 );

                    vector<XString> spropParts;
                    sprop.Split( ",", spropParts );

                    if( spropParts.size() > 0 )
                        _encodedSPS = spropParts[0];

                    if( spropParts.size() > 1 )
                        _encodedPPS = spropParts[1];
                }
            }
        }
    }
}

uint32_t ResultParser::_GetFrameIndex( uint32_t frameNumber ) const
{
    // _numFrames - the number of frames in this result
    // _currentFrameNumber - our position in the current result set (0 .. (_numFrames-1))
    // _rate - Whether we are playing backwards or forwards.

    return (_rate >= 0.0) ? frameNumber : ((_numFrames-1)-frameNumber);
}
