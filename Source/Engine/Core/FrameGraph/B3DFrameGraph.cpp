//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "FrameGraph/B3DFrameGraph.h"

#include "B3DApplication.h"
#include "GpuBackend/Allocators/B3DGpuTransientHeapPool.h"
#include "GpuBackend/B3DGpuBackendUtility.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/B3DGpuDeviceCapabilities.h"
#include "GpuBackend/B3DGpuFramebuffer.h"
#include "GpuBackend/B3DGpuBarrierValidator.h"
#include "GpuBackend/B3DGpuParameterSet.h"
#include "GpuBackend/B3DGpuWorkContext.h"
#include "GpuBackend/B3DRenderTexture.h"
#include "GpuBackend/B3DRenderWindow.h"
#include "Profiling/B3DProfilerCPU.h"
#include "Profiling/B3DProfilerGPU.h"

namespace b3d
{
	TConfigVariable<bool> gFrameGraphAsyncCompute("gpu.FrameGraphAsyncCompute",
		"If enabled, compute passes that prefer the async compute queue run on it. If disabled, every pass runs on the graphics queue.", true);

#if B3D_BUILD_TYPE_DEVELOPMENT
	TConfigVariable<bool> gFrameGraphSerialRecording("gpu.FrameGraphSerialRecording",
		"If enabled, frame graphs that record in parallel record their passes one at a time instead, like other frame graphs, which validates their barriers.", false);
#endif
} // namespace b3d

namespace b3d::render
{
	namespace
	{
#if B3D_BUILD_TYPE_DEVELOPMENT
		/** Source of frame graph ids. Zero is never used, as it marks objects that belong to no graph. */
		std::atomic<u32> gNextFrameGraphId = 1;

		/** True while a frame graph creates a render texture for a raster pass on the calling thread. */
		thread_local bool gIsCreatingRenderTexture = false;

		/** Id of the frame graph that is executing, or zero if none is. */
		std::atomic<u32> gExecutingFrameGraphId = 0;

		/** True while a task of a frame graph that records in parallel records a submission on the calling thread. */
		thread_local bool gIsRecordingSubmission = false;

		/** Returns a new frame graph id. */
		u32 CreateFrameGraphId()
		{
			u32 id = gNextFrameGraphId.fetch_add(1, std::memory_order_relaxed);
			while(id == 0)
				id = gNextFrameGraphId.fetch_add(1, std::memory_order_relaxed);

			return id;
		}
#endif

		/** Command buffer pools and render textures that the frame graphs of one work context share. */
		class FrameGraphContextData : public IGpuWorkContextLocal
		{
		public:
			/** Render texture built from the attachments of a raster pass. */
			struct RenderTextureEntry
			{
				TShared<RenderTexture> Target;
				RenderTextureCreateInformation Information;

				/** Native resource of each surface when the render texture was built. A recreated surface needs a new render texture. */
				IGpuResource* GpuResources[B3D_MAXIMUM_RENDER_TARGET_COUNT + 1] = {};
				u64 LastUsedFrame = 0;
			};

			/**
			 * Pools of the command buffers of each queue type. Graphs record on the first pool of the queue, and graphs that record in
			 * parallel on a pool per submission of the queue, as a pool records one command buffer at a time.
			 */
			Vector<TShared<GpuCommandBufferPool>> CommandBufferPools[GQT_COUNT];
			Vector<RenderTextureEntry> RenderTextures;
		};

		/** Key the context data is registered under. */
		const char kFrameGraphContextDataKey = 0;

		/** Returns the frame graph data of @p context, creating it on first use. */
		FrameGraphContextData& GetContextData(GpuWorkContext& context)
		{
			return context.GetOrCreateLocal<FrameGraphContextData>(&kFrameGraphContextDataKey, []() { return B3DMakeUnique<FrameGraphContextData>(); });
		}

		/** Returns the other one of the graphics and the compute queue. */
		GpuQueueType GetOtherQueueType(GpuQueueType queueType)
		{
			return queueType == GQT_GRAPHICS ? GQT_COMPUTE : GQT_GRAPHICS;
		}

		/** Returns true if @p a and @p b describe the same surface of the same texture. */
		bool IsSameSurface(const RenderSurfaceInformation& a, const RenderSurfaceInformation& b)
		{
			return a.Texture == b.Texture && a.Face == b.Face && a.FaceCount == b.FaceCount && a.MipLevel == b.MipLevel;
		}

		/** Returns the native resource of the texture of @p surface, or null if there is none. */
		IGpuResource* GetSurfaceGpuResource(const RenderSurfaceInformation& surface)
		{
			return surface.Texture != nullptr ? surface.Texture->GetGpuResource() : nullptr;
		}

		/** Returns the stages, access and layout of a color attachment. */
		GpuAccessState GetColorAttachmentState()
		{
			return GpuAccessState(GpuStageFlag::ColorAttachment, GpuAccessFlag::Read | GpuAccessFlag::Write, GpuImageLayout::ColorAttachment);
		}

		/** Adds the surfaces of an attachment loaded with @p load to the masks of a render pass. */
		void AddLoadMask(FrameGraphAttachmentLoad load, RenderSurfaceMask surfaces, RenderSurfaceMask& loadMask, RenderSurfaceMask& clearMask)
		{
			if(load == FrameGraphAttachmentLoad::Load)
				loadMask |= surfaces;
			else if(load == FrameGraphAttachmentLoad::Clear)
				clearMask |= surfaces;
		}
	} // namespace

	void FrameGraphPass::AddParameterSet(const TShared<GpuParameterSet>& parameters)
	{
		if(parameters != nullptr)
			mParameterSets.Add(parameters);
	}

	void FrameGraphPass::Export(const TShared<Texture>& texture)
	{
		if(texture != nullptr)
			mExportedTextures.Add(texture);
	}

	void FrameGraphPass::Export(const TShared<GpuBuffer>& buffer)
	{
		if(buffer != nullptr)
			mExportedBuffers.Add(buffer);
	}

	void FrameGraphRasterPass::SetColorAttachment(u32 index, const TShared<Texture>& texture, FrameGraphAttachmentLoad load, u32 mipLevel, u32 face, u32 faceCount)
	{
		if(!B3D_ENSURE_LOG(index < B3D_MAXIMUM_RENDER_TARGET_COUNT, "Color attachment index {0} is out of range.", index))
			return;

		Attachment& attachment = mColorAttachments[index];
		attachment.Texture = texture;
		attachment.Load = load;
		attachment.MipLevel = mipLevel;
		attachment.Face = face;
		attachment.FaceCount = faceCount;
	}

	void FrameGraphRasterPass::SetDepthStencilAttachment(const TShared<Texture>& texture, FrameGraphDepthStencilMode mode, FrameGraphAttachmentLoad load, u32 mipLevel, u32 face, u32 faceCount)
	{
		mDepthStencilAttachment.Texture = texture;
		mDepthStencilAttachment.Load = load;
		mDepthStencilAttachment.MipLevel = mipLevel;
		mDepthStencilAttachment.Face = face;
		mDepthStencilAttachment.FaceCount = faceCount;
		mDepthStencilMode = mode;
	}

	void FrameGraphRasterPass::SetRenderTarget(const TShared<RenderTarget>& target, FrameGraphAttachmentLoad load)
	{
		mRenderTarget = target;
		mRenderTargetLoad = load;
	}

	void FrameGraphCopyPass::AddSource(const TShared<Texture>& texture, const TextureSurface& surface)
	{
		CopyResource resource;
		resource.Texture = texture;
		resource.Surface = surface;

		mResources.Add(resource);
	}

	void FrameGraphCopyPass::AddSource(const TShared<GpuBuffer>& buffer)
	{
		CopyResource resource;
		resource.Buffer = buffer;

		mResources.Add(resource);
	}

	void FrameGraphCopyPass::AddDestination(const TShared<Texture>& texture, const TextureSurface& surface)
	{
		CopyResource resource;
		resource.Texture = texture;
		resource.Surface = surface;
		resource.IsDestination = true;

		mResources.Add(resource);
	}

	void FrameGraphCopyPass::AddDestination(const TShared<GpuBuffer>& buffer)
	{
		CopyResource resource;
		resource.Buffer = buffer;
		resource.IsDestination = true;

		mResources.Add(resource);
	}

	GpuBarriers FrameGraph::BarrierList::GetBarriers() const
	{
		return GpuBarriers(TArrayView<const GpuBufferBarrier>(BufferBarriers.data(), (u32)BufferBarriers.size()),
			TArrayView<const GpuTextureBarrier>(TextureBarriers.data(), (u32)TextureBarriers.size()),
			TArrayView<const GpuRenderTargetBarrier>(RenderTargetBarriers.data(), (u32)RenderTargetBarriers.size()));
	}

	FrameGraph::FrameGraph(GpuWorkContext& context, const StringView& name, FrameGraphFlags flags)
		: mContext(context), mDevice(context.GetDevice()), mAllocator(context.GetTransientResourceAllocator()), mName(name), mFlags(flags)
#if B3D_BUILD_TYPE_DEVELOPMENT
		, mId(CreateFrameGraphId())
#endif
	{ }

	FrameGraph::~FrameGraph()
	{
		if(mIsExecuted)
			return;

		B3D_ENSURE_LOG(!mIsCompiled, "A compiled frame graph was destroyed without being executed.");
		if(mScope != nullptr)
		{
			mAllocator.EndScope();
			mScope = nullptr;
		}

		ReleaseResources();
	}

	TShared<Texture> FrameGraph::CreateTexture(const StringView& name, const TextureCreateInformation& information)
	{
		B3D_ASSERT(!mIsCompiled && "Resources cannot be created after Compile().");

		TextureCreateInformation namedInformation = information;
		namedInformation.Name = name;

		const TShared<Texture> texture = mAllocator.GetOrCreateTexture(namedInformation);
		if(texture == nullptr)
			return nullptr;

		const TextureProperties& properties = texture->GetProperties();

		Resource resource;
		resource.Type = ResourceType::Transient;
		resource.Texture = texture;
		resource.TextureInformation = namedInformation;
		resource.FullRange = GpuTextureSubresourceRange(0, properties.MipMapCount + 1, 0, properties.GetFaceCount(), GpuBackendUtility::GetFormatAspects(properties.Format));
		resource.RestingState = GpuBackendUtility::GetRestingState(*texture, GQT_GRAPHICS);
		resource.IsHeldUnplaced = true;

		AddResource(std::move(resource));

		return texture;
	}

	TShared<GpuBuffer> FrameGraph::CreateBuffer(const StringView& name, const GpuBufferCreateInformation& information)
	{
		B3D_ASSERT(!mIsCompiled && "Resources cannot be created after Compile().");

		const TShared<GpuBuffer> buffer = mAllocator.GetOrCreateBuffer(information);
		if(buffer == nullptr)
			return nullptr;

		buffer->SetName(name);

		Resource resource;
		resource.Type = ResourceType::Transient;
		resource.Buffer = buffer;
		resource.BufferInformation = information;
		resource.RestingState = GpuBackendUtility::GetRestingState(*buffer, GQT_GRAPHICS);
		resource.IsHeldUnplaced = true;

		AddResource(std::move(resource));

		return buffer;
	}

	void FrameGraph::ImportTexture(const TShared<Texture>& texture)
	{
		B3D_ASSERT(!mIsCompiled && "Resources cannot be imported after Compile().");
		if(texture == nullptr)
			return;

		// A texture the graph already has needs no second import
		if(texture->GetFrameGraphResourceIndex() != ~0u)
		{
#if B3D_BUILD_TYPE_DEVELOPMENT
			B3D_ENSURE_LOG(texture->GetFrameGraphId() == mId, "Texture '{0}' is already a resource of another frame graph.", texture->GetName());
#endif
			return;
		}

		// The graph takes imports to be at rest, and a texture initialized while it is a graph resource does not start at rest (See Texture::StartsAtRest())
		if(!B3D_ENSURE_LOG(texture->IsInitialized(), "Texture '{0}' must be initialized before it is imported into a frame graph.", texture->GetName()))
			return;

		const TextureProperties& properties = texture->GetProperties();

		Resource resource;
		resource.Type = ResourceType::Imported;
		resource.Texture = texture;
		resource.FullRange = GpuTextureSubresourceRange(0, properties.MipMapCount + 1, 0, properties.GetFaceCount(), GpuBackendUtility::GetFormatAspects(properties.Format));
		resource.RestingState = GpuBackendUtility::GetRestingState(*texture, GQT_GRAPHICS);

		AddResource(std::move(resource));
	}

	void FrameGraph::ImportBuffer(const TShared<GpuBuffer>& buffer)
	{
		B3D_ASSERT(!mIsCompiled && "Resources cannot be imported after Compile().");
		if(buffer == nullptr)
			return;

		// A buffer the graph already has needs no second import
		if(buffer->GetFrameGraphResourceIndex() != ~0u)
		{
#if B3D_BUILD_TYPE_DEVELOPMENT
			B3D_ENSURE_LOG(buffer->GetFrameGraphId() == mId, "Buffer '{0}' is already a resource of another frame graph.", buffer->GetName());
#endif
			return;
		}

		Resource resource;
		resource.Type = ResourceType::Imported;
		resource.Buffer = buffer;
		resource.RestingState = GpuBackendUtility::GetRestingState(*buffer, GQT_GRAPHICS);

		AddResource(std::move(resource));
	}

	void FrameGraph::ImportRenderTarget(const TShared<RenderTarget>& target)
	{
		B3D_ASSERT(!mIsCompiled && "Resources cannot be imported after Compile().");
		if(target == nullptr)
			return;

		if(!target->GetProperties().IsWindow)
		{
			// The surfaces of a render texture are ordinary textures
			const RenderTexture& renderTexture = static_cast<const RenderTexture&>(*target);
			for(u32 colorIndex = 0; colorIndex < B3D_MAXIMUM_RENDER_TARGET_COUNT; colorIndex++)
				ImportTexture(renderTexture.GetColorSurfaceInformation(colorIndex).Texture);

			ImportTexture(renderTexture.GetDepthStencilSurfaceInformation().Texture);
			return;
		}

		if(mImportedTargetResourceIndices.find(target.get()) != mImportedTargetResourceIndices.end())
			return;

		// A window's surfaces cannot be accessed as textures, so the graph tracks them through the window
		Resource colorSurface;
		colorSurface.Type = ResourceType::Surface;
		colorSurface.Target = target;
		colorSurface.Surface = RT_COLOR0;
		colorSurface.FullRange = GpuTextureSubresourceRange(0, 1, 0, 1, GpuTextureAspectFlag::Color);
		colorSurface.RestingState = GpuBackendUtility::GetRestingState(*target, RT_COLOR0, GQT_GRAPHICS);

		mImportedTargetResourceIndices[target.get()] = AddResource(std::move(colorSurface));

		if(static_cast<const RenderWindow&>(*target).HasDepthBuffer())
		{
			Resource depthStencilSurface;
			depthStencilSurface.Type = ResourceType::Surface;
			depthStencilSurface.Target = target;
			depthStencilSurface.Surface = RT_DEPTH;
			depthStencilSurface.FullRange = GpuTextureSubresourceRange(0, 1, 0, 1, GpuTextureAspectFlag::Depth | GpuTextureAspectFlag::Stencil);
			depthStencilSurface.RestingState = GpuBackendUtility::GetRestingState(*target, RT_DEPTH, GQT_GRAPHICS);

			AddResource(std::move(depthStencilSurface));
		}
	}

	void FrameGraph::ExtractTexture(const TShared<Texture>& texture)
	{
		B3D_ASSERT(!mIsCompiled && "Resources cannot be extracted after Compile().");

		const u32 resourceIndex = texture != nullptr ? FindResource(*texture) : ~0u;
		if(!B3D_ENSURE_LOG(resourceIndex != ~0u && mResources[resourceIndex].Type == ResourceType::Transient, "Only a transient of the graph can be extracted."))
			return;

		mResources[resourceIndex].IsExtracted = true;
	}

	void FrameGraph::ExtractBuffer(const TShared<GpuBuffer>& buffer)
	{
		B3D_ASSERT(!mIsCompiled && "Resources cannot be extracted after Compile().");

		const u32 resourceIndex = buffer != nullptr ? FindResource(*buffer) : ~0u;
		if(!B3D_ENSURE_LOG(resourceIndex != ~0u && mResources[resourceIndex].Type == ResourceType::Transient, "Only a transient of the graph can be extracted."))
			return;

		mResources[resourceIndex].IsExtracted = true;
	}

	void FrameGraph::AddRasterPass(const StringView& name, FrameGraphRasterPass&& pass, Function<void(FrameGraphPassContext&)> execute)
	{
		Pass& output = AddPass(name, PassType::Raster, pass, std::move(execute));
		RenderPassAttachments& attachments = output.Attachments;

		// States of the depth and stencil aspects of the depth-stencil attachment
		const GpuStageFlags depthStencilStages = GpuStageFlag::EarlyFragmentTests | GpuStageFlag::LateFragmentTests;
		GpuAccessState depthState(depthStencilStages, GpuAccessFlag::Read | GpuAccessFlag::Write, GpuImageLayout::DepthStencilAttachment);
		GpuAccessState stencilState = depthState;
		if(pass.mDepthStencilMode == FrameGraphDepthStencilMode::ReadOnly)
		{
			depthState = GpuAccessState(depthStencilStages, GpuAccessFlag::Read, GpuImageLayout::DepthStencilReadOnly);
			stencilState = depthState;
			attachments.ReadOnlyMask = RT_DEPTH_STENCIL;
		}
		else if(pass.mDepthStencilMode == FrameGraphDepthStencilMode::DepthReadOnly)
		{
			depthState = GpuAccessState(depthStencilStages, GpuAccessFlag::Read, GpuImageLayout::DepthReadOnlyStencilAttachment);
			stencilState = GpuAccessState(depthStencilStages, GpuAccessFlag::Read | GpuAccessFlag::Write, GpuImageLayout::DepthReadOnlyStencilAttachment);
			attachments.ReadOnlyMask = RT_DEPTH;
		}

		// Declares the access of a color attachment to @p range
		const auto fnDeclareColorAttachmentAccess = [this, &output](u32 resourceIndex, const GpuTextureSubresourceRange& range, bool discardsContents)
		{
			DeclareAccess(output, resourceIndex, range, GetColorAttachmentState(), discardsContents);
		};

		// Declares the access of a depth-stencil attachment to @p range. The depth and stencil aspects each get their own state.
		const auto fnDeclareDepthStencilAttachmentAccess = [this, &output, &depthState, &stencilState](u32 resourceIndex, const GpuTextureSubresourceRange& range, bool discardsContents)
		{
			GpuTextureSubresourceRange depthRange = range;
			depthRange.AspectMask = range.AspectMask & GpuTextureAspectFlag::Depth;
			if(depthRange.AspectMask != GpuTextureAspectFlags())
				DeclareAccess(output, resourceIndex, depthRange, depthState, discardsContents);

			GpuTextureSubresourceRange stencilRange = range;
			stencilRange.AspectMask = range.AspectMask & GpuTextureAspectFlag::Stencil;
			if(stencilRange.AspectMask != GpuTextureAspectFlags())
				DeclareAccess(output, resourceIndex, stencilRange, stencilState, discardsContents);
		};

		// Declares the subresources of an attachment texture with @p fnDeclareAccess, and adds its surfaces to the load and clear masks
		const auto fnDeclareTexture = [this, &output, &attachments](const FrameGraphRasterPass::Attachment& attachment, RenderSurfaceMask surfaces, const auto& fnDeclareAccess)
		{
			if(attachment.Texture == nullptr)
				return;

			const u32 resourceIndex = FindResource(*attachment.Texture);
			if(!B3D_ENSURE_LOG(resourceIndex != ~0u, "Pass '{0}' renders into '{1}', which is not a resource of the graph. Import it first.", output.Name, attachment.Texture->GetName()))
				return;

			const Resource& resource = mResources[resourceIndex];
			const u32 faceCount = attachment.FaceCount != 0 ? attachment.FaceCount : resource.FullRange.ArrayLayerCount - attachment.Face;
			const GpuTextureSubresourceRange range(attachment.MipLevel, 1, attachment.Face, faceCount, resource.FullRange.AspectMask);

			fnDeclareAccess(resourceIndex, range, attachment.Load != FrameGraphAttachmentLoad::Load);
			AddLoadMask(attachment.Load, surfaces, attachments.LoadMask, attachments.ClearMask);
		};

		// Attachments set one by one are rendered through a render texture that Compile() builds from them
		if(pass.mRenderTarget == nullptr)
		{
			for(u32 colorIndex = 0; colorIndex < B3D_MAXIMUM_RENDER_TARGET_COUNT; colorIndex++)
			{
				attachments.ColorAttachments[colorIndex] = pass.mColorAttachments[colorIndex];
				fnDeclareTexture(pass.mColorAttachments[colorIndex], (RenderSurfaceMaskBits)(RT_COLOR0 << colorIndex), fnDeclareColorAttachmentAccess);
			}

			attachments.DepthStencilAttachment = pass.mDepthStencilAttachment;
			fnDeclareTexture(pass.mDepthStencilAttachment, RT_DEPTH_STENCIL, fnDeclareDepthStencilAttachmentAccess);
			return;
		}

		const TShared<RenderTarget>& target = pass.mRenderTarget;
		const FrameGraphAttachmentLoad load = pass.mRenderTargetLoad;
		attachments.Target = target;

		// The surfaces of a render texture are textures of the graph
		if(!target->GetProperties().IsWindow)
		{
			const RenderTexture& renderTexture = static_cast<const RenderTexture&>(*target);
			const auto fnDeclareSurface = [&fnDeclareTexture, load](const RenderSurfaceInformation& surface, RenderSurfaceMask surfaces, const auto& fnDeclareAccess)
			{
				FrameGraphRasterPass::Attachment attachment;
				attachment.Texture = surface.Texture;
				attachment.Load = load;
				attachment.MipLevel = surface.MipLevel;
				attachment.Face = surface.Face;
				attachment.FaceCount = surface.FaceCount;

				fnDeclareTexture(attachment, surfaces, fnDeclareAccess);
			};

			for(u32 colorIndex = 0; colorIndex < B3D_MAXIMUM_RENDER_TARGET_COUNT; colorIndex++)
				fnDeclareSurface(renderTexture.GetColorSurfaceInformation(colorIndex), (RenderSurfaceMaskBits)(RT_COLOR0 << colorIndex), fnDeclareColorAttachmentAccess);

			fnDeclareSurface(renderTexture.GetDepthStencilSurfaceInformation(), RT_DEPTH_STENCIL, fnDeclareDepthStencilAttachmentAccess);
			return;
		}

		// The surfaces of a window are resources of their own, the depth-stencil surface right after the color surface
		const auto found = mImportedTargetResourceIndices.find(target.get());
		if(!B3D_ENSURE_LOG(found != mImportedTargetResourceIndices.end(), "Pass '{0}' renders into a window that is not imported. Import it with ImportRenderTarget() first.", output.Name))
			return;

		const bool discardsContents = load != FrameGraphAttachmentLoad::Load;
		const u32 colorResourceIndex = found->second;
		fnDeclareColorAttachmentAccess(colorResourceIndex, mResources[colorResourceIndex].FullRange, discardsContents);
		AddLoadMask(load, RT_COLOR0, attachments.LoadMask, attachments.ClearMask);

		const u32 depthStencilResourceIndex = colorResourceIndex + 1;
		if(depthStencilResourceIndex < (u32)mResources.size() && mResources[depthStencilResourceIndex].Target == target)
		{
			fnDeclareDepthStencilAttachmentAccess(depthStencilResourceIndex, mResources[depthStencilResourceIndex].FullRange, discardsContents);
			AddLoadMask(load, RT_DEPTH_STENCIL, attachments.LoadMask, attachments.ClearMask);
		}
	}

	void FrameGraph::AddComputePass(const StringView& name, FrameGraphComputePass&& pass, Function<void(FrameGraphPassContext&)> execute)
	{
		AddPass(name, PassType::Compute, pass, std::move(execute));
	}

	void FrameGraph::AddCopyPass(const StringView& name, FrameGraphCopyPass&& pass, Function<void(FrameGraphPassContext&)> execute)
	{
		Pass& output = AddPass(name, PassType::Copy, pass, std::move(execute));
		for(const FrameGraphCopyPass::CopyResource& copyResource : pass.mResources)
		{
			const u32 resourceIndex = copyResource.Texture != nullptr ? FindResource(*copyResource.Texture) : copyResource.Buffer != nullptr ? FindResource(*copyResource.Buffer) : ~0u;

			// A copy accesses the resource outside of its resting state, so it must be declared
			if(!B3D_ENSURE_LOG(resourceIndex != ~0u, "Pass '{0}' copies a resource that is not a resource of the graph. Import it first.", output.Name))
				continue;

			// A copy may write only part of a subresource, so the rest of its contents must be preserved
			const GpuAccessState state = copyResource.IsDestination ? GpuAccessState(GpuStageFlag::Transfer, GpuAccessFlag::Write, GpuImageLayout::TransferDestination)
				: GpuAccessState(GpuStageFlag::Transfer, GpuAccessFlag::Read, GpuImageLayout::TransferSource);

			const GpuTextureSubresourceRange range = copyResource.Texture != nullptr ? GpuBackendUtility::GetSurfaceRange(mResources[resourceIndex].FullRange, copyResource.Surface) : GpuTextureSubresourceRange();
			DeclareAccess(output, resourceIndex, range, state);
		}
	}

	/** State the stages of FrameGraph::Compile() pass on to each other. Lives in the frame allocator scope of Compile(). */
	struct FrameGraph::CompileContext
	{
		/** Barrier of one subresource, or of a whole resource, as the planner produces it. */
		struct PlannedBarrier
		{
			u32 ResourceIndex = 0; /**< Index into mResources. */
			u32 SubresourceIndex = 0; /**< Index relative to the resource's first subresource. Ignored if IsWholeResource is true. */
			bool IsWholeResource = false;
			GpuAccessState Source;
			GpuAccessState Destination;
			GpuBarrierFlags Flags;

			/**
			 * Index into mCompiledPassIndices of the pass the release goes after, or -1 for the start of submission ReleaseSubmissionIndex.
			 * AcquirePassIndex - 1 for full barriers.
			 */
			i32 ReleasePassIndex = -1;

			/** Index into mSubmissions of the submission a release at -1 starts, or ~0u for the start of the graph: the first graphics submission. */
			u32 ReleaseSubmissionIndex = ~0u;

			u32 AcquirePassIndex = 0; /**< Index into mCompiledPassIndices of the pass the barrier precedes, or ~0u for the end of the graph. */
			GpuQueueType ReleaseQueueType = GQT_GRAPHICS; /**< Queue of the release, or of the whole barrier if it is full. */
			GpuQueueType AcquireQueueType = GQT_GRAPHICS; /**< Queue of the acquire, or of the whole barrier if it is full. */
			bool IsSplit = false;
			bool IsExport = false; /**< True for a barrier to rest after the pass at ReleasePassIndex. */
		};

		/** Planning state of one subresource. */
		struct SubresourceState
		{
			bool HasState = false; /**< False until a transient's first access. */

			/**
			 * State the last barrier established, its destination, or the resting state of an import. Later accesses join it while they fit.
			 * Its queues include the other queue if it reads the subresource in it.
			 */
			GpuAccessState EstablishedState;

			GpuQueueType EstablishedQueueType = GQT_GRAPHICS; /**< Queue of the barrier that established EstablishedState. Graphics for the resting state. */
			i32 EstablishedPassIndex = -1; /**< Index into mCompiledPassIndices of the pass the barrier that established EstablishedState precedes, or -1 if none does. */
			GpuStageFlags StagesSinceEstablished[GQT_COUNT]; /**< Stages in which each queue accessed the subresource since EstablishedState was established. */
			GpuAccessFlags AccessSinceEstablished[GQT_COUNT]; /**< Reads and writes of each queue since EstablishedState was established. */

			/**
			 * Index into mCompiledPassIndices of the last pass of each queue that accessed the subresource or acquired its state, or -1 for the
			 * start of the graph.
			 */
			i32 LastPassIndices[GQT_COUNT] = { -1, -1, -1 };

			u32 BarrierIndex = ~0u; /**< Index into PlannedBarriers of the barrier that established EstablishedState, or ~0u if none did. */

			/** Index into PlannedBarriers of the split barrier that returns the subresource from an export on the compute queue, while its acquire is not placed. */
			u32 PendingComputeExportBarrierIndex = ~0u;
		};

		u32 SubresourceCount = 0; /**< Number of subresources of all resources. See Resource::FirstSubresourceIndex. */
		bool UsesComputeQueue = false; /**< True if a surviving pass runs on the compute queue. */

		/** Index into mCompiledPassIndices of the previous pass on the queue of each surviving pass, or -1 for the first pass of a queue. */
		FrameVector<i32> PreviousQueuePassIndices;

		FrameVector<SubresourceState> SubresourceStates; /**< Planning state of each subresource of all resources. */
		FrameVector<PlannedBarrier> PlannedBarriers;

		/** Queue waits: a pass, or -1 for the start of the graph on the graphics queue, and a pass of the other queue that must wait for it. */
		FrameVector<std::pair<i32, u32>> WaitEdges;

		FrameVector<u32> FirstAccessBarrierIndices; /**< Indices into PlannedBarriers of the barriers that start the lifetimes of the transients, in pass order. */
		FrameVector<u32> ComputeExportBarrierIndices; /**< Indices into PlannedBarriers of the split barriers that return exports from the compute queue to rest. */

		u32 FirstGraphicsSubmissionIndex = 0; /**< Index into mSubmissions of the first submission on the graphics queue. */
		u32 LastSubmissionIndex = 0; /**< Index into mSubmissions of the last submission, which is on the graphics queue. */
	};

	bool FrameGraph::Resource::IsShared() const
	{
		return Buffer != nullptr || (Texture != nullptr && GpuBackendUtility::IsConcurrentlyReadable(Texture->GetProperties().Usage));
	}

	GpuAccessState FrameGraph::Resource::GetQueueRestingState(GpuQueueType queueType) const
	{
		return Texture != nullptr ? GpuBackendUtility::GetRestingState(*Texture, queueType) : GpuBackendUtility::GetRestingState(*Buffer, queueType);
	}

	void FrameGraph::Compile()
	{
		B3D_ASSERT(!mIsCompiled && "A frame graph can only be compiled once.");
		mIsCompiled = true;

		const CpuProfilingScope compileSample("FrameGraph::Compile");

		FrameAllocatorScope frameScope;

		mStatistics = FrameGraphStatistics();
		mStatistics.PassCount = (u32)mPasses.size();

		// The subresources of all resources are numbered one after another, so per-subresource state lives in flat arrays
		CompileContext context;
		for(Resource& resource : mResources)
		{
			resource.FirstSubresourceIndex = context.SubresourceCount;
			context.SubresourceCount += resource.SubresourceCount;

			if(resource.ExportPassIndex.has_value())
				B3D_ENSURE_LOG(resource.Type != ResourceType::Transient || resource.IsExtracted, "Only imported and extracted resources can be exported.");
		}

		CullPasses(context);
		OrderPasses();
		ResolveQueues(context);
		PlanBarriers(context);
		BuildSubmissions(context);
		PlaceTransients(context);
		CreateRenderTextures();
		CompleteAliasAcquires(context);
		ResolveDeferredBarrierPoints(context);
		BatchBarriers(context);

#if B3D_BUILD_TYPE_DEVELOPMENT
		ValidateSchedule(context);
#endif
	}

