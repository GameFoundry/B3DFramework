//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Image/BsSpriteTexture.h"
#include "Private/RTTI/BsSpriteTextureRTTI.h"
#include "Image/BsTexture.h"
#include "Resources/BsResources.h"
#include "Resources/BsBuiltinResources.h"
#include "CoreObject/BsCoreObjectSync.h"
#include "Text/BsFont.h"

using namespace bs;

Rect2 SpriteImageBase::EvaluateAnimation(float t) const
{
	if(mInformation.AnimationPlayback == SpriteAnimationPlayback::None)
		return mInformation.UVRange;

	u32 row;
	u32 column;
	GetAnimationFrame(t, row, column);

	Rect2 output;

	// Note: These could be pre-calculated
	output.Width = mInformation.UVRange.Width / (float)mInformation.Animation.ColumnCount;
	output.Height = mInformation.UVRange.Height / (float)mInformation.Animation.RowCount;

	output.X = mInformation.UVRange.X + (float)column * output.Width;
	output.Y = mInformation.UVRange.Y + (float)row * output.Height;

	return output;
}

void SpriteImageBase::GetAnimationFrame(float t, u32& outRow, u32& outColumn) const
{
	if(mInformation.AnimationPlayback == SpriteAnimationPlayback::None)
	{
		outRow = 0;
		outColumn = 0;

		return;
	}

	// Note: Duration could be pre-calculated
	float duration = 0.0f;
	if(mInformation.Animation.FramesPerSecond > 0)
		duration = mInformation.Animation.FrameCount / (float)mInformation.Animation.FramesPerSecond;

	switch(mInformation.AnimationPlayback)
	{
	default:
	case SpriteAnimationPlayback::Normal:
		t = Math::Clamp(t, 0.0f, duration);
		break;
	case SpriteAnimationPlayback::Loop:
		t = Math::Repeat(t, duration);
		break;
	case SpriteAnimationPlayback::PingPong:
		t = Math::PingPong(t, duration);
		break;
	}

	const float percent = t / duration;
	u32 frame = 0;

	if(mInformation.Animation.FrameCount > 0)
		frame = Math::Clamp(Math::FloorToPosInt(percent * mInformation.Animation.FrameCount), 0U, mInformation.Animation.FrameCount - 1);

	outRow = frame / mInformation.Animation.ColumnCount;
	outColumn = frame % mInformation.Animation.ColumnCount;
}

template<bool IsRenderProxy>
Size2UI TSpriteImage<IsRenderProxy>::GetSize() const
{
	const TextureType& atlasTexture = GetAtlasTexture();
	const TextureProperties& atlasTextureProperties = atlasTexture->GetProperties();

	return Size2UI(
		Math::RoundToI32(atlasTextureProperties.Width * mInformation.UVRange.Width),
		Math::RoundToI32(atlasTextureProperties.Height * mInformation.UVRange.Height));
}

template<bool IsRenderProxy>
Size2UI TSpriteImage<IsRenderProxy>::GetAnimationFrameSize() const
{
	const Size2UI size = GetSize();

	return Size2UI(
		size.Width / Math::Max(1U, mInformation.Animation.ColumnCount),
		size.Height / Math::Max(1U, mInformation.Animation.RowCount));
}

template class TSpriteImage<true>;
template class TSpriteImage<false>;

namespace bs
{
	B3D_SYNC_BLOCK_BEGIN(SpriteImage, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY(mInformation)
	B3D_SYNC_BLOCK_END
}

bool SpriteImage::CheckIsLoaded(const HSpriteImage& image)
{
	return image != nullptr && image.IsLoaded(false) && image->GetAtlasTexture() != nullptr && image->GetAtlasTexture().IsLoaded(false);
}

void SpriteImage::MarkRenderProxyDataDirtyInternal()
{
	MarkRenderProxyDataDirty();
}

SPtr<ct::RenderProxy> SpriteImage::CreateRenderProxy() const
{
	ct::SpriteImage* const renderProxy = new(B3DAllocate<ct::SpriteImage>()) ct::SpriteImage(mInformation);

	SPtr<ct::SpriteImage> renderProxyShared = B3DMakeSharedFromExisting<ct::SpriteImage>(renderProxy);
	renderProxyShared->SetShared(renderProxyShared);

	return renderProxyShared;
}

RenderProxySyncPacket* SpriteImage::CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags)
{
	return allocator.Construct<SyncPacket>(*this, allocator, flags);
}

RTTITypeBase* SpriteImage::GetRttiStatic()
{
	return SpriteImageRTTI::Instance();
}

RTTITypeBase* SpriteImage::GetRtti() const
{
	return GetRttiStatic();
}

namespace bs { namespace ct
{
void SpriteImage::SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator)
{
	auto* const syncPacket = data.GetSyncPacket<bs::SpriteImage::SyncPacket>();
	if(!syncPacket)
		return;

	syncPacket->ApplySyncData(this);
}
}}

