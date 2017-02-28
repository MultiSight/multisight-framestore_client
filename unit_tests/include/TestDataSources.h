#ifndef _TestDatSources_h_
#define _TestDatSources_h_

#include "XSDK/XMemory.h"
#include "XSDK/XTaskBase.h"

#include "framework.h"

class TestDatSources : public test_fixture, public XSDK::XTaskBase
{
public:
    TEST_SUITE(TestDatSources);
        TEST(TestDatSources::TestEmptyDataSources);
        TEST(TestDatSources::TestOneDataSource);
        TEST(TestDatSources::TestMultiDataSourceValues);
    TEST_SUITE_END();

    virtual ~TestDatSources() throw();

    void setup();
    void teardown();

    void TestEmptyDataSources();
    void TestOneDataSource();
    void TestMultiDataSourceValues();
private:
protected:
    virtual void* EntryPoint();
};


#endif