//************************************ B3D Framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "VectorGraphics/B3DVectorSpriteAtlas.h"
#include "CoreObject/B3DRenderThread.h"
#include "GUI/B3DGUIManager.h"
#include "Image/B3DSpriteTexture.h"
#include "Image/B3DTexture.h"
#include "RenderAPI/B3DGpuBackend.h"
#include "RenderAPI/B3DGpuCommandBuffer.h"
#include "RenderAPI/B3DRenderTexture.h"
#include "Renderer/B3DRenderer.h"
#include "Renderer/B3DRendererManager.h"
#include "Renderer/B3DRendererUtility.h"
#include "Utility/B3DTime.h"

using namespace b3d;

static TreeTextureAtlasLayoutSettings GetGUIVectorSpriteAtlasSettings(u32 pageSize)
{
	TreeTextureAtlasLayoutSettings settings;
	settings.Size = Size2UI(pageSize, pageSize);
	settings.Alignment = Size2UI(32, 32);

	return settings;
}

GUIVectorSpriteAtlas::GUIVectorSpriteAtlas(const GUIVectorSpriteAtlasSettings& settings)
	: mAtlasLayout(TreeTextureAtlasLayoutSettings(GetGUIVectorSpriteAtlasSettings(settings.AtlasPageSize))), mSettings(settings)
{
	
}

GUIVectorSpriteAtlas::~GUIVectorSpriteAtlas()
{
	DestroyPendingReleasedAllocations();
}

GUIVectorSpriteAtlasAllocation GUIVectorSpriteAtlas::Allocate(const VectorPath& vectorPath, const VectorGraphicsSettings& settings)
{
	if(!EnsureMainThread())
		return GUIVectorSpriteAtlasAllocation();

	GUIVectorSpriteAtlasAllocationHandle::Key key(vectorPath, settings);

	{
		Lock lock(mAllocationsMutex);

		if(auto found = mAllocations.find(key); found != mAllocations.end())
		{
			const AllocationInformation& allocationInformation = found->second;

			return GUIVectorSpriteAtlasAllocation(allocationInformation.AtlasTexture, allocationInformation.UVRange, allocationInformation.AllocationHandle->shared_from_this());
		}
	}

	const Size2UI requestedSize = Size2UI((u32)settings.Size.Width, (u32)settings.Size.Height);
	const bool useUniqueTexture = requestedSize.Width >= mSettings.UniqueAllocationSize || requestedSize.Height >= mSettings.UniqueAllocationSize;

	const SPtr<render::VectorPathRenderable> renderable = vectorPath.CreateRenderable(settings);

	HTexture atlasTexture;
	Area2 uvRange;

	u32 textureId = ~0u;
	TOptional<TreeTextureAtlasLayout::Allocation> layoutAllocation;
	if(useUniqueTexture)
	{
		atlasTexture = CreateOrFindTexture(requestedSize);
		uvRange = Area2(0.0f, 0.0f, 1.0f, 1.0f);

		textureId = GetNextUniqueTextureId();
		mUniqueTextures[textureId] = atlasTexture;
	}
	else
	{
		layoutAllocation = mAtlasLayout.AddElement(requestedSize);
		if(!layoutAllocation)
			return GUIVectorSpriteAtlasAllocation();

		const Size2UI& atlasPageSize = mAtlasLayout.GetSize();

		if(auto found = mAtlasLayoutTextures.find(layoutAllocation->PageId); found == mAtlasLayoutTextures.end())
		{
			atlasTexture = CreateOrFindTexture(mAtlasLayout.GetSize());
			mAtlasLayoutTextures[layoutAllocation->PageId] = atlasTexture;
		}
		else
		{
			atlasTexture = found->second;
		}

		const Vector2 uvOffset(
			(float)layoutAllocation->Position.X / (float)atlasPageSize.Width,
			(float)layoutAllocation->Position.Y / (float)atlasPageSize.Height);

		const Size2 uvSize(
			(float)requestedSize.Width / (float)atlasPageSize.Width,
			(float)requestedSize.Height / (float)atlasPageSize.Height);

		uvRange = Area2(uvOffset, uvSize);
	}

	GUIVectorSpriteAtlasAllocationHandle* const allocationHandle = B3DNew<GUIVectorSpriteAtlasAllocationHandle>(this, key.VectorPathId, layoutAllocation, textureId, renderable);
	SPtr<GUIVectorSpriteAtlasAllocationHandle> allocationHandleShared = B3DMakeSharedFromExisting<GUIVectorSpriteAtlasAllocationHandle>(allocationHandle,
		[](GUIVectorSpriteAtlasAllocationHandle* allocationHandle)
		{
			GUIVectorSpriteAtlas* owner = allocationHandle->GetOwner();
			owner->NotifyAllocationReleased(allocationHandle);
	});

	{
		Lock lock(mAllocationsMutex);
		mAllocations[key] = AllocationInformation(atlasTexture, uvRange, allocationHandle);
	}

	DirtySpriteInformation dirtySpriteInformation;
	dirtySpriteInformation.Texture = B3DGetRenderProxy(atlasTexture);
	dirtySpriteInformation.Renderable = renderable;
	dirtySpriteInformation.UVRegion = uvRange;
	dirtySpriteInformation.Size = requestedSize;

	B3D_ASSERT(dirtySpriteInformation.Size.Width > 0 && dirtySpriteInformation.Size.Height > 0);

	mDirtySpriteBuffers[mDirtySpriteWriteBufferIndex].push_back(dirtySpriteInformation);
	return GUIVectorSpriteAtlasAllocation(atlasTexture, uvRange, allocationHandleShared);
}

