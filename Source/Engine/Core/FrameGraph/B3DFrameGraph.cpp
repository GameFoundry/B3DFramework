//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "FrameGraph/B3DFrameGraph.h"

#include "GpuBackend/Allocators/B3DGpuTransientHeapPool.h"
#include "GpuBackend/B3DGpuBackendUtility.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/B3DGpuDeviceCapabilities.h"
#include "GpuBackend/B3DGpuExplicitBarrierValidator.h"
#include "GpuBackend/B3DGpuParameterSet.h"
#include "GpuBackend/B3DGpuWorkContext.h"
#include "GpuBackend/B3DRenderTexture.h"
#include "GpuBackend/B3DRenderWindow.h"
#include "Profiling/B3DProfilerCPU.h"
#include "Profiling/B3DProfilerGPU.h"

namespace b3d
{
#if B3D_BUILD_TYPE_DEVELOPMENT
	TConfigVariable<bool> gFrameGraphSerialRecording("gpu.FrameGraphSerialRecording",
		"If enabled, frame graphs record their passes one at a time, in GPU execution order, and validate them with one shared explicit barrier validator.", true);
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

		/** Returns a new frame graph id. */
		u32 CreateFrameGraphId()
		{
			u32 id = gNextFrameGraphId.fetch_add(1, std::memory_order_relaxed);
			while(id == 0)
				id = gNextFrameGraphId.fetch_add(1, std::memory_order_relaxed);

			return id;
		}
#endif

		/** Command buffer pool and render textures that the frame graphs of one work context share. */
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

