//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Image/B3DSpriteGlyph.h"
#include "RTTI/B3DSpriteGlyphRTTI.h"
#include "Image/B3DTexture.h"
#include "Resources/B3DResources.h"
#include "Resources/B3DBuiltinResources.h"
#include "CoreObject/B3DCoreObjectSync.h"
#include "Text/B3DFont.h"

using namespace b3d;

namespace b3d
{
	B3D_SYNC_BLOCK_BEGIN(SpriteGlyphAllocation, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY_PACKET_BASE(SpriteImageAllocation, SpriteImageSyncPacket)
	B3D_SYNC_BLOCK_END
}

SPtr<SpriteGlyphAllocation> SpriteGlyphAllocation::Create(const WeakSPtr<SpriteImageType>& owner, const TextureType& texture, const Area2& uvRange, float sizeInPoints)
{
	SpriteGlyphAllocation* allocation = new(B3DAllocate<SpriteGlyphAllocation>()) SpriteGlyphAllocation(owner, texture, uvRange, sizeInPoints);
	SPtr<SpriteGlyphAllocation> allocationShared = B3DMakeSharedFromExisting<SpriteGlyphAllocation>(allocation);
	allocationShared->SetShared(allocationShared);
	allocationShared->Initialize();

	return allocationShared;
}

SPtr<render::RenderProxy> SpriteGlyphAllocation::CreateRenderProxy() const
{
	const SPtr<render::SpriteImage> owner = B3DGetRenderProxy(mOwner.lock());
	const SPtr<render::Texture> atlasTexture = B3DGetRenderProxy(mTexture);

	render::SpriteGlyphAllocation* const renderProxy = new(B3DAllocate<render::SpriteGlyphAllocation>()) render::SpriteGlyphAllocation(owner, atlasTexture, mUVRange, GetSizeInPoints());

	SPtr<render::SpriteGlyphAllocation> renderProxyShared = B3DMakeSharedFromExisting<render::SpriteGlyphAllocation>(renderProxy);
	renderProxyShared->SetShared(renderProxyShared);

	return renderProxyShared;
}

RenderProxySyncPacket* SpriteGlyphAllocation::CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags)
{
	auto syncPacket = allocator.Construct<SyncPacket>(*this, allocator, flags);
	if(B3D_ENSURE(syncPacket))
		syncPacket->SpriteImageSyncPacket = SpriteImageAllocation::CreateRenderProxySyncPacket(allocator, flags);

	return syncPacket;
}

namespace b3d
{
	B3D_SYNC_BLOCK_BEGIN(SpriteGlyph, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY_PACKET_BASE(SpriteImage, SpriteImageSyncPacket)
	B3D_SYNC_BLOCK_END
}

SpriteGlyph::SpriteGlyph(const SpriteGlyphCreateInformation& createInformation)
	: SpriteImage(createInformation), mFont(createInformation.Font), mGlyph(createInformation.Glyph), mDefaultGlyphSize(Font::GetQuantizedFontSize(createInformation.DefaultSize))
{
}

SPtr<SpriteImageAllocation> SpriteGlyph::FindOrAllocateImageToFitArea(const Size2I& size)
{
	if(!mFont.IsLoaded(false))
		return nullptr;

	const float sizeInPoints = Font::GetQuantizedFontSize(mFont->GetPointSizeForGlyphThatFitsArea(mGlyph, size));

	auto foundImage = std::find_if(mScaledAllocatedImages.begin(), mScaledAllocatedImages.end(), [sizeInPoints](const SpriteImageAllocation* allocation) {
		return static_cast<const SpriteGlyphAllocation*>(allocation)->GetSizeInPoints() == sizeInPoints;
	});

	if(foundImage != mScaledAllocatedImages.end())
		return std::static_pointer_cast<SpriteImageAllocation>((*foundImage)->GetShared());

	return AllocateImage(sizeInPoints);
}

SPtr<SpriteImageAllocation> SpriteGlyph::FindOrAllocateScaledImage(float scale)
{
	const float sizeInPoints = Font::GetQuantizedFontSize(mDefaultGlyphSize * scale);
	auto foundImage = std::find_if(mScaledAllocatedImages.begin(), mScaledAllocatedImages.end(), [sizeInPoints](const SpriteImageAllocation* allocation) {
		return static_cast<const SpriteGlyphAllocation*>(allocation)->GetSizeInPoints() == sizeInPoints;
	});

	if(foundImage != mScaledAllocatedImages.end())
		return std::static_pointer_cast<SpriteImageAllocation>((*foundImage)->GetShared());

	return AllocateImage(sizeInPoints);
}

SPtr<SpriteGlyphAllocation> SpriteGlyph::AllocateImage(float sizeInPoints)
{
	TInlineArray<u32, 1> glyphs = { mGlyph };
	mFont->RenderGlyphs(sizeInPoints, glyphs);

	SPtr<FontBitmapInformation> bitmapInformation = mFont->GetBitmap(sizeInPoints);
	if(!bitmapInformation)
		return nullptr;

	HTexture atlasTexture;
	Area2 uvRange;

	auto found = bitmapInformation->Characters.find(mGlyph);
	if(found != bitmapInformation->Characters.end())
	{
		atlasTexture = mFont->GetPage(found->second.Page).Texture;
		uvRange = Area2(found->second.UvX, found->second.UvY, found->second.UvWidth, found->second.UvHeight);
	}

	SPtr<SpriteGlyphAllocation> allocation = SpriteGlyphAllocation::Create(std::static_pointer_cast<SpriteGlyph>(GetShared()), atlasTexture, uvRange, sizeInPoints);
	mScaledAllocatedImages.Add(allocation.get());

	// TODO - We should have the ability to release glyphs from the font when no longer needed

	return allocation;
}

void SpriteGlyph::Initialize()
{
	mDefaultAllocatedImage = AllocateImage(mDefaultGlyphSize);
	AddResourceDependency(mFont);

	Resource::Initialize();
}

SPtr<render::RenderProxy> SpriteGlyph::CreateRenderProxy() const
{
	render::SpriteGlyphCreateInformation createInformation(mInformation);
	render::SpriteGlyph* const renderProxy = new(B3DAllocate<render::SpriteGlyph>()) render::SpriteGlyph(createInformation, B3DGetRenderProxy(mDefaultAllocatedImage));

	SPtr<render::SpriteGlyph> renderProxyShared = B3DMakeSharedFromExisting<render::SpriteGlyph>(renderProxy);
	renderProxyShared->SetShared(renderProxyShared);

	return renderProxyShared;
}

RenderProxySyncPacket* SpriteGlyph::CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags)
{
	auto syncPacket = allocator.Construct<SyncPacket>(*this, allocator, flags);
	if(B3D_ENSURE(syncPacket))
		syncPacket->SpriteImageSyncPacket = SpriteImage::CreateRenderProxySyncPacket(allocator, flags);

	return syncPacket;
}

