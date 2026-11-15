//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Testing/B3DTestSuite.h"

namespace b3d
{
	/** Tests the D3D12 objects behind acceleration structures. The tests run only on D3D12 devices with acceleration structures. */
	class D3D12RayTracingTestSuite : public TestSuite
	{
	public:
		D3D12RayTracingTestSuite();

	private:
		/**
		 * Checks that buffers that can be scratch memory have addresses aligned to the scratch offset alignment, instance buffers to 16 bytes,
		 * and that acceleration structures are buffers created for them, with addresses aligned to 256 bytes and a shader resource view.
		 */
		void TestAddresses();

		/**
		 * Checks that instances are written as D3D12_RAYTRACING_INSTANCE_DESC, with the address of their bottom level structure, and with
		 * counterclockwise front faces unless the instance flips the engine's facing.
		 */
		void TestInstanceLayout();

		/**
		 * Records the barriers that acceleration structure builds and traces need, as D3D12BarrierUtility builds them, and checks that the
		 * debug layer accepts them. Skipped without the debug layer.
		 */
		void TestBarriers();
	};
} // namespace b3d