	void FrameGraph::CullPasses(const CompileContext& context)
	{
		// Walking backwards, a pass survives if it writes contents that a later surviving pass or the end of the graph observes.
		// The contents of imported and extracted resources are observed after the graph ends, and exported contents after their pass.
		FrameVector<bool> isObserved(context.SubresourceCount, false);
		for(const Resource& resource : mResources)
		{
			if(resource.Type != ResourceType::Transient || resource.IsExtracted)
				std::fill(isObserved.begin() + resource.FirstSubresourceIndex, isObserved.begin() + resource.FirstSubresourceIndex + resource.SubresourceCount, true);
		}

		for(u32 passIndex = (u32)mPasses.size(); passIndex-- > 0;)
		{
			Pass& pass = mPasses[passIndex];
			for(u32 resourceIndex : pass.ExportedResourceIndices)
			{
				const Resource& resource = mResources[resourceIndex];
				std::fill(isObserved.begin() + resource.FirstSubresourceIndex, isObserved.begin() + resource.FirstSubresourceIndex + resource.SubresourceCount, true);
			}

			bool isAlive = false;
			for(const SubresourceAccess& access : pass.Accesses)
			{
				if(access.State.Access.IsSet(GpuAccessFlag::Write) && isObserved[mResources[access.ResourceIndex].FirstSubresourceIndex + access.SubresourceIndex])
					isAlive = true;
			}

			pass.IsCulled = !isAlive;
			if(!isAlive)
			{
				mStatistics.CulledPassCount++;
				continue;
			}

			// A write that discards the earlier contents hides them from everything after it
			for(const SubresourceAccess& access : pass.Accesses)
			{
				const u32 subresourceIndex = mResources[access.ResourceIndex].FirstSubresourceIndex + access.SubresourceIndex;
				if(!access.DiscardsContents)
					isObserved[subresourceIndex] = true;
				else if(access.State.Access.IsSet(GpuAccessFlag::Write))
					isObserved[subresourceIndex] = false;
			}
		}
	}

	void FrameGraph::OrderPasses()
	{
		// Surviving passes keep their declaration order, and each resource learns its first and last surviving pass.
		for(u32 passIndex = 0; passIndex < (u32)mPasses.size(); passIndex++)
		{
			const Pass& pass = mPasses[passIndex];
			if(pass.IsCulled)
				continue;

			const i32 compiledIndex = (i32)mCompiledPassIndices.size();
			mCompiledPassIndices.push_back(passIndex);

			for(const SubresourceAccess& access : pass.Accesses)
			{
				Resource& resource = mResources[access.ResourceIndex];
				if(resource.FirstPassIndex == -1)
					resource.FirstPassIndex = compiledIndex;

				resource.LastPassIndex = compiledIndex;
			}
		}
	}

	void FrameGraph::ResolveQueues(CompileContext& context)
	{
		// Passes that prefer the async compute queue run on it if the device has one, and every other pass on the graphics queue
		const bool isComputeQueueEnabled = gFrameGraphAsyncCompute && mDevice.GetQueueCount(GQT_COMPUTE) > 0;
		for(u32 passIndex : mCompiledPassIndices)
		{
			Pass& pass = mPasses[passIndex];
			pass.QueueType = pass.Flags.IsSet(FrameGraphPassFlag::AsyncCompute) && isComputeQueueEnabled ? GQT_COMPUTE : GQT_GRAPHICS;
			context.UsesComputeQueue |= pass.QueueType == GQT_COMPUTE;
		}

		// The previous pass on the queue of each pass
		context.PreviousQueuePassIndices.resize(mCompiledPassIndices.size(), -1);
		i32 lastQueuePassIndices[GQT_COUNT] = { -1, -1, -1 };
		for(u32 compiledIndex = 0; compiledIndex < (u32)mCompiledPassIndices.size(); compiledIndex++)
		{
			i32& lastQueuePassIndex = lastQueuePassIndices[GetCompiledPass(compiledIndex).QueueType];
			context.PreviousQueuePassIndices[compiledIndex] = lastQueuePassIndex;
			lastQueuePassIndex = (i32)compiledIndex;
		}
	}

	void FrameGraph::PlanBarriers(CompileContext& context)
	{
		// Walking the surviving passes, each subresource tracks its current state, the queue that established it, and the
		// accesses of each queue since. An access joins the current state when it can. Otherwise it plans a barrier into its own state: on
		// its queue if only that queue accessed the subresource since, or a split barrier from the other queue that transfers the subresource
		// between the queues. Exported resources and, at the end of the graph, imported and extracted resources plan barriers back to rest on
		// the graphics queue. Accesses that depend on work of the other queue record a queue wait, which the submissions are cut along. Alias
		// acquires get their sources and their split points after placement, which needs those submissions.

		// Imports start in their resting state. Transients have no state until their first access.
		context.SubresourceStates.resize(context.SubresourceCount);
		for(const Resource& resource : mResources)
		{
			if(resource.Type == ResourceType::Transient)
				continue;

			for(u32 subresourceIndex = 0; subresourceIndex < resource.SubresourceCount; subresourceIndex++)
			{
				CompileContext::SubresourceState& state = context.SubresourceStates[resource.FirstSubresourceIndex + subresourceIndex];
				state.HasState = true;
				state.EstablishedState = resource.RestingState;
			}
		}

		for(u32 compiledIndex = 0; compiledIndex < (u32)mCompiledPassIndices.size(); compiledIndex++)
		{
			const Pass& pass = GetCompiledPass(compiledIndex);
			const GpuQueueType queueType = pass.QueueType;
			const GpuQueueId queueId(queueType, 0);

			for(const SubresourceAccess& access : pass.Accesses)
			{
				const Resource& resource = mResources[access.ResourceIndex];
				CompileContext::SubresourceState& state = context.SubresourceStates[resource.FirstSubresourceIndex + access.SubresourceIndex];

				if(!state.HasState)
				{
					// The first access of a transient starts its lifetime with one barrier over the whole resource, into the state of the
					// pass's first access of it. Transients in persistent memory have no earlier resources to wait on, the others get an alias
					// acquire, completed once they are placed.
					CompileContext::PlannedBarrier barrier;
					barrier.ResourceIndex = access.ResourceIndex;
					barrier.IsWholeResource = true;
					barrier.Destination = access.State;
					barrier.AcquirePassIndex = compiledIndex;
					barrier.ReleasePassIndex = (i32)compiledIndex - 1;
					barrier.ReleaseQueueType = queueType;
					barrier.AcquireQueueType = queueType;
					if(!resource.IsExtracted)
						barrier.Flags = GpuBarrierFlag::AliasAcquire;

					const u32 barrierIndex = (u32)context.PlannedBarriers.size();
					context.PlannedBarriers.push_back(barrier);
					context.FirstAccessBarrierIndices.push_back(barrierIndex);

					for(u32 subresourceIndex = 0; subresourceIndex < resource.SubresourceCount; subresourceIndex++)
					{
						CompileContext::SubresourceState& subresourceState = context.SubresourceStates[resource.FirstSubresourceIndex + subresourceIndex];
						subresourceState.HasState = true;
						subresourceState.EstablishedState = access.State;
						subresourceState.EstablishedQueueType = queueType;
						subresourceState.EstablishedPassIndex = (i32)compiledIndex;
						subresourceState.LastPassIndices[queueType] = (i32)compiledIndex;
						subresourceState.BarrierIndex = barrierIndex;
					}
				}

				// A subresource an export from the compute queue returns to rest is acquired back on the graphics queue before its next
				// declared access, if that comes before the point the acquire would otherwise go
				if(state.PendingComputeExportBarrierIndex != ~0u)
				{
					B3D_ENSURE_LOG(queueType == GQT_GRAPHICS, "Pass '{0}' declares a resource on the async compute queue after an async compute pass exported it. Read it without a declaration instead.",
						pass.Name);

					context.PlannedBarriers[state.PendingComputeExportBarrierIndex].AcquirePassIndex = compiledIndex;
					context.WaitEdges.push_back(std::make_pair(context.PlannedBarriers[state.PendingComputeExportBarrierIndex].ReleasePassIndex, compiledIndex));
					state.PendingComputeExportBarrierIndex = ~0u;
					state.EstablishedPassIndex = (i32)compiledIndex;
					state.LastPassIndices[GQT_GRAPHICS] = (i32)compiledIndex;
				}

				// The access joins the established state if that state can cover it, and otherwise plans a barrier into a state of its own.
				const GpuAccessState& wanted = access.State;
				const GpuAccessState& established = state.EstablishedState;
				const bool hasBarrier = state.BarrierIndex != ~0u;
				const bool isSameLayout = wanted.Layout == established.Layout;

				bool isUsedSinceEstablished = false;
				bool hasWritesSinceEstablished = false;
				for(GpuQueueType usedQueueType : { GQT_GRAPHICS, GQT_COMPUTE })
				{
					isUsedSinceEstablished |= state.StagesSinceEstablished[usedQueueType] != GpuStageFlag::None;
					hasWritesSinceEstablished |= state.AccessSinceEstablished[usedQueueType].IsSet(GpuAccessFlag::Write);
				}

				// The access, the established state and every access since only read
				const bool isReadOnly = !wanted.Access.IsSet(GpuAccessFlag::Write) && !established.Access.IsSet(GpuAccessFlag::Write) && !hasWritesSinceEstablished;

				bool joinsState = false;
				bool joinsStateFromOtherQueue = false;
				if(isSameLayout && state.EstablishedQueueType == queueType)
				{
					// We join state if:
					// - A barrier has been planned, but resource has not been used since, we just expand the barrier
					// - A barrier has been planed, but the resource is only being read, we just expand the barrier
					// - The resource is only being read since the established state, but no barrier was issued. This means we are only safe
					//   to read the established set of stages, as some potential prior write was synchronized only against those stages. Any
					//   other stages require a new barrier.
					
					const bool isUnusedSinceBarrier = hasBarrier && !isUsedSinceEstablished;
					joinsState = isUnusedSinceBarrier || (isReadOnly && (hasBarrier || established.Stages.IsSetAll(wanted.Stages)));
				}
				else if(isSameLayout && isReadOnly && resource.IsShared())
				{
					// Reads on the other queue share the read state too, if both queues can read the resource at once
					joinsStateFromOtherQueue = true;
				}

				// Allow two queues two read the same resource at the same time. We only issue a wait on the first barrier that establishes the state, 
				// all further reads on either queue are done without barriers or waits.
				if(joinsStateFromOtherQueue)
				{
					if(state.BarrierIndex != ~0u)
					{
						context.PlannedBarriers[state.BarrierIndex].Destination.Queues |= queueId;
						context.WaitEdges.push_back(std::make_pair(state.EstablishedPassIndex, compiledIndex));
					}

					state.EstablishedState.Queues |= queueId;
				}
				// Expand existing access and/or barrier with new stages/access
				else if(joinsState)
				{
					if(state.BarrierIndex != ~0u)
					{
						GpuAccessState& destination = context.PlannedBarriers[state.BarrierIndex].Destination;
						destination.Stages |= wanted.Stages;
						destination.Access |= wanted.Access;
					}

					state.EstablishedState.Stages |= wanted.Stages;
					state.EstablishedState.Access |= wanted.Access;
				}
				// Cannot join, need to plan a new barrier
				else
					PlanBarrierWithAutomaticSourceQueue(context, access.ResourceIndex, access.SubresourceIndex, wanted, compiledIndex, queueType, access.DiscardsContents);

				state.StagesSinceEstablished[queueType] |= wanted.Stages;
				state.AccessSinceEstablished[queueType] |= wanted.Access;
				state.LastPassIndices[queueType] = (i32)compiledIndex;
			}

			// Exported resources return to rest right after the pass. A resource that rests on the graphics queue returns there from the
			// compute queue through a split barrier, acquired where the graphics queue waits for the compute queue anyway, so later readers
			// on the graphics queue must wait for the pass. Other resources rest the same way on both queues.
			for(u32 resourceIndex : pass.ExportedResourceIndices)
			{
				const Resource& resource = mResources[resourceIndex];
				const bool returnsToGraphicsQueue = queueType != GQT_GRAPHICS && !resource.IsShared();
				for(u32 subresourceIndex = 0; subresourceIndex < resource.SubresourceCount; subresourceIndex++)
				{
					CompileContext::SubresourceState& state = context.SubresourceStates[resource.FirstSubresourceIndex + subresourceIndex];
					if(!state.HasState || state.BarrierIndex == ~0u)
						continue;

					if(!returnsToGraphicsQueue)
					{
						// Passes of both queues may read a shared resource at rest without declaring it
						GpuAccessState restingState = queueType == GQT_GRAPHICS ? resource.RestingState : resource.GetQueueRestingState(queueType);
						if(context.UsesComputeQueue && resource.IsShared())
							restingState.Queues = GpuQueueMask(GpuQueueId(GQT_GRAPHICS, 0)) | GpuQueueId(GQT_COMPUTE, 0);

						PlanBarrierWithExplicitSourceQueue(context, resourceIndex, subresourceIndex, restingState, compiledIndex, queueType, queueType, false, true);
						continue;
					}

					// Note: Resting on graphics queue is needed only for D3D12, as it uses different image layout types depending on queue type.
					PlanBarrierWithExplicitSourceQueue(context, resourceIndex, subresourceIndex, resource.RestingState, ~0u, GQT_GRAPHICS, queueType, false, false);
					state.PendingComputeExportBarrierIndex = (u32)context.PlannedBarriers.size() - 1;
					state.EstablishedPassIndex = -1;

					context.ComputeExportBarrierIndices.push_back(state.PendingComputeExportBarrierIndex);
				}
			}
		}

		// Imported and extracted resources end the graph at rest on the graphics queue
		for(u32 resourceIndex = 0; resourceIndex < (u32)mResources.size(); resourceIndex++)
		{
			const Resource& resource = mResources[resourceIndex];
			if(resource.Type == ResourceType::Transient && !resource.IsExtracted)
				continue;

			const GpuAccessState& restingState = resource.RestingState;
			const bool isShared = resource.IsShared();
			for(u32 subresourceIndex = 0; subresourceIndex < resource.SubresourceCount; subresourceIndex++)
			{
				CompileContext::SubresourceState& state = context.SubresourceStates[resource.FirstSubresourceIndex + subresourceIndex];
				if(!state.HasState) // Must be an extracted transient as imported resources start with state
				{
					// An extracted resource nothing accessed still has to reach its resting layout
					state.HasState = true;
					PlanBarrierWithExplicitSourceQueue(context, resourceIndex, subresourceIndex, restingState, ~0u, GQT_GRAPHICS, GQT_GRAPHICS, true, false);
					continue;
				}

				// An export from the compute queue is on its way back to rest
				if(state.PendingComputeExportBarrierIndex != ~0u)
					continue;

				// No barrier was ever issued, or barrier was issued that didn't change the rest state. A state established on the compute
				// queue only rests the same way if both queues can read the resource at once.
				const bool hasWrites = state.AccessSinceEstablished[GQT_GRAPHICS].IsSet(GpuAccessFlag::Write) || state.AccessSinceEstablished[GQT_COMPUTE].IsSet(GpuAccessFlag::Write) ||
					state.EstablishedState.Access.IsSet(GpuAccessFlag::Write);
				const bool isRestingQueue = state.EstablishedQueueType == GQT_GRAPHICS ? state.EstablishedState.Stages.IsSetAll(restingState.Stages) : isShared;
				const bool isAtRest = state.BarrierIndex == ~0u || (!hasWrites && state.EstablishedState.Layout == restingState.Layout && isRestingQueue);

				if(!isAtRest)
					PlanBarrierWithAutomaticSourceQueue(context, resourceIndex, subresourceIndex, restingState, ~0u, GQT_GRAPHICS, false);
			}
		}
	}