HSpriteGlyph SpriteGlyph::Create(const HFont& font, u32 glyph, float size)
{
	SPtr<SpriteGlyph> spriteGlyph = CreateShared(font, glyph, size);

	return B3DStaticResourceCast<SpriteGlyph>(GetResources().CreateResourceHandle(spriteGlyph));
}

HSpriteGlyph SpriteGlyph::Create(const SpriteGlyphCreateInformation& createInformation)
{
	SPtr<SpriteGlyph> spriteGlyph = CreateShared(createInformation);

	return B3DStaticResourceCast<SpriteGlyph>(GetResources().CreateResourceHandle(spriteGlyph));
}

SPtr<SpriteGlyph> SpriteGlyph::CreateShared(const HFont& font, u32 glyph, float size)
{
	SpriteGlyphCreateInformation createInformation;
	createInformation.Font = font;
	createInformation.Glyph = glyph;
	createInformation.DefaultSize = size;

	return CreateShared(createInformation);
}

SPtr<SpriteGlyph> SpriteGlyph::CreateShared(const SpriteGlyphCreateInformation& createInformation)
{
	SPtr<SpriteGlyph> spriteGlyph = B3DMakeSharedFromExisting<SpriteGlyph>(new(B3DAllocate<SpriteGlyph>()) SpriteGlyph(createInformation));

	spriteGlyph->SetShared(spriteGlyph);
	spriteGlyph->Initialize();

	return spriteGlyph;
}

SPtr<SpriteGlyph> SpriteGlyph::CreateEmpty()
{
	SPtr<SpriteGlyph> texture = B3DMakeSharedFromExisting<SpriteGlyph>(new(B3DAllocate<SpriteGlyph>()) SpriteGlyph(SpriteGlyphCreateInformation()));
	texture->SetShared(texture);

	return texture;
}

RTTIType* SpriteGlyph::GetRttiStatic()
{
	return SpriteGlyphRTTI::Instance();
}

RTTIType* SpriteGlyph::GetRtti() const
{
	return GetRttiStatic();
}

namespace b3d { namespace render
{
SpriteGlyphAllocation::SpriteGlyphAllocation(const WeakSPtr<SpriteImageType>& owner, const TextureType& atlasTexture, const Area2& uvRange, float sizeInPoints)
	: TSpriteGlyphAllocation(owner, atlasTexture, uvRange, sizeInPoints)
{ }

void SpriteGlyphAllocation::SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator)
{
	auto* const syncPacket = data.GetSyncPacket<b3d::SpriteGlyphAllocation::SyncPacket>();
	if(!syncPacket)
		return;

	syncPacket->ApplySyncData(this);
}

SpriteGlyph::SpriteGlyph(const SpriteGlyphCreateInformation& createInformation, const SPtr<SpriteImageAllocation>& defaultAllocatedImage)
	: SpriteImage(createInformation, defaultAllocatedImage)
{ }

void SpriteGlyph::SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator)
{
	auto* const syncPacket = data.GetSyncPacket<b3d::SpriteGlyph::SyncPacket>();
	if(!syncPacket)
		return;

	syncPacket->ApplySyncData(this);
}
}}
