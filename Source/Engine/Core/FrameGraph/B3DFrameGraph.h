//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuSubmissionTimeline.h"
#include "GpuBackend/B3DGpuTransientResourceAllocator.h"
#include "GpuBackend/B3DRenderTarget.h"
#include "Utility/B3DConfigVariable.h"

namespace b3d
{
	class GpuWorkContext;

#if B3D_BUILD_TYPE_DEVELOPMENT
	/**
	 * If enabled, frame graphs record their passes one at a time, in the order the GPU executes them, and validate every command buffer
	 * of an execution with one shared GpuExplicitBarrierValidator. If disabled, frame graphs validate nothing. Development builds only.
	 */
	extern B3D_EXPORT TConfigVariable<bool> gFrameGraphSerialRecording;
#endif

	namespace render
	{
		class FrameGraph;
		class GpuExplicitBarrierValidator;

		/** @addtogroup Renderer
		 *  @{
		 */

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
			 * Adds @p parameters to the parameter sets of the pass. Sets added here are the only sets the pass may bind. The graph resources
			 * they hold when the pass is added to the graph are what the pass declares. See RendererMaterialBase::CreateGpuParameterSet() for
			 * a helper that creates a set for a material and adds it.
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

			FrameGraphPass() = default;

			TInlineArray<TShared<GpuParameterSet>, 4> mParameterSets;
			TInlineArray<TShared<Texture>, 2> mExportedTextures;
			TInlineArray<TShared<GpuBuffer>, 2> mExportedBuffers;
		};

		/** Describes a pass of a FrameGraph that draws into attachments. The graph opens the render pass before the pass executes. */
		class B3D_EXPORT FrameGraphRasterPass : public FrameGraphPass
		{
		public:
			FrameGraphRasterPass() = default;

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

			/**
			 * Lets draws of the pass bind @p parameters without adding it with AddParameterSet(). The graph declares nothing for the
			 * resources it holds, so they must be at rest and only read, such as the parameter sets of render queue draws. The graph
			 * passes the set to the render pass in RenderPassCreateInformation::Parameters, where backends prepare its descriptors.
			 *
			 * TODO - Remove together with RenderPassCreateInformation::Parameters.
			 */
			void AddRenderPassParameterSet(const TShared<GpuParameterSet>& parameters);

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

			Vector<TShared<GpuParameterSet>> mRenderPassParameterSets;
		};

		/** Describes a pass of a FrameGraph that dispatches compute work. */
		class B3D_EXPORT FrameGraphComputePass : public FrameGraphPass
		{
		public:
			FrameGraphComputePass() = default;
		};

		/** Describes a pass of a FrameGraph that copies between resources with transfer operations, such as GpuCommandBuffer::CopyTexture(). */
		class B3D_EXPORT FrameGraphCopyPass : public FrameGraphPass
		{
		public:
			FrameGraphCopyPass() = default;

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
		};

		/**
		 * Describes the GPU work of a frame, or a part of it, as passes and the resources they read and write, then records it with
		 * explicit barriers. Passes are declared first with AddRasterPass(), AddComputePass() and AddCopyPass(), then Compile() plans the
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
		 * Without explicit mode (gGpuExplicitBarriers), the graph records on an automatic command buffer and plans no barriers:
		 * the resource tracker derives them, and exports do nothing. Every transient that a surviving pass uses gets memory of its own,
		 * because the tracker does not know of memory aliasing. This lets systems move to graphs before explicit mode is turned on.
		 * TODO - Remove automatic mode once every recorder uses explicit barriers.
		 *
		 * @note	Not thread safe. Only the thread that owns the work context may use the graph. Graphs of different work contexts share
		 *			the device's transient heaps, so their GPU work must follow the scope contract of GpuTransientResourceAllocator. Executes
		 *			on the graphics queue, as one submission.
		 */
		class B3D_EXPORT FrameGraph
		{
		public:
			/**
			 * Creates a graph that records with @p context, and places its transients with the context's transient resource allocator.
			 * @p name names the graph's profiling samples and its command buffer.
			 */
			FrameGraph(GpuWorkContext& context, const StringView& name);

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

