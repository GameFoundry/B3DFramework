//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Testing/B3DTestSuite.h"

namespace b3d
{
	/** Tests render::FrameGraph: culling, transient placement, barrier planning, the async compute queue and the development checks. */
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
		 * A graph's transients reuse the memory of an earlier graph's transients, which the earlier graph's last pass still reads when the
		 * later graph starts. Their first use waits for the earlier graph's work.
		 */
		void TestCrossGraphOrdering();

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
		 * records through its context's profiler nests in the pass's sample. A graph that uses the async compute queue records one sample
		 * for each of its command buffers, named after the graph, the queue and the command buffer's position on it, with the passes of
		 * the command buffer in it.
		 */
		void TestProfiling();

		/**
		 * A raster pass draws with a renderer material through a parameter set it declares, and through a set the graph does not
		 * declare, which holds a texture at rest.
		 */
		void TestRasterPass();

		/**
		 * A raster pass samples the depth of its depth-stencil attachment while its stencil tests may write the stencil
		 * (FrameGraphDepthStencilMode::DepthReadOnly), after a pass that cleared both.
		 */
		void TestDepthReadOnly();

		/**
		 * Raster passes store the attachments whose contents a later surviving pass or the end of the graph observes, and discard the
		 * others: the last write of a transient, a transient a later pass clears again, and the stencil of a depth-stencil texture whose
		 * depth alone is sampled later. Imported, extracted and exported textures are stored, and so is a texture array of which a later
		 * pass reads one layer. Read-only attachments are never discarded.
		 */
		void TestStoreInference();

		/**
		 * A raster pass that loads transients no earlier pass wrote does not load them, and development builds report it. Loads of
		 * transients an earlier pass wrote, and of imported textures, are kept.
		 */
		void TestLoadInference();

		/**
		 * A raster pass that clears its target and alternates between queries of two occlusion query pools, one precise and one binary,
		 * reports the samples of each draw to the query active around it, and none to a query around no draw.
		 */
		void TestOcclusionQueries();

		/**
		 * A depth-stencil transient and a color transient are first used by a later pass, on memory no earlier transient of the graph
		 * used. The color transient's alias acquire is a split barrier released at the start of the graph; the depth-stencil one, which
		 * gets no split barriers, is a full barrier at the start of the graph.
		 */
		void TestTwoAspectAliasAcquire();

		/**
		 * Execute callbacks run once each during Execute(), in the order they were added, after the pass functions recorded the passes.
		 * A graph without passes runs them too.
		 */
		void TestExecuteCallbacks();

		/**
		 * An async compute pass writes a buffer and a texture that a graphics pass reads. A prologue on the graphics queue releases the
		 * imports to the compute queue, and both go back to the graphics queue for the reader.
		 */
		void TestAsyncComputeProducer();

		/**
		 * Async compute passes sample a texture a graphics dispatch wrote, and the depth of a depth-stencil texture a raster pass cleared.
		 * Both are transferred to the compute queue, and the texture back to the graphics queue at the end of the graph.
		 */
		void TestAsyncComputeConsumer();

		/**
		 * A graphics pass reads what an earlier graphics pass wrote, in a later graphics submission that waits for an async compute pass.
		 * The barrier between them is a split barrier on the graphics queue, released in one submission and acquired in the next.
		 */
		void TestSplitAcrossSubmissions();

		/**
		 * A transient whose memory a transient of an earlier submission on the same queue used gets its alias acquire released at the start
		 * of its own submission. Repeated with transient aliasing disabled, with the same results.
		 */
		void TestAliasAcquireAcrossSubmissions();

		/**
		 * A texture that both queues can read is read by a graphics pass and an async compute pass in one state, with no barrier and no
		 * transfer between them. Repeated with a graphics write after the reads, which waits for the async read and is a full barrier on
		 * the graphics queue.
		 */
		void TestSharedReaders();

		/**
		 * A render target texture that rests on the graphics queue is written on graphics and sampled on async compute. The graph ends with
		 * a graphics epilogue that returns it, and a buffer the async pass wrote, to rest on the graphics queue, where a later graph reads
		 * them at rest.
		 */
		void TestOwnershipTransfer();

		/**
		 * Async compute work that depends on no graphics work of the graph is submitted first, and waits for no submission of the graph.
		 * The graphics pass that reads its results waits for it.
		 */
		void TestIndependentCompute();

		/**
		 * Transients of both queues, used in submissions that are and are not ordered after each other, hold their values with transient
		 * aliasing enabled and disabled.
		 */
		void TestTransientAliasingAcrossQueues();

		/** A transient written on the async compute queue and extracted is at rest on the graphics queue for a later graph that imports it. */
		void TestAsyncComputeExtract();

		/** The producer graph of TestAsyncComputeProducer() is one graphics submission with gFrameGraphAsyncCompute disabled, with the same results. */
		void TestAsyncComputeDisabled();

		/**
		 * Async compute passes export a buffer and a texture that both queues can read, which a later async compute pass and a graphics
		 * pass read without declaring them. Another exports a texture that rests on the graphics queue, which returns there through a split
		 * barrier acquired where the graphics queue waits for the export, before a graphics pass reads it without declaring it.
		 */
		void TestAsyncComputeExport();

		/** A copy pass with FrameGraphPassFlag::AsyncCompute copies what a graphics dispatch wrote on the async compute queue. */
		void TestAsyncCopy();

		/**
		 * The async compute tests again, with frame graphs that record their submissions in parallel (FrameGraphFlag::RecordInParallel),
		 * with the same results.
		 */
		void TestParallelRecording();

		/**
		 * Frames of a graph that records in parallel, whose passes alternate between the queues in nine submissions. Every pass after the
		 * first binds one parameter set, so the set is bound on both queues and on several threads at once.
		 */
		void TestParallelRecordingStress();

#if B3D_BUILD_TYPE_DEVELOPMENT
		/**
		 * The development checks: writes to resources at rest, conflicting declarations in one pass, writes after an export, dependencies
		 * between dispatches of one pass, writes through parameter sets the pass did not create, undeclared reads of a written resource
		 * that was not exported, and use of a transient that was returned unplaced.
		 */
		void TestValidation();

		/** An async compute pass that samples a texture at rest that only the graphics queue can read reports it when it is added. */
		void TestAsyncComputeResting();

		/** A graph executed from a pass of another graph that executes reports the overlap. */
		void TestInterleavedGraphs();

		/** A raster pass created with FrameGraphPassFlag::AsyncCompute reports it, and runs on the graphics queue. */
		void TestAsyncRasterPass();
#endif
	};
} // namespace b3d