void GUIVectorSpriteAtlas::NotifyAllocationReleased(GUIVectorSpriteAtlasAllocationHandle* allocationHandle)
{
	Lock lock(mAllocationsMutex);

	auto found = mAllocations.find(allocationHandle->GetKey());
	if(!B3D_ENSURE(found != mAllocations.end()))
		return;

	mFreeAllocations.push_back(found->second);
	mAllocations.erase(found);
}

void GUIVectorSpriteAtlas::Update()
{
	DestroyPendingReleasedAllocations();

	const u64 currentFrameIndex = GetTime().GetCurrentFrameIndex();
	for(auto it = mFreeTextureCache.begin(); it != mFreeTextureCache.end();)
	{
		const u64 frameDelta = currentFrameIndex - it->second.LastUsedFrame;
		if(frameDelta < mSettings.KeepUnusedTexturesFor)
		{
			++it;
			continue;
		}

		it = mFreeTextureCache.erase(it);
	}

	GetRenderThread().PostCommand([this, bufferIndex = mDirtySpriteWriteBufferIndex]()
	{
		RenderDirtySprites(bufferIndex);
	});

	mDirtySpriteWriteBufferIndex = (mDirtySpriteWriteBufferIndex + 1) % B3DSize(mDirtySpriteBuffers);
	B3D_ENSURE(mDirtySpriteBuffers[mDirtySpriteWriteBufferIndex].empty());
}

void GUIVectorSpriteAtlas::DestroyPendingReleasedAllocations()
{
	{
		Lock lock(mAllocationsMutex);

		if(!mFreeAllocations.empty())
			mFreeAllocations.swap(mFreeAllocationsTemp);
	}

	if(!mFreeAllocationsTemp.empty())
	{
		for(const auto& entry : mFreeAllocationsTemp)
		{
			if(entry.AllocationHandle->mLayoutAllocation)
			{
				const TreeTextureAtlasLayout::Allocation& layoutAllocation = entry.AllocationHandle->mLayoutAllocation.value();
				mAtlasLayout.RemoveElement(layoutAllocation.PageId, layoutAllocation.NodeId);

				if(mAtlasLayout.IsPageEmpty(layoutAllocation.PageId))
				{
					mAtlasLayoutTextures.erase(layoutAllocation.PageId);
					ReleaseTexture(entry.AtlasTexture);
				}
			}
			else
			{
				ReleaseTexture(entry.AtlasTexture);
				ReleaseTextureId(entry.AllocationHandle->mTextureId);

				mUniqueTextures.erase(entry.AllocationHandle->mTextureId);
			}

			B3DDelete(entry.AllocationHandle);
		}

		mFreeAllocationsTemp.clear();
	}
}

