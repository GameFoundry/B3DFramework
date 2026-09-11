//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Testing/B3DTestSuite.h"

namespace b3d
{
	/** CPU tests of native image state ownership, deferred accesses and submission hazards. */
	class GpuImageMetadataStateTestSuite : public TestSuite
	{
	public:
		GpuImageMetadataStateTestSuite();

	private:
		/** Verifies independent partitions inherit deferred accesses and barriers in command order. */
		void TestRangeSplits();

		/** Verifies only selected native writes acquire readers and publish a writer epoch. */
		void TestSubmissionSelection();

		/** Verifies render-pass aggregation, base-typed accesses and post-barrier callbacks use static backend dispatch. */
		void TestStaticDispatch();

		/** Verifies an internal write participates in layout ordering, range splitting and reset. */
		void TestInternalAccess();

		/** Verifies compute attachment clears use deferred hazards without changing attachment layouts. */
		void TestAttachmentClear();

		/** Verifies shader-binding validation is distinct from sequential operations and repeated writes remain ordered. */
		void TestShaderBindingAccess();
		void TestShaderBindingReuse();
	};
}