	void FrameGraph::PlanBarrierWithExplicitSourceQueue(CompileContext& context, u32 resourceIndex, u32 subresourceIndex, const GpuAccessState& destination,
		u32 compiledIndex, GpuQueueType queueType, GpuQueueType sourceQueueType, bool discardsContents, bool isExport)
	{
		const Resource& resource = mResources[resourceIndex];
		CompileContext::SubresourceState& state = context.SubresourceStates[resource.FirstSubresourceIndex + subresourceIndex];

		// The source covers the accesses of the source queue since the state was established. Without any, it chains to the barrier that
		// established the state. A resting state also covers the undeclared readers before the graph.
		CompileContext::PlannedBarrier barrier;
		barrier.ResourceIndex = resourceIndex;
		barrier.SubresourceIndex = subresourceIndex;
		barrier.Source.Stages = state.StagesSinceEstablished[sourceQueueType];
		barrier.Source.Access = state.AccessSinceEstablished[sourceQueueType];
		barrier.Source.Layout = state.EstablishedState.Layout;
		barrier.Source.Queues = state.EstablishedState.Queues;

		// No barrier was issued, or barrier was issued but resource was not used since
		if(state.BarrierIndex == ~0u || state.StagesSinceEstablished[sourceQueueType] == GpuStageFlag::None)
		{
			barrier.Source.Stages |= state.EstablishedState.Stages; // Chain with prior barrier (if any), or on prior readers for imported resource
			if(state.BarrierIndex == ~0u) // State is still in import resting state, wait on prior readers (which are not known)
			{
				barrier.Source.Access |= state.EstablishedState.Access;
				barrier.Source.Compression = state.EstablishedState.Compression;
			}
		}

		// Each half names only stages of the queue it is recorded on
		barrier.Source.Stages &= GpuBackendUtility::GetQueueStageFlags(sourceQueueType);

		if(discardsContents && resource.Buffer == nullptr)
			barrier.Source.Layout = GpuImageLayout::Undefined;

		barrier.Destination = destination;
		barrier.ReleaseQueueType = sourceQueueType;
		barrier.AcquireQueueType = queueType;
		barrier.IsExport = isExport;

		const i32 lastPassIndex = state.LastPassIndices[sourceQueueType];
		if(isExport)
		{
			barrier.ReleasePassIndex = (i32)compiledIndex;
			barrier.AcquirePassIndex = compiledIndex + 1;
		}
		else if(sourceQueueType != queueType)
		{
			// Barriers between queues are always split: released on the source queue after its last access, and acquired on this queue,
			// whose submission waits for the release's
			barrier.IsSplit = true;
			barrier.ReleasePassIndex = lastPassIndex;
			barrier.AcquirePassIndex = compiledIndex;

			if(compiledIndex != ~0u)
				context.WaitEdges.push_back(std::make_pair(lastPassIndex, compiledIndex));
		}
		else if(compiledIndex == ~0u)
		{
			barrier.ReleasePassIndex = lastPassIndex;
			barrier.AcquirePassIndex = ~0u;
		}
		else
		{
			// Barrier is split only if it records some work in-between two halves (i.e. its last and next pass are not adjacent). Additionally:
			// - Window surfaces are acquired by the barrier's command buffer, so their barriers are never split
			barrier.IsSplit = resource.Type != ResourceType::Surface && lastPassIndex < context.PreviousQueuePassIndices[compiledIndex];
			barrier.ReleasePassIndex = barrier.IsSplit ? lastPassIndex : (i32)compiledIndex - 1;
			barrier.AcquirePassIndex = compiledIndex;
		}

		state.EstablishedState = destination;
		state.EstablishedQueueType = queueType;
		state.EstablishedPassIndex = (i32)compiledIndex;
		for(u32 queueTypeIndex = 0; queueTypeIndex < GQT_COUNT; queueTypeIndex++)
		{
			state.StagesSinceEstablished[queueTypeIndex] = GpuStageFlag::None;
			state.AccessSinceEstablished[queueTypeIndex] = GpuAccessFlag::None;
		}

		state.BarrierIndex = (u32)context.PlannedBarriers.size();
		if(compiledIndex != ~0u)
			state.LastPassIndices[queueType] = (i32)compiledIndex;

		context.PlannedBarriers.push_back(barrier);
	}

	void FrameGraph::PlanBarrierWithAutomaticSourceQueue(CompileContext& context, u32 resourceIndex, u32 subresourceIndex, const GpuAccessState& wanted, u32 compiledIndex,
		GpuQueueType queueType, bool discardsContents)
	{
		const CompileContext::SubresourceState& state = context.SubresourceStates[mResources[resourceIndex].FirstSubresourceIndex + subresourceIndex];
		const GpuQueueType otherQueueType = GetOtherQueueType(queueType);
		const bool isUsedOnOtherQueue = state.StagesSinceEstablished[otherQueueType] != GpuStageFlag::None;
		const bool isUsedOnQueue = state.StagesSinceEstablished[queueType] != GpuStageFlag::None;

		if(isUsedOnOtherQueue && isUsedOnQueue)
		{
			// Both queues read the subresource since its state was established. A split barrier only orders the reads of the queue that releases
			// it, so whichever queue releases it, the layout change between its halves could overlap the other queue's reads. Instead the pass
			// waits for the other queue's last read, and a full barrier on this queue, recorded after that wait, orders this queue's reads.
			if(compiledIndex != ~0u)
				context.WaitEdges.push_back(std::make_pair(state.LastPassIndices[otherQueueType], compiledIndex));

			PlanBarrierWithExplicitSourceQueue(context, resourceIndex, subresourceIndex, wanted, compiledIndex, queueType, queueType, discardsContents, false);

			if(compiledIndex != ~0u)
			{
				CompileContext::PlannedBarrier& barrier = context.PlannedBarriers.back();
				barrier.IsSplit = false;
				barrier.ReleasePassIndex = (i32)compiledIndex - 1;
			}

			return;
		}

		// The queue that accessed the subresource since its state was established, or the queue that established it
		const GpuQueueType sourceQueueType = isUsedOnOtherQueue ? otherQueueType : isUsedOnQueue ? queueType : state.EstablishedQueueType;
		PlanBarrierWithExplicitSourceQueue(context, resourceIndex, subresourceIndex, wanted, compiledIndex, queueType, sourceQueueType, discardsContents, false);
	}

	void FrameGraph::BuildSubmissions(CompileContext& context)
	{
		// The passes of each queue are cut into submissions where a pass waits for a pass of the other queue: the other pass's
		// submission ends after it, and this pass's queue starts a new submission that waits for it, unless its last submission already does.
		// Edges always go from an earlier pass to a later one, so each submission's passes come after those of the submissions it waits for.
		/** Submission of one queue, before the submit order is known. */
		struct SubmissionDraft
		{
			FrameVector<u32> PassIndices; /**< Indices into mCompiledPassIndices, in execution order. */
			i32 RequiredDraftIndex = -1; /**< Index into the other queue's drafts of the latest draft this one waits for, or -1. */
			u32 SubmissionIndex = 0; /**< Index into mSubmissions, once submit order is known. */
		};

		FrameVector<SubmissionDraft> drafts[2];
		FrameVector<u32> passDraftIndices(mCompiledPassIndices.size(), 0);

		// Releases from the start of the graph to the compute queue go into a graphics prologue, so the compute work that acquires them does
		// not wait for the first graphics submission's work
		const bool hasPrologue = std::any_of(context.WaitEdges.begin(), context.WaitEdges.end(), [](const std::pair<i32, u32>& edge) { return edge.first == -1; });
		if(hasPrologue)
			drafts[GQT_GRAPHICS].emplace_back();

		std::stable_sort(context.WaitEdges.begin(), context.WaitEdges.end(), [](const std::pair<i32, u32>& a, const std::pair<i32, u32>& b) { return a.second < b.second; });

		{
			// Whether no more passes go into the queue's last draft. The prologue takes none.
			bool isLastDraftEnded[2] = { true, true };
			u32 edgeIndex = 0;
			for(u32 compiledIndex = 0; compiledIndex < (u32)mCompiledPassIndices.size(); compiledIndex++)
			{
				const GpuQueueType queueType = GetCompiledPass(compiledIndex).QueueType;
				const GpuQueueType otherQueueType = GetOtherQueueType(queueType);

				// The latest draft of the other queue the pass waits for. A wait for the start of the graph waits for the prologue.
				i32 requiredDraftIndex = -1;
				for(; edgeIndex < (u32)context.WaitEdges.size() && context.WaitEdges[edgeIndex].second == compiledIndex; edgeIndex++)
				{
					const i32 sourcePassIndex = context.WaitEdges[edgeIndex].first;
					B3D_ASSERT(sourcePassIndex == -1 || GetCompiledPass(sourcePassIndex).QueueType == otherQueueType);

					requiredDraftIndex = std::max(requiredDraftIndex, sourcePassIndex == -1 ? 0 : (i32)passDraftIndices[sourcePassIndex]);
				}

				// No pass can join this draft on the other queue, since we're waiting on it. Passes only join a queue's last draft, so an
				// earlier draft has ended already.
				const bool waitsForLastDraft = requiredDraftIndex != -1 && requiredDraftIndex == (i32)drafts[otherQueueType].size() - 1;
				if(waitsForLastDraft)
					isLastDraftEnded[otherQueueType] = true;

				// The pass goes into the queue's last draft, unless that draft has ended or does not wait for what the pass waits for
				FrameVector<SubmissionDraft>& queueDrafts = drafts[queueType];
				if(isLastDraftEnded[queueType] || queueDrafts.back().RequiredDraftIndex < requiredDraftIndex)
				{
					queueDrafts.emplace_back().RequiredDraftIndex = requiredDraftIndex;
					isLastDraftEnded[queueType] = false;
				}

				queueDrafts.back().PassIndices.push_back(compiledIndex);
				passDraftIndices[compiledIndex] = (u32)queueDrafts.size() - 1;
			}
		}

		// The graph ends joined on the graphics queue: its last submission waits for the compute queue's last one. A graphics epilogue adds
		// that wait if the last graphics submission does not already have it, and holds the end barriers.
		const i32 lastComputeDraftIndex = (i32)drafts[GQT_COMPUTE].size() - 1;
		if(drafts[GQT_GRAPHICS].empty() || drafts[GQT_GRAPHICS].back().RequiredDraftIndex < lastComputeDraftIndex)
			drafts[GQT_GRAPHICS].emplace_back().RequiredDraftIndex = lastComputeDraftIndex;

		// Submit order merges the queues' submissions, each once those it waits for are submitted. Compute submissions go first when they
		// can, so their waits for the latest graphics work cover no more graphics work than they need. A wait that the previous submission of
		// the queue already covers is left out.
		{
			// Number of each queue's drafts submitted so far, which is also the index of the queue's next draft
			u32 submittedDraftCounts[2] = { 0, 0 };
			const auto fnCanSubmitNextDraft = [&drafts, &submittedDraftCounts](GpuQueueType queueType)
			{
				const u32 nextDraftIndex = submittedDraftCounts[queueType];
				if(nextDraftIndex == (u32)drafts[queueType].size())
					return false;

				// The draft of the other queue it waits for, if any, must be submitted already
				const i32 requiredDraftIndex = drafts[queueType][nextDraftIndex].RequiredDraftIndex;
				return requiredDraftIndex < (i32)submittedDraftCounts[GetOtherQueueType(queueType)];
			};

			while(submittedDraftCounts[GQT_GRAPHICS] < (u32)drafts[GQT_GRAPHICS].size() || submittedDraftCounts[GQT_COMPUTE] < (u32)drafts[GQT_COMPUTE].size())
			{
				const GpuQueueType queueType = fnCanSubmitNextDraft(GQT_COMPUTE) ? GQT_COMPUTE : GQT_GRAPHICS;
				const GpuQueueType otherQueueType = GetOtherQueueType(queueType);
				B3D_ASSERT(fnCanSubmitNextDraft(queueType));

				const u32 draftIndex = submittedDraftCounts[queueType];
				SubmissionDraft& draft = drafts[queueType][draftIndex];
				draft.SubmissionIndex = (u32)mSubmissions.size();

				GpuQueueMask waitQueues;
				if(draft.RequiredDraftIndex != -1)
				{
					const u32 requiredSubmissionIndex = drafts[otherQueueType][draft.RequiredDraftIndex].SubmissionIndex;
					const bool isCovered = draftIndex > 0 && mTimeline.IsOrderedBefore(requiredSubmissionIndex, drafts[queueType][draftIndex - 1].SubmissionIndex);
					if(!isCovered)
					{
						waitQueues |= GpuQueueId(otherQueueType, 0);
						mStatistics.QueueWaitCount++;
					}
				}

				mTimeline.AddSubmission(GpuQueueId(queueType, 0), waitQueues);

				Submission& submission = mSubmissions.emplace_back();
				submission.QueueType = queueType;
				submission.SyncMask = waitQueues;
				submission.PassIndices.assign(draft.PassIndices.begin(), draft.PassIndices.end());

				for(u32 compiledIndex : draft.PassIndices)
					GetCompiledPass(compiledIndex).SubmissionIndex = draft.SubmissionIndex;

				submittedDraftCounts[queueType]++;
			}
		}

		// The first submission of each queue also waits for the work before the graph. Earlier graphs end on the graphics queue, so the first
		// graphics submission waits for every queue except the compute queue this graph uses, whose own work comes after it. The first compute
		// submission waits for the graphics queue, which follows the earlier graphs' work and the context's uploads (see GpuWorkContext).
		context.FirstGraphicsSubmissionIndex = drafts[GQT_GRAPHICS][0].SubmissionIndex;
		context.LastSubmissionIndex = drafts[GQT_GRAPHICS].back().SubmissionIndex;
		B3D_ASSERT(context.LastSubmissionIndex == (u32)mSubmissions.size() - 1);

		mSubmissions[context.FirstGraphicsSubmissionIndex].SyncMask |= context.UsesComputeQueue ? GpuQueueMask::kAll & ~GpuQueueMask(GpuQueueId(GQT_COMPUTE, 0)) : GpuQueueMask::kAll;
		if(!drafts[GQT_COMPUTE].empty())
			mSubmissions[drafts[GQT_COMPUTE][0].SubmissionIndex].SyncMask |= GpuQueueMask(GpuQueueId(GQT_GRAPHICS, 0)) | GpuQueueMask(GpuQueueId(GQT_TRANSFER, 0));

		mStatistics.SubmissionCount = (u32)mSubmissions.size();
	}

	void FrameGraph::PlaceTransients(const CompileContext& context)
	{
		// Extracted transients leave the allocator for memory of their own, and transients that only culled passes use go back
		// unplaced. The rest are placed in pass order and released after their last pass, so transients whose passes do not overlap can
		// share memory.
		mScope = &mAllocator.BeginScope(mTimeline);

		for(Resource& resource : mResources)
		{
			if(resource.Type != ResourceType::Transient || !resource.IsExtracted)
				continue;

			if(resource.Texture != nullptr)
			{
				mAllocator.Detach(*resource.Texture);

				const GpuMemoryRequirements memoryRequirements = mDevice.GetMemoryRequirements(resource.TextureInformation);
				resource.Texture->SetAllocation(GpuAllocation::CreatePending(mDevice.GetPersistentAllocator(memoryRequirements.MemoryType)), GpuObjectCreateFlag::None);
				if(!resource.Texture->IsInitialized())
					resource.Texture->Initialize();
			}
			else
			{
				mAllocator.Detach(*resource.Buffer);

				const GpuMemoryRequirements memoryRequirements = mDevice.GetMemoryRequirements(resource.BufferInformation);
				resource.Buffer->SetAllocation(GpuAllocation::CreatePending(mDevice.GetPersistentAllocator(memoryRequirements.MemoryType)), GpuObjectCreateFlag::None);
				if(!resource.Buffer->IsInitialized())
					resource.Buffer->Initialize();
			}

			resource.IsHeldUnplaced = false;
		}

		for(Resource& resource : mResources)
		{
			if(resource.Type != ResourceType::Transient || resource.IsExtracted || resource.FirstPassIndex != -1)
				continue;

			if(resource.Texture != nullptr)
				mAllocator.Return(*resource.Texture);
			else
				mAllocator.Return(*resource.Buffer);

			resource.IsHeldUnplaced = false;
		}

		const auto fnIsAliased = [](const Resource& resource)
		{
			return resource.Type == ResourceType::Transient && !resource.IsExtracted;
		};

		// Last accesses on each queue of each subresource of the transients: the last write, and every read after it. Indexed by subresource
		// and queue.
		constexpr u32 kQueueTypeCount = 2;
		FrameVector<GpuAccessScope> lastUses(context.SubresourceCount * kQueueTypeCount);

		// Index into mCompiledPassIndices of the last pass of each queue that used each resource, or -1 if none did. Indexed by resource and queue.
		FrameVector<i32> lastQueuePassIndices(mResources.size() * kQueueTypeCount, -1);

		for(u32 compiledIndex = 0; compiledIndex < (u32)mCompiledPassIndices.size(); compiledIndex++)
		{
			const Pass& pass = GetCompiledPass(compiledIndex);

			// Transients are placed before the transients of the same pass are released, so they never share memory
			for(const SubresourceAccess& access : pass.Accesses)
			{
				Resource& resource = mResources[access.ResourceIndex];
				if(!fnIsAliased(resource) || resource.FirstPassIndex != (i32)compiledIndex || !resource.IsHeldUnplaced)
					continue;

				resource.Placement = resource.Texture != nullptr ? mScope->PlaceTexture(*resource.Texture, pass.SubmissionIndex) : mScope->PlaceBuffer(*resource.Buffer, pass.SubmissionIndex);
				resource.IsHeldUnplaced = false;
			}

			for(const SubresourceAccess& access : pass.Accesses)
			{
				const Resource& resource = mResources[access.ResourceIndex];
				if(!fnIsAliased(resource))
					continue;

				GpuAccessScope& lastUse = lastUses[(resource.FirstSubresourceIndex + access.SubresourceIndex) * kQueueTypeCount + pass.QueueType];
				if(access.State.Access.IsSet(GpuAccessFlag::Write))
					lastUse = GpuAccessScope();

				lastUse.Add(access.State.Stages, access.State.Access);
				lastQueuePassIndices[access.ResourceIndex * kQueueTypeCount + pass.QueueType] = (i32)compiledIndex;
			}

			for(u32 accessIndex = 0; accessIndex < (u32)pass.Accesses.size(); accessIndex++)
			{
				// Accesses are sorted by resource, so each resource is released once
				const SubresourceAccess& access = pass.Accesses[accessIndex];
				if(accessIndex > 0 && pass.Accesses[accessIndex - 1].ResourceIndex == access.ResourceIndex)
					continue;

				const Resource& resource = mResources[access.ResourceIndex];
				if(!fnIsAliased(resource) || resource.LastPassIndex != (i32)compiledIndex)
					continue;

				// One last use for each queue that used the resource
				TInlineArray<GpuTransientLastUse, 2> resourceLastUses;
				for(GpuQueueType queueType : { GQT_GRAPHICS, GQT_COMPUTE })
				{
					const i32 lastQueuePassIndex = lastQueuePassIndices[access.ResourceIndex * kQueueTypeCount + queueType];
					if(lastQueuePassIndex == -1)
						continue;

					GpuAccessScope lastAccess;
					for(u32 subresourceIndex = 0; subresourceIndex < resource.SubresourceCount; subresourceIndex++)
						lastAccess.Add(lastUses[(resource.FirstSubresourceIndex + subresourceIndex) * kQueueTypeCount + queueType]);

					resourceLastUses.Add(GpuTransientLastUse(GetCompiledPass(lastQueuePassIndex).SubmissionIndex, lastAccess, (u32)lastQueuePassIndex));
				}

				const TArrayView<const GpuTransientLastUse> lastUsesView(resourceLastUses.Data(), resourceLastUses.Size());
				if(resource.Texture != nullptr)
					mScope->Release(*resource.Texture, lastUsesView);
				else
					mScope->Release(*resource.Buffer, lastUsesView);
			}
		}
	}

	void FrameGraph::CreateRenderTextures()
	{
		// A raster pass whose attachments were set one by one renders through a render texture of those surfaces. The
		// work context keeps them across graphs, until a surface is recreated or the render texture goes unused for too long.
		// TODO - Evict render textures together with the transient textures they reference. The transient allocator evicts on its own
		// schedule, and only compiling a later graph on this context evicts render textures, so a render texture can keep an evicted
		// transient texture alive after its heap was released, indefinitely if the context builds no more graphs.
		FrameGraphContextData& contextData = GetContextData(mContext);
		const u64 frameIndex = mDevice.GetFrameIndex();

		// A render texture refers to the native resources of its surfaces, so one whose surface was recreated is stale
		const auto fnIsRecreated = [this](const TShared<Texture>& texture)
		{
			if(texture == nullptr)
				return false;

			const u32 resourceIndex = FindResource(*texture);
			if(resourceIndex == ~0u)
				return false;

			const Resource& resource = mResources[resourceIndex];
			return resource.Type == ResourceType::Transient && (resource.IsExtracted || resource.Placement.Recreated);
		};

		// A transient the allocator does not cache, or that leaves it, is a new texture in every graph
		const auto fnIsKeptAcrossGraphs = [this](const TShared<Texture>& texture)
		{
			if(texture == nullptr)
				return true;

			const u32 resourceIndex = FindResource(*texture);
			if(resourceIndex == ~0u)
				return true;

			const Resource& resource = mResources[resourceIndex];
			return resource.Type != ResourceType::Transient || (!resource.IsExtracted && resource.Placement.Cached);
		};

		Vector<FrameGraphContextData::RenderTextureEntry>& entries = contextData.RenderTextures;
		entries.erase(std::remove_if(entries.begin(), entries.end(), [&fnIsRecreated, frameIndex](const FrameGraphContextData::RenderTextureEntry& entry)
		{
			if(entry.LastUsedFrame + gGpuTransientIdleFrames < frameIndex)
				return true;

			for(const RenderSurfaceInformation& surface : entry.Information.ColorSurfaces)
			{
				if(fnIsRecreated(surface.Texture))
					return true;
			}

			return fnIsRecreated(entry.Information.DepthStencilSurface.Texture);
		}), entries.end());

		for(u32 passIndex : mCompiledPassIndices)
		{
			Pass& pass = mPasses[passIndex];
			if(pass.Type != PassType::Raster || pass.Attachments.Target != nullptr)
				continue;

			RenderTextureCreateInformation information;
			const auto fnSetSurface = [](RenderSurfaceInformation& surface, const FrameGraphRasterPass::Attachment& attachment)
			{
				surface.Texture = attachment.Texture;
				surface.Face = attachment.Face;
				surface.FaceCount = attachment.FaceCount;
				surface.MipLevel = attachment.MipLevel;
			};

			bool hasAttachments = pass.Attachments.DepthStencilAttachment.Texture != nullptr;
			for(u32 colorIndex = 0; colorIndex < B3D_MAXIMUM_RENDER_TARGET_COUNT; colorIndex++)
			{
				fnSetSurface(information.ColorSurfaces[colorIndex], pass.Attachments.ColorAttachments[colorIndex]);
				hasAttachments |= pass.Attachments.ColorAttachments[colorIndex].Texture != nullptr;
			}

			fnSetSurface(information.DepthStencilSurface, pass.Attachments.DepthStencilAttachment);
			if(!B3D_ENSURE_LOG(hasAttachments, "Raster pass '{0}' has no attachments.", pass.Name))
				continue;

			bool isKept = fnIsKeptAcrossGraphs(information.DepthStencilSurface.Texture);
			for(u32 colorIndex = 0; colorIndex < B3D_MAXIMUM_RENDER_TARGET_COUNT; colorIndex++)
				isKept &= fnIsKeptAcrossGraphs(information.ColorSurfaces[colorIndex].Texture);

			const auto found = !isKept ? entries.end() : std::find_if(entries.begin(), entries.end(), [&information](const FrameGraphContextData::RenderTextureEntry& entry)
			{
				for(u32 colorIndex = 0; colorIndex < B3D_MAXIMUM_RENDER_TARGET_COUNT; colorIndex++)
				{
					const RenderSurfaceInformation& surface = information.ColorSurfaces[colorIndex];
					if(!IsSameSurface(entry.Information.ColorSurfaces[colorIndex], surface) || entry.GpuResources[colorIndex] != GetSurfaceGpuResource(surface))
						return false;
				}

				const RenderSurfaceInformation& surface = information.DepthStencilSurface;
				return IsSameSurface(entry.Information.DepthStencilSurface, surface) && entry.GpuResources[B3D_MAXIMUM_RENDER_TARGET_COUNT] == GetSurfaceGpuResource(surface);
			});

			if(found != entries.end())
			{
				found->LastUsedFrame = frameIndex;
				pass.Attachments.Target = found->Target;
				continue;
			}

			FrameGraphContextData::RenderTextureEntry entry;
			entry.Information = information;
			entry.LastUsedFrame = frameIndex;
			for(u32 colorIndex = 0; colorIndex < B3D_MAXIMUM_RENDER_TARGET_COUNT; colorIndex++)
				entry.GpuResources[colorIndex] = GetSurfaceGpuResource(information.ColorSurfaces[colorIndex]);

			entry.GpuResources[B3D_MAXIMUM_RENDER_TARGET_COUNT] = GetSurfaceGpuResource(information.DepthStencilSurface);

#if B3D_BUILD_TYPE_DEVELOPMENT
			gIsCreatingRenderTexture = true;
#endif
			entry.Target = RenderTexture::Create(information);
#if B3D_BUILD_TYPE_DEVELOPMENT
			gIsCreatingRenderTexture = false;
#endif

			pass.Attachments.Target = entry.Target;
			if(isKept)
				entries.push_back(std::move(entry));
		}
	}

	void FrameGraph::CompleteAliasAcquires(CompileContext& context)
	{
		// A transient's memory may span two kinds of memory. Memory that earlier transients of this graph released (its
		// predecessors), whose last accesses on the acquiring queue are acquire.Source, the latest in pass latestPredecessorPassIndex. And memory
		// that no transient of this graph used, which graphs submitted earlier may still use (acquire.HasUnknownSourceAccess). Predecessors on
		// the other queue are ordered before the first use by a queue wait, as placement checked.
		// True for each resource whose lifetime an alias release ended. Index into mResources.
		FrameVector<bool> isAliasReleased(mResources.size(), false);

		// Index into mResources of each aliased texture, by its GPU resource. Alias acquires name their predecessors by GPU resource.
		FrameUnorderedMap<const IGpuResource*, u32> aliasedTextureIndices;
		for(u32 resourceIndex = 0; resourceIndex < (u32)mResources.size(); resourceIndex++)
		{
			const Resource& resource = mResources[resourceIndex];
			if(resource.Type == ResourceType::Transient && !resource.IsExtracted && resource.Texture != nullptr && resource.Texture->GetGpuResource() != nullptr)
				aliasedTextureIndices[resource.Texture->GetGpuResource()] = resourceIndex;
		}

		for(u32 barrierIndex : context.FirstAccessBarrierIndices)
		{
			if(!context.PlannedBarriers[barrierIndex].Flags.IsSet(GpuBarrierFlag::AliasAcquire))
				continue;

			CompileContext::PlannedBarrier& barrier = context.PlannedBarriers[barrierIndex];
			const Resource& resource = mResources[barrier.ResourceIndex];
			const u32 compiledIndex = barrier.AcquirePassIndex;
			const Pass& pass = GetCompiledPass(compiledIndex);
			const GpuQueueType queueType = pass.QueueType;
			const u32 submissionFirstPassIndex = mSubmissions[pass.SubmissionIndex].PassIndices[0];

			const GpuAliasAcquire& acquire = resource.Placement.Acquire;
			const TOptional<u32>& latestPredecessorPassIndex = resource.Placement.LatestPredecessorPassIndex;
			const bool hasPredecessors = latestPredecessorPassIndex.has_value();
			const bool hasUnusedMemory = acquire.HasUnknownSourceAccess;

			barrier.Source = GpuAccessState(acquire.Source.GetStages(), acquire.Source.GetAccess());

			// Nothing else orders this graph after the graphs submitted earlier, and their accesses to the unused memory are unknown,
			// so the acquire waits for all earlier work on the queue
			if(hasUnusedMemory)
			{
				barrier.Source.Stages |= GpuStageFlags(GpuStageFlag::All) & ~GpuStageFlags(GpuStageFlag::Host);
				barrier.Source.Access |= GpuAccessFlag::Write;
			}

			barrier.Source.Stages &= GpuBackendUtility::GetQueueStageFlags(queueType);

			// A split release goes in the first use's submission, because placement checked the other queue's uses only against it
			if(hasPredecessors)
			{
				// Released after the predecessors' last pass if passes of the queue run between it and this one, which discards the old
				// contents while they run. A release there also waits for the graphs submitted earlier. A predecessor in an earlier
				// submission of the queue gets the release at the start of this submission instead.
				if((i32)*latestPredecessorPassIndex < context.PreviousQueuePassIndices[compiledIndex])
				{
					if(GetCompiledPass(*latestPredecessorPassIndex).SubmissionIndex == pass.SubmissionIndex)
					{
						barrier.IsSplit = true;
						barrier.ReleasePassIndex = (i32)*latestPredecessorPassIndex;
					}
					else if(submissionFirstPassIndex != compiledIndex)
					{
						barrier.IsSplit = true;
						barrier.ReleasePassIndex = -1;
						barrier.ReleaseSubmissionIndex = pass.SubmissionIndex;
					}
				}
			}
			else if(hasUnusedMemory && submissionFirstPassIndex != compiledIndex)
			{
				// Only earlier graphs used the memory, so the release goes at the start of the submission, where it waits for no pass of
				// this graph, and the acquire rarely waits. Images with two aspects get no split barriers (see the workaround below), so
				// theirs is a full barrier at the start of the submission instead. No pass of the queue uses the memory before it,
				// because acquire.Source and the predecessors cover the queue.
				if(resource.Texture != nullptr && resource.FullRange.GetAspectCount() == 2)
				{
					barrier.AcquirePassIndex = submissionFirstPassIndex;
					barrier.ReleasePassIndex = (i32)submissionFirstPassIndex - 1;
				}
				else
				{
					barrier.IsSplit = true;
					barrier.ReleasePassIndex = -1;
					barrier.ReleaseSubmissionIndex = pass.SubmissionIndex;
				}
			}

			mStatistics.AliasAcquireCount++;
			if(barrier.IsSplit)
				mStatistics.SplitAliasAcquireCount++;

			// D3D12 needs an alias release on every texture the acquire supersedes, to flush its writes before the memory is reused. Other
			// backends record nothing for it. The release goes with the acquire, in the same call or split, once per texture, with the first
			// acquire that supersedes it. Textures of earlier graphs and textures last used on the other queue need none, as the command
			// buffer boundary and the queue wait flush their writes.
			const CompileContext::PlannedBarrier acquireBarrier = barrier;
			const GpuAccessState releaseDestination(acquireBarrier.Destination.Stages, GpuAccessFlag::None, GpuImageLayout::Undefined);
			for(IGpuResource* predecessor : acquire.Predecessors)
			{
				const auto found = aliasedTextureIndices.find(predecessor);
				if(found == aliasedTextureIndices.end() || isAliasReleased[found->second])
					continue;

				const u32 predecessorIndex = found->second;
				const Resource& predecessorResource = mResources[predecessorIndex];

				isAliasReleased[predecessorIndex] = true;
				if(predecessorResource.LastPassIndex == -1 || GetCompiledPass(predecessorResource.LastPassIndex).QueueType != queueType)
					continue;

				mStatistics.AliasReleaseCount++;

				for(u32 subresourceIndex = 0; subresourceIndex < predecessorResource.SubresourceCount; subresourceIndex++)
				{
					if(!context.SubresourceStates[predecessorResource.FirstSubresourceIndex + subresourceIndex].HasState)
						continue;

					PlanBarrierWithExplicitSourceQueue(context, predecessorIndex, subresourceIndex, releaseDestination, compiledIndex, queueType, queueType, false, false);

					CompileContext::PlannedBarrier& release = context.PlannedBarriers.back();
					release.Flags = GpuBarrierFlag::AliasRelease;
					release.IsSplit = acquireBarrier.IsSplit;
					release.ReleasePassIndex = acquireBarrier.ReleasePassIndex;
					release.ReleaseSubmissionIndex = acquireBarrier.ReleaseSubmissionIndex;
					release.AcquirePassIndex = acquireBarrier.AcquirePassIndex;
				}
			}
		}
	}

	void FrameGraph::ResolveDeferredBarrierPoints(CompileContext& context)
	{
		// Exports from the compute queue that no later declared access acquired are acquired before the first pass of the first later
		// graphics submission that waits for them, or at the end of the graph
		for(u32 barrierIndex : context.ComputeExportBarrierIndices)
		{
			CompileContext::PlannedBarrier& barrier = context.PlannedBarriers[barrierIndex];
			if(barrier.AcquirePassIndex != ~0u)
				continue;

			const u32 releaseSubmissionIndex = GetCompiledPass(barrier.ReleasePassIndex).SubmissionIndex;
			for(u32 submissionIndex = releaseSubmissionIndex + 1; submissionIndex <= context.LastSubmissionIndex; submissionIndex++)
			{
				const Submission& submission = mSubmissions[submissionIndex];
				if(submission.QueueType == GQT_GRAPHICS && !submission.PassIndices.empty() && mTimeline.IsOrderedBefore(releaseSubmissionIndex, submissionIndex))
				{
					barrier.AcquirePassIndex = submission.PassIndices[0];
					break;
				}
			}
		}

		// Releases at the start of the graph go into the first graphics submission, which is the prologue if there is one
		for(CompileContext::PlannedBarrier& barrier : context.PlannedBarriers)
		{
			if(barrier.ReleasePassIndex == -1 && barrier.ReleaseSubmissionIndex == ~0u)
				barrier.ReleaseSubmissionIndex = context.FirstGraphicsSubmissionIndex;
		}
	}

	void FrameGraph::BatchBarriers(CompileContext& context)
	{
		// Workaround for the Vulkan validation layer: depth-stencil images get no split barriers. Sync validation reports a false write-after-read
		// hazard when vkCmdWaitEvents2 transitions the layout of a depth-stencil image that an earlier submission read, such as the previous
		// frame's render pass that loaded it, even when this frame has written the image since. The report itself says the read is already
		// synchronized with every stage (each frame's first submission waits for the previous frame), but that the layout transition does not
		// synchronize with them, although the barrier's source stages are among them. By the specification the split barrier is correct;
		// pipeline barriers with the same scopes are not reported. Not reduced to a minimal repro. Seen with split barriers whose depth and
		// stencil aspects differ in kind, and with MSAA, where the coverage stencil, the resolved scene depth and the SSR stencil get split
		// barriers of single aspects. Split barriers between queues use no events, and must stay split.
		for(CompileContext::PlannedBarrier& barrier : context.PlannedBarriers)
		{
			const Resource& resource = mResources[barrier.ResourceIndex];
			if(!barrier.IsSplit || barrier.ReleaseQueueType != barrier.AcquireQueueType || resource.Buffer != nullptr || resource.FullRange.GetAspectCount() != 2)
				continue;

			barrier.IsSplit = false;
			barrier.ReleasePassIndex = (i32)barrier.AcquirePassIndex - 1;
		}

		// Planned barriers that differ only in their subresource merge into subresource ranges. Each range goes to the list of
		// the call that records it: a pass's alias acquires or full barriers, the barrier batch of its split release and acquire, a pass's
		// export barriers, or the end of the graph.
		const auto fnKey = [](const CompileContext::PlannedBarrier& barrier)
		{
			return std::make_tuple(barrier.AcquirePassIndex, barrier.ReleasePassIndex, barrier.ReleaseSubmissionIndex, barrier.IsSplit, barrier.IsExport, (u32)barrier.ReleaseQueueType,
				(u32)barrier.AcquireQueueType, barrier.ResourceIndex, barrier.IsWholeResource, (u32)barrier.Flags, (u32)barrier.Source.Stages, (u32)barrier.Source.Access,
				(u32)barrier.Source.Layout, (u32)barrier.Source.Compression, barrier.Source.Queues.Mask, (u32)barrier.Destination.Stages, (u32)barrier.Destination.Access,
				(u32)barrier.Destination.Layout, (u32)barrier.Destination.Compression, barrier.Destination.Queues.Mask);
		};

		FrameVector<u32> order(context.PlannedBarriers.size());
		for(u32 barrierIndex = 0; barrierIndex < (u32)order.size(); barrierIndex++)
			order[barrierIndex] = barrierIndex;

		std::stable_sort(order.begin(), order.end(), [&](u32 a, u32 b)
		{
			const auto keyA = fnKey(context.PlannedBarriers[a]);
			const auto keyB = fnKey(context.PlannedBarriers[b]);
			if(keyA != keyB)
				return keyA < keyB;

			return context.PlannedBarriers[a].SubresourceIndex < context.PlannedBarriers[b].SubresourceIndex;
		});

		// Returns the list that records barriers like @p barrier
		const auto fnGetList = [this, &context](const CompileContext::PlannedBarrier& barrier) -> BarrierList&
		{
			if(barrier.IsExport)
				return GetCompiledPass(barrier.ReleasePassIndex).ExportBarriers;

			if(barrier.IsSplit)
			{
				const auto found = std::find_if(mBarrierBatches.begin(), mBarrierBatches.end(), [&barrier](const BarrierBatch& batch)
				{
					return batch.ReleasePassIndex == barrier.ReleasePassIndex && batch.AcquirePassIndex == barrier.AcquirePassIndex && batch.ReleaseQueueType == barrier.ReleaseQueueType &&
						batch.AcquireQueueType == barrier.AcquireQueueType && (barrier.ReleasePassIndex != -1 || batch.ReleaseSubmissionIndex == barrier.ReleaseSubmissionIndex);
				});

				if(found != mBarrierBatches.end())
					return found->Barriers;

				const u32 batchIndex = (u32)mBarrierBatches.size();
				BarrierBatch& batch = mBarrierBatches.emplace_back();
				batch.ReleasePassIndex = barrier.ReleasePassIndex;
				batch.ReleaseSubmissionIndex = barrier.ReleaseSubmissionIndex;
				batch.AcquirePassIndex = barrier.AcquirePassIndex;
				batch.ReleaseQueueType = barrier.ReleaseQueueType;
				batch.AcquireQueueType = barrier.AcquireQueueType;
				batch.SplitBarrier = barrier.ReleaseQueueType != barrier.AcquireQueueType ? mDevice.CreateSplitBarrier(barrier.ReleaseQueueType, barrier.AcquireQueueType) :
					mDevice.CreateSplitBarrier();

				if(barrier.ReleasePassIndex == -1)
					mSubmissions[barrier.ReleaseSubmissionIndex].StartBarrierBatchIndices.Add(batchIndex);
				else
					GetCompiledPass(barrier.ReleasePassIndex).ReleasedBarrierBatchIndices.Add(batchIndex);

				if(barrier.AcquirePassIndex == ~0u)
					mSubmissions[context.LastSubmissionIndex].EndBarrierBatchIndices.Add(batchIndex);
				else
					GetCompiledPass(barrier.AcquirePassIndex).AcquiredBarrierBatchIndices.Add(batchIndex);

				mStatistics.BarrierBatchCount++;
				return batch.Barriers;
			}

			if(barrier.AcquirePassIndex == ~0u)
				return mSubmissions[context.LastSubmissionIndex].EndBarriers;

			Pass& pass = GetCompiledPass(barrier.AcquirePassIndex);
			return barrier.Flags.IsSetAny(GpuBarrierFlag::AliasAcquire | GpuBarrierFlag::AliasRelease) ? pass.AliasAcquireBarriers : pass.Barriers;
		};

		for(u32 runStart = 0; runStart < (u32)order.size();)
		{
			const CompileContext::PlannedBarrier& first = context.PlannedBarriers[order[runStart]];
			const auto firstKey = fnKey(first);

			u32 runEnd = runStart + 1;
			while(runEnd < (u32)order.size() && fnKey(context.PlannedBarriers[order[runEnd]]) == firstKey)
				runEnd++;

			BarrierList& list = fnGetList(first);
			const Resource& resource = mResources[first.ResourceIndex];

			TInlineArray<GpuTextureSubresourceRange, 4> ranges;
			if(first.IsWholeResource || resource.Buffer != nullptr)
				ranges.Add(GpuTextureSubresourceRange::AllSubresources());
			else
			{
				// Each aspect and layer gets runs of consecutive mips, and layers with the same runs merge
				const GpuTextureSubresourceRange& fullRange = resource.FullRange;
				FrameVector<bool> isIncluded(resource.SubresourceCount, false);
				for(u32 orderIndex = runStart; orderIndex < runEnd; orderIndex++)
					isIncluded[context.PlannedBarriers[order[orderIndex]].SubresourceIndex] = true;

				for(GpuTextureAspectFlag aspect : kGpuTextureAspects)
				{
					if(!fullRange.AspectMask.IsSet(aspect))
						continue;

					for(u32 layer = 0; layer < fullRange.ArrayLayerCount; layer++)
					{
						for(u32 mip = 0; mip < fullRange.MipLevelCount;)
						{
							if(!isIncluded[fullRange.GetSubresourceIndex(mip, layer, aspect)])
							{
								mip++;
								continue;
							}

							u32 mipEnd = mip + 1;
							while(mipEnd < fullRange.MipLevelCount && isIncluded[fullRange.GetSubresourceIndex(mipEnd, layer, aspect)])
								mipEnd++;

							// Extend a range of the previous layer with the same mips
							bool isMerged = false;
							for(GpuTextureSubresourceRange& range : ranges)
							{
								if(range.BaseMipLevel != mip || range.MipLevelCount != mipEnd - mip)
									continue;

								if(range.AspectMask == GpuTextureAspectFlags(aspect) && range.BaseArrayLayer + range.ArrayLayerCount == layer)
								{
									range.ArrayLayerCount++;
									isMerged = true;
									break;
								}
							}

							if(!isMerged)
								ranges.Add(GpuTextureSubresourceRange(mip, mipEnd - mip, layer, 1, aspect));

							mip = mipEnd;
						}
					}
				}

				// Ranges of different aspects that cover the same mips and layers merge
				for(u32 rangeIndex = 0; rangeIndex < ranges.Size(); rangeIndex++)
				{
					for(u32 otherIndex = rangeIndex + 1; otherIndex < ranges.Size();)
					{
						GpuTextureSubresourceRange& range = ranges[rangeIndex];
						const GpuTextureSubresourceRange& other = ranges[otherIndex];
						if(range.BaseMipLevel == other.BaseMipLevel && range.MipLevelCount == other.MipLevelCount && range.BaseArrayLayer == other.BaseArrayLayer &&
							range.ArrayLayerCount == other.ArrayLayerCount)
						{
							range.AspectMask |= other.AspectMask;
							ranges.Erase(ranges.Begin() + otherIndex);
						}
						else
							otherIndex++;
					}
				}
			}

			for(const GpuTextureSubresourceRange& range : ranges)
			{
				if(resource.Buffer != nullptr)
					list.BufferBarriers.emplace_back(resource.Buffer, first.Source, first.Destination, first.Flags);
				else if(resource.Texture != nullptr)
					list.TextureBarriers.emplace_back(resource.Texture, first.Source, first.Destination, range, first.Flags);
				else
					list.RenderTargetBarriers.emplace_back(resource.Target, resource.Surface, first.Source, first.Destination, range, first.Flags);

				if(first.IsSplit)
				{
					mStatistics.SplitBarrierCount++;
					if(first.ReleaseQueueType != first.AcquireQueueType)
						mStatistics.QueueTransferCount++;
				}
				else
					mStatistics.FullBarrierCount++;
			}

			runStart = runEnd;
		}

		// The submission that acquires a split barrier from another queue waits on that queue itself, as the backends require, even where
		// an earlier submission of its queue already orders it after the release
		for(const BarrierBatch& batch : mBarrierBatches)
		{
			if(batch.ReleaseQueueType == batch.AcquireQueueType)
				continue;

			Submission& submission = mSubmissions[batch.AcquirePassIndex == ~0u ? context.LastSubmissionIndex : GetCompiledPass(batch.AcquirePassIndex).SubmissionIndex];
			const GpuQueueMask releaseQueue = GpuQueueId(batch.ReleaseQueueType, 0);
			if((submission.SyncMask & releaseQueue).IsEmpty())
			{
				submission.SyncMask |= releaseQueue;
				mStatistics.QueueWaitCount++;
			}
		}
	}

#if B3D_BUILD_TYPE_DEVELOPMENT
	void FrameGraph::ValidateSchedule(const CompileContext& context)
	{
		// Every split barrier's release and every queue wait's source must be submitted before the work that depends on them, and in a
		// submission that work waits for
		const auto fnGetSubmissionIndex = [this, &context](i32 compiledIndex)
		{
			return compiledIndex == -1 ? context.FirstGraphicsSubmissionIndex : GetCompiledPass((u32)compiledIndex).SubmissionIndex;
		};

		for(const BarrierBatch& batch : mBarrierBatches)
		{
			const u32 releaseSubmissionIndex = batch.ReleasePassIndex == -1 ? batch.ReleaseSubmissionIndex : GetCompiledPass(batch.ReleasePassIndex).SubmissionIndex;
			const u32 acquireSubmissionIndex = batch.AcquirePassIndex == ~0u ? context.LastSubmissionIndex : GetCompiledPass(batch.AcquirePassIndex).SubmissionIndex;
			B3D_ENSURE_LOG(mTimeline.IsOrderedBefore(releaseSubmissionIndex, acquireSubmissionIndex),
				"Frame graph '{0}' releases a split barrier in submission {1}, but submission {2}, which acquires it, does not wait for it.", mName, releaseSubmissionIndex, acquireSubmissionIndex);
		}

		for(const std::pair<i32, u32>& edge : context.WaitEdges)
		{
			const u32 sourceSubmissionIndex = fnGetSubmissionIndex(edge.first);
			const u32 destinationSubmissionIndex = fnGetSubmissionIndex((i32)edge.second);
			B3D_ENSURE_LOG(mTimeline.IsOrderedBefore(sourceSubmissionIndex, destinationSubmissionIndex),
				"Frame graph '{0}' plans submission {1} to wait for submission {2}, but it does not.", mName, destinationSubmissionIndex, sourceSubmissionIndex);
		}
	}
#endif

	void FrameGraph::Execute()
	{
		B3D_ASSERT(mIsCompiled && !mIsExecuted && "A frame graph must be compiled before it is executed, and can only be executed once.");
		mIsExecuted = true;

#if B3D_BUILD_TYPE_DEVELOPMENT
		// The submissions of a graph must reach the queues without another graph's submissions between them, so executions must not overlap
		u32 executingGraphId = 0;
		const bool isExecutionRegistered = gExecutingFrameGraphId.compare_exchange_strong(executingGraphId, mId);
		B3D_ENSURE_LOG(isExecutionRegistered, "Frame graph '{0}' executes while another frame graph executes. Execute frame graphs one at a time, so their submissions do not interleave.",
			mName);
#endif

		const CpuProfilingScope graphCpuSample(mName.c_str());

		// Each submission records its passes on a command buffer of its own, in submit order. A graph of one submission names it after
		// the graph, and graphs of several name them after the graph, the queue and the submission's position on the queue.
		const u32 submissionCount = (u32)mSubmissions.size();
		Vector<String> names(submissionCount, mName);
		Vector<u32> queuePositions(submissionCount, 0);
		u32 queueSubmissionCounts[GQT_COUNT] = {};
		for(u32 submissionIndex = 0; submissionIndex < submissionCount; submissionIndex++)
		{
			const GpuQueueType queueType = mSubmissions[submissionIndex].QueueType;
			queuePositions[submissionIndex] = queueSubmissionCounts[queueType]++;
			if(submissionCount > 1)
				names[submissionIndex] += (queueType == GQT_COMPUTE ? " Compute " : " Graphics ") + ToString(queuePositions[submissionIndex]);
		}

		// Returns the command buffer pool at @p poolIndex among the pools of @p queueType
		FrameGraphContextData& contextData = GetContextData(mContext);
		const auto fnGetPool = [this, &contextData](GpuQueueType queueType, u32 poolIndex) -> GpuCommandBufferPool&
		{
			Vector<TShared<GpuCommandBufferPool>>& pools = contextData.CommandBufferPools[queueType];
			while((u32)pools.size() <= poolIndex)
				pools.push_back(mDevice.CreateGpuCommandBufferPool(GpuCommandBufferPoolCreateInformation::CreateForThisThread(queueType)));

			return *pools[poolIndex];
		};

		bool isRecordedInParallel = mFlags.IsSet(FrameGraphFlag::RecordInParallel) && submissionCount > 1;
#if B3D_BUILD_TYPE_DEVELOPMENT
		isRecordedInParallel &= !gFrameGraphSerialRecording;
#endif

		Vector<TShared<GpuCommandBuffer>> commandBuffers(submissionCount);
#if B3D_BUILD_TYPE_DEVELOPMENT
		TUnique<GpuBarrierValidator> validator;
#endif
		if(!isRecordedInParallel)
		{
#if B3D_BUILD_TYPE_DEVELOPMENT
			// One validator follows every resource across the passes, of every command buffer. It keeps the parameter sets it saw alive until
			// it is destroyed.
			validator = B3DMakeUnique<GpuBarrierValidator>();
			for(const Resource& resource : mResources)
			{
				if(resource.Type != ResourceType::Transient || resource.IsExtracted || resource.FirstPassIndex == -1)
					continue;

				if(resource.Texture != nullptr && resource.Texture->GetGpuResource() != nullptr)
					validator->DeclareAliasAcquire(*resource.Texture, resource.Placement.Acquire);
				else if(resource.Buffer != nullptr && resource.Buffer->GetGpuResource() != nullptr)
					validator->DeclareAliasAcquire(*resource.Buffer, resource.Placement.Acquire);
			}
#endif

			for(u32 submissionIndex = 0; submissionIndex < submissionCount; submissionIndex++)
			{
				const Submission& submission = mSubmissions[submissionIndex];
				const TShared<GpuCommandBuffer>& commandBuffer = commandBuffers[submissionIndex] = fnGetPool(submission.QueueType, 0).FindOrCreate(
					GpuCommandBufferCreateInformation::Create(names[submissionIndex]));

#if B3D_BUILD_TYPE_DEVELOPMENT
				commandBuffer->SetValidator(validator.get());
#endif

				RecordSubmission(submission, *commandBuffer, names[submissionIndex]);

#if B3D_BUILD_TYPE_DEVELOPMENT
				// The validator is destroyed below, while the command buffer waits for its reset
				commandBuffer->SetValidator(nullptr);
#endif
			}
		}
		else
		{
			// Returns the attachments of the render pass of @p pass, as parameter sets resolve layouts against them. The surfaces of a window
			// are left out, as no parameter set can hold them.
			const auto fnGetRenderPassAttachments = [](const Pass& pass)
			{
				GpuRenderPassAttachmentUsageArray output;

				const TShared<RenderTarget>& target = pass.Attachments.Target;
				if(pass.Type != PassType::Raster || target == nullptr || target->GetProperties().IsWindow)
					return output;

				const RenderSurfaceMask readOnlyMask = pass.Attachments.ReadOnlyMask;
				const auto fnAddAttachment = [&output, readOnlyMask](const RenderSurfaceInformation& surfaceInformation, RenderSurfaceMaskBits surface, GpuTextureAspectFlag aspect)
				{
					IGpuImageResource* const image = static_cast<IGpuImageResource*>(GetSurfaceGpuResource(surfaceInformation));
					if(image == nullptr || !image->GetRange().AspectMask.IsSet(aspect))
						return;

					const u32 faceCount = surfaceInformation.FaceCount != 0 ? surfaceInformation.FaceCount : image->GetRange().ArrayLayerCount - surfaceInformation.Face;

					GpuRenderPassAttachmentUsage usage;
					usage.Image = image;
					usage.Range = GpuTextureSubresourceRange(surfaceInformation.MipLevel, 1, surfaceInformation.Face, faceCount, aspect);
					usage.Surface = surface;
					usage.Access = readOnlyMask.IsSet(surface) ? GpuAccessFlags(GpuAccessFlag::Read) : GpuAccessFlag::Read | GpuAccessFlag::Write;
					usage.Layout = GpuBackendUtility::GetRenderPassAttachmentLayout(surface, readOnlyMask);
					output.Add(usage);
				};

				// Render textures the graph built from the attachments of the pass describe them as well
				const RenderTexture& renderTexture = static_cast<const RenderTexture&>(*target);
				for(u32 colorIndex = 0; colorIndex < B3D_MAXIMUM_RENDER_TARGET_COUNT; colorIndex++)
					fnAddAttachment(renderTexture.GetColorSurfaceInformation(colorIndex), (RenderSurfaceMaskBits)(RT_COLOR0 << colorIndex), GpuTextureAspectFlag::Color);

				fnAddAttachment(renderTexture.GetDepthStencilSurfaceInformation(), RT_DEPTH, GpuTextureAspectFlag::Depth);
				fnAddAttachment(renderTexture.GetDepthStencilSurfaceInformation(), RT_STENCIL, GpuTextureAspectFlag::Stencil);
				return output;
			};

			// Parameter sets are only written on the thread that owns them, and bound read-only on other threads, so the sets of the passes
			// are flushed here, for the render passes that bind them. A set holds one layout per sampled texture, so every pass that binds
			// it must sample its textures in the layouts of the first.
			struct FlushedParameterSet
			{
				const Pass* FlushingPass = nullptr;
				GpuRenderPassAttachmentUsageArray Attachments;
			};

			UnorderedMap<const GpuParameterSet*, FlushedParameterSet> flushedParameterSets;
			for(u32 compiledIndex = 0; compiledIndex < (u32)mCompiledPassIndices.size(); compiledIndex++)
			{
				const Pass& pass = GetCompiledPass(compiledIndex);
				const GpuRenderPassAttachmentUsageArray attachments = fnGetRenderPassAttachments(pass);
				for(const TShared<GpuParameterSet>& parameters : pass.ParameterSets)
				{
					const auto [found, isFirst] = flushedParameterSets.try_emplace(parameters.get());
					if(isFirst)
					{
						found->second.FlushingPass = &pass;
						found->second.Attachments = attachments;

						parameters->Flush(attachments);
						continue;
					}

#if B3D_BUILD_TYPE_DEVELOPMENT
					const FlushedParameterSet& flushed = found->second;
					parameters->ForEachBoundResource(nullptr, [this, &pass, &flushed, &attachments](const GpuParameterSetResource& bound)
					{
						IGpuImageResource* const image = bound.Texture != nullptr ? static_cast<IGpuImageResource*>(bound.Texture->GetGpuResource()) : nullptr;
						if(image == nullptr || bound.Uniform->Type != GpuParameterType::SampledTexture)
							return;

						// Sampled views of depth-stencil textures select the depth aspect
						GpuTextureSubresourceRange range = image->GetRange(bound.Surface);
						if(range.AspectMask.IsSet(GpuTextureAspectFlag::Depth))
							range.AspectMask = GpuTextureAspectFlag::Depth;

						const GpuImageLayout flushedLayout = GpuBackendUtility::ResolveShaderImageLayout(flushed.Attachments, image, range, GpuImageLayout::Undefined);
						const GpuImageLayout layout = GpuBackendUtility::ResolveShaderImageLayout(attachments, image, range, GpuImageLayout::Undefined);
						B3D_ENSURE_LOG(layout == flushedLayout, "Frame graph '{0}' records in parallel, but passes '{1}' and '{2}' bind one parameter set that samples "
							"texture '{3}' in different layouts, as only one of them renders into it. Give the passes a parameter set each.", mName,
							flushed.FlushingPass->Name, pass.Name, bound.Texture->GetName());
					});
#endif
				}
			}

			// Submissions that use a window are recorded on this thread, as only the thread that owns a window may use its swap chain
			const auto fnUsesWindow = [this](const Submission& submission)
			{
				const auto fnHasWindowBarriers = [](const BarrierList& list) { return !list.RenderTargetBarriers.empty(); };
				const auto fnHasWindowBatches = [this, &fnHasWindowBarriers](TArrayView<const u32> batchIndices)
				{
					return std::any_of(batchIndices.begin(), batchIndices.end(), [this, &fnHasWindowBarriers](u32 batchIndex) { return fnHasWindowBarriers(mBarrierBatches[batchIndex].Barriers); });
				};

				if(fnHasWindowBatches(submission.StartBarrierBatchIndices) || fnHasWindowBatches(submission.EndBarrierBatchIndices) || fnHasWindowBarriers(submission.EndBarriers))
					return true;

				return std::any_of(submission.PassIndices.begin(), submission.PassIndices.end(), [this, &fnHasWindowBarriers, &fnHasWindowBatches](u32 compiledIndex)
				{
					const Pass& pass = mPasses[mCompiledPassIndices[compiledIndex]];
					return (pass.Attachments.Target != nullptr && pass.Attachments.Target->GetProperties().IsWindow) || fnHasWindowBarriers(pass.AliasAcquireBarriers) ||
						fnHasWindowBatches(pass.AcquiredBarrierBatchIndices) || fnHasWindowBarriers(pass.Barriers) || fnHasWindowBatches(pass.ReleasedBarrierBatchIndices) ||
						fnHasWindowBarriers(pass.ExportBarriers);
				});
			};

			// Every submission records on a pool of its own, as a pool records one command buffer at a time. A task takes the pool over to
			// record the submission, and this thread takes it back once every task completed.
			WaitGroup waitGroup;
			FrameVector<u32> windowSubmissionIndices;
			for(u32 submissionIndex = 0; submissionIndex < submissionCount; submissionIndex++)
			{
				const Submission& submission = mSubmissions[submissionIndex];
				if(fnUsesWindow(submission))
				{
					windowSubmissionIndices.push_back(submissionIndex);
					continue;
				}

				GpuCommandBufferPool& pool = fnGetPool(submission.QueueType, queuePositions[submissionIndex]);
				waitGroup.Increment();
				GetApplication().GetTaskScheduler().Post(SchedulerTask([this, &pool, &submission, &commandBuffers, &names, &waitGroup, submissionIndex]()
				{
					pool.SetThisAsOwnerThread();

#if B3D_BUILD_TYPE_DEVELOPMENT
					B3D_ENSURE_LOG(!gIsRecordingSubmission, "Frame graph '{0}' records two submissions on one thread at once. The execute functions of the passes of "
						"frame graphs that record in parallel must not yield.", mName);
					gIsRecordingSubmission = true;
#endif

					// The thread may not be sampled, as it runs tasks of any kind
					ProfilerCPU& profiler = GetProfilerCPU();
					const bool isThreadSampled = profiler.IsThreadSampled();
					if(!isThreadSampled)
						profiler.BeginThread("FrameGraphRecording");

					commandBuffers[submissionIndex] = pool.FindOrCreate(GpuCommandBufferCreateInformation::Create(names[submissionIndex]));
					RecordSubmission(submission, *commandBuffers[submissionIndex], names[submissionIndex]);

					if(!isThreadSampled)
						profiler.EndThread();

#if B3D_BUILD_TYPE_DEVELOPMENT
					gIsRecordingSubmission = false;
#endif

					waitGroup.NotifyDone();
				}, "FrameGraphRecording"));
			}

			for(u32 submissionIndex : windowSubmissionIndices)
			{
				const Submission& submission = mSubmissions[submissionIndex];
				commandBuffers[submissionIndex] = fnGetPool(submission.QueueType, queuePositions[submissionIndex]).FindOrCreate(GpuCommandBufferCreateInformation::Create(names[submissionIndex]));
				RecordSubmission(submission, *commandBuffers[submissionIndex], names[submissionIndex]);
			}

			waitGroup.Wait();

			for(u32 submissionIndex = 0; submissionIndex < submissionCount; submissionIndex++)
				fnGetPool(mSubmissions[submissionIndex].QueueType, queuePositions[submissionIndex]).SetThisAsOwnerThread();
		}

		// Submissions follow the timeline, and the last one signals the fences
		for(u32 submissionIndex = 0; submissionIndex < submissionCount; submissionIndex++)
		{
			GpuSubmissionInformation submissionInformation;
			submissionInformation.CommandBuffer = commandBuffers[submissionIndex];
			submissionInformation.SyncMask = mSubmissions[submissionIndex].SyncMask;
			if(submissionIndex == submissionCount - 1)
			{
				for(const GpuTimelineFenceAndValue& signalFence : mSignalFences)
					submissionInformation.SignalFences.Add(signalFence);
			}

			mContext.SubmitCommandBuffer(submissionInformation);
		}

		mAllocator.EndScope();
		mScope = nullptr;

		ReleaseResources();

#if B3D_BUILD_TYPE_DEVELOPMENT
		if(isExecutionRegistered)
			gExecutingFrameGraphId.store(0);
#endif

		for(Function<void()>& callback : mExecuteCallbacks)
			callback();

		mExecuteCallbacks.clear();
	}

