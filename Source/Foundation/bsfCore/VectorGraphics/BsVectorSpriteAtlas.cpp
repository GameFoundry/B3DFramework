//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "VectorGraphics/BsVectorSpriteAtlas.h"
#include "CoreObject/BsRenderThread.h"
#include "GUI/BsGUIManager.h"
#include "Image/BsSpriteTexture.h"
#include "Image/BsTexture.h"
#include "RenderAPI/BsGpuBackend.h"
#include "RenderAPI/BsGpuCommandBuffer.h"
#include "RenderAPI/BsRenderTexture.h"
#include "Renderer/BsRenderer.h"
#include "Renderer/BsRendererManager.h"
#include "Renderer/BsRendererUtility.h"

using namespace bs;

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

SPtr<GUIVectorSpriteAtlasAllocation> GUIVectorSpriteAtlas::Allocate(const VectorPath& vectorPath, const VectorGraphicsSettings& settings)
{
	if(!EnsureMainThread())
		return nullptr;

	GUIVectorSpriteAtlasAllocation::Key key(vectorPath, settings);

	{
		Lock lock(mAllocationsMutex);

		if(auto found = mAllocations.find(key); found != mAllocations.end())
			return found->second->shared_from_this();
	}

	const Size2UI requestedSize = Size2UI((u32)settings.Size.Width, (u32)settings.Size.Height);
	const bool useUniqueTexture = requestedSize.Width >= mSettings.UniqueAllocationSize || requestedSize.Height >= mSettings.UniqueAllocationSize;

	const SPtr<ct::VectorPathRenderable> renderable = vectorPath.CreateRenderable(settings);

	HTexture atlasTexture;
	Area2 uvRange;

	u32 textureId = ~0u;
	Optional<TreeTextureAtlasLayout::Allocation> layoutAllocation;
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
			return nullptr;

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

	GUIVectorSpriteAtlasAllocation* const allocation = B3DNew<GUIVectorSpriteAtlasAllocation>(this, key.VectorPathId, atlasTexture, uvRange, layoutAllocation, textureId, renderable);
	SPtr<GUIVectorSpriteAtlasAllocation> allocationShared = B3DMakeSharedFromExisting<GUIVectorSpriteAtlasAllocation>(allocation,
		[](GUIVectorSpriteAtlasAllocation* allocation)
		{
			GUIVectorSpriteAtlas* owner = allocation->GetOwner();
			owner->NotifyAllocationReleased(allocation);
	});

	{
		Lock lock(mAllocationsMutex);
		mAllocations[key] = allocation;
	}

	DirtySpriteInformation dirtySpriteInformation;
	dirtySpriteInformation.Texture = B3DGetRenderProxy(atlasTexture);
	dirtySpriteInformation.Renderable = renderable;
	dirtySpriteInformation.UVRegion = uvRange;
	dirtySpriteInformation.Size = requestedSize;

	B3D_ASSERT(dirtySpriteInformation.Size.Width > 0 && dirtySpriteInformation.Size.Height > 0);

	mDirtySpriteBuffers[mDirtySpriteWriteBufferIndex].push_back(dirtySpriteInformation);
	return allocationShared;
}

