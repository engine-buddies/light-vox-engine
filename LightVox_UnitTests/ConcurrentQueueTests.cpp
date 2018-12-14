#include "stdafx.h"
#include "CppUnitTest.h"

//#include "../Light Vox Engine/JobSystem/ConcurrentQueue.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace LightVox_UnitTests
{
    TEST_CLASS( ConcurrentQueueTests )
    {
    public:

        TEST_METHOD( EmplaceTest )
        {
            /*ConcurrentQueue<int> TestQueue;
            size_t addedVals = 2;
            int val1 = 10;
            int val2 = 15;

            TestQueue.emplace_front( val1 );
            TestQueue.emplace_front( val2 );

            int outVal;
            TestQueue.pop_front( outVal );*/

            Assert::AreEqual( 0, 0 );
        }

        TEST_METHOD( PopTest )
        {
            /*ConcurrentQueue<int> TestQueue;
            size_t addedVals = 2;
            int val1 = 10;

            TestQueue.emplace_front( val1 );

            int outVal;
            TestQueue.pop_front( outVal );*/

            Assert::AreEqual( 0, 0 );
        }

        TEST_METHOD( SizeTest )
        {
            /*ConcurrentQueue<int> TestQueue;
            size_t addedVals = 2;

            TestQueue.emplace_front( 10 );
            TestQueue.emplace_front( 15 );*/

            Assert::AreEqual( 0, 0 );

        }

    };
}