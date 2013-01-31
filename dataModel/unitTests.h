#include "NKeypoint.h"
#include "NVector.h"
#include "NTag.h"
#include "NRoi.h"
#include "Poco/Data/SQLChannel.h"
#include "Poco/Data/SessionFactory.h"
#include "Poco/Data/SQLite/Connector.h"
#include "Poco/Data/SQLite/Utility.h"
#include <../cpptest.h>
#include <../appData/GameImage.h>

class DataModelTests : public Test::Suite
{
public:
    DataModelTests()
    {
        TEST_ADD(DataModelTests::storeAndLoadNKeypoints)
		TEST_ADD(DataModelTests::storeAndLoadNVector)
        TEST_ADD(DataModelTests::storeAndLoadGameImage)
        TEST_ADD(DataModelTests::storeAndLoadNTag)
        TEST_ADD(DataModelTests::storeAndLoadNRoi)
    }

private:
    void storeAndLoadNKeypoints();
    void storeAndLoadNVector();
    void storeAndLoadGameImage();
    void storeAndLoadNTag();
    void storeAndLoadNRoi();
};
