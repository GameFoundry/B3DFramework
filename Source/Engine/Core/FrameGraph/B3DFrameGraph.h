//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuSubmissionTimeline.h"
#include "GpuBackend/B3DGpuTimelineFence.h"
#include "GpuBackend/B3DGpuTransientResourceAllocator.h"
#include "GpuBackend/B3DRenderTarget.h"
#include "Utility/B3DConfigVariable.h"

namespace b3d
{
	class GpuWorkContext;

	/**
	 * If enabled, passes created with render::FrameGraphPassFlag::AsyncCompute run on the async compute queue. If disabled, every pass
	 * runs on the graphics queue.
	 */
	extern B3D_EXPORT TConfigVariable<bool> gFrameGraphAsyncCompute;

#if B3D_BUILD_TYPE_DEVELOPMENT
	/**
	 * If enabled, frame graphs created with render::FrameGraphFlag::RecordInParallel record their passes one at a time, like other frame
	 * graphs. Graphs that record one pass at a time do so in the order the GPU executes them, and validate every command buffer of an
	 * execution with one shared GpuBarrierValidator. Graphs that record in parallel validate nothing. Development builds only.
	 */
	extern B3D_EXPORT TConfigVariable<bool> gFrameGraphSerialRecording;
#endif

	namespace render
	{
		class FrameGraph;
		class GpuBarrierValidator;

		/** @addtogroup Renderer
		 *  @{
		 */

		/** Options of a pass of a FrameGraph. */
		enum class FrameGraphPassFlag : u8
		{
			None = 0,

			/**
			 * The pass prefers the async compute queue, so it may overlap graphics work. Compute and copy passes only: a raster pass reports
			 * it and runs on the graphics queue. The pass runs on the graphics queue too if gFrameGraphAsyncCompute is disabled or the device
			 * has no compute queue. A pass that prefers the async compute queue may only read resources at rest that can be read on several
			 * queues at once (see GpuBackendUtility::IsConcurrentlyReadable()). Others must be imported.
			 */
			AsyncCompute = 1 << 0
		};

		typedef Flags<FrameGraphPassFlag, u8> FrameGraphPassFlags;
		B3D_FLAGS_OPERATORS_EXT(FrameGraphPassFlag, u8)

		/** Options of a FrameGraph. */
		enum class FrameGraphFlag : u8
		{
			None = 0,

			/**
			 * The graph records its command buffers in parallel, one task per submission, if it has several submissions. Submissions with
			 * passes that use a window are recorded on the calling thread. The execute functions of the passes run on several threads at
			 * once, and must not:
			 *  - use the work context of the graph,
			 *  - write parameter sets or buffers, such as with Set* calls or TextureUtility::Write(),
			 *  - create pipeline states, parameter layouts or materials,
			 *  - render to or copy from a window,
			 *  - discard-write, relocate or recreate resources,
			 *  - yield the fiber they run on.
			 *
			 * The graph flushes the parameter sets added to the passes with FrameGraphPass::AddParameterSet() for their passes before it
			 * records (see GpuParameterSet::Flush()). Sets a pass binds without adding them must be flushed by the caller. A set holds one
			 * layout per sampled texture, so passes that add the same set must sample its textures in the same layouts: either all render
			 * into a texture they sample, or none do.
			 */
			RecordInParallel = 1 << 0
		};

		typedef Flags<FrameGraphFlag, u8> FrameGraphFlags;
		B3D_FLAGS_OPERATORS_EXT(FrameGraphFlag, u8)

		/** Determines what a raster pass does with the existing contents of an attachment. */
		enum class FrameGraphAttachmentLoad
		{
			Load, /**< The pass reads the existing contents, such as for blending. */
			Clear, /**< The pass clears the attachment to the clear value of its texture or render target. */
			DontCare /**< The pass overwrites every pixel it reads. The existing contents are discarded. */
		};

		/** Determines how a raster pass accesses its depth-stencil attachment. */
		enum class FrameGraphDepthStencilMode
		{
			Write, /**< Depth and stencil tests may write the attachment. */
			ReadOnly, /**< Depth and stencil tests only read the attachment, which the pass may also sample. */
			DepthReadOnly /**< Depth tests only read the depth aspect, which the pass may also sample. Stencil tests may write the stencil aspect. */
		};

