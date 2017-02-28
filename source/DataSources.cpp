#include "FrameStoreClient/DataSources.h"
#include "XSDK/XJSON.h"
#include "XSDK/XRef.h"
#include "XSDK/XIRef.h"
#include "XSDK/XSocket.h"
#include "Webby/ClientSideRequest.h"
#include "Webby/ClientSideResponse.h"
#include "Webby/WebbyException.h"
#include "Webby/StatusCodes.h"

using namespace FRAME_STORE_CLIENT;
using namespace XSDK;
using namespace WEBBY;

namespace FRAME_STORE_CLIENT
{

X_API std::vector<DataSource> FetchDataSources( const XSDK::XString& recorderIP, int recorderPort )
{
    XRef<XSocket> sok = new XSocket;

    sok->Connect( recorderIP, recorderPort );

    ClientSideRequest request;
    request.SetRequestType( kWebbyGETRequest );
    request.SetURI( "/data_sources" );
    request.WriteRequest( sok );

    ClientSideResponse response;
    response.ReadResponse( sok );
    sok->Close();

    if( response.GetStatus() == WEBBY::kWebbyResponseNotFound )
        X_STHROW( HTTP404Exception, ("Recorder return 404.") );

    if( response.GetBodySize() == 0 )
        X_STHROW( HTTP500Exception, ( "Zero length Recorder response to URI." ) );

    XString json = response.GetBodyAsString();
    XIRef<XJSONItem> parsed = XJSONItem::ParseDocument(json);
    XIRef<XJSONArray> array = XJSONItem::Find(parsed,"data/data_sources");
    
    std::vector<DataSource> sources;
    for ( size_t i = 0; i < array->Count(); ++i  )
    {
        XIRef<XJSONObject> object = array->Index(i);
        DataSource source;
        source._id = object->Index("id")->Get<XString>();
        XString type = object->Index("type")->Get<XString>();
        source._type = type == "video" ? SourceType::Video : ( type == "audio" ? SourceType::Audio : SourceType::MetaData ) ;
        source._sourceUrl = object->Index("source_url")->Get<XString>();
        source._recordingVideo = object->Index("recording_video")->Get<XString>() == "true";
        source._videoHealty = object->Index("video_status")->Get<XString>() == "healthy";
        source._recordingAudio = object->Index("recording_audio")->Get<XString>() == "true";
        source._audioHealthy = object->Index("audio_status")->Get<XString>() == "healthy";
        source._recordingMetaData = object->Index("recording_md")->Get<XString>() == "true";
        source._metaDataHealthy = object->Index("md_status")->Get<XString>() == "healthy";
        sources.push_back(source);
    }

    return sources;
}

}