			/**
			 * Culls passes, places transients and plans every barrier. No passes can be added afterwards. Plans no barriers in automatic
			 * mode.
			 */
			void Compile();

			/**
			 * Records the passes and submits them. Imported and extracted resources are at rest once the submission executes, and the
			 * graph's resources are unregistered.
			 */
			void Execute();

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

			/** Barriers of one call to GpuCommandBuffer::IssueExplicitBarriers(), ReleaseBarriers() or AcquireBarriers(). */
			struct BarrierList
			{
				/** Returns a view of the barriers, which remains valid while the list is unchanged. */
				GpuExplicitBarriers GetBarriers() const;

				/** Returns true if the list has no barriers. */
				bool IsEmpty() const { return BufferBarriers.empty() && TextureBarriers.empty() && RenderTargetBarriers.empty(); }

				Vector<GpuExplicitBufferBarrier> BufferBarriers;
				Vector<GpuExplicitTextureBarrier> TextureBarriers;
				Vector<GpuExplicitRenderTargetBarrier> RenderTargetBarriers;
			};

			/** Split barriers released at one point and acquired at another, recorded with one GpuSplitBarrier. */
			struct BarrierBatch
			{
				i32 ReleasePassIndex = -1; /**< Index into mCompiledPassIndices of the pass the batch is released after, or -1 for the start of the graph. */
				u32 AcquirePassIndex = 0; /**< Index into mCompiledPassIndices of the pass the batch is acquired before. */
				TShared<GpuSplitBarrier> SplitBarrier;
				BarrierList Barriers;
			};

			/** Pass added to the graph. */
			struct Pass
			{
				String Name;
				PassType Type = PassType::Compute;
				Function<void(FrameGraphPassContext&)> Execute;
				TInlineArray<TShared<GpuParameterSet>, 4> ParameterSets;
				Vector<SubresourceAccess> Accesses; /**< Sorted by resource and subresource, one entry per subresource. */
				TInlineArray<u32, 2> ExportedResourceIndices; /**< Indices into mResources of the resources exported after the pass. */
				RenderPassAttachments Attachments; /**< Only valid for raster passes. */
				Vector<TShared<GpuParameterSet>> RenderPassParameterSets; /**< See FrameGraphRasterPass::AddRenderPassParameterSet(). */

				// Compile results
				bool IsCulled = false;
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

			/** Records the barriers of @p list as one call of @p phase. */
			void RecordBarriers(GpuCommandBuffer& commandBuffer, const BarrierList& list, GpuBarrierPhase phase, const TShared<GpuSplitBarrier>& split = nullptr);

			/** Unregisters every resource of the graph, and gives back transients that were never placed. */
			void ReleaseResources();

			GpuWorkContext& mContext;
			GpuDevice& mDevice;
			GpuTransientResourceAllocator& mAllocator;
			const String mName;
			const bool mExplicitBarriers; /**< True in explicit mode, false in automatic mode. See the class description. */
#if B3D_BUILD_TYPE_DEVELOPMENT
			const u32 mId; /**< Identifies the graph's resources apart from those of other live graphs. */
#endif

			Vector<Resource> mResources;
			Vector<Pass> mPasses;
			Vector<u32> mCompiledPassIndices; /**< Indices into mPasses of the surviving passes, in execution order. */
			Vector<BarrierBatch> mBarrierBatches;
			TInlineArray<u32, 2> mStartBarrierBatchIndices; /**< Indices into mBarrierBatches of the barrier batches released at the start of the graph. */
			BarrierList mEndBarriers; /**< Barriers to rest at the end of the graph. */

			/** Index into mResources of the first surface of each imported render target that is not a texture. */
			UnorderedMap<const RenderTarget*, u32> mImportedTargetResourceIndices;

			GpuSubmissionTimeline mTimeline;
			GpuTransientScope* mScope = nullptr;
			FrameGraphStatistics mStatistics;
			bool mIsCompiled = false;
			bool mIsExecuted = false;
		};

		/** @} */
	} // namespace render
} // namespace b3d
