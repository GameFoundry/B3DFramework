//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Image/BsSpriteGlyph.h"
#include "Private/RTTI/BsSpriteGlyphRTTI.h"
#include "Image/BsTexture.h"
#include "Resources/BsResources.h"
#include "Resources/BsBuiltinResources.h"
#include "CoreObject/BsCoreObjectSync.h"
#include "Text/BsFont.h"

using namespace bs;

namespace bs
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

SPtr<SpriteGlyphAllocation> SpriteGlyphAllocation::Create(const WeakSPtr<SpriteImageType>& owner, const TextureType& texture, const Area2& uvRange, const Size2I& sizeInPixels)
{
	SpriteGlyphAllocation* allocation = new(B3DAllocate<SpriteGlyphAllocation>()) SpriteGlyphAllocation(owner, texture, uvRange, sizeInPixels);
	SPtr<SpriteGlyphAllocation> allocationShared = B3DMakeSharedFromExisting<SpriteGlyphAllocation>(allocation);
	allocationShared->SetShared(allocationShared);
	allocationShared->Initialize();

	return allocationShared;
}

SPtr<ct::RenderProxy> SpriteGlyphAllocation::CreateRenderProxy() const
{
	ct::SpriteGlyphAllocation* const renderProxy = new(B3DAllocate<ct::SpriteGlyphAllocation>()) ct::SpriteGlyphAllocation();

	SPtr<ct::SpriteGlyphAllocation> renderProxyShared = B3DMakeSharedFromExisting<ct::SpriteGlyphAllocation>(renderProxy);
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

namespace bs
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

#if 1
	const float sizeInPoints = Font::GetQuantizedFontSize(Math::Min(
		mFont->GetPointSizeForGlyphPixelWidth(mGlyph, size.Width),
		mFont->GetPointSizeForGlyphPixelHeight(mGlyph, size.Height)));

	auto foundImage = std::find_if(mScaledAllocatedImages.begin(), mScaledAllocatedImages.end(), [sizeInPoints](const SpriteImageAllocation* allocation) {
		return static_cast<const SpriteGlyphAllocation*>(allocation)->GetSizeInPoints() == sizeInPoints;
	});

	if(foundImage != mScaledAllocatedImages.end())
		return std::static_pointer_cast<SpriteImageAllocation>((*foundImage)->GetShared());

	return AllocateImage(sizeInPoints);
#else

	auto foundImage = std::find_if(mScaledAllocatedImages.begin(), mScaledAllocatedImages.end(), [size](const SpriteImageAllocation* allocation) {
		return static_cast<const SpriteGlyphAllocation*>(allocation)->GetSizeInPixels() == size;
	});

	if(foundImage != mScaledAllocatedImages.end())
		return std::static_pointer_cast<SpriteImageAllocation>((*foundImage)->GetShared());
	
	return AllocateImage(size);
#endif
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

SPtr<SpriteGlyphAllocation> SpriteGlyph::AllocateImage(const Size2I& sizeInPixels)
{
	TInlineArray<u32, 1> glyphs = { mGlyph };
	mFont->RenderGlyphs(sizeInPixels, glyphs);

	HTexture atlasTexture;
	Area2 uvRange;

	const CharacterInformation* characterInformation = mFont->FindCharacterInformation(mGlyph, sizeInPixels);
	if(characterInformation != nullptr)
	{
		atlasTexture = mFont->GetPage(characterInformation->Page).Texture;
		uvRange = Area2(characterInformation->UvX, characterInformation->UvY, characterInformation->UvWidth, characterInformation->UvHeight);
	}

	SPtr<SpriteGlyphAllocation> allocation = SpriteGlyphAllocation::Create(std::static_pointer_cast<SpriteGlyph>(GetShared()), atlasTexture, uvRange, sizeInPixels);
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

SPtr<ct::RenderProxy> SpriteGlyph::CreateRenderProxy() const
{
	ct::SpriteGlyphCreateInformation createInformation(mInformation);
	ct::SpriteGlyph* const renderProxy = new(B3DAllocate<ct::SpriteGlyph>()) ct::SpriteGlyph(createInformation);

	SPtr<ct::SpriteGlyph> renderProxyShared = B3DMakeSharedFromExisting<ct::SpriteGlyph>(renderProxy);
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

namespace bs { namespace ct
{
void SpriteGlyphAllocation::SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator)
{
	auto* const syncPacket = data.GetSyncPacket<bs::SpriteGlyphAllocation::SyncPacket>();
	if(!syncPacket)
		return;

	syncPacket->ApplySyncData(this);
}

SpriteGlyph::SpriteGlyph(const SpriteGlyphCreateInformation& createInformation)
	: SpriteImage(createInformation)
{ }

void SpriteGlyph::SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator)
{
	auto* const syncPacket = data.GetSyncPacket<bs::SpriteGlyph::SyncPacket>();
	if(!syncPacket)
		return;

	syncPacket->ApplySyncData(this);
}
}}
