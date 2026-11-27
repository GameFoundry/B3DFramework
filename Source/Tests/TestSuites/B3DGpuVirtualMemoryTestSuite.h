//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Testing/B3DTestSuite.h"

namespace b3d
{
	/** Tests for virtual buffers and textures: the page pool, the page layout of virtual textures and page tables. */
	class GpuVirtualMemoryTestSuite : public TestSuite
	{
	public:
		GpuVirtualMemoryTestSuite();

	private:
		/**
		 * Verifies the page pool on a heap backend stub: best fit and longest-run allocation, heaps added for missing pages, all-or-nothing
		 * failure, pages returned by Free() only once their frame completes, pages returned by FreeAndReclaim() at once, and the release
		 * of idle heaps except for the spares the pool keeps.
		 */
		void TestPagePool();

		/** Verifies the page order of virtual texture layouts, with a tail per layer and a shared tail, and the Null device's tile sizes. */
		void TestTextureLayout();

		/**
		 * Verifies the page table of a virtual buffer on devices that support virtual buffers: commits, decommits whose removal is pending until
		 * their frame completes, re-commits that get the old memory back, commits of memory from an allocation, rejected over pages with
		 * memory or a pending removal, interleaved committed and empty pages, the memory a destroyed buffer returns, and the rejection of invalid ranges
		 * and resources.
		 */
		void TestPageTable();

		/** Verifies that a virtual texture gets a page table with the pages of its layout, on devices that support virtual textures. */
		void TestVirtualTexture();
	};
} // namespace b3d
