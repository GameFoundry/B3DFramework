//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Image/BsSpriteVectorPath.h"

#include "Private/RTTI/BsSpriteVectorPathRTTI.h"
#include "Image/BsTexture.h"
#include "Resources/BsResources.h"
#include "Resources/BsBuiltinResources.h"
#include "CoreObject/BsCoreObjectSync.h"
#include "GUI/BsGUIManager.h"
#include "VectorGraphics/BsVectorGraphics.h"
#include "VectorGraphics/BsVectorSpriteAtlas.h"

using namespace bs;

namespace bs
{
	B3D_SYNC_BLOCK_BEGIN(SpriteVectorPath, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY(mAtlasTexture)
		B3D_SYNC_BLOCK_ENTRY_PACKET_BASE(SpriteImage, SpriteImageSyncPacket)
	B3D_SYNC_BLOCK_END
}

SpriteVectorPath::SpriteVectorPath(const SpriteVectorPathCreateInformation& createInformation)
	: SpriteImage(createInformation), mVectorPath(createInformation.VectorPath), mSize(createInformation.Size), mScalingMode(createInformation.ScalingMode)
{
}

void SpriteVectorPath::SetVectorPath(const HVectorPath& vectorPath)
{
	if(mVectorPath == vectorPath)
		return;

	RemoveResourceDependency(mVectorPath);
	mVectorPath = vectorPath;
	AddResourceDependency(mVectorPath);

	RenderVectorPath();
}

void SpriteVectorPath::RenderVectorPath()
{
	mAtlasTexture = nullptr;
	mInformation.UVRange = Area2::kEmpty;

	if(!mVectorPath.IsLoaded(false))
		return;

	if(mSize.Width == 0 || mSize.Height == 0)
		return;

	VectorGraphicsSettings vectorGraphicsSettings;
	vectorGraphicsSettings.Size = Size2((float)mSize.Width, (float)mSize.Height);
	vectorGraphicsSettings.ScalingMode = mScalingMode;

	GUIVectorSpriteAtlas& vectorSpriteAtlas = GetGUIManager().GetVectorSpriteAtlas();

	mSpriteAtlasAllocation = vectorSpriteAtlas.Allocate(*mVectorPath, vectorGraphicsSettings);
	if(!B3D_ENSURE(mSpriteAtlasAllocation))
		return;

	mAtlasTexture = mSpriteAtlasAllocation->AtlasTexture;
	mInformation.UVRange = mSpriteAtlasAllocation->UVRange;

	MarkDependenciesDirty();
	MarkRenderProxyDataDirtyInternal();
}

void SpriteVectorPath::Initialize()
{
	RenderVectorPath();
	AddResourceDependency(mVectorPath);

	Resource::Initialize();
}

SPtr<ct::RenderProxy> SpriteVectorPath::CreateRenderProxy() const
{
	SPtr<ct::Texture> atlasRenderProxy = B3DGetRenderProxy(mAtlasTexture);

	ct::SpriteVectorPathCreateInformation createInformation(mInformation, std::move(atlasRenderProxy));
	ct::SpriteVectorPath* const renderProxy = new(B3DAllocate<ct::SpriteVectorPath>()) ct::SpriteVectorPath(createInformation);

	SPtr<ct::SpriteVectorPath> renderProxyShared = B3DMakeSharedFromExisting<ct::SpriteVectorPath>(renderProxy);
	renderProxyShared->SetShared(renderProxyShared);

	return renderProxyShared;
}

RenderProxySyncPacket* SpriteVectorPath::CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags)
{
	auto syncPacket = allocator.Construct<SyncPacket>(*this, allocator, flags);
	if(B3D_ENSURE(syncPacket))
		syncPacket->SpriteImageSyncPacket = SpriteImage::CreateRenderProxySyncPacket(allocator, flags);

	return syncPacket;
}

void SpriteVectorPath::GetCoreDependencies(Vector<CoreObject*>& dependencies)
{
	if(mAtlasTexture.IsLoaded())
		dependencies.push_back(mAtlasTexture.Get());
}

HSpriteVectorPath SpriteVectorPath::Create(const HVectorPath& vectorPath, const Size2UI& size)
{
	SPtr<SpriteVectorPath> spriteVectorPath = CreateShared(vectorPath, size);

	return B3DStaticResourceCast<SpriteVectorPath>(GetResources().CreateResourceHandle(spriteVectorPath));
}

HSpriteVectorPath SpriteVectorPath::Create(const SpriteVectorPathCreateInformation& createInformation)
{
	SPtr<SpriteVectorPath> spriteVectorPath = CreateShared(createInformation);

	return B3DStaticResourceCast<SpriteVectorPath>(GetResources().CreateResourceHandle(spriteVectorPath));
}

SPtr<SpriteVectorPath> SpriteVectorPath::CreateShared(const HVectorPath& vectorPath, const Size2UI& size)
{
	SpriteVectorPathCreateInformation createInformation;
	createInformation.VectorPath = vectorPath;
	createInformation.Size = size;

	return CreateShared(createInformation);
}

SPtr<SpriteVectorPath> SpriteVectorPath::CreateShared(const SpriteVectorPathCreateInformation& createInformation)
{
	SPtr<SpriteVectorPath> spriteVectorPath = B3DMakeSharedFromExisting<SpriteVectorPath>(new(B3DAllocate<SpriteVectorPath>()) SpriteVectorPath(createInformation));

	spriteVectorPath->SetShared(spriteVectorPath);
	spriteVectorPath->Initialize();

	return spriteVectorPath;
}

SPtr<SpriteVectorPath> SpriteVectorPath::CreateEmpty()
{
	SPtr<SpriteVectorPath> spriteVectorPath = B3DMakeSharedFromExisting<SpriteVectorPath>(new(B3DAllocate<SpriteVectorPath>()) SpriteVectorPath(SpriteVectorPathCreateInformation()));
	spriteVectorPath->SetShared(spriteVectorPath);

	return spriteVectorPath;
}

RTTIType* SpriteVectorPath::GetRttiStatic()
{
	return SpriteVectorPathRTTI::Instance();
}

RTTIType* SpriteVectorPath::GetRtti() const
{
	return GetRttiStatic();
}

namespace bs { namespace ct
{
SpriteVectorPath::SpriteVectorPath(const SpriteVectorPathCreateInformation& createInformation)
	: SpriteImage(createInformation)
{
	mAtlasTexture = createInformation.AtlasTexture;
}

void SpriteVectorPath::SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator)
{
	auto* const syncPacket = data.GetSyncPacket<bs::SpriteVectorPath::SyncPacket>();
	if(!syncPacket)
		return;

	syncPacket->ApplySyncData(this);
}
}}
