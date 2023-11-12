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
