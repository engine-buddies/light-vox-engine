#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace LightVoxUnitTest
{
    TEST_CLASS( UnitTest1 )
    {
    public:

        TEST_METHOD( AddJobTest )
        {
            using namespace Jobs;
            JobManager* man = JobManager::GetInstance();

            //Assert::AreEqual( 1, 1 );


            Assert::IsNotNull( man );

            JobManager::ReleaseInstance();
            // TODO: Your test code here
        }

    };
}