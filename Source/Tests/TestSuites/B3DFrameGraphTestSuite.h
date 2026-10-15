//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Testing/B3DTestSuite.h"

namespace b3d
{
	/**
	 * Tests render::FrameGraph: culling, transient placement, barrier planning and the development checks. Every test runs in explicit
	 * mode and in automatic mode (see gGpuExplicitBarriers), except TestValidation, which runs in explicit mode only.
	 */
	class FrameGraphTestSuite : public TestSuite
	{
	public:
		FrameGraphTestSuite();

	private:
		/**
		 * A producer, two unrelated passes and a consumer: the consumer's barrier is released after the producer and acquired before the
		 * consumer, and the consumer reads what the producer wrote.
		 */
		void TestSplitBarriers();

		/**
		 * Two clears of a transient followed by a read: the first clear is culled, as is a pass whose writes nothing reads. Raster passes
		 * render into attachments through a render texture the graph builds.
		 */
		void TestCulling();

		/**
		 * A transient whose memory earlier transients used gets an alias acquire, released after the last pass of the earlier transients
		 * when passes lie between that pass and its first use.
		 */
		void TestSplitAliasAcquire();

		/**
		 * An extracted transient survives the graph at rest in its own memory, and a later graph imports and reads it. Repeated with the
		 * transient also exported by the pass that writes it.
		 */
		void TestExtract();

		/** An exported resource and a resource returned at the end of the graph are both at rest for the next graph that imports them. */
		void TestExport();

		/**
		 * Resources at rest are read without an import or a barrier: a texture uploaded in an earlier frame, and a render target texture an
		 * earlier graph wrote and returned to rest at its end.
		 */
		void TestRestingReads();

		/**
		 * A transient set into a parameter set before Compile places it holds the dispatch's values, also when a later graph places the
		 * cached transient at other memory.
		 */
		void TestLateMemory();

		/**
		 * TextureUtility::ReadAsync() and GpuBufferUtility::ReadAsync() read transients through copy passes into imported staging buffers.
		 * The graph's end makes the copies visible to the host, and the reads complete with the graph's command buffer.
		 */
		void TestReadback();

		/**
		 * Typed parameter handles cached from one parameter set write another set of the same layout, as materials fill the sets of
		 * frame graph passes. The pass declares the graph resources set through the handles.
		 */
		void TestParameterHandles();

		/**
		 * The graph's GPU sample holds a sample for each surviving pass, named after the pass and in execution order, and a sample a pass
		 * records through its context's profiler nests in the pass's sample.
		 */
		void TestProfiling();

		/**
		 * A raster pass draws with a renderer material through a parameter set it declares, and through a set added with
		 * FrameGraphRasterPass::AddRenderPassParameterSet() that holds a texture at rest.
		 */
		void TestRasterPass();

		/**
		 * A raster pass samples the depth of its depth-stencil attachment while its stencil tests may write the stencil
		 * (FrameGraphDepthStencilMode::DepthReadOnly), after a pass that cleared both.
		 */
		void TestDepthReadOnly();

#if B3D_BUILD_TYPE_DEVELOPMENT
		/**
		 * The development checks: writes to resources at rest, conflicting declarations in one pass, writes after an export, dependencies
		 * between dispatches of one pass, writes through parameter sets the pass did not create, undeclared reads of a written resource
		 * that was not exported, and use of a transient that was returned unplaced.
		 */
		void TestValidation();
#endif
	};
} // namespace b3d