		/** Gives the execute function of a pass access to the command buffer the graph records the pass on. */
		class B3D_EXPORT FrameGraphPassContext
		{
		public:
			/**
			 * Returns the command buffer to record the pass's commands on. In a raster pass the render pass is open. The pass must not
			 * open or close render passes, or issue barriers.
			 */
			GpuCommandBuffer& GetCommandBuffer() const { return mCommandBuffer; }

#if B3D_PROFILING_ENABLED
			/**
			 * Returns the profiler that records the GPU samples of the graph, or null if the device has no timer queries. Samples the pass
			 * records with it nest in the pass's sample, and must end within the pass.
			 */
			const TShared<GpuCommandBufferProfiler>& GetProfiler() const { return mCommandBuffer.GetProfiler(); }
#endif

		private:
			friend class FrameGraph;

			FrameGraphPassContext(GpuCommandBuffer& commandBuffer)
				: mCommandBuffer(commandBuffer)
			{ }

			GpuCommandBuffer& mCommandBuffer;
		};

		/**
		 * Describes what a pass of a FrameGraph reads and writes. The graph resources a pass binds are declared by placing them in parameter
		 * sets added with AddParameterSet(), before the pass is added to the graph. Resources that are not graph resources are at rest and
		 * may only be read, so they need no declaration. Holds no reference to a graph.
		 */
		class B3D_EXPORT FrameGraphPass
		{
		public:
			/**
			 * Adds @p parameters to the parameter sets of the pass. The graph resources they hold when the pass is added to the graph are what
			 * the pass declares. Sets that hold no graph resources may be bound without being added. See
			 * RendererMaterialBase::CreateGpuParameterSet() for a helper that creates a set for a material and adds it.
			 */
			void AddParameterSet(const TShared<GpuParameterSet>& parameters);

			/**
			 * Returns @p texture to its resting state right after the pass, so later work may read it without a declaration. Later passes
			 * must not write it. Only imported and extracted resources can be exported.
			 */
			void Export(const TShared<Texture>& texture);

			/** @copydoc Export(const TShared<Texture>&) */
			void Export(const TShared<GpuBuffer>& buffer);

		protected:
			friend class FrameGraph;

			explicit FrameGraphPass(FrameGraphPassFlags flags)
				: mFlags(flags)
			{ }

			FrameGraphPassFlags mFlags;
			TInlineArray<TShared<GpuParameterSet>, 4> mParameterSets;
			TInlineArray<TShared<Texture>, 2> mExportedTextures;
			TInlineArray<TShared<GpuBuffer>, 2> mExportedBuffers;
		};

		/** Describes a pass of a FrameGraph that draws into attachments. The graph opens the render pass before the pass executes. */
		class B3D_EXPORT FrameGraphRasterPass : public FrameGraphPass
		{
		public:
			/** Creates a pass with @p flags. Raster passes support none: FrameGraphPassFlag::AsyncCompute is reported and ignored. */
			explicit FrameGraphRasterPass(FrameGraphPassFlags flags = FrameGraphPassFlag::None)
				: FrameGraphPass(flags)
			{ }

			/**
			 * Renders into @p texture as color attachment @p index.
			 *
			 * @param	index		Index of the attachment, smaller than B3D_MAXIMUM_RENDER_TARGET_COUNT.
			 * @param	texture		Texture of the graph, created with TextureUsageFlag::RenderTarget.
			 * @param	load		What the pass does with the existing contents of the attachment.
			 * @param	mipLevel	Mip level to render into.
			 * @param	face		First array layer to render into.
			 * @param	faceCount	Number of array layers to render into.
			 */
			void SetColorAttachment(u32 index, const TShared<Texture>& texture, FrameGraphAttachmentLoad load, u32 mipLevel = 0, u32 face = 0, u32 faceCount = 1);