			TShared<GpuCommandBufferPool> CommandBufferPool;
			Vector<RenderTextureEntry> RenderTextures;
		};

		/** Key the context data is registered under. */
		const char kFrameGraphContextDataKey = 0;

		/** Returns the frame graph data of @p context, creating it on first use. */
		FrameGraphContextData& GetContextData(GpuWorkContext& context)
		{
			return context.GetOrCreateLocal<FrameGraphContextData>(&kFrameGraphContextDataKey, []() { return B3DMakeUnique<FrameGraphContextData>(); });
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

	void FrameGraphRasterPass::AddRenderPassParameterSet(const TShared<GpuParameterSet>& parameters)
	{
		if(parameters != nullptr)
			mRenderPassParameterSets.push_back(parameters);
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

	GpuExplicitBarriers FrameGraph::BarrierList::GetBarriers() const
	{
		return GpuExplicitBarriers(TArrayView<const GpuExplicitBufferBarrier>(BufferBarriers.data(), (u32)BufferBarriers.size()),
			TArrayView<const GpuExplicitTextureBarrier>(TextureBarriers.data(), (u32)TextureBarriers.size()),
			TArrayView<const GpuExplicitRenderTargetBarrier>(RenderTargetBarriers.data(), (u32)RenderTargetBarriers.size()));
	}

	FrameGraph::FrameGraph(GpuWorkContext& context, const StringView& name)
		: mContext(context), mDevice(context.GetDevice()), mAllocator(context.GetTransientResourceAllocator()), mName(name)
		, mExplicitBarriers(gGpuExplicitBarriers && mDevice.GetCapabilities().HasCapability(RSC_EXPLICIT_BARRIERS))
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
		output.RenderPassParameterSets = std::move(pass.mRenderPassParameterSets);
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

	void FrameGraph::Compile()
	{
		B3D_ASSERT(!mIsCompiled && "A frame graph can only be compiled once.");
		mIsCompiled = true;

		const CpuProfilingScope compileSample("FrameGraph::Compile");

		FrameAllocatorScope frameScope;

		mStatistics = FrameGraphStatistics();
		mStatistics.PassCount = (u32)mPasses.size();

		// The subresources of all resources are numbered one after another, so per-subresource state lives in flat arrays
		u32 subresourceCount = 0;
		for(Resource& resource : mResources)
		{
			resource.FirstSubresourceIndex = subresourceCount;
			subresourceCount += resource.SubresourceCount;

			if(resource.ExportPassIndex.has_value())
				B3D_ENSURE_LOG(resource.Type != ResourceType::Transient || resource.IsExtracted, "Only imported and extracted resources can be exported.");
		}

		// Culling. Walking backwards, a pass survives if it writes contents that a later surviving pass or the end of the graph observes.
		// The contents of imported and extracted resources are observed after the graph ends, and exported contents after their pass.
		{
			FrameVector<bool> isObserved(subresourceCount, false);
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

		// Execution order. Surviving passes keep their declaration order, and each resource learns its first and last surviving pass.
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

		// The whole graph is one submission on the graphics queue
		mTimeline.AddSubmission(GpuQueueId(GQT_GRAPHICS, 0), GpuQueueMask::kNone);
		mScope = &mAllocator.BeginScope(mTimeline);

		// Placement. Extracted transients leave the allocator for memory of their own, and transients that only culled passes use go back
		// unplaced. The rest are placed in pass order and released after their last pass, so transients whose passes do not overlap can
		// share memory. In automatic mode they get memory of their own too, because the resource tracker does not know of aliasing.
		{
			for(Resource& resource : mResources)
			{
				const bool hasOwnMemory = resource.IsExtracted || (!mExplicitBarriers && resource.FirstPassIndex != -1);
				if(resource.Type != ResourceType::Transient || !hasOwnMemory)
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

			const auto fnIsAliased = [this](const Resource& resource)
			{
				return mExplicitBarriers && resource.Type == ResourceType::Transient && !resource.IsExtracted;
			};

			// Last accesses of each subresource of the transients: the last write, and every read after it
			FrameVector<GpuAccessScope> lastUses(subresourceCount);
			for(u32 compiledIndex = 0; compiledIndex < (u32)mCompiledPassIndices.size(); compiledIndex++)
			{
				const Pass& pass = mPasses[mCompiledPassIndices[compiledIndex]];

				// Transients are placed before the transients of the same pass are released, so they never share memory
				for(const SubresourceAccess& access : pass.Accesses)
				{
					Resource& resource = mResources[access.ResourceIndex];
					if(!fnIsAliased(resource) || resource.FirstPassIndex != (i32)compiledIndex || !resource.IsHeldUnplaced)
						continue;

					resource.Placement = resource.Texture != nullptr ? mScope->PlaceTexture(*resource.Texture, 0) : mScope->PlaceBuffer(*resource.Buffer, 0);
					resource.IsHeldUnplaced = false;
				}

				for(const SubresourceAccess& access : pass.Accesses)
				{
					const Resource& resource = mResources[access.ResourceIndex];
					if(!fnIsAliased(resource))
						continue;

					GpuAccessScope& lastUse = lastUses[resource.FirstSubresourceIndex + access.SubresourceIndex];
					if(access.State.Access.IsSet(GpuAccessFlag::Write))
						lastUse = GpuAccessScope();

					lastUse.Add(access.State.Stages, access.State.Access);
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

					GpuAccessScope lastAccess;
					for(u32 subresourceIndex = 0; subresourceIndex < resource.SubresourceCount; subresourceIndex++)
						lastAccess.Add(lastUses[resource.FirstSubresourceIndex + subresourceIndex]);

					const GpuTransientLastUse lastUse(0, lastAccess, compiledIndex);
					if(resource.Texture != nullptr)
						mScope->Release(*resource.Texture, TArrayView<const GpuTransientLastUse>(&lastUse, 1));
					else
						mScope->Release(*resource.Buffer, TArrayView<const GpuTransientLastUse>(&lastUse, 1));
				}
			}
		}

		// Render textures. A raster pass whose attachments were set one by one renders through a render texture of those surfaces. The
		// work context keeps them across graphs, until a surface is recreated or the render texture goes unused for too long.
		// TODO - Evict render textures together with the transient textures they reference. The transient allocator evicts on its own
		// schedule, and only compiling a later graph on this context evicts render textures, so a render texture can keep an evicted
		// transient texture alive after its heap was released, indefinitely if the context builds no more graphs.
		{
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

				// Transients are new textures in every graph in automatic mode, so their render textures are not kept for later graphs
				const auto found = !mExplicitBarriers ? entries.end() : std::find_if(entries.begin(), entries.end(), [&information](const FrameGraphContextData::RenderTextureEntry& entry)
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
				if(mExplicitBarriers)
					entries.push_back(std::move(entry));
			}
		}

		// The resource tracker derives every barrier in automatic mode
		if(!mExplicitBarriers)
			return;

		// Barrier planning. Walking the surviving passes, each subresource tracks its current state, and the accesses since the barrier
		// that established it. An access joins the current state when it can, and otherwise plans a barrier of the subresource into the
		// access's state. Exported resources and, at the end of the graph, imported and extracted resources plan barriers back to rest.

		/** Barrier of one subresource, or of a whole resource, as the planner produces it. */
		struct PlannedBarrier
		{
			u32 ResourceIndex = 0; /**< Index into mResources. */
			u32 SubresourceIndex = 0; /**< Index relative to the resource's first subresource. Ignored if IsWholeResource is true. */
			bool IsWholeResource = false;
			GpuAccessState Source;
			GpuAccessState Destination;
			GpuBarrierFlags Flags;

			i32 ReleasePassIndex = -1; /** Index into mCompiledPassIndices of the pass the release goes after, or -1 for the start of the graph. AcquirePassIndex - 1 for full barriers. */

			u32 AcquirePassIndex = 0; /**< Index into mCompiledPassIndices of the pass the barrier precedes, or ~0u for the end of the graph. */
			bool IsSplit = false;
			bool IsExport = false; /**< True for a barrier to rest after the pass at ReleasePassIndex. */
		};

		/** Planning state of one subresource. */
		struct SubresourceState
		{
			bool HasState = false; /**< False until a transient's first access. */
			GpuAccessState State; /**< Destination of the last barrier, or the resting state of an import. */
			GpuStageFlags UsedStages = GpuStageFlag::None; /**< Stages that accessed the subresource since State was established. */
			GpuAccessFlags UsedAccess = GpuAccessFlag::None; /**< Accesses since State was established. */

			/** Index into mCompiledPassIndices of the last pass that accessed the subresource or acquired its state, or -1 for the start of the graph. */
			i32 LastPassIndex = -1;

			u32 BarrierIndex = ~0u; /**< Index into plannedBarriers of the barrier that established State, or ~0u if none did. */
		};

		// Imports start in their resting state. Transients have no state until their first access.
		FrameVector<SubresourceState> states(subresourceCount);
		for(const Resource& resource : mResources)
		{
			if(resource.Type == ResourceType::Transient)
				continue;

			for(u32 subresourceIndex = 0; subresourceIndex < resource.SubresourceCount; subresourceIndex++)
			{
				SubresourceState& state = states[resource.FirstSubresourceIndex + subresourceIndex];
				state.HasState = true;
				state.State = resource.RestingState;
			}
		}

		FrameVector<PlannedBarrier> plannedBarriers;

		// Plans a barrier of one subresource from its current state into @p destination, before the compiled pass at @p compiledIndex
		const auto fnPlanBarrier = [&](u32 resourceIndex, u32 subresourceIndex, const GpuAccessState& destination, u32 compiledIndex, bool discardsContents, bool isExport)
		{
			const Resource& resource = mResources[resourceIndex];
			SubresourceState& state = states[resource.FirstSubresourceIndex + subresourceIndex];

			// The source covers the accesses since the state was established. Without any, it chains to the barrier that established the
			// state. A resting state also covers the undeclared readers before the graph.
			PlannedBarrier barrier;
			barrier.ResourceIndex = resourceIndex;
			barrier.SubresourceIndex = subresourceIndex;
			barrier.Source.Stages = state.UsedStages;
			barrier.Source.Access = state.UsedAccess;
			barrier.Source.Layout = state.State.Layout;

			// No barrier was issued, or barrier was issued but resource was not used since
			if(state.BarrierIndex == ~0u || state.UsedStages == GpuStageFlag::None)
			{
				barrier.Source.Stages |= state.State.Stages; // Chain with prior barrier (if any), or on prior readers for imported resource
				if(state.BarrierIndex == ~0u) // State is still in import resting state, wait on prior readers (which are not known)
				{
					barrier.Source.Access |= state.State.Access;
					barrier.Source.Compression = state.State.Compression;
				}
			}

			if(discardsContents && resource.Buffer == nullptr)
				barrier.Source.Layout = GpuImageLayout::Undefined;

			barrier.Destination = destination;
			barrier.IsExport = isExport;
			if(isExport)
			{
				barrier.ReleasePassIndex = (i32)compiledIndex;
				barrier.AcquirePassIndex = compiledIndex + 1;
			}
			else if(compiledIndex == ~0u)
			{
				barrier.ReleasePassIndex = state.LastPassIndex;
				barrier.AcquirePassIndex = ~0u;
			}
			else
			{
				// Work between the last access and this one may overlap the barrier. Window surfaces are acquired by the barrier's command
				// buffer, so their barriers are never split.
				barrier.IsSplit = resource.Type != ResourceType::Surface && state.LastPassIndex < (i32)compiledIndex - 1;
				barrier.ReleasePassIndex = barrier.IsSplit ? state.LastPassIndex : (i32)compiledIndex - 1;
				barrier.AcquirePassIndex = compiledIndex;
			}

			state.State = destination;
			state.UsedStages = GpuStageFlag::None;
			state.UsedAccess = GpuAccessFlag::None;
			state.BarrierIndex = (u32)plannedBarriers.size();
			if(compiledIndex != ~0u)
				state.LastPassIndex = (i32)compiledIndex;

			plannedBarriers.push_back(barrier);
		};

		for(u32 compiledIndex = 0; compiledIndex < (u32)mCompiledPassIndices.size(); compiledIndex++)
		{
			const Pass& pass = mPasses[mCompiledPassIndices[compiledIndex]];
			for(const SubresourceAccess& access : pass.Accesses)
			{
				const Resource& resource = mResources[access.ResourceIndex];
				SubresourceState& state = states[resource.FirstSubresourceIndex + access.SubresourceIndex];

				if(!state.HasState)
				{
					// The first access of a transient starts its lifetime with one barrier over the whole resource, into the state of the
					// pass's first access of it. Transients in persistent memory have no earlier resources to wait on.
					const bool isAliased = !resource.IsExtracted;

					PlannedBarrier barrier;
					barrier.ResourceIndex = access.ResourceIndex;
					barrier.IsWholeResource = true;
					barrier.Destination = access.State;
					barrier.AcquirePassIndex = compiledIndex;
					barrier.ReleasePassIndex = (i32)compiledIndex - 1;
					if(isAliased)
					{
						// The alias acquire waits on the last accesses of the earlier resources on the transient's memory
						const GpuAliasAcquire& acquire = resource.Placement.Acquire;
						barrier.Source = GpuAccessState(acquire.Source.GetStages(), acquire.Source.GetAccess());
						barrier.Flags = GpuBarrierFlag::AliasAcquire;

						// The release discards the old contents while the passes between the earlier resources' last pass and this one run
						const TOptional<u32>& latestSourcePass = resource.Placement.LatestSourcePass;
						if(latestSourcePass.has_value() && (i32)*latestSourcePass < (i32)compiledIndex - 1)
						{
							barrier.IsSplit = true;
							barrier.ReleasePassIndex = (i32)*latestSourcePass;
						}
					}

					const u32 barrierIndex = (u32)plannedBarriers.size();
					plannedBarriers.push_back(barrier);

					for(u32 subresourceIndex = 0; subresourceIndex < resource.SubresourceCount; subresourceIndex++)
					{
						SubresourceState& subresourceState = states[resource.FirstSubresourceIndex + subresourceIndex];
						subresourceState.HasState = true;
						subresourceState.State = access.State;
						subresourceState.LastPassIndex = (i32)compiledIndex;
						subresourceState.BarrierIndex = barrierIndex;
					}

					if(isAliased)
					{
						mStatistics.AliasAcquireCount++;
						if(barrier.IsSplit)
							mStatistics.SplitAliasAcquireCount++;
					}
				}

				const GpuAccessState& wanted = access.State;
				const bool isRead = !wanted.Access.IsSet(GpuAccessFlag::Write);
				const bool isSameLayout = wanted.Layout == state.State.Layout;

				bool needsBarrier = true;
				if(isSameLayout && state.BarrierIndex != ~0u && state.UsedStages == GpuStageFlag::None)
				{
					// Nothing accessed the subresource since its last barrier, which can also cover this access
					needsBarrier = false;
				}
				else if(isSameLayout && isRead && !state.UsedAccess.IsSet(GpuAccessFlag::Write) && !state.State.Access.IsSet(GpuAccessFlag::Write))
				{
					// Reads in one layout share the read state. Stages the state lacks join the barrier that established it.
					needsBarrier = !state.State.Stages.IsSetAll(wanted.Stages) && state.BarrierIndex == ~0u;
				}

				if(!needsBarrier)
				{
					if(state.BarrierIndex != ~0u)
					{
						GpuAccessState& destination = plannedBarriers[state.BarrierIndex].Destination;
						destination.Stages |= wanted.Stages;
						destination.Access |= wanted.Access;
					}

					state.State.Stages |= wanted.Stages;
					state.State.Access |= wanted.Access;
				}
				else
					fnPlanBarrier(access.ResourceIndex, access.SubresourceIndex, wanted, compiledIndex, access.DiscardsContents, false);

				state.UsedStages |= wanted.Stages;
				state.UsedAccess |= wanted.Access;
				state.LastPassIndex = (i32)compiledIndex;
			}

			// Exported resources return to rest right after the pass
			for(u32 resourceIndex : pass.ExportedResourceIndices)
			{
				const Resource& resource = mResources[resourceIndex];
				for(u32 subresourceIndex = 0; subresourceIndex < resource.SubresourceCount; subresourceIndex++)
				{
					const SubresourceState& state = states[resource.FirstSubresourceIndex + subresourceIndex];
					if(state.HasState && state.BarrierIndex != ~0u)
						fnPlanBarrier(resourceIndex, subresourceIndex, resource.RestingState, compiledIndex, false, true);
				}
			}
		}

		// Imported and extracted resources end the graph at rest
		for(u32 resourceIndex = 0; resourceIndex < (u32)mResources.size(); resourceIndex++)
		{
			const Resource& resource = mResources[resourceIndex];
			if(resource.Type == ResourceType::Transient && !resource.IsExtracted)
				continue;

			const GpuAccessState& restingState = resource.RestingState;
			for(u32 subresourceIndex = 0; subresourceIndex < resource.SubresourceCount; subresourceIndex++)
			{
				SubresourceState& state = states[resource.FirstSubresourceIndex + subresourceIndex];
				if(!state.HasState) // Must be an extracted transient as imported resources start with state
				{
					// An extracted resource nothing accessed still has to reach its resting layout
					state.HasState = true;
					fnPlanBarrier(resourceIndex, subresourceIndex, restingState, ~0u, true, false);
					continue;
				}

				// No barrier was ever issued, or barrier was issued that didn't change the rest state
				const bool isAtRest = state.BarrierIndex == ~0u || (!state.UsedAccess.IsSet(GpuAccessFlag::Write) && !state.State.Access.IsSet(GpuAccessFlag::Write) &&
					state.State.Layout == restingState.Layout && state.State.Stages.IsSetAll(restingState.Stages));

				if(!isAtRest)
					fnPlanBarrier(resourceIndex, subresourceIndex, restingState, ~0u, false, false);
			}
		}

#if 1
		// Workaround for the Vulkan validation layer: if enabled, the depth and stencil aspects of an image use the same barrier kind. Sync
		// validation reports a false write-after-read hazard when one aspect of an image gets a split barrier while the other aspect gets a full
		// barrier.
		bool anyBarrierMadeFull = true;
		while(anyBarrierMadeFull)
		{
			anyBarrierMadeFull = false;
			for(PlannedBarrier& barrier : plannedBarriers)
			{
				const Resource& resource = mResources[barrier.ResourceIndex];
				if(!barrier.IsSplit || barrier.IsWholeResource || resource.Buffer != nullptr || resource.FullRange.GetAspectCount() != 2)
					continue;

				// Subresources are ordered by aspect, so the other aspect's subresource is half the subresource count away
				const u32 aspectSubresourceCount = resource.SubresourceCount / 2;
				const u32 otherSubresourceIndex = (barrier.SubresourceIndex + aspectSubresourceCount) % resource.SubresourceCount;

				const bool hasOverlappingFullBarrier = std::any_of(plannedBarriers.begin(), plannedBarriers.end(), [&barrier, otherSubresourceIndex](const PlannedBarrier& other)
				{
					return other.ResourceIndex == barrier.ResourceIndex && !other.IsWholeResource && !other.IsSplit && !other.IsExport &&
						other.SubresourceIndex == otherSubresourceIndex && other.AcquirePassIndex != ~0u &&
						(i32)other.AcquirePassIndex > barrier.ReleasePassIndex && other.AcquirePassIndex <= barrier.AcquirePassIndex;
				});

				if(hasOverlappingFullBarrier)
				{
					barrier.IsSplit = false;
					barrier.ReleasePassIndex = (i32)barrier.AcquirePassIndex - 1;
					anyBarrierMadeFull = true;
				}
			}
		}
#endif

		// Batching. Planned barriers that differ only in their subresource merge into subresource ranges. Each range goes to the list of
		// the call that records it: a pass's alias acquires or full barriers, the barrier batch of its split release and acquire, a pass's
		// export barriers, or the end of the graph.
		const auto fnKey = [](const PlannedBarrier& barrier)
		{
			return std::make_tuple(barrier.AcquirePassIndex, barrier.ReleasePassIndex, barrier.IsSplit, barrier.IsExport, barrier.ResourceIndex, barrier.IsWholeResource,
				(u32)barrier.Flags, (u32)barrier.Source.Stages, (u32)barrier.Source.Access, (u32)barrier.Source.Layout, (u32)barrier.Source.Compression,
				(u32)barrier.Destination.Stages, (u32)barrier.Destination.Access, (u32)barrier.Destination.Layout, (u32)barrier.Destination.Compression);
		};

		FrameVector<u32> order(plannedBarriers.size());
		for(u32 barrierIndex = 0; barrierIndex < (u32)order.size(); barrierIndex++)
			order[barrierIndex] = barrierIndex;

		std::stable_sort(order.begin(), order.end(), [&](u32 a, u32 b)
		{
			const auto keyA = fnKey(plannedBarriers[a]);
			const auto keyB = fnKey(plannedBarriers[b]);
			if(keyA != keyB)
				return keyA < keyB;

			return plannedBarriers[a].SubresourceIndex < plannedBarriers[b].SubresourceIndex;
		});

		// Returns the list that records barriers like @p barrier
		const auto fnGetList = [this](const PlannedBarrier& barrier) -> BarrierList&
		{
			if(barrier.AcquirePassIndex == ~0u)
				return mEndBarriers;

			if(barrier.IsExport)
				return mPasses[mCompiledPassIndices[barrier.ReleasePassIndex]].ExportBarriers;

			if(barrier.IsSplit)
			{
				const auto found = std::find_if(mBarrierBatches.begin(), mBarrierBatches.end(), [&barrier](const BarrierBatch& batch)
				{
					return batch.ReleasePassIndex == barrier.ReleasePassIndex && batch.AcquirePassIndex == barrier.AcquirePassIndex;
				});

				if(found != mBarrierBatches.end())
					return found->Barriers;

				const u32 batchIndex = (u32)mBarrierBatches.size();
				BarrierBatch& batch = mBarrierBatches.emplace_back();
				batch.ReleasePassIndex = barrier.ReleasePassIndex;
				batch.AcquirePassIndex = barrier.AcquirePassIndex;
				batch.SplitBarrier = mDevice.CreateSplitBarrier();

				if(barrier.ReleasePassIndex == -1)
					mStartBarrierBatchIndices.Add(batchIndex);
				else
					mPasses[mCompiledPassIndices[barrier.ReleasePassIndex]].ReleasedBarrierBatchIndices.Add(batchIndex);

				mPasses[mCompiledPassIndices[barrier.AcquirePassIndex]].AcquiredBarrierBatchIndices.Add(batchIndex);
				mStatistics.BarrierBatchCount++;
				return batch.Barriers;
			}

			Pass& pass = mPasses[mCompiledPassIndices[barrier.AcquirePassIndex]];
			return barrier.Flags.IsSet(GpuBarrierFlag::AliasAcquire) ? pass.AliasAcquireBarriers : pass.Barriers;
		};

		for(u32 runStart = 0; runStart < (u32)order.size();)
		{
			const PlannedBarrier& first = plannedBarriers[order[runStart]];
			const auto firstKey = fnKey(first);

			u32 runEnd = runStart + 1;
			while(runEnd < (u32)order.size() && fnKey(plannedBarriers[order[runEnd]]) == firstKey)
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
					isIncluded[plannedBarriers[order[orderIndex]].SubresourceIndex] = true;

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
					mStatistics.SplitBarrierCount++;
				else
					mStatistics.FullBarrierCount++;
			}

			runStart = runEnd;
		}
	}

	void FrameGraph::Execute()
	{
		B3D_ASSERT(mIsCompiled && !mIsExecuted && "A frame graph must be compiled before it is executed, and can only be executed once.");
		mIsExecuted = true;

		const CpuProfilingScope graphCpuSample(mName.c_str());

		// The barrier mode is set before start-up, so every graph of a work context records in the same mode
		TShared<GpuCommandBufferPool>& pool = GetContextData(mContext).CommandBufferPool;
		if(pool == nullptr)
		{
			GpuCommandBufferPoolCreateInformation poolInformation = GpuCommandBufferPoolCreateInformation::CreateForThisThread(GQT_GRAPHICS);
			poolInformation.ExplicitBarriers = mExplicitBarriers;

			pool = mDevice.CreateGpuCommandBufferPool(poolInformation);
		}

		B3D_ASSERT(pool->UsesExplicitBarriers() == mExplicitBarriers && "The barrier mode changed after a work context recorded its first frame graph.");

		const TShared<GpuCommandBuffer> commandBuffer = pool->FindOrCreate(GpuCommandBufferCreateInformation::Create(mName));

#if B3D_BUILD_TYPE_DEVELOPMENT
		// One validator follows every resource across the passes. It keeps the parameter sets it saw alive until it is destroyed.
		TUnique<GpuExplicitBarrierValidator> validator;
		if(mExplicitBarriers && gFrameGraphSerialRecording)
		{
			validator = B3DMakeUnique<GpuExplicitBarrierValidator>();
			commandBuffer->SetValidator(validator.get());

			for(const Resource& resource : mResources)
			{
				if(resource.Type != ResourceType::Transient || resource.IsExtracted || resource.FirstPassIndex == -1)
					continue;

				if(resource.Texture != nullptr && resource.Texture->GetGpuResource() != nullptr)
					validator->DeclareAliasAcquire(*resource.Texture, resource.Placement.Acquire);
				else if(resource.Buffer != nullptr && resource.Buffer->GetGpuResource() != nullptr)
					validator->DeclareAliasAcquire(*resource.Buffer, resource.Placement.Acquire);
			}
		}
#endif

#if B3D_PROFILING_ENABLED
		commandBuffer->BeginProfiling(ProfilerString(mName.data(), mName.size()));
#endif

		{
			const GpuProfilingScope graphGpuSample(*commandBuffer, ProfilerString(mName.data(), mName.size()));

			for(u32 batchIndex : mStartBarrierBatchIndices)
				RecordBarriers(*commandBuffer, mBarrierBatches[batchIndex].Barriers, GpuBarrierPhase::Release, mBarrierBatches[batchIndex].SplitBarrier);

			for(u32 passIndex : mCompiledPassIndices)
			{
				Pass& pass = mPasses[passIndex];
				commandBuffer->BeginLabel(pass.Name);

				const CpuProfilingScope passCpuSample(pass.Name.c_str());
				const GpuProfilingScope passGpuSample(*commandBuffer, ProfilerString(pass.Name.data(), pass.Name.size()));

				RecordBarriers(*commandBuffer, pass.AliasAcquireBarriers, GpuBarrierPhase::Full);
				for(u32 batchIndex : pass.AcquiredBarrierBatchIndices)
					RecordBarriers(*commandBuffer, mBarrierBatches[batchIndex].Barriers, GpuBarrierPhase::Acquire, mBarrierBatches[batchIndex].SplitBarrier);

				RecordBarriers(*commandBuffer, pass.Barriers, GpuBarrierPhase::Full);

				const bool isRaster = pass.Type == PassType::Raster && pass.Attachments.Target != nullptr;
				if(isRaster)
				{
					RenderPassCreateInformation renderPassInformation(pass.Attachments.Target, pass.Attachments.ReadOnlyMask, pass.Attachments.LoadMask);
					renderPassInformation.ClearMask = pass.Attachments.ClearMask;

					// Backends prepare the descriptors of every set a render pass binds when the render pass begins
					for(const TShared<GpuParameterSet>& parameters : pass.ParameterSets)
						renderPassInformation.Parameters.Add(parameters);

					for(const TShared<GpuParameterSet>& parameters : pass.RenderPassParameterSets)
						renderPassInformation.Parameters.Add(parameters);

					commandBuffer->BeginRenderPass(renderPassInformation);
				}

				if(pass.Execute)
				{
					FrameGraphPassContext passContext(*commandBuffer);
					pass.Execute(passContext);
				}

				if(isRaster)
					commandBuffer->EndRenderPass();

				for(u32 batchIndex : pass.ReleasedBarrierBatchIndices)
					RecordBarriers(*commandBuffer, mBarrierBatches[batchIndex].Barriers, GpuBarrierPhase::Release, mBarrierBatches[batchIndex].SplitBarrier);

				RecordBarriers(*commandBuffer, pass.ExportBarriers, GpuBarrierPhase::Full);
				commandBuffer->EndLabel();
			}

			RecordBarriers(*commandBuffer, mEndBarriers, GpuBarrierPhase::Full);
		}

#if B3D_PROFILING_ENABLED
		commandBuffer->EndProfiling();
#endif

#if B3D_BUILD_TYPE_DEVELOPMENT
		// The validator is destroyed below, while the command buffer waits for its reset
		if(validator != nullptr)
			commandBuffer->SetValidator(nullptr);
#endif

		// The first submission of a graph waits on every queue, which orders it after the uploads its resources depend on
		mContext.SubmitCommandBuffer(commandBuffer, GpuQueueMask::kAll);

		mAllocator.EndScope();
		mScope = nullptr;

		ReleaseResources();
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
		output.Execute = std::move(execute);
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
			commandBuffer.IssueExplicitBarriers(list.GetBarriers());
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
