//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "2D/BsVectorSprite.h"
#include "2D/BsSpriteManager.h"
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

VectorSprite::~VectorSprite()
{
	ClearMesh();
}

void VectorSprite::Update(const VectorSpriteInformation& information, u64 groupId)
{
	if(!information.VectorPath.IsLoaded() || information.Width == 0 || information.Height == 0)
	{
		ClearMesh();
		return;
	}

	// Actually generate a mesh
	if(mCachedRenderElements.size() < 1)
		mCachedRenderElements.resize(1);

	VectorGraphicsSettings vectorGraphicsSettings;
	vectorGraphicsSettings.Size = Size2((float)information.Width, (float)information.Height);

	GUIVectorSpriteAtlas& vectorSpriteAtlas = GetGUIManager().GetVectorSpriteAtlas();

	mSpriteAtlasAllocation = vectorSpriteAtlas.Allocate(*information.VectorPath, vectorGraphicsSettings);
	if(!B3D_ENSURE(mSpriteAtlasAllocation))
		return;

	HSpriteTexture image = mSpriteAtlasAllocation->Image;

	RenderElementData& renderElementData = mCachedRenderElements[0];
	SpriteRenderElement& renderElement = renderElementData.RenderElement;
	if(renderElement.VertexCount < 4)
	{
		renderElement.VertexPositions = mPositionBuffer.data();
		renderElement.VertexUVs = mUVBuffer.data();
		renderElement.Indices = mIndexBuffer.data();
		renderElement.VertexCount = 4;
		renderElement.IndexCount = 6;
	}

	SpriteMaterialInfo& materialInformation = renderElementData.MaterialInformation;
	materialInformation.GroupId = groupId;
	materialInformation.Texture = image->GetTexture();
	materialInformation.Tint = information.Color;

	renderElement.Material = SpriteManager::Instance().GetImageMaterial(SpriteMaterialTransparency::Premultiplied);
	renderElement.MaterialInformation = &renderElementData.MaterialInformation;

	renderElement.Indices[0] = 0;
	renderElement.Indices[1] = 1;
	renderElement.Indices[2] = 2;
	renderElement.Indices[3] = 1;
	renderElement.Indices[4] = 3;
	renderElement.Indices[5] = 2;

	renderElement.VertexPositions[0] = Vector2(0.0f, 0.0f);
	renderElement.VertexPositions[1] = Vector2((float)information.Width, 0.0f);
	renderElement.VertexPositions[2] = Vector2(0.0f, (float)information.Height);
	renderElement.VertexPositions[3] = Vector2((float)information.Width, (float)information.Height);

	renderElement.VertexUVs[0] = image->TransformUv(Vector2(0.0f, 0.0f));
	renderElement.VertexUVs[1] = image->TransformUv(Vector2(1.0f, 0.0f));
	renderElement.VertexUVs[2] = image->TransformUv(Vector2(0.0f, 1.0f));
	renderElement.VertexUVs[3] = image->TransformUv(Vector2(1.0f, 1.0f));

	UpdateBounds();
}

void VectorSprite::ClearMesh()
{
	mCachedRenderElements.clear();
	mSpriteAtlasAllocation = nullptr;
	UpdateBounds();
}

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

	HSpriteTexture image;
	u32 textureId = ~0u;
	Optional<TreeTextureAtlasLayout::Allocation> layoutAllocation;
	if(useUniqueTexture)
	{
		const HTexture texture = CreateOrFindTexture(requestedSize);

		textureId = GetNextUniqueTextureId();
		mUniqueTextures[textureId] = texture;

		image = SpriteTexture::Create(Vector2::kZero, Vector2::kOne, texture);
	}
	else
	{
		layoutAllocation = mAtlasLayout.AddElement(Size2UI((u32)settings.Size.Width, (u32)settings.Size.Height));
		if(!layoutAllocation)
			return nullptr;

		const Size2UI& atlasPageSize = mAtlasLayout.GetSize();

		HTexture texture;
		if(auto found = mAtlasLayoutTextures.find(layoutAllocation->PageId); found == mAtlasLayoutTextures.end())
		{
			texture = CreateOrFindTexture(mAtlasLayout.GetSize());
			mAtlasLayoutTextures[layoutAllocation->PageId] = texture;
		}
		else
		{
			texture = found->second;
		}

		const Vector2 uvOffset(
			(float)layoutAllocation->Position.X / (float)atlasPageSize.Width,
			(float)layoutAllocation->Position.Y / (float)atlasPageSize.Height);

		const Vector2 uvSize(
			(float)requestedSize.Width / (float)atlasPageSize.Width,
			(float)requestedSize.Height / (float)atlasPageSize.Height);

		image = SpriteTexture::Create(uvOffset, uvSize, texture);
	}

	GUIVectorSpriteAtlasAllocation* const allocation = B3DNew<GUIVectorSpriteAtlasAllocation>(this, key.VectorPathId, image, layoutAllocation, textureId, renderable);
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
	dirtySpriteInformation.Texture = B3DGetRenderProxy(image->GetTexture());
	dirtySpriteInformation.Renderable = renderable;
	dirtySpriteInformation.UVRegion = Rect2(image->GetOffset().X, image->GetOffset().Y, image->GetScale().X, image->GetScale().Y);
	dirtySpriteInformation.Size = Size2UI(image->GetWidth(), image->GetHeight());

	B3D_ASSERT(image->GetWidth() > 0 && image->GetHeight() > 0);

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


	mDirtySpriteWriteBufferIndex = (mDirtySpriteWriteBufferIndex + 1) % B3DSize(mDirtySpriteBuffers);
	mDirtySpriteBuffers[mDirtySpriteWriteBufferIndex].clear();

	{
		Lock lock(mDirtySpriteMutex);
		mDirtySpriteReadBufferIndex = (mDirtySpriteReadBufferIndex + 1) % B3DSize(mDirtySpriteBuffers);
	}
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
					ReleaseTexture(entry->Image->GetTexture());
				}
			}
			else
			{
				ReleaseTexture(entry->Image->GetTexture());
				ReleaseTextureId(entry->mTextureId);

				mUniqueTextures.erase(entry->mTextureId);
			}

			B3DDelete(entry);
		}

		mFreeAllocationsTemp.clear();
	}
}

void GUIVectorSpriteAtlas::RenderDirtySprites()
{
	if(!EnsureRenderThread())
		return;

	u32 readBufferIndex = 0;
	{
		Lock lock(mDirtySpriteMutex);
		readBufferIndex = mDirtySpriteReadBufferIndex;
	}

	Vector<DirtySpriteInformation>& dirtySprites = mDirtySpriteBuffers[readBufferIndex];

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
		commandBuffer->SetViewport(Rect2(0.0f, 0.0f, 1.0f, 1.0f));
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