			/**
			 * Renders with @p texture as the depth-stencil attachment.
			 *
			 * @param	texture		Texture of the graph, created with TextureUsageFlag::DepthStencil.
			 * @param	mode		Whether depth and stencil tests may write the attachment.
			 * @param	load		What the pass does with the existing contents of the attachment.
			 * @param	mipLevel	Mip level to render into.
			 * @param	face		First array layer to render into.
			 * @param	faceCount	Number of array layers to render into.
			 */
			void SetDepthStencilAttachment(const TShared<Texture>& texture, FrameGraphDepthStencilMode mode, FrameGraphAttachmentLoad load, u32 mipLevel = 0, u32 face = 0, u32 faceCount = 1);

			/**
			 * Renders into every surface of @p target, which must be imported with FrameGraph::ImportRenderTarget(). Replaces the attachments
			 * set with SetColorAttachment() and SetDepthStencilAttachment().
			 */
			void SetRenderTarget(const TShared<RenderTarget>& target, FrameGraphAttachmentLoad load);

		private:
			friend class FrameGraph;

			/** Attachment set by SetColorAttachment() or SetDepthStencilAttachment(). */
			struct Attachment
			{
				TShared<render::Texture> Texture;
				FrameGraphAttachmentLoad Load = FrameGraphAttachmentLoad::Load;
				u32 MipLevel = 0;
				u32 Face = 0;
				u32 FaceCount = 1;
			};

			Attachment mColorAttachments[B3D_MAXIMUM_RENDER_TARGET_COUNT];
			Attachment mDepthStencilAttachment;
			FrameGraphDepthStencilMode mDepthStencilMode = FrameGraphDepthStencilMode::Write;

			TShared<RenderTarget> mRenderTarget;
			FrameGraphAttachmentLoad mRenderTargetLoad = FrameGraphAttachmentLoad::Load;
		};

		/** Describes a pass of a FrameGraph that dispatches compute work. */
		class B3D_EXPORT FrameGraphComputePass : public FrameGraphPass
		{
		public:
			/** Creates a pass with @p flags. */
			explicit FrameGraphComputePass(FrameGraphPassFlags flags = FrameGraphPassFlag::None)
				: FrameGraphPass(flags)
			{ }
		};

		/** Describes a pass of a FrameGraph that copies between resources with transfer operations, such as GpuCommandBuffer::CopyTexture(). */
		class B3D_EXPORT FrameGraphCopyPass : public FrameGraphPass
		{
		public:
			/** Creates a pass with @p flags. */
			explicit FrameGraphCopyPass(FrameGraphPassFlags flags = FrameGraphPassFlag::None)
				: FrameGraphPass(flags)
			{ }

			/** Declares that the pass copies from @p surface of @p texture, which must be a resource of the graph. */
			void AddSource(const TShared<Texture>& texture, const TextureSurface& surface = TextureSurface::kComplete);

			/** Declares that the pass copies from @p buffer, which must be a resource of the graph. */
			void AddSource(const TShared<GpuBuffer>& buffer);

			/** Declares that the pass copies into @p surface of @p texture, which must be a resource of the graph. */
			void AddDestination(const TShared<Texture>& texture, const TextureSurface& surface = TextureSurface::kComplete);

			/** Declares that the pass copies into @p buffer, which must be a resource of the graph. */
			void AddDestination(const TShared<GpuBuffer>& buffer);

		private:
			friend class FrameGraph;

			/** Texture or buffer declared by AddSource() or AddDestination(). */
			struct CopyResource
			{
				TShared<render::Texture> Texture;
				TShared<GpuBuffer> Buffer;
				TextureSurface Surface;
				bool IsDestination = false;
			};

			TInlineArray<CopyResource, 4> mResources;
		};

		/** Pass and barrier counts of a compiled FrameGraph. */
		struct FrameGraphStatistics
		{
			u32 PassCount = 0; /**< Number of passes added to the graph. */
			u32 CulledPassCount = 0; /**< Number of passes culled because nothing observes their writes. */
			u32 FullBarrierCount = 0; /**< Number of barriers issued as full barriers, alias acquires included. */
			u32 SplitBarrierCount = 0; /**< Number of barriers issued as split barriers, alias acquires included. */
			u32 BarrierBatchCount = 0; /**< Number of barrier batches: GpuSplitBarrier objects that record the split barriers. */
			u32 AliasAcquireCount = 0; /**< Number of alias acquires, full and split. */
			u32 SplitAliasAcquireCount = 0; /**< Number of alias acquires released after an earlier pass, as split barriers. */
			u32 AliasReleaseCount = 0; /**< Number of textures whose lifetimes an alias release ended, see GpuBarrierFlag::AliasRelease. */
			u32 SubmissionCount = 0; /**< Number of command buffers the graph submits. */
			u32 QueueTransferCount = 0; /**< Number of barriers issued as split barriers between queues. Also counted by SplitBarrierCount. */

