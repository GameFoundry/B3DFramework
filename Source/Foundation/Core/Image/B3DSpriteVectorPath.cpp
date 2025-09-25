//************************************ B3D Framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Image/B3DSpriteVectorPath.h"

#include "RTTI/B3DSpriteVectorPathRTTI.h"
#include "Image/B3DTexture.h"
#include "Resources/B3DResources.h"
#include "Resources/B3DBuiltinResources.h"
#include "CoreObject/B3DCoreObjectSync.h"
#include "GUI/B3DGUIManager.h"
#include "VectorGraphics/B3DVectorGraphics.h"
#include "VectorGraphics/B3DVectorSpriteAtlas.h"

using namespace b3d;

namespace b3d
{
	B3D_SYNC_BLOCK_BEGIN(SpriteVectorPathAllocation, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY(mVectorSpriteAtlasAllocationHandle)
		B3D_SYNC_BLOCK_ENTRY_PACKET_BASE(SpriteImageAllocation, SpriteImageSyncPacket)
	B3D_SYNC_BLOCK_END
}

SPtr<SpriteVectorPathAllocation> SpriteVectorPathAllocation::Create(const WeakSPtr<SpriteImageType>& owner, const GUIVectorSpriteAtlasAllocation& vectorSpriteAtlasAllocation)
{
	SpriteVectorPathAllocation* allocation = new(B3DAllocate<SpriteVectorPathAllocation>()) SpriteVectorPathAllocation(owner, vectorSpriteAtlasAllocation);
	SPtr<SpriteVectorPathAllocation> allocationShared = B3DMakeSharedFromExisting<SpriteVectorPathAllocation>(allocation);
	allocationShared->SetShared(allocationShared);
	allocationShared->Initialize();

	return allocationShared;
}

SPtr<render::RenderProxy> SpriteVectorPathAllocation::CreateRenderProxy() const
{
	const SPtr<render::SpriteImage> owner = B3DGetRenderProxy(mOwner.lock());
	const SPtr<render::Texture> atlasTexture = B3DGetRenderProxy(mTexture);

	render::SpriteVectorPathAllocation* const renderProxy = new(B3DAllocate<render::SpriteVectorPathAllocation>()) render::SpriteVectorPathAllocation(owner, atlasTexture, mUVRange, mVectorSpriteAtlasAllocationHandle);

	SPtr<render::SpriteVectorPathAllocation> renderProxyShared = B3DMakeSharedFromExisting<render::SpriteVectorPathAllocation>(renderProxy);
	renderProxyShared->SetShared(renderProxyShared);

	return renderProxyShared;
}

RenderProxySyncPacket* SpriteVectorPathAllocation::CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags)
{
	auto syncPacket = allocator.Construct<SyncPacket>(*this, allocator, flags);
	if(B3D_ENSURE(syncPacket))
		syncPacket->SpriteImageSyncPacket = SpriteImageAllocation::CreateRenderProxySyncPacket(allocator, flags);

	return syncPacket;
}