void GUIVectorSpriteAtlas::RenderDirtySprites(u32 bufferIndex)
{
	if(!EnsureRenderThread())
		return;

	Vector<DirtySpriteInformation>& dirtySprites = mDirtySpriteBuffers[bufferIndex];

	if(dirtySprites.empty())
		return;

	const SPtr<GpuDevice> gpuDevice = GetApplication().GetPrimaryGpuDevice();

	// Create a command buffer
	const SPtr<render::GpuCommandBufferPool>& commandBufferPool = RendererManager::Instance().GetActive()->GetCommandBufferPool();
	SPtr<render::GpuCommandBuffer> commandBuffer = commandBufferPool->Create(render::GpuCommandBufferCreateInformation::Create("GUIVectorSpriteAtlas"));

	FrameScope frameScope;
	FrameUnorderedMap<render::Texture*, SPtr<render::RenderTexture>> atlasRenderTextures;

	for(const auto& entry : dirtySprites)
	{
		TextureCreateInformation colorTextureCreateInformation;
		colorTextureCreateInformation.Width = entry.Size.Width;
		colorTextureCreateInformation.Height = entry.Size.Height;
		colorTextureCreateInformation.Format = PF_RGBA8;
		colorTextureCreateInformation.Usage = TU_RENDERTARGET;

		const SPtr<render::Texture> colorTexture = gpuDevice->CreateTexture(colorTextureCreateInformation);

		TextureCreateInformation stencilTextureCreateInformation;
		stencilTextureCreateInformation.Width = entry.Size.Width;
		stencilTextureCreateInformation.Height = entry.Size.Height;
		stencilTextureCreateInformation.Format = PF_D32_S8X24;
		stencilTextureCreateInformation.Usage = TU_DEPTHSTENCIL;

		const SPtr<render::Texture> stencilTexture = gpuDevice->CreateTexture(stencilTextureCreateInformation);

		render::RenderTextureCreateInformation renderTextureCreateInformation;
		renderTextureCreateInformation.ColorSurfaces[0].Texture = colorTexture;
		renderTextureCreateInformation.DepthStencilSurface.Texture = stencilTexture;

		SPtr<render::RenderTexture> renderTarget = render::RenderTexture::Create(renderTextureCreateInformation);

		// Prepare GPU parameters just before render pass
		SPtr<render::GpuParameters> gpuParameters = entry.Renderable->Prepare();

		// Begin render pass WITH GPU parameters
		commandBuffer->BeginRenderPass(render::RenderPassCreateInformation(renderTarget, gpuParameters, RT_NONE, RT_NONE));
		commandBuffer->SetViewport(Area2(0.0f, 0.0f, 1.0f, 1.0f));
		commandBuffer->ClearRenderTarget(FBT_COLOR | FBT_DEPTH | FBT_STENCIL, Color::kZero, 1, 0, 0xFF);

		entry.Renderable->Render(*commandBuffer);

		SPtr<render::RenderTexture> atlasRenderTexture;
		if(auto found = atlasRenderTextures.find(entry.Texture.get()); found != atlasRenderTextures.end())
		{
			atlasRenderTexture = found->second;
		}
		else
		{
			render::RenderTextureCreateInformation atlasTextureCreateInformation;
			atlasTextureCreateInformation.ColorSurfaces[0].Texture = entry.Texture;

			atlasRenderTexture = render::RenderTexture::Create(atlasTextureCreateInformation);
			atlasRenderTextures[entry.Texture.get()] = atlasRenderTexture;
		}

		commandBuffer->EndRenderPass();

		render::BlitInformation blitInformation = render::BlitInformation::BlitColor(colorTexture, atlasRenderTexture, Area2I::kEmpty, RT_NONE, RT_COLOR0);
		blitInformation.OutputArea = entry.UVRegion;

		render::GetRendererUtility().Blit(*commandBuffer, blitInformation);
	}

	gpuDevice->SubmitCommandBuffer(commandBuffer);
	dirtySprites.clear();	
}

HTexture GUIVectorSpriteAtlas::CreateOrFindTexture(Size2UI size) const
{
	if(auto found = mFreeTextureCache.find(FreeTextureInformation::Key(size)); found != mFreeTextureCache.end())
	{
		HTexture texture = found->second.Texture;
		mFreeTextureCache.erase(found);

		return texture;
	}
	else
	{
		TextureCreateInformation textureCreateInformation;
		textureCreateInformation.Width = size.Width;
		textureCreateInformation.Height = size.Height;
		textureCreateInformation.Format = PF_RGBA8;
		textureCreateInformation.Usage = TU_RENDERTARGET;
		textureCreateInformation.Name = "VectorPathAtlas";

		HTexture texture = Texture::Create(textureCreateInformation);
		B3D_ENSURE(texture != nullptr);

		return texture;
	}
}

void GUIVectorSpriteAtlas::ReleaseTexture(const HTexture& texture)
{
	// Skip invalid textures, should only be happening at shutdown
	if(!texture.IsValid())
		return;

	const TextureProperties& properties = texture->GetProperties();
	const FreeTextureInformation::Key key(Size2UI(properties.Width, properties.Height));

	mFreeTextureCache.insert(std::make_pair(key, FreeTextureInformation(texture, GetTime().GetCurrentFrameIndex())));
}

u32 GUIVectorSpriteAtlas::GetNextUniqueTextureId() const
{
	if(mFreeUniqueTextureIds.empty())
		return mNextUniqueTextureId++;

	const u32 id = mFreeUniqueTextureIds.back();
	mFreeUniqueTextureIds.pop_back();
	
	return id;
}

void GUIVectorSpriteAtlas::ReleaseTextureId(u32 id)
{
	mFreeUniqueTextureIds.push_back(id);
}

size_t GUIVectorSpriteAtlas::FreeTextureInformation::Key::Hash::operator()(const Key& value) const
{
	size_t hash = 0;
	B3DCombineHash(hash, value.Size);

	return hash;
}

size_t GUIVectorSpriteAtlasAllocationHandle::Key::Hash::operator()(const Key& value) const
{
	size_t hash = 0;
	B3DCombineHash(hash, value.VectorPathId);
	B3DCombineHash(hash, value.Settings);

	return hash;
}

GUIVectorSpriteAtlasAllocationHandle::Key GUIVectorSpriteAtlasAllocationHandle::GetKey() const
{
	return Key(mVectorPathId, mRenderable->GetSettings());
}

