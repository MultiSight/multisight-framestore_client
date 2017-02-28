#include "XSDK/XException.h"
#include "XSDK/XSocket.h"

#include "Webby/ServerSideRequest.h"
#include "Webby/ServerSideResponse.h"
#include "Webby/WebbyException.h"
#include "Webby/StatusCodes.h"

#include <stdio.h>
#include <iostream> 
#include <fstream>  
#include <thread>  

#include "TestDataSources.h"

#include "FrameStoreClient/DataSources.h"


using namespace XSDK;
using namespace std;
using namespace WEBBY;
using namespace FRAME_STORE_CLIENT;




REGISTER_TEST_FIXTURE(TestDatSources);


TestDatSources::~TestDatSources(void) throw()
{
}

void* TestDatSources::EntryPoint()
{
    // XSocket socket;
    // _currPort = socket.BindEphemeral("127.0.0.1");
    // socket.Listen();

    // XRef<XSocket> clientSocket = socket.Accept();    
    // ServerSideRequest request;
    // request.ReadRequest(clientSocket);    
    // URI uri = request.GetURI();

    // ServerSideResponse response;

    // XString body = "";
    // switch(_testNum)
    // {
    //     case 0:
            
    //         break;
    //     case 1:
    //         body = "{\"data\":{\"data_sources\":[{\"id\":\"foo\",\"type\":\"video\",\"source_url\":\"bar\",\"recording_video\":\"true\",\"recording_audio\":\"true\",\"recording_md\":\"true\",\"transport_pref\":\"tcp\",\"video_status\":\"healthy\",\"audio_status\":\"healthy\",\"md_status\":\"healthy\"}]}}";
    //         break;
    //     case 2:
    //         body = "{\"data\":{\"data_sources\":["\
    //             "{\"id\":\"1\",\"type\":\"video\",\"source_url\":\"1\",\"recording_video\":\"true\",\"recording_audio\":\"false\",\"recording_md\":\"true\",\"transport_pref\":\"tcp\",\"video_status\":\"healthy\",\"audio_status\":\"healthy\",\"md_status\":\"unhealthy\"},"\
    //             "{\"id\":\"2\",\"type\":\"audio\",\"source_url\":\"2\",\"recording_video\":\"false\",\"recording_audio\":\"true\",\"recording_md\":\"false\",\"transport_pref\":\"tcp\",\"video_status\":\"healthy\",\"audio_status\":\"healthy\",\"md_status\":\"unhealthy\"},"\
    //             "{\"id\":\"3\",\"type\":\"md\",\"source_url\":\"3\",\"recording_video\":\"false\",\"recording_audio\":\"false\",\"recording_md\":\"true\",\"transport_pref\":\"tcp\",\"video_status\":\"unhealthy\",\"audio_status\":\"healthy\",\"md_status\":\"healthy\"}"\
    //             "]}}";
    //         break;
    //     default:
    //         break;
    // }
    // response.SetBody(body);
    // response.WriteResponse(clientSocket);

    // clientSocket->Shutdown(SOCKET_SHUT_SEND_FLAGS);
    // clientSocket->Close();
    // socket.Shutdown(SOCKET_SHUT_RECV_FLAGS);
    // socket.Close();
}


void TestDatSources::setup()
{
}

void TestDatSources::teardown()
{
}



void TestDatSources::TestEmptyDataSources()
{
    printf("TestDatSources::TestEmptyDataSources\n");
    XSocket socket;
    int port = socket.BindEphemeral("127.0.0.1");

    auto th = std::thread([&]{
            // XSocket socket;
            // port = socket.BindEphemeral("127.0.0.1");
            socket.Listen();

            XRef<XSocket> clientSocket = socket.Accept();    
            ServerSideRequest request;
            request.ReadRequest(clientSocket);    
            URI uri = request.GetURI();

            ServerSideResponse response;

            XString body = "{\"data\":{\"data_sources\":[]}}";

            response.SetBody(body);
            response.WriteResponse(clientSocket);

            clientSocket->Shutdown(SOCKET_SHUT_SEND_FLAGS);
            clientSocket->Close();
            socket.Shutdown(SOCKET_SHUT_RECV_FLAGS);
            socket.Close();
    });
    th.detach();
    sleep(2);

    std::vector<DataSource> result = FetchDataSources("127.0.0.1",port);
    UT_ASSERT_EQUAL(0,result.size());
}