	void FrameGraph::RecordSubmission(const Submission& submission, GpuCommandBuffer& commandBuffer, const String& name)
	{
#if B3D_PROFILING_ENABLED
		commandBuffer.BeginProfiling(ProfilerString(name.data(), name.size()));
#endif

		{
			const GpuProfilingScope commandBufferGpuSample(commandBuffer, ProfilerString(name.data(), name.size()));

			for(u32 batchIndex : submission.StartBarrierBatchIndices)
				RecordBarriers(commandBuffer, mBarrierBatches[batchIndex].Barriers, GpuBarrierPhase::Release, mBarrierBatches[batchIndex].SplitBarrier);

			for(u32 compiledIndex : submission.PassIndices)
			{
				Pass& pass = mPasses[mCompiledPassIndices[compiledIndex]];
				commandBuffer.BeginLabel(pass.Name);

				const CpuProfilingScope passCpuSample(pass.Name.c_str());
				const GpuProfilingScope passGpuSample(commandBuffer, ProfilerString(pass.Name.data(), pass.Name.size()));

				RecordBarriers(commandBuffer, pass.AliasAcquireBarriers, GpuBarrierPhase::Full);
				for(u32 batchIndex : pass.AcquiredBarrierBatchIndices)
					RecordBarriers(commandBuffer, mBarrierBatches[batchIndex].Barriers, GpuBarrierPhase::Acquire, mBarrierBatches[batchIndex].SplitBarrier);

				RecordBarriers(commandBuffer, pass.Barriers, GpuBarrierPhase::Full);

				const bool isRaster = pass.Type == PassType::Raster && pass.Attachments.Target != nullptr;
				if(isRaster)
				{
					RenderPassCreateInformation renderPassInformation(pass.Attachments.Target, pass.Attachments.ReadOnlyMask, pass.Attachments.LoadMask);
					renderPassInformation.ClearMask = pass.Attachments.ClearMask;
					commandBuffer.BeginRenderPass(renderPassInformation);
				}

				if(pass.Execute)
				{
					FrameGraphPassContext passContext(commandBuffer);
					pass.Execute(passContext);
				}

				if(isRaster)
					commandBuffer.EndRenderPass();

				for(u32 batchIndex : pass.ReleasedBarrierBatchIndices)
					RecordBarriers(commandBuffer, mBarrierBatches[batchIndex].Barriers, GpuBarrierPhase::Release, mBarrierBatches[batchIndex].SplitBarrier);

				RecordBarriers(commandBuffer, pass.ExportBarriers, GpuBarrierPhase::Full);
				commandBuffer.EndLabel();
			}

			for(u32 batchIndex : submission.EndBarrierBatchIndices)
				RecordBarriers(commandBuffer, mBarrierBatches[batchIndex].Barriers, GpuBarrierPhase::Acquire, mBarrierBatches[batchIndex].SplitBarrier);

			RecordBarriers(commandBuffer, submission.EndBarriers, GpuBarrierPhase::Full);
		}

#if B3D_PROFILING_ENABLED
		commandBuffer.EndProfiling();
#endif
	}

	void FrameGraph::AddExecuteCallback(Function<void()> callback)
	{
		B3D_ASSERT(!mIsExecuted && "Execute callbacks cannot be added after Execute().");
		mExecuteCallbacks.push_back(std::move(callback));
	}

	void FrameGraph::AddSignalFence(const TShared<GpuTimelineFence>& fence, u64 value)
	{
		B3D_ASSERT(!mIsExecuted && "Signal fences cannot be added after Execute().");
		mSignalFences.Add(GpuTimelineFenceAndValue{ fence, value });
	}

	bool FrameGraph::IsPassCulled(u32 passIndex) const
	{
		B3D_ASSERT(mIsCompiled && passIndex < (u32)mPasses.size());
		return mPasses[passIndex].IsCulled;
	}

#if B3D_BUILD_TYPE_DEVELOPMENT
	bool FrameGraph::IsCreatingRenderTexture()
	{
		return gIsCreatingRenderTexture;
	}
#endif

	u32 FrameGraph::AddResource(Resource&& resource)
	{
		if(resource.Buffer == nullptr)
			resource.SubresourceCount = resource.FullRange.GetSubresourceCount();

		const u32 resourceIndex = (u32)mResources.size();
		RegisterResource(resource, resourceIndex);

		mResources.push_back(std::move(resource));
		return resourceIndex;
	}

	void FrameGraph::RegisterResource(const Resource& resource, u32 resourceIndex) const
	{
		if(resource.Texture != nullptr)
			resource.Texture->SetFrameGraphResourceIndex(resourceIndex);
		else if(resource.Buffer != nullptr)
			resource.Buffer->SetFrameGraphResourceIndex(resourceIndex);

#if B3D_BUILD_TYPE_DEVELOPMENT
		const u32 graphId = resourceIndex != ~0u ? mId : 0;
		if(resource.Texture != nullptr)
			resource.Texture->SetFrameGraphId(graphId);
		else if(resource.Buffer != nullptr)
			resource.Buffer->SetFrameGraphId(graphId);
#endif
	}

	u32 FrameGraph::FindResource(const Texture& texture) const
	{
#if B3D_BUILD_TYPE_DEVELOPMENT
		if(texture.GetFrameGraphId() != mId)
			return ~0u;
#endif

		return texture.GetFrameGraphResourceIndex();
	}

	u32 FrameGraph::FindResource(const GpuBuffer& buffer) const
	{
#if B3D_BUILD_TYPE_DEVELOPMENT
		if(buffer.GetFrameGraphId() != mId)
			return ~0u;
#endif

		return buffer.GetFrameGraphResourceIndex();
	}

	FrameGraph::Pass& FrameGraph::AddPass(const StringView& name, PassType type, FrameGraphPass& pass, Function<void(FrameGraphPassContext&)>&& execute)
	{
		B3D_ASSERT(!mIsCompiled && "Passes cannot be added after Compile().");

		const u32 passIndex = (u32)mPasses.size();
		mPasses.emplace_back();

		Pass& output = mPasses.back();
		output.Name = name;
		output.Type = type;
		output.Flags = pass.mFlags;
		output.Execute = std::move(execute);

		// Raster passes need the graphics queue
		if(type == PassType::Raster && !B3D_ENSURE_LOG(!output.Flags.IsSet(FrameGraphPassFlag::AsyncCompute),
			"Raster pass '{0}' is created with FrameGraphPassFlag::AsyncCompute, which raster passes do not support. It runs on the graphics queue.", output.Name))
		{
			output.Flags.Unset(FrameGraphPassFlag::AsyncCompute);
		}
		output.ParameterSets = std::move(pass.mParameterSets);

		// Every graph resource in a slot of the pass's sets is declared. The shader stages come from the slot.
		for(const TShared<GpuParameterSet>& parameters : output.ParameterSets)
		{
			parameters->ForEachBoundResource(nullptr, [this, &output](const GpuParameterSetResource& bound)
			{
				const u32 resourceIndex = bound.Texture != nullptr ? bound.Texture->GetFrameGraphResourceIndex() : bound.Buffer->GetFrameGraphResourceIndex();
				const StringView resourceName = bound.Texture != nullptr ? StringView(bound.Texture->GetName()) : StringView(bound.Buffer->GetName());
				if(resourceIndex == ~0u)
				{
					// Resources outside of the graph are at rest, which only allows reads
					B3D_ENSURE_LOG(!bound.Access.IsSet(GpuAccessFlag::Write), "Pass '{0}' writes '{1}' through slot '{2}', but it is not a resource of the graph. Import it first.",
						output.Name, resourceName, bound.Uniform->Name);

					// Textures that cannot be read on several queues at once rest on the graphics queue. The check uses the preference, so content
					// behaves the same whether or not gFrameGraphAsyncCompute moves the pass to the compute queue.
					if(output.Flags.IsSet(FrameGraphPassFlag::AsyncCompute) && bound.Texture != nullptr)
					{
						B3D_ENSURE_LOG(GpuBackendUtility::IsConcurrentlyReadable(bound.Texture->GetProperties().Usage),
							"Pass '{0}' reads '{1}' through slot '{2}' on the async compute queue, but '{1}' rests on the graphics queue. Import it first.",
							output.Name, resourceName, bound.Uniform->Name);
					}

					return;
				}

#if B3D_BUILD_TYPE_DEVELOPMENT
				const u32 graphId = bound.Texture != nullptr ? bound.Texture->GetFrameGraphId() : bound.Buffer->GetFrameGraphId();
				if(!B3D_ENSURE_LOG(graphId == mId, "Pass '{0}' binds '{1}' in slot '{2}', which is a resource of another frame graph.", output.Name, resourceName, bound.Uniform->Name))
					return;
#endif

				const Resource& resource = mResources[resourceIndex];
				const GpuResourceUseFlags stageUse = GpuBackendUtility::GetShaderResourceUseFlags(bound.Uniform->Usage);

				switch(bound.Uniform->Type)
				{
				case GpuParameterType::UniformBuffer:
					DeclareAccess(output, resourceIndex, GpuTextureSubresourceRange(), GpuAccessState(GpuBackendUtility::GetStageFlags(stageUse | GpuResourceUseFlag::UniformBuffer), bound.Access));
					break;
				case GpuParameterType::StorageBuffer:
					DeclareAccess(output, resourceIndex, GpuTextureSubresourceRange(), GpuAccessState(GpuBackendUtility::GetStageFlags(stageUse | GpuResourceUseFlag::ShaderAccess), bound.Access));
					break;
				case GpuParameterType::SampledTexture:
				{
					// Sampled depth/stencil views only read the depth aspect
					GpuTextureSubresourceRange range = GpuBackendUtility::GetSurfaceRange(resource.FullRange, bound.Surface);
					if(range.AspectMask.IsSet(GpuTextureAspectFlag::Depth))
						range.AspectMask = GpuTextureAspectFlag::Depth;

					DeclareAccess(output, resourceIndex, range, GpuAccessState(GpuBackendUtility::GetStageFlags(stageUse | GpuResourceUseFlag::ShaderAccess), bound.Access, GpuImageLayout::ShaderReadOnly));
					break;
				}
				case GpuParameterType::StorageTexture:
				{
					const GpuTextureSubresourceRange range = GpuBackendUtility::GetSurfaceRange(resource.FullRange, bound.Surface);
					DeclareAccess(output, resourceIndex, range, GpuAccessState(GpuBackendUtility::GetStageFlags(stageUse | GpuResourceUseFlag::ShaderAccess), bound.Access, GpuImageLayout::General));
					break;
				}
				default:
					break;
				}
			});
		}

		const auto fnExport = [this, &output, passIndex](u32 resourceIndex)
		{
			if(!B3D_ENSURE_LOG(resourceIndex != ~0u, "Pass '{0}' exports a resource that is not a resource of the graph.", output.Name))
				return;

			Resource& resource = mResources[resourceIndex];
			if(!B3D_ENSURE_LOG(!resource.ExportPassIndex.has_value(), "Pass '{0}' exports a resource that an earlier pass already exported.", output.Name))
				return;

			resource.ExportPassIndex = passIndex;
			output.ExportedResourceIndices.Add(resourceIndex);
		};

		for(const TShared<Texture>& texture : pass.mExportedTextures)
			fnExport(FindResource(*texture));

		for(const TShared<GpuBuffer>& buffer : pass.mExportedBuffers)
			fnExport(FindResource(*buffer));

		return output;
	}

	void FrameGraph::DeclareAccess(Pass& pass, u32 resourceIndex, const GpuTextureSubresourceRange& range, const GpuAccessState& state, bool discardsContents)
	{
		const Resource& resource = mResources[resourceIndex];

		const StringView resourceName = resource.Texture != nullptr ? StringView(resource.Texture->GetName()) : resource.Buffer != nullptr ? StringView(resource.Buffer->GetName()) : StringView("Window");

#if B3D_BUILD_TYPE_DEVELOPMENT
		// Readers of an exported resource rely on its resting state
		const u32 passIndex = (u32)(&pass - mPasses.data());
		B3D_ENSURE_LOG(!state.Access.IsSet(GpuAccessFlag::Write) || !resource.ExportPassIndex.has_value() || *resource.ExportPassIndex >= passIndex,
			"Pass '{0}' writes '{1}' after it was exported.", pass.Name, resourceName);
#endif

		const auto fnDeclareSubresource = [&](u32 subresourceIndex)
		{
			const auto position = std::lower_bound(pass.Accesses.begin(), pass.Accesses.end(), std::make_pair(resourceIndex, subresourceIndex), [](const SubresourceAccess& access, const std::pair<u32, u32>& key)
			{
				return access.ResourceIndex < key.first || (access.ResourceIndex == key.first && access.SubresourceIndex < key.second);
			});

			if(position == pass.Accesses.end() || position->ResourceIndex != resourceIndex || position->SubresourceIndex != subresourceIndex)
			{
				SubresourceAccess access;
				access.ResourceIndex = resourceIndex;
				access.SubresourceIndex = subresourceIndex;
				access.State = state;
				access.DiscardsContents = discardsContents;
				pass.Accesses.insert(position, access);
				return;
			}

			// A read-only depth attachment may also be sampled, in the attachment's layout
			SubresourceAccess& existing = *position;
			GpuImageLayout layout = existing.State.Layout;
			if(layout != state.Layout)
			{
				const auto fnIsReadOnlyDepthLayout = [](GpuImageLayout candidate)
				{
					return candidate == GpuImageLayout::DepthStencilReadOnly || candidate == GpuImageLayout::DepthReadOnlyStencilAttachment;
				};

				const bool isReadOnly = !existing.State.Access.IsSet(GpuAccessFlag::Write) && !state.Access.IsSet(GpuAccessFlag::Write);
				const bool isSampledAttachment = (fnIsReadOnlyDepthLayout(existing.State.Layout) && state.Layout == GpuImageLayout::ShaderReadOnly) ||
					(existing.State.Layout == GpuImageLayout::ShaderReadOnly && fnIsReadOnlyDepthLayout(state.Layout));

				if(isReadOnly && isSampledAttachment)
					layout = existing.State.Layout == GpuImageLayout::ShaderReadOnly ? state.Layout : existing.State.Layout;
				else
				{
					B3D_ENSURE_LOG(false, "Pass '{0}' accesses the same subresource of '{1}' in two layouts. No barrier can separate them, so split the pass.", pass.Name, resourceName);
				}
			}

			// The contents are only discarded if no other access of the pass uses them
			existing.State.Stages |= state.Stages;
			existing.State.Access |= state.Access;
			existing.State.Layout = layout;
			existing.DiscardsContents &= discardsContents;
		};

		if(resource.Buffer != nullptr)
		{
			fnDeclareSubresource(0);
			return;
		}

		const GpuTextureSubresourceRange clampedRange = GpuBackendUtility::ClampRange(range, resource.FullRange);
		for(GpuTextureAspectFlag aspect : kGpuTextureAspects)
		{
			if(!clampedRange.AspectMask.IsSet(aspect))
				continue;

			for(u32 layer = clampedRange.BaseArrayLayer; layer < clampedRange.BaseArrayLayer + clampedRange.ArrayLayerCount; layer++)
			{
				for(u32 mip = clampedRange.BaseMipLevel; mip < clampedRange.BaseMipLevel + clampedRange.MipLevelCount; mip++)
					fnDeclareSubresource(resource.FullRange.GetSubresourceIndex(mip, layer, aspect));
			}
		}
	}

	void FrameGraph::RecordBarriers(GpuCommandBuffer& commandBuffer, const BarrierList& list, GpuBarrierPhase phase, const TShared<GpuSplitBarrier>& split)
	{
		if(list.IsEmpty())
			return;

		switch(phase)
		{
		case GpuBarrierPhase::Full:
			commandBuffer.IssueBarriers(list.GetBarriers());
			break;
		case GpuBarrierPhase::Release:
			commandBuffer.ReleaseBarriers(list.GetBarriers(), split);
			break;
		case GpuBarrierPhase::Acquire:
			commandBuffer.AcquireBarriers(list.GetBarriers(), split);
			break;
		}
	}

	void FrameGraph::ReleaseResources()
	{
		for(Resource& resource : mResources)
		{
			if(resource.IsHeldUnplaced)
			{
				if(resource.Texture != nullptr)
					mAllocator.Return(*resource.Texture);
				else
					mAllocator.Return(*resource.Buffer);

				resource.IsHeldUnplaced = false;
			}

			RegisterResource(resource, ~0u);
		}
	}
} // namespace b3d::render