			/**
			 * Number of queues the submissions wait on for earlier submissions of the graph, summed over the submissions. Waits for work
			 * submitted before the graph are not counted.
			 */
			u32 QueueWaitCount = 0;
		};

		/**
		 * Describes the GPU work of a frame, or a part of it, as passes and the resources they read and write, then records it with
		 * barriers. Passes are declared first with AddRasterPass(), AddComputePass() and AddCopyPass(), then Compile() plans the
		 * work and Execute() records and submits it.
		 *
		 * Resources:
		 *  - Transient: created by CreateTexture() or CreateBuffer(). Memory is shared with transients of disjoint lifetimes, through
		 *    the work context's GpuTransientResourceAllocator. Valid only within the graph, unless extracted.
		 *  - Imported: existing resources registered with ImportTexture(), ImportBuffer() or ImportRenderTarget(). They are at rest when
		 *    imported, and the graph returns them to rest after their export or at the end of the graph.
		 *  - Resting: every other resource. Passes may read them without a declaration, but never write them.
		 *
		 * A pass reads whatever the pass declared before it wrote. Passes whose writes nothing observes are culled, along with their
		 * transients. Barriers are split whenever other passes run between the accesses they order.
		 *
		 * Passes run on the graphics queue, unless a compute or copy pass prefers the async compute queue (FrameGraphPassFlag::AsyncCompute).
		 * The graph then submits its passes as several command buffers on both queues, with the queue waits their accesses require, and
		 * ends with the graphics queue waiting on the compute queue.
		 *
		 * Graphs created with FrameGraphFlag::RecordInParallel record their submissions on several threads.
		 *
		 * @note	Not thread safe. Only the thread that owns the work context may use the graph. Graphs of different work contexts share
		 *			the device's transient heaps, so their GPU work must follow the scope contract of GpuTransientResourceAllocator. The
		 *			submissions of one graph must reach the queues without another graph's submissions between them: execute graphs one at
		 *			a time. Development builds assert when executions overlap.
		 */
		class B3D_EXPORT FrameGraph
		{
		public:
			/**
			 * Creates a graph that records with @p context, and places its transients with the context's transient resource allocator.
			 * @p name names the graph's profiling samples and its command buffer. @p flags are the options of the graph.
			 */
			FrameGraph(GpuWorkContext& context, const StringView& name, FrameGraphFlags flags = FrameGraphFlag::None);

			/** Unregisters the graph's resources and gives back unplaced transients. A compiled graph must have been executed. */
			~FrameGraph();

			FrameGraph(const FrameGraph&) = delete;
			FrameGraph& operator=(const FrameGraph&) = delete;

			/**
			 * Creates a transient texture. Its properties are valid immediately, and it may be placed in parameter sets and attachments, but
			 * it has no memory until Compile(). The texture must be accessed by the GPU only.
			 */
			TShared<Texture> CreateTexture(const StringView& name, const TextureCreateInformation& information);

			/** Creates a transient buffer. See CreateTexture(). */
			TShared<GpuBuffer> CreateBuffer(const StringView& name, const GpuBufferCreateInformation& information);

			/** Registers @p texture, which must be at rest, as a resource of the graph so passes may write it. */
			void ImportTexture(const TShared<Texture>& texture);

			/** Registers @p buffer, which must be at rest, as a resource of the graph so passes may write it. */
			void ImportBuffer(const TShared<GpuBuffer>& buffer);

			/**
			 * Registers every surface of @p target as a resource of the graph, so raster passes can render into it with
			 * FrameGraphRasterPass::SetRenderTarget(). The surfaces of a window rest in their attachment layouts.
			 */
			void ImportRenderTarget(const TShared<RenderTarget>& target);