void TestDatSources::TestOneDataSource(void)
{
    printf("TestDatSources::TestOneDataSource\n");
    XSocket socket;
    int port = socket.BindEphemeral("127.0.0.1");

    auto th = std::thread([&]{
            socket.Listen();

            XRef<XSocket> clientSocket = socket.Accept();    
            ServerSideRequest request;
            request.ReadRequest(clientSocket);    
            URI uri = request.GetURI();

            ServerSideResponse response;

            XString body = "{\"data\":{\"data_sources\":[{\"id\":\"foo\",\"type\":\"video\",\"source_url\":\"bar\",\"recording_video\":\"true\",\"recording_audio\":\"true\",\"recording_md\":\"true\",\"transport_pref\":\"tcp\",\"video_status\":\"healthy\",\"audio_status\":\"healthy\",\"md_status\":\"healthy\"}]}}";

            response.SetBody(body);
            response.WriteResponse(clientSocket);

            clientSocket->Shutdown(SOCKET_SHUT_SEND_FLAGS);
            clientSocket->Close();
            socket.Shutdown(SOCKET_SHUT_RECV_FLAGS);
            socket.Close();
    });
    th.detach();
    sleep(2);

    std::vector<DataSource> result = FetchDataSources("127.0.0.1",port);
    UT_ASSERT_EQUAL(1,result.size());
}

void TestDatSources::TestMultiDataSourceValues(void)
{
    printf("TestDatSources::TestMultiDataSourceValues\n");
    XSocket socket;
    int port = socket.BindEphemeral("127.0.0.1");
    auto th = std::thread([&]{
            socket.Listen();

            XRef<XSocket> clientSocket = socket.Accept();    
            ServerSideRequest request;
            request.ReadRequest(clientSocket);    
            URI uri = request.GetURI();

            ServerSideResponse response;

            XString body = "{\"data\":{\"data_sources\":["\
                        "{\"id\":\"1\",\"type\":\"video\",\"source_url\":\"1\",\"recording_video\":\"true\",\"recording_audio\":\"false\",\"recording_md\":\"true\",\"transport_pref\":\"tcp\",\"video_status\":\"healthy\",\"audio_status\":\"healthy\",\"md_status\":\"unhealthy\"},"\
                        "{\"id\":\"2\",\"type\":\"audio\",\"source_url\":\"2\",\"recording_video\":\"false\",\"recording_audio\":\"true\",\"recording_md\":\"false\",\"transport_pref\":\"tcp\",\"video_status\":\"healthy\",\"audio_status\":\"healthy\",\"md_status\":\"unhealthy\"},"\
                        "{\"id\":\"3\",\"type\":\"md\",\"source_url\":\"3\",\"recording_video\":\"false\",\"recording_audio\":\"false\",\"recording_md\":\"true\",\"transport_pref\":\"tcp\",\"video_status\":\"unhealthy\",\"audio_status\":\"healthy\",\"md_status\":\"healthy\"}"\
                        "]}}";

            response.SetBody(body);
            response.WriteResponse(clientSocket);

            clientSocket->Shutdown(SOCKET_SHUT_SEND_FLAGS);
            clientSocket->Close();
            socket.Shutdown(SOCKET_SHUT_RECV_FLAGS);
            socket.Close();
    });
    th.detach();
    sleep(2);

    std::vector<DataSource> result = FetchDataSources("127.0.0.1",port);
    UT_ASSERT_EQUAL(3,result.size());    
    //Check IDS
    UT_ASSERT_EQUAL("1",result[0]._id);
    UT_ASSERT_EQUAL("2",result[1]._id);
    UT_ASSERT_EQUAL("3",result[2]._id);
    ///CHeck URL
    UT_ASSERT_EQUAL("1",result[0]._sourceUrl);
    UT_ASSERT_EQUAL("2",result[1]._sourceUrl);
    UT_ASSERT_EQUAL("3",result[2]._sourceUrl);
    ///Check type
    UT_ASSERT_EQUAL(SourceType::Video,result[0]._type);
    UT_ASSERT_EQUAL(SourceType::Audio,result[1]._type);
    UT_ASSERT_EQUAL(SourceType::MetaData,result[2]._type);
    ///Is recording
    UT_ASSERT(result[0]._recordingVideo);
    UT_ASSERT(!result[1]._recordingVideo);
    UT_ASSERT(!result[2]._recordingVideo);

    UT_ASSERT(!result[0]._recordingAudio);
    UT_ASSERT(result[1]._recordingAudio);
    UT_ASSERT(!result[2]._recordingAudio);

    UT_ASSERT(result[0]._recordingMetaData);
    UT_ASSERT(!result[1]._recordingMetaData);
    UT_ASSERT(result[2]._recordingMetaData);
    ///Is Healthy
    UT_ASSERT(result[0]._videoHealty);
    UT_ASSERT(result[1]._videoHealty);
    UT_ASSERT(!result[2]._videoHealty);

    UT_ASSERT(result[0]._audioHealthy);
    UT_ASSERT(result[1]._audioHealthy);
    UT_ASSERT(result[2]._audioHealthy);

    UT_ASSERT(!result[0]._metaDataHealthy);
    UT_ASSERT(!result[1]._metaDataHealthy);
    UT_ASSERT(result[2]._metaDataHealthy);
}
