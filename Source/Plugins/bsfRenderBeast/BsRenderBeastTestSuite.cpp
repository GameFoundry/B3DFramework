//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Testing/BsTestSuite.h"
#include "Utility/BsTextureRowAllocator.h"

namespace bs
{
	/** Runs unit tests for systems specific to the RenderBeast plugin. */
	class RenderBeastTestSuite : public TestSuite
	{
	public:
		RenderBeastTestSuite();

	private:
		void TestTextureRowAllocator();
	};

	RenderBeastTestSuite::RenderBeastTestSuite()
	{
		BS_ADD_TEST(RenderBeastTestSuite::TestTextureRowAllocator);
	}

	void RenderBeastTestSuite::TestTextureRowAllocator()
	{
		ct::TextureRowAllocator<128, 128> alloc;

		auto a0 = alloc.Alloc(16);
		BS_TEST_ASSERT(a0.x == 0 && a0.y == 0 && a0.length == 16);

		auto a1 = alloc.Alloc(16);
		BS_TEST_ASSERT(a1.x == (a0.x + a0.length));

		auto a2 = alloc.Alloc(8);
		auto a3 = alloc.Alloc(8);
		auto a4 = alloc.Alloc(16);
		auto a5 = alloc.Alloc(16);
		auto a6 = alloc.Alloc(8);
		auto a7 = alloc.Alloc(8);
		alloc.Alloc(32);

		// Test if free space can get re-allocated
		alloc.Free(a1);

		auto a8 = alloc.Alloc(16);
		BS_TEST_ASSERT(a8.x == a1.x);

		// Test if free space gets merged and can be reallocated
		alloc.Free(a4);
		alloc.Free(a2);
		alloc.Free(a3);
		alloc.Free(a6);
		alloc.Free(a7);
		alloc.Free(a5);

		auto a9 = alloc.Alloc(64);
		BS_TEST_ASSERT(a9.x == a2.x && a9.y == 0 && a9.length == 64);

		// Test if allocation to another row works
		auto a10 = alloc.Alloc(64);
		BS_TEST_ASSERT(a10.x == 0 && a10.y == 1 && a10.length == 64);

		// Test if allocation that doesn't fit goes to a new row
		auto a11 = alloc.Alloc(128);
		BS_TEST_ASSERT(a11.x == 0 && a11.y == 2 && a11.length == 128);

		// Test if too large allocation fails
		auto a12 = alloc.Alloc(256);
		BS_TEST_ASSERT(a12.length == 0);

		// Test if zero allocation is handled gracefully
		auto a13 = alloc.Alloc(0);
		BS_TEST_ASSERT(a13.length == 0);
	}
}