			/**
			 * Keeps @p texture, a transient of the graph, after the graph ends. Compile() gives it its own memory, and Execute() returns
			 * it to rest. The caller may then use it as an ordinary texture, and import it into later graphs.
			 */
			void ExtractTexture(const TShared<Texture>& texture);

			/** Keeps @p buffer, a transient of the graph, after the graph ends. See ExtractTexture(). */
			void ExtractBuffer(const TShared<GpuBuffer>& buffer);

			/**
			 * Adds a pass that draws into the attachments of @p pass. @p execute records the pass's commands, after Compile(), with the
			 * render pass already open.
			 */
			void AddRasterPass(const StringView& name, FrameGraphRasterPass&& pass, Function<void(FrameGraphPassContext&)> execute);

			/** Adds a pass that dispatches compute work. @p execute records the pass's commands after Compile(). */
			void AddComputePass(const StringView& name, FrameGraphComputePass&& pass, Function<void(FrameGraphPassContext&)> execute);

			/** Adds a pass that copies between the resources of @p pass. @p execute records the pass's commands after Compile(). */
			void AddCopyPass(const StringView& name, FrameGraphCopyPass&& pass, Function<void(FrameGraphPassContext&)> execute);

			/** Culls passes, places transients and plans every barrier. No passes can be added afterwards. */
			void Compile();

			/**
			 * Records the passes and submits them. Imported and extracted resources are at rest once the submission executes, and the
			 * graph's resources are unregistered.
			 */
			void Execute();

			/**
			 * Registers @p callback to run once at the end of Execute(), after the graph's work is submitted. Callbacks run in the order
			 * they were registered. Callbacks of a graph that is never executed are destroyed without running.
			 */
			void AddExecuteCallback(Function<void()> callback);

			/**
			 * Makes the last submission of Execute() signal @p fence with @p value once the graph's work completes on the GPU. Wait for it
			 * with GpuQueue::WaitUntilFence() on the first graphics queue, which the graph's last submission is on.
			 */
			void AddSignalFence(const TShared<GpuTimelineFence>& fence, u64 value);

			/** Returns true if the pass at @p passIndex, in the order passes were added, was culled. Only valid after Compile(). */
			bool IsPassCulled(u32 passIndex) const;

			/** Returns pass and barrier counts. Only valid after Compile(). */
			const FrameGraphStatistics& GetStatistics() const { return mStatistics; }

			/** Returns the work context the graph records with. */
			GpuWorkContext& GetWorkContext() const { return mContext; }

#if B3D_BUILD_TYPE_DEVELOPMENT
			/** Returns true while the calling thread creates a render texture for a raster pass. Development builds only. */
			static bool IsCreatingRenderTexture();
#endif

		private:
			/** Kind of a graph resource. */
			enum class ResourceType : u8
			{
				Transient, /**< Created by the graph, placed by the transient resource allocator. */
				Imported, /**< Existing texture or buffer. */
				Surface /**< Surface of an imported render target that is not a texture, such as a window's. */
			};

			/** Type of a pass. */
			enum class PassType : u8
			{
				Raster,
				Compute,
				Copy
			};

			/** Texture, buffer or render target surface the graph tracks. */
			struct Resource
			{
				/** Returns true if the queues can read the resource at the same time, without transferring it between them. */
				bool IsShared() const;

				/** Returns the resting state of the texture or buffer on @p queueType. Not for surfaces. RestingState is the one of the graphics queue. */
				GpuAccessState GetQueueRestingState(GpuQueueType queueType) const;

				ResourceType Type = ResourceType::Transient;
				TShared<render::Texture> Texture; /**< Set for textures. */
				TShared<GpuBuffer> Buffer; /**< Set for buffers. */
				TShared<RenderTarget> Target; /**< Set for render target surfaces. */
				RenderSurfaceMaskBits Surface = RT_NONE; /**< Surface of Target. */

				GpuTextureSubresourceRange FullRange; /**< Every subresource of a texture or surface. Buffers have one subresource. */
				u32 FirstSubresourceIndex = 0; /**< Index of the resource's first subresource among the subresources of all resources. */
				u32 SubresourceCount = 1;

