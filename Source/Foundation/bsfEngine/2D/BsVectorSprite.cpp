//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "2D/BsVectorSprite.h"
#include "2D/BsSpriteManager.h"
#include "Image/BsSpriteTexture.h"
#include "Image/BsTexture.h"

using namespace bs;

VectorSprite::~VectorSprite()
{
	ClearMesh();
}

void VectorSprite::Update(const VectorSpriteInformation& information, u64 groupId)
{
	if(!information.VectorPath.IsLoaded())
	{
		ClearMesh();
		return;
	}

	// Actually generate a mesh
	if(mCachedRenderElements.size() < 1)
		mCachedRenderElements.resize(1);

	SpriteRenderElementData& renderElementData = mCachedRenderElements[0];
	{
		if(renderElementData.QuadCount != 1)
		{
			renderElementData.VertexPositions = mPositionBuffer.data();
			renderElementData.VertexUVs = mUVBuffer.data();
			renderElementData.Indices = mIndexBuffer.data();
			renderElementData.QuadCount = 1;
		}

		SpriteMaterialInfo& matInfo = renderElementData.MaterialInformation;
		matInfo.GroupId = groupId;
		//matInfo.Texture = tex; // TODO
		matInfo.Tint = information.Color;

		// TODO - Use premultiplied material
		//renderElementData.Material = SpriteManager::Instance().GetImageMaterial(
		//	information.Transparent ? SpriteMaterialTransparency::Alpha : SpriteMaterialTransparency::Opaque, animated);
	}

	renderElementData.Indices[0] = 0;
	renderElementData.Indices[1] = 1;
	renderElementData.Indices[2] = 2;
	renderElementData.Indices[3] = 1;
	renderElementData.Indices[4] = 3;
	renderElementData.Indices[5] = 2;

	renderElementData.VertexPositions[0] = Vector2(0.0f, 0.0f);
	renderElementData.VertexPositions[1] = Vector2((float)information.Width, 0.0f);
	renderElementData.VertexPositions[2] = Vector2(0.0f, (float)information.Height);
	renderElementData.VertexPositions[3] = Vector2((float)information.Width, (float)information.Height);

	// TODO
	//renderElementData.VertexUVs[0] = Vector2(uvOffset.X, uvOffset.Y);
	//renderElementData.VertexUVs[1] = Vector2(uvOffset.X + uvScale.X, uvOffset.Y);
	//renderElementData.VertexUVs[2] = Vector2(uvOffset.X, uvOffset.Y + uvScale.Y);
	//renderElementData.VertexUVs[3] = Vector2(uvOffset.X + uvScale.X, uvOffset.Y + uvScale.Y);

	UpdateBounds();
}

void VectorSprite::ClearMesh()
{
	mCachedRenderElements.clear();
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

SPtr<GUIVectorSpriteAtlasAllocation> GUIVectorSpriteAtlas::Allocate(const VectorPath& vectorPath, const VectorGraphicsSettings& settings)
{
	GUIVectorSpriteAtlasAllocation::Key key(vectorPath, settings);

	if(auto found = mAllocations.find(key); found != mAllocations.end())
		return found->second->shared_from_this();

	const Size2UI requestedSize = Size2UI((u32)settings.Size.Width, (u32)settings.Size.Height);
	const bool useUniqueTexture = requestedSize.Width >= mSettings.UniqueAllocationSize || requestedSize.Height >= mSettings.UniqueAllocationSize;

	const SPtr<ct::VectorPathRenderable> renderable = vectorPath.CreateRenderable(settings);

	HSpriteTexture spriteTexture;
	u32 textureId = ~0u;
	Optional<TreeTextureAtlasLayout::Allocation> layoutAllocation;
	if(useUniqueTexture)
	{
		const HTexture texture = CreateOrFindTexture(requestedSize);

		textureId = mNextUniqueTextureId++; // TODO - Need to re-use these IDs?
		mAtlasLayoutTextures[textureId] = texture;

		spriteTexture = SpriteTexture::Create(Vector2::kZero, Vector2::kOne, texture);
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
			texture = CreateOrFindTexture(requestedSize);
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

		spriteTexture = SpriteTexture::Create(uvOffset, uvSize, texture);
	}

	// TODO - Need to mark allocations as dirty, and render them as needed

	GUIVectorSpriteAtlasAllocation* const allocation = B3DNew<GUIVectorSpriteAtlasAllocation>(this, key.VectorPathId, spriteTexture, layoutAllocation, textureId, renderable);
	SPtr<GUIVectorSpriteAtlasAllocation> allocationShared = B3DMakeSharedFromExisting<GUIVectorSpriteAtlasAllocation>(allocation,
		[](GUIVectorSpriteAtlasAllocation* allocation)
		{
			GUIVectorSpriteAtlas* owner = allocation->GetOwner();
			owner->NotifyAllocationReleased(allocation);

			B3DDelete(allocation);
	});

	mAllocations[key] = allocation;
	return allocationShared;
}

void GUIVectorSpriteAtlas::NotifyAllocationReleased(GUIVectorSpriteAtlasAllocation* allocation)
{
	Lock lock(mFreeAllocationMutex);
	mFreeAllocations.push_back(allocation);
}

void GUIVectorSpriteAtlas::Update()
{
	{
		Lock lock(mFreeAllocationMutex);

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
					mAtlasLayoutTextures.erase(layoutAllocation.PageId);

				ReleaseTexture(entry->Texture->GetTexture());
			}
			else
			{
				ReleaseTexture(entry->Texture->GetTexture());
				ReleaseTextureId(entry->mTextureId);

				mUniqueTextures.erase(entry->mTextureId);
			}
			
			B3D_ENSURE(mAllocations.erase(entry->GetKey()) == 1);
		}

		mFreeAllocationsTemp.clear();
	}

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
}

void GUIVectorSpriteAtlas::RenderDirtySprites()
{
	// TODO
	// TODO - Check if core thread
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

