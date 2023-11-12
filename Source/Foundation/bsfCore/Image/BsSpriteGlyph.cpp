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
	B3D_SYNC_BLOCK_BEGIN(SpriteGlyph, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY(mAtlasTexture)
		B3D_SYNC_BLOCK_ENTRY_PACKET_BASE(SpriteImage, SpriteImageSyncPacket)
	B3D_SYNC_BLOCK_END
}

SpriteGlyph::SpriteGlyph(const SpriteGlyphCreateInformation& createInformation)
	: SpriteImage(createInformation), mFont(createInformation.Font), mGlyph(createInformation.Glyph), mGlyphSize(createInformation.Size)
{
}

void SpriteGlyph::SetFont(const HFont& font)
{
	if(mFont == font)
		return;

	RemoveResourceDependency(mFont);
	mFont = font;
	AddResourceDependency(mFont);

	UpdateGlyphAtlasInformation();
}

void SpriteGlyph::SetGlyph(u32 glyph)
{
	if(mGlyph == glyph)
		return;

	mGlyph = glyph;
	UpdateGlyphAtlasInformation();
}

void SpriteGlyph::SetGlyphSize(float size)
{
	if(mGlyphSize == size)
		return;

	mGlyphSize = size;
	UpdateGlyphAtlasInformation();
}

void SpriteGlyph::UpdateGlyphAtlasInformation()
{
	mAtlasTexture = nullptr;
	mInformation.UVRange = Rect2::kEmpty;

	if(!mFont.IsLoaded(false))
		return;

	TInlineArray<u32, 1> glyphs = { mGlyph };
	mFont->RenderGlyphs(mGlyphSize, glyphs);

	SPtr<FontBitmapInformation> bitmapInformation = mFont->GetBitmap(mGlyphSize);
	if(!bitmapInformation)
		return;

	auto found = bitmapInformation->Characters.find(mGlyph);
	if(found != bitmapInformation->Characters.end())
	{
		mAtlasTexture = bitmapInformation->TexturePages[found->second.Page].Texture;
		mInformation.UVRange = Rect2(found->second.UvX, found->second.UvY, found->second.UvWidth, found->second.UvHeight);
	}

	MarkDependenciesDirty();
	MarkRenderProxyDataDirtyInternal();
}

void SpriteGlyph::Initialize()
{
	UpdateGlyphAtlasInformation();
	AddResourceDependency(mFont);

	Resource::Initialize();
}

SPtr<ct::RenderProxy> SpriteGlyph::CreateRenderProxy() const
{
	SPtr<ct::Texture> atlasRenderProxy = B3DGetRenderProxy(mAtlasTexture);

	ct::SpriteGlyphCreateInformation createInformation(mInformation, std::move(atlasRenderProxy));
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

void SpriteGlyph::GetCoreDependencies(Vector<CoreObject*>& dependencies)
{
	if(mAtlasTexture.IsLoaded())
		dependencies.push_back(mAtlasTexture.Get());
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
	createInformation.Size = size;

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

RTTITypeBase* SpriteGlyph::GetRttiStatic()
{
	return SpriteGlyphRTTI::Instance();
}

RTTITypeBase* SpriteGlyph::GetRtti() const
{
	return GetRttiStatic();
}

namespace bs { namespace ct
{
SpriteGlyph::SpriteGlyph(const SpriteGlyphCreateInformation& createInformation)
	: SpriteImage(createInformation)
{
	mAtlasTexture = createInformation.AtlasTexture;
}

void SpriteGlyph::SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator)
{
	auto* const syncPacket = data.GetSyncPacket<bs::SpriteGlyph::SyncPacket>();
	if(!syncPacket)
		return;

	syncPacket->ApplySyncData(this);
}
}}
