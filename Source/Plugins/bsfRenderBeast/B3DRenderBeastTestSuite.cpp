//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Testing/B3DTestSuite.h"
#include "Utility/B3DTextureRowAllocator.h"

using namespace b3d;

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
	B3D_ADD_TEST(RenderBeastTestSuite::TestTextureRowAllocator);
}

void RenderBeastTestSuite::TestTextureRowAllocator()
{
	render::TextureRowAllocator<128, 128> alloc;

	auto a0 = alloc.Alloc(16);
	B3D_TEST_ASSERT(a0.X == 0 && a0.Y == 0 && a0.Length == 16);

	auto a1 = alloc.Alloc(16);
	B3D_TEST_ASSERT(a1.X == (a0.X + a0.Length));

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
	B3D_TEST_ASSERT(a8.X == a1.X);

	// Test if free space gets merged and can be reallocated
	alloc.Free(a4);
	alloc.Free(a2);
	alloc.Free(a3);
	alloc.Free(a6);
	alloc.Free(a7);
	alloc.Free(a5);

	auto a9 = alloc.Alloc(64);
	B3D_TEST_ASSERT(a9.X == a2.X && a9.Y == 0 && a9.Length == 64);

	// Test if allocation to another row works
	auto a10 = alloc.Alloc(64);
	B3D_TEST_ASSERT(a10.X == 0 && a10.Y == 1 && a10.Length == 64);

	// Test if allocation that doesn't fit goes to a new row
	auto a11 = alloc.Alloc(128);
	B3D_TEST_ASSERT(a11.X == 0 && a11.Y == 2 && a11.Length == 128);

	// Test if too large allocation fails
	auto a12 = alloc.Alloc(256);
	B3D_TEST_ASSERT(a12.Length == 0);

	// Test if zero allocation is handled gracefully
	auto a13 = alloc.Alloc(0);
	B3D_TEST_ASSERT(a13.Length == 0);
}