void GUIVectorSpriteAtlas::NotifyAllocationReleased(GUIVectorSpriteAtlasAllocation* allocation)
{
	Lock lock(mAllocationsMutex);
	mFreeAllocations.push_back(allocation);
			
	B3D_ENSURE(mAllocations.erase(allocation->GetKey()) == 1);
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
			if(entry->mLayoutAllocation)
			{
				const TreeTextureAtlasLayout::Allocation& layoutAllocation = entry->mLayoutAllocation.value();
				mAtlasLayout.RemoveElement(layoutAllocation.PageId, layoutAllocation.NodeId);

				if(mAtlasLayout.IsPageEmpty(layoutAllocation.PageId))
				{
					mAtlasLayoutTextures.erase(layoutAllocation.PageId);
					ReleaseTexture(entry->AtlasTexture);
				}
			}
			else
			{
				ReleaseTexture(entry->AtlasTexture);
				ReleaseTextureId(entry->mTextureId);

				mUniqueTextures.erase(entry->mTextureId);
			}

			B3DDelete(entry);
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

	const SPtr<GpuDevice> gpuDevice = GetCoreApplication().GetPrimaryGpuDevice();

	// Create a command buffer
	const SPtr<ct::GpuCommandBufferPool>& commandBufferPool = RendererManager::Instance().GetActive()->GetCommandBufferPool();
	SPtr<ct::GpuCommandBuffer> commandBuffer = commandBufferPool->Create(ct::GpuCommandBufferCreateInformation::Create("GUIVectorSpriteAtlas"));

	FrameScope frameScope;
	FrameUnorderedMap<ct::Texture*, SPtr<ct::RenderTexture>> atlasRenderTextures;

	for(const auto& entry : dirtySprites)
	{
		TextureCreateInformation colorTextureCreateInformation;
		colorTextureCreateInformation.Width = entry.Size.Width;
		colorTextureCreateInformation.Height = entry.Size.Height;
		colorTextureCreateInformation.Format = PF_RGBA8;
		colorTextureCreateInformation.Usage = TU_RENDERTARGET;

		const SPtr<ct::Texture> colorTexture = gpuDevice->CreateTexture(colorTextureCreateInformation);

		TextureCreateInformation stencilTextureCreateInformation;
		stencilTextureCreateInformation.Width = entry.Size.Width;
		stencilTextureCreateInformation.Height = entry.Size.Height;
		stencilTextureCreateInformation.Format = PF_D32_S8X24;
		stencilTextureCreateInformation.Usage = TU_DEPTHSTENCIL;

		const SPtr<ct::Texture> stencilTexture = gpuDevice->CreateTexture(stencilTextureCreateInformation);

		ct::RenderTextureCreateInformation renderTextureCreateInformation;
		renderTextureCreateInformation.ColorSurfaces[0].Texture = colorTexture;
		renderTextureCreateInformation.DepthStencilSurface.Texture = stencilTexture;

		SPtr<ct::RenderTexture> renderTarget = ct::RenderTexture::Create(renderTextureCreateInformation);
		
		// Bind render surface & clear it
		commandBuffer->SetRenderTarget(renderTarget, 0, RT_NONE);
		commandBuffer->SetViewport(Area2(0.0f, 0.0f, 1.0f, 1.0f));
		commandBuffer->ClearRenderTarget(FBT_COLOR | FBT_DEPTH | FBT_STENCIL, Color::kZero, 1, 0, 0xFF);

		entry.Renderable->Render(*commandBuffer);

		SPtr<ct::RenderTexture> atlasRenderTexture;
		if(auto found = atlasRenderTextures.find(entry.Texture.get()); found != atlasRenderTextures.end())
		{
			atlasRenderTexture = found->second;
		}
		else
		{
			ct::RenderTextureCreateInformation atlasTextureCreateInformation;
			atlasTextureCreateInformation.ColorSurfaces[0].Texture = entry.Texture;

			atlasRenderTexture = ct::RenderTexture::Create(atlasTextureCreateInformation);
			atlasRenderTextures[entry.Texture.get()] = atlasRenderTexture;
		}

		commandBuffer->SetRenderTarget(atlasRenderTexture, 0, RT_COLOR0);
		commandBuffer->SetViewport(entry.UVRegion);
		ct::GetRendererUtility().Blit(*commandBuffer, colorTexture);
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

		HTexture texture = Texture::Create(textureCreateInformation);
		B3D_ENSURE(texture != nullptr);

		return texture;
	}
}

void GUIVectorSpriteAtlas::ReleaseTexture(const HTexture& texture)
{
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

size_t GUIVectorSpriteAtlasAllocation::Key::Hash::operator()(const Key& value) const
{
	size_t hash = 0;
	B3DCombineHash(hash, value.VectorPathId);
	B3DCombineHash(hash, value.Settings);

	return hash;
}

GUIVectorSpriteAtlasAllocation::Key GUIVectorSpriteAtlasAllocation::GetKey() const
{
	return Key(mVectorPathId, mRenderable->GetSettings());
}