namespace b3d
{
	B3D_SYNC_BLOCK_BEGIN(SpriteVectorPath, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY_PACKET_BASE(SpriteImage, SpriteImageSyncPacket)
	B3D_SYNC_BLOCK_END
}

SpriteVectorPath::SpriteVectorPath(const SpriteVectorPathCreateInformation& createInformation)
	: SpriteImage(createInformation), mVectorPath(createInformation.VectorPath), mDefaultSize(createInformation.DefaultSize), mScalingMode(createInformation.ScalingMode)
{
}

SPtr<SpriteImageAllocation> SpriteVectorPath::FindOrAllocateImageToFitArea(const Size2I& size)
{
	if(!mVectorPath.IsLoaded(false))
		return nullptr;

	if(mDefaultSize.Width == 0 || mDefaultSize.Height == 0)
		return nullptr;

	auto foundImage = std::find_if(mScaledAllocatedImages.begin(), mScaledAllocatedImages.end(), [&size](const SpriteImageAllocation* allocation) {
		return allocation->GetSize() == size;
	});

	if(foundImage != mScaledAllocatedImages.end())
		return std::static_pointer_cast<SpriteImageAllocation>((*foundImage)->GetShared());

	SPtr<SpriteVectorPathAllocation> allocation = AllocateImage(size);
	mScaledAllocatedImages.Add(allocation.get());

	return allocation;
}

SPtr<SpriteImageAllocation> SpriteVectorPath::FindOrAllocateScaledImage(float scale)
{
	const Size2I scaledSize(
		Math::RoundToI32((float)mDefaultSize.Width * scale),
		Math::RoundToI32((float)mDefaultSize.Height * scale)
		);

	return FindOrAllocateImageToFitArea(scaledSize);
}

SPtr<SpriteVectorPathAllocation> SpriteVectorPath::AllocateImage(const Size2I& size)
{
	VectorGraphicsSettings vectorGraphicsSettings;
	vectorGraphicsSettings.Size = Size2((float)size.Width, (float)size.Height);
	vectorGraphicsSettings.ScalingMode = mScalingMode;

	GUIVectorSpriteAtlasAllocation spriteAtlasAllocation;
	if(size.Width != 0 && size.Height != 0)
	{
		GUIVectorSpriteAtlas& vectorSpriteAtlas = GetGUIManager().GetVectorSpriteAtlas();
		spriteAtlasAllocation = vectorSpriteAtlas.Allocate(*mVectorPath, vectorGraphicsSettings);
	}

	return SpriteVectorPathAllocation::Create(std::static_pointer_cast<SpriteVectorPath>(GetShared()), spriteAtlasAllocation);
}

void SpriteVectorPath::Initialize()
{
	mDefaultAllocatedImage = AllocateImage(mDefaultSize);
	AddResourceDependency(mVectorPath);

	Resource::Initialize();
}

SPtr<render::RenderProxy> SpriteVectorPath::CreateRenderProxy() const
{
	render::SpriteVectorPathCreateInformation createInformation(mInformation);
	render::SpriteVectorPath* const renderProxy = new(B3DAllocate<render::SpriteVectorPath>()) render::SpriteVectorPath(createInformation, B3DGetRenderProxy(mDefaultAllocatedImage));

	SPtr<render::SpriteVectorPath> renderProxyShared = B3DMakeSharedFromExisting<render::SpriteVectorPath>(renderProxy);
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

HSpriteVectorPath SpriteVectorPath::Create(const HVectorPath& vectorPath, const Size2I& defaultSize)
{
	SPtr<SpriteVectorPath> spriteVectorPath = CreateShared(vectorPath, defaultSize);

	return B3DStaticResourceCast<SpriteVectorPath>(GetResources().CreateResourceHandle(spriteVectorPath));
}

HSpriteVectorPath SpriteVectorPath::Create(const SpriteVectorPathCreateInformation& createInformation)
{
	SPtr<SpriteVectorPath> spriteVectorPath = CreateShared(createInformation);

	return B3DStaticResourceCast<SpriteVectorPath>(GetResources().CreateResourceHandle(spriteVectorPath));
}

SPtr<SpriteVectorPath> SpriteVectorPath::CreateShared(const HVectorPath& vectorPath, const Size2I& defaultSize)
{
	SpriteVectorPathCreateInformation createInformation;
	createInformation.VectorPath = vectorPath;
	createInformation.DefaultSize = defaultSize;

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

namespace b3d { namespace render
{
SpriteVectorPathAllocation::SpriteVectorPathAllocation(const WeakSPtr<SpriteImageType>& owner, const TextureType& atlasTexture, const Area2& uvRange, const SPtr<GUIVectorSpriteAtlasAllocationHandle>& vectorSpriteAtlasAllocationHandle)
	: SpriteImageAllocation(owner, atlasTexture, uvRange), mVectorSpriteAtlasAllocationHandle(vectorSpriteAtlasAllocationHandle)
{ }

void SpriteVectorPathAllocation::SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator)
{
	auto* const syncPacket = data.GetSyncPacket<b3d::SpriteVectorPathAllocation::SyncPacket>();
	if(!syncPacket)
		return;

	syncPacket->ApplySyncData(this);
}

SpriteVectorPath::SpriteVectorPath(const SpriteVectorPathCreateInformation& createInformation, const SPtr<SpriteImageAllocation>& defaultAllocatedImage)
	: SpriteImage(createInformation, defaultAllocatedImage)
{
}

void SpriteVectorPath::SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator)
{
	auto* const syncPacket = data.GetSyncPacket<b3d::SpriteVectorPath::SyncPacket>();
	if(!syncPacket)
		return;

	syncPacket->ApplySyncData(this);
}
}}
