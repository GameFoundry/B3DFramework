//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Image/B3DSpriteTexture.h"
#include "RTTI/B3DSpriteTextureRTTI.h"
#include "Image/B3DTexture.h"
#include "Resources/B3DResources.h"
#include "Resources/B3DBuiltinResources.h"
#include "CoreObject/B3DCoreObjectSync.h"
#include "Text/B3DFont.h"

using namespace b3d;

namespace b3d
{
	B3D_SYNC_BLOCK_BEGIN(SpriteTexture, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY(mAtlasTexture)
		B3D_SYNC_BLOCK_ENTRY(mUVRange)
		B3D_SYNC_BLOCK_ENTRY_PACKET_BASE(SpriteImage, SpriteImageSyncPacket)
	B3D_SYNC_BLOCK_END
}

SpriteTexture::SpriteTexture(const SpriteTextureCreateInformation& createInformation)
	: SpriteImage(createInformation)
{
	mAtlasTexture = createInformation.AtlasTexture;
	mUVRange = createInformation.UVRange;
}

void SpriteTexture::Initialize()
{
	mDefaultAllocatedImage = SpriteImageAllocation::Create(std::static_pointer_cast<SpriteTexture>(GetShared()), mAtlasTexture, mUVRange);
	AddResourceDependency(mAtlasTexture);

	SpriteImage::Initialize();
}

SPtr<render::RenderProxy> SpriteTexture::CreateRenderProxy() const
{
	SPtr<render::Texture> atlasRenderProxy = B3DGetRenderProxy(mAtlasTexture);

	render::SpriteTextureCreateInformation createInformation(mInformation, std::move(atlasRenderProxy));
	render::SpriteTexture* const renderProxy = new(B3DAllocate<render::SpriteTexture>()) render::SpriteTexture(createInformation, B3DGetRenderProxy(mDefaultAllocatedImage));

	SPtr<render::SpriteTexture> renderProxyShared = B3DMakeSharedFromExisting<render::SpriteTexture>(renderProxy);
	renderProxyShared->SetShared(renderProxyShared);

	return renderProxyShared;
}

RenderProxySyncPacket* SpriteTexture::CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags)
{
	auto syncPacket = allocator.Construct<SyncPacket>(*this, allocator, flags);
	if(B3D_ENSURE(syncPacket))
		syncPacket->SpriteImageSyncPacket = SpriteImage::CreateRenderProxySyncPacket(allocator, flags);

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

RTTIType* SpriteTexture::GetRttiStatic()
{
	return SpriteTextureRTTI::Instance();
}

RTTIType* SpriteTexture::GetRtti() const
{
	return GetRttiStatic();
}

namespace b3d { namespace render
{
SpriteTexture::SpriteTexture(const SpriteTextureCreateInformation& createInformation, const SPtr<SpriteImageAllocation>& defaultAllocatedImage)
	: SpriteImage(createInformation, defaultAllocatedImage)
{
	mAtlasTexture = createInformation.AtlasTexture;
	mUVRange = createInformation.UVRange;
}

void SpriteTexture::SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator)
{
	auto* const syncPacket = data.GetSyncPacket<b3d::SpriteTexture::SyncPacket>();
	if(!syncPacket)
		return;

	syncPacket->ApplySyncData(this);
}
}}