				TextureCreateInformation TextureInformation; /**< Description of a transient texture. */
				GpuBufferCreateInformation BufferInformation; /**< Description of a transient buffer. */
				bool IsHeldUnplaced = false; /**< True while a transient is held by the allocator without memory. */

				bool IsExtracted = false;
				TOptional<u32> ExportPassIndex; /**< Index into mPasses of the pass the resource is exported after, if any. */
				GpuAccessState RestingState; /**< State the resource returns to after its export, and at the end of the graph if it outlives the graph. */

				// Compile results
				i32 FirstPassIndex = -1; /**< Index into mCompiledPassIndices of the first surviving pass that accesses the resource, or -1 if none does. */
				i32 LastPassIndex = -1; /**< Index into mCompiledPassIndices of the last surviving pass that accesses the resource, or -1 if none does. */
				GpuTransientPlacement Placement; /**< Placement of a transient that is not extracted. */
			};

			/** Access of a pass to one subresource. */
			struct SubresourceAccess
			{
				u32 ResourceIndex = 0; /**< Index into mResources. */
				u32 SubresourceIndex = 0; /**< Index relative to the resource's first subresource. */
				GpuAccessState State; /**< Stages, access and layout the pass needs. */

				/**
				 * True if the pass overwrites the whole subresource without using its earlier contents: an attachment that is cleared or not
				 * loaded. The access flags cannot tell, because blending reads an attachment the pass cleared, and a copy that writes part of
				 * a subresource keeps the rest.
				 */
				bool DiscardsContents = false;
			};

			/** Attachments of a raster pass, and the render target that binds them. */
			struct RenderPassAttachments
			{
				FrameGraphRasterPass::Attachment ColorAttachments[B3D_MAXIMUM_RENDER_TARGET_COUNT];
				FrameGraphRasterPass::Attachment DepthStencilAttachment;
				TShared<RenderTarget> Target; /**< Imported render target, or the render texture the graph built from the attachments. */
				RenderSurfaceMask LoadMask = RT_NONE;
				RenderSurfaceMask ClearMask = RT_NONE;
				RenderSurfaceMask ReadOnlyMask = RT_NONE;
			};

			/** Barriers of one call to GpuCommandBuffer::IssueBarriers(), ReleaseBarriers() or AcquireBarriers(). */
			struct BarrierList
			{
				/** Returns a view of the barriers, which remains valid while the list is unchanged. */
				GpuBarriers GetBarriers() const;

				/** Returns true if the list has no barriers. */
				bool IsEmpty() const { return BufferBarriers.empty() && TextureBarriers.empty() && RenderTargetBarriers.empty(); }

				Vector<GpuBufferBarrier> BufferBarriers;
				Vector<GpuTextureBarrier> TextureBarriers;
				Vector<GpuRenderTargetBarrier> RenderTargetBarriers;
			};

			/** Split barriers released at one point and acquired at another, recorded with one GpuSplitBarrier. */
			struct BarrierBatch
			{
				/** Index into mCompiledPassIndices of the pass the batch is released after, or -1 for the start of submission ReleaseSubmissionIndex. */
				i32 ReleasePassIndex = -1;
				u32 ReleaseSubmissionIndex = 0; /**< Index into mSubmissions of the submission the batch is released at the start of. Only used if ReleasePassIndex is -1. */
				u32 AcquirePassIndex = 0; /**< Index into mCompiledPassIndices of the pass the batch is acquired before, or ~0u for the end of the graph. */
				GpuQueueType ReleaseQueueType = GQT_GRAPHICS; /**< Queue the release is recorded on. */
				GpuQueueType AcquireQueueType = GQT_GRAPHICS; /**< Queue the acquire is recorded on. The batch transfers its resources if it differs from ReleaseQueueType. */
				TShared<GpuSplitBarrier> SplitBarrier;
				BarrierList Barriers;
			};