namespace bs
{
	B3D_SYNC_BLOCK_BEGIN(SpriteTexture, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY(mAtlasTexture)
		B3D_SYNC_BLOCK_ENTRY_PACKET_BASE(SpriteImage, SpriteImageSyncPacket)
	B3D_SYNC_BLOCK_END
}

SpriteTexture::SpriteTexture(const SpriteTextureCreateInformation& createInformation)
	: SpriteImage(createInformation)
{
	mAtlasTexture = createInformation.AtlasTexture;
}

bool SpriteTexture::CheckIsLoaded(const HSpriteTexture& texture)
{
	return texture != nullptr && texture.IsLoaded(false) && texture->GetAtlasTexture() != nullptr && texture->GetAtlasTexture().IsLoaded(false);
}

void SpriteTexture::SetAtlasTexture(const HTexture& texture)
{
	RemoveResourceDependency(mAtlasTexture);
	mAtlasTexture = texture;
	AddResourceDependency(mAtlasTexture);

	MarkDependenciesDirty();
	MarkRenderProxyDataDirtyInternal();
}

void SpriteTexture::Initialize()
{
	AddResourceDependency(mAtlasTexture);

	Resource::Initialize();
}

SPtr<ct::RenderProxy> SpriteTexture::CreateRenderProxy() const
{
	SPtr<ct::Texture> atlasRenderProxy = B3DGetRenderProxy(mAtlasTexture);

	ct::SpriteTextureCreateInformation createInformation(mInformation, std::move(atlasRenderProxy));
	ct::SpriteTexture* const renderProxy = new(B3DAllocate<ct::SpriteTexture>()) ct::SpriteTexture(createInformation);

	SPtr<ct::SpriteTexture> renderProxyShared = B3DMakeSharedFromExisting<ct::SpriteTexture>(renderProxy);
	renderProxyShared->SetShared(renderProxyShared);

	return renderProxyShared;
}

RenderProxySyncPacket* SpriteTexture::CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags)
{
	auto syncPacket = allocator.Construct<SyncPacket>(*this, allocator, flags);
	if(B3D_ENSURE(syncPacket))
		syncPacket->SpriteImageSyncPacket = allocator.Construct<SpriteImage::SyncPacket>(*this, allocator, flags);

	return syncPacket;
}

void SpriteTexture::GetCoreDependencies(Vector<CoreObject*>& dependencies)
{
	if(mAtlasTexture.IsLoaded())
		dependencies.push_back(mAtlasTexture.Get());
}

HSpriteTexture SpriteTexture::Create(const HTexture& texture)
{
	SPtr<SpriteTexture> texturePtr = CreateShared(texture);

	return B3DStaticResourceCast<SpriteTexture>(GetResources().CreateResourceHandle(texturePtr));
}

HSpriteTexture SpriteTexture::Create(const SpriteTextureCreateInformation& createInformation)
{
	SPtr<SpriteTexture> texture = CreateShared(createInformation);

	return B3DStaticResourceCast<SpriteTexture>(GetResources().CreateResourceHandle(texture));
}

SPtr<SpriteTexture> SpriteTexture::CreateShared(const HTexture& texture)
{
	SpriteTextureCreateInformation createInformation;
	createInformation.AtlasTexture = texture;

	return CreateShared(createInformation);
}

SPtr<SpriteTexture> SpriteTexture::CreateShared(const SpriteTextureCreateInformation& createInformation)
{
	SPtr<SpriteTexture> texture = B3DMakeSharedFromExisting<SpriteTexture>(new(B3DAllocate<SpriteTexture>()) SpriteTexture(createInformation));

	texture->SetShared(texture);
	texture->Initialize();

	return texture;
}

SPtr<SpriteTexture> SpriteTexture::CreateEmpty()
{
	SPtr<SpriteTexture> texture = B3DMakeSharedFromExisting<SpriteTexture>(new(B3DAllocate<SpriteTexture>()) SpriteTexture(SpriteTextureCreateInformation()));
	texture->SetShared(texture);

	return texture;
}

RTTITypeBase* SpriteTexture::GetRttiStatic()
{
	return SpriteTextureRTTI::Instance();
}

RTTITypeBase* SpriteTexture::GetRtti() const
{
	return GetRttiStatic();
}

namespace bs { namespace ct
{
SpriteTexture::SpriteTexture(const SpriteTextureCreateInformation& createInformation)
	: SpriteImage(createInformation)
{
	mAtlasTexture = createInformation.AtlasTexture;
}

void SpriteTexture::SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator)
{
	auto* const syncPacket = data.GetSyncPacket<bs::SpriteTexture::SyncPacket>();
	if(!syncPacket)
		return;

	syncPacket->ApplySyncData(this);
}
}}

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

void SpriteGlyph::SetGlyphSize(u32 size)
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
		syncPacket->SpriteImageSyncPacket = allocator.Construct<SpriteImage::SyncPacket>(*this, allocator, flags);

	return syncPacket;
}

void SpriteGlyph::GetCoreDependencies(Vector<CoreObject*>& dependencies)
{
	if(mAtlasTexture.IsLoaded())
		dependencies.push_back(mAtlasTexture.Get());
}

HSpriteGlyph SpriteGlyph::Create(const HFont& font, u32 glyph, u32 size)
{
	SPtr<SpriteGlyph> spriteGlyph = CreateShared(font, glyph, size);

	return B3DStaticResourceCast<SpriteGlyph>(GetResources().CreateResourceHandle(spriteGlyph));
}

HSpriteGlyph SpriteGlyph::Create(const SpriteGlyphCreateInformation& createInformation)
{
	SPtr<SpriteGlyph> spriteGlyph = CreateShared(createInformation);

	return B3DStaticResourceCast<SpriteGlyph>(GetResources().CreateResourceHandle(spriteGlyph));
}

SPtr<SpriteGlyph> SpriteGlyph::CreateShared(const HFont& font, u32 glyph, u32 size)
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