			/** Command buffer of the graph, submitted to one queue. */
			struct Submission
			{
				GpuQueueType QueueType = GQT_GRAPHICS;
				Vector<u32> PassIndices; /**< Indices into mCompiledPassIndices of the passes the submission records, in execution order. */
				GpuQueueMask SyncMask; /**< Queues the submission waits for. */
				TInlineArray<u32, 2> StartBarrierBatchIndices; /**< Indices into mBarrierBatches of the barrier batches released at the start of the submission. */
				TInlineArray<u32, 2> EndBarrierBatchIndices; /**< Indices into mBarrierBatches of the barrier batches acquired at the end of the graph. Last submission only. */
				BarrierList EndBarriers; /**< Barriers to rest at the end of the graph. Last submission only. */
			};

			/** Pass added to the graph. */
			struct Pass
			{
				String Name;
				PassType Type = PassType::Compute;
				FrameGraphPassFlags Flags; /**< Flags of the pass's description, without those its type does not support. */
				Function<void(FrameGraphPassContext&)> Execute;
				TInlineArray<TShared<GpuParameterSet>, 4> ParameterSets;
				Vector<SubresourceAccess> Accesses; /**< Sorted by resource and subresource, one entry per subresource. */
				TInlineArray<u32, 2> ExportedResourceIndices; /**< Indices into mResources of the resources exported after the pass. */
				RenderPassAttachments Attachments; /**< Only valid for raster passes. */

				// Compile results
				bool IsCulled = false;
				GpuQueueType QueueType = GQT_GRAPHICS; /**< Queue the pass runs on. */
				u32 SubmissionIndex = 0; /**< Index into mSubmissions of the submission that records the pass. */
				BarrierList AliasAcquireBarriers; /**< Full alias acquires, issued first in the pass's prologue. */

				/** Indices into mBarrierBatches of the barrier batches acquired in the pass's prologue, after the alias acquires. */
				TInlineArray<u32, 2> AcquiredBarrierBatchIndices;

				BarrierList Barriers; /**< Full barriers, issued last in the pass's prologue. */
				TInlineArray<u32, 2> ReleasedBarrierBatchIndices; /**< Indices into mBarrierBatches of the barrier batches released after the pass. */
				BarrierList ExportBarriers; /**< Barriers to rest of the resources exported after the pass. */
			};

			/** Registers a new resource of the graph and returns its index. */
			u32 AddResource(Resource&& resource);

			/**
			 * Stores @p resourceIndex on the texture or buffer of @p resource, so the graph finds it from the object. Unregisters it if
			 * @p resourceIndex is ~0u.
			 */
			void RegisterResource(const Resource& resource, u32 resourceIndex) const;

			/** Returns the index of @p texture among the graph's resources, or ~0u if it is not one. */
			u32 FindResource(const Texture& texture) const;

			/** Returns the index of @p buffer among the graph's resources, or ~0u if it is not one. */
			u32 FindResource(const GpuBuffer& buffer) const;

			/** Adds a pass, and declares the accesses of its parameter sets and exports. */
			Pass& AddPass(const StringView& name, PassType type, FrameGraphPass& pass, Function<void(FrameGraphPassContext&)>&& execute);

			/**
			 * Declares an access of @p pass to @p range of the resource at @p resourceIndex. Merges the access with earlier accesses of the
			 * pass to the same subresources. See SubresourceAccess::DiscardsContents for @p discardsContents.
			 */
			void DeclareAccess(Pass& pass, u32 resourceIndex, const GpuTextureSubresourceRange& range, const GpuAccessState& state, bool discardsContents = false);

			/** State the stages of Compile() pass on to each other. */
			struct CompileContext;

			/** Returns the surviving pass at @p compiledIndex in mCompiledPassIndices. */
			Pass& GetCompiledPass(u32 compiledIndex) { return mPasses[mCompiledPassIndices[compiledIndex]]; }

			/** Culls the passes whose writes nothing observes: no later surviving pass, export, or the end of the graph. */
			void CullPasses(const CompileContext& context);

			/** Puts the surviving passes in execution order, and finds the first and last surviving pass of each resource. */
			void OrderPasses();

			/** Picks the queue of each surviving pass, and finds the previous pass on its queue. */
			void ResolveQueues(CompileContext& context);

			/**
			 * Plans the barriers of every subresource, in pass order and at the end of the graph, and the queue waits between passes of
			 * different queues. Alias acquires of transients are completed after placement.
			 */
			void PlanBarriers(CompileContext& context);

			/**
			 * Plans a barrier of one subresource from its current state into @p destination, for accesses on @p queueType before the
			 * compiled pass at @p compiledIndex, or ~0u for the end of the graph. The barrier orders the accesses of @p sourceQueueType
			 * since the state was established, and transfers the subresource between the queues if they differ. If @p isExport is true,
			 * the barrier returns the subresource to rest after the pass instead.
			 */
			void PlanBarrierWithExplicitSourceQueue(CompileContext& context, u32 resourceIndex, u32 subresourceIndex, const GpuAccessState& destination,
				u32 compiledIndex, GpuQueueType queueType, GpuQueueType sourceQueueType, bool discardsContents, bool isExport);

			/**
			 * Plans a barrier of one subresource into @p wanted, for an access on @p queueType before the compiled pass at @p compiledIndex,
			 * or ~0u for the end of the graph. The barrier orders the accesses since the state was established on whichever queue made them.
			 */
			void PlanBarrierWithAutomaticSourceQueue(CompileContext& context, u32 resourceIndex, u32 subresourceIndex, const GpuAccessState& wanted,
				u32 compiledIndex, GpuQueueType queueType, bool discardsContents);

			/** Cuts the passes of each queue into submissions along the queue waits, and puts the submissions in submit order in mTimeline. */
			void BuildSubmissions(CompileContext& context);

			/** Places the transients in the submissions of their passes, and detaches the extracted ones. */
			void PlaceTransients(const CompileContext& context);

			/** Builds or finds the render textures of the raster passes whose attachments were set one by one. */
			void CreateRenderTextures();

			/** Gives the alias acquires of the placed transients their sources and split points, and plans the alias releases. */
			void CompleteAliasAcquires(CompileContext& context);

			/**
			 * Places the barrier halves whose position depends on the submissions: the acquires of exports from the compute queue that no
			 * later access placed, and the releases at the start of the graph.
			 */
			void ResolveDeferredBarrierPoints(CompileContext& context);

			/** Merges the planned barriers into the barrier lists and batches that Execute() records, and adds the waits their transfers need. */
			void BatchBarriers(CompileContext& context);

#if B3D_BUILD_TYPE_DEVELOPMENT
			/** Checks that every split barrier and queue wait is ordered by the submissions. */
			void ValidateSchedule(const CompileContext& context);
#endif

			/** Records the barriers and passes of @p submission on @p commandBuffer, in a profiling sample named @p name. */
			void RecordSubmission(const Submission& submission, GpuCommandBuffer& commandBuffer, const String& name);

			/** Records the barriers of @p list as one call of @p phase. */
			void RecordBarriers(GpuCommandBuffer& commandBuffer, const BarrierList& list, GpuBarrierPhase phase, const TShared<GpuSplitBarrier>& split = nullptr);

			/** Unregisters every resource of the graph, and gives back transients that were never placed. */
			void ReleaseResources();

			GpuWorkContext& mContext;
			GpuDevice& mDevice;
			GpuTransientResourceAllocator& mAllocator;
			const String mName;
			const FrameGraphFlags mFlags;
#if B3D_BUILD_TYPE_DEVELOPMENT
			const u32 mId; /**< Identifies the graph's resources apart from those of other live graphs. */
#endif

			Vector<Resource> mResources;
			Vector<Pass> mPasses;
			Vector<u32> mCompiledPassIndices; /**< Indices into mPasses of the surviving passes, in execution order. */
			Vector<BarrierBatch> mBarrierBatches;
			Vector<Submission> mSubmissions; /**< In submit order, which is the order of mTimeline. The last one is on the graphics queue. */

			/** Index into mResources of the first surface of each imported render target that is not a texture. */
			UnorderedMap<const RenderTarget*, u32> mImportedTargetResourceIndices;

			Vector<Function<void()>> mExecuteCallbacks;
			TInlineArray<GpuTimelineFenceAndValue, 1> mSignalFences;

			GpuSubmissionTimeline mTimeline;
			GpuTransientScope* mScope = nullptr;
			FrameGraphStatistics mStatistics;
			bool mIsCompiled = false;
			bool mIsExecuted = false;
		};

		/** @} */
	} // namespace render
} // namespace b3d
