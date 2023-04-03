//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Renderer/BsReflectionProbe.h"
#include "Private/RTTI/BsReflectionProbeRTTI.h"
#include "RTTI/BsMathRTTI.h"
#include "Scene/BsSceneObject.h"
#include "Image/BsTexture.h"
#include "Renderer/BsRenderer.h"
#include "Utility/BsUUID.h"
#include "Renderer/BsIBLUtility.h"
#include "CoreThread/BsCoreObjectSync.h"
#include "RenderAPI/BsRenderAPI.h"

using namespace bs;

ReflectionProbeBase::ReflectionProbeBase(ReflectionProbeType type, float radius, const Vector3& extents)
	: mType(type), mRadius(radius), mExtents(extents)
{}

float ReflectionProbeBase::GetRadius() const
{
	Vector3 scale = mTransform.GetScale();
	return mRadius * std::max(std::max(scale.X, scale.Y), scale.Z);
}

void ReflectionProbeBase::UpdateBounds()
{
	Vector3 position = mTransform.GetPosition();
	Vector3 scale = mTransform.GetScale();

	switch(mType)
	{
	case ReflectionProbeType::Sphere:
		mBounds = Sphere(position, mRadius * std::max(std::max(scale.X, scale.Y), scale.Z));
		break;
	case ReflectionProbeType::Box:
		mBounds = Sphere(position, (mExtents * scale).Length());
		break;
	}
}

template <bool Core>
template <class P>
void TReflectionProbe<Core>::RttiEnumFields(P p)
{
	p(mType);
	p(mRadius);
	p(mExtents);
	p(mTransitionDistance);
	p(mBounds);
	p(mFilteredTexture);
}

ReflectionProbe::ReflectionProbe(ReflectionProbeType type, float radius, const Vector3& extents)
	: TReflectionProbe(type, radius, extents)
{
	// Calling virtual method is okay here because this is the most derived type
	UpdateBounds();
}

ReflectionProbe::~ReflectionProbe()
{
	if(mRendererTask)
		mRendererTask->Cancel();
}

void ReflectionProbe::Capture()
{
	if(mCustomTexture != nullptr)
		return;

	CaptureAndFilter();
}

void ReflectionProbe::Filter()
{
	if(mCustomTexture == nullptr)
		return;

	CaptureAndFilter();
}

void ReflectionProbe::CaptureAndFilter()
{
	// If previous rendering task exists, cancel it
	if(mRendererTask != nullptr)
		mRendererTask->Cancel();

	TextureCreateInformation cubemapDesc;
	cubemapDesc.Name = "ReflectionProbe Cubemap";
	cubemapDesc.Type = TEX_TYPE_CUBE_MAP;
	cubemapDesc.Format = PF_RG11B10F;
	cubemapDesc.Width = ct::IBLUtility::kReflectionCubemapSize;
	cubemapDesc.Height = ct::IBLUtility::kReflectionCubemapSize;
	cubemapDesc.MipMapCount = PixelUtil::GetMaxMipmaps(cubemapDesc.Width, cubemapDesc.Height, 1, cubemapDesc.Format);
	cubemapDesc.Usage = TU_STATIC | TU_RENDERTARGET;

	mFilteredTexture = Texture::CreateShared(cubemapDesc);

	auto renderComplete = [this]()
	{
		mRendererTask = nullptr;
	};

	SPtr<ct::ReflectionProbe> coreProbe = GetCore();
	SPtr<ct::Texture> coreTexture = mFilteredTexture->GetCore();

	if(mCustomTexture == nullptr)
	{
		auto renderReflProbe = [coreTexture, coreProbe]()
		{
			const SPtr<ct::CommandBuffer> commandBuffer = ct::GetRenderAPI().GetMainCommandBuffer();
			float radius = coreProbe->mType == ReflectionProbeType::Sphere ? coreProbe->mRadius : coreProbe->mExtents.Length();

			ct::CaptureSettings settings;
			settings.EncodeDepth = true;
			settings.DepthEncodeNear = radius;
			settings.DepthEncodeFar = radius + 1; // + 1 arbitrary, make it a customizable value?

			ct::GetRenderer()->CaptureSceneCubeMap(coreTexture, coreProbe->GetTransform().GetPosition(), settings);
			ct::GetIBLUtility().FilterCubemapForSpecular(*commandBuffer, coreTexture, nullptr);

			coreProbe->mFilteredTexture = coreTexture;
			ct::GetRenderer()->NotifyReflectionProbeUpdated(coreProbe.get(), true);

			return true;
		};

		mRendererTask = ct::RendererTask::Create("ReflProbeRender", renderReflProbe);
	}
	else
	{
		SPtr<ct::Texture> coreCustomTex = mCustomTexture->GetCore();
		auto filterReflProbe = [coreCustomTex, coreTexture, coreProbe]()
		{
			const SPtr<ct::CommandBuffer> commandBuffer = ct::GetRenderAPI().GetMainCommandBuffer();

			ct::GetIBLUtility().ScaleCubemap(*commandBuffer, coreCustomTex, 0, coreTexture, 0);
			ct::GetIBLUtility().FilterCubemapForSpecular(*commandBuffer, coreTexture, nullptr);

			coreProbe->mFilteredTexture = coreTexture;
			ct::GetRenderer()->NotifyReflectionProbeUpdated(coreProbe.get(), true);

			return true;
		};

		mRendererTask = ct::RendererTask::Create("ReflProbeRender", filterReflProbe);
	}

	mRendererTask->OnComplete.Connect(renderComplete);
	ct::GetRenderer()->AddTask(mRendererTask);
}

SPtr<ct::ReflectionProbe> ReflectionProbe::GetCore() const
{
	return std::static_pointer_cast<ct::ReflectionProbe>(mCoreSpecific);
}

SPtr<ReflectionProbe> ReflectionProbe::CreateSphere(float radius)
{
	ReflectionProbe* probe = new(B3DAllocate<ReflectionProbe>()) ReflectionProbe(ReflectionProbeType::Sphere, radius, Vector3::kZero);
	SPtr<ReflectionProbe> probePtr = B3DMakeCoreFromExisting<ReflectionProbe>(probe);
	probePtr->SetShared(probePtr);
	probePtr->Initialize();

	return probePtr;
}

SPtr<ReflectionProbe> ReflectionProbe::CreateBox(const Vector3& extents)
{
	ReflectionProbe* probe = new(B3DAllocate<ReflectionProbe>()) ReflectionProbe(ReflectionProbeType::Box, 1.0f, extents);
	SPtr<ReflectionProbe> probePtr = B3DMakeCoreFromExisting<ReflectionProbe>(probe);
	probePtr->SetShared(probePtr);
	probePtr->Initialize();

	return probePtr;
}

SPtr<ReflectionProbe> ReflectionProbe::CreateEmpty()
{
	ReflectionProbe* probe = new(B3DAllocate<ReflectionProbe>()) ReflectionProbe();
	SPtr<ReflectionProbe> probePtr = B3DMakeCoreFromExisting<ReflectionProbe>(probe);
	probePtr->SetShared(probePtr);

	return probePtr;
}

SPtr<ct::CoreObject> ReflectionProbe::CreateCore() const
{
	SPtr<ct::Texture> filteredTexture;
	if(mFilteredTexture != nullptr)
		filteredTexture = mFilteredTexture->GetCore();

	ct::ReflectionProbe* probe = new(B3DAllocate<ct::ReflectionProbe>()) ct::ReflectionProbe(mType, mRadius, mExtents, filteredTexture);
	SPtr<ct::ReflectionProbe> probePtr = B3DMakeSharedFromExisting<ct::ReflectionProbe>(probe);
	probePtr->SetShared(probePtr);

	return probePtr;
}

CoreSyncData ReflectionProbe::SyncToCore(FrameAlloc* allocator)
{
	u32 size = 0;
	size += B3DRTTISize(GetCoreDirtyFlags()).Bytes;
	size += CoreSyncGetSize((SceneActor&)*this);
	size += CoreSyncGetSize(*this);

	u8* buffer = allocator->Alloc(size);

	Bitstream stream(buffer, size);
	B3DRTTIWrite(GetCoreDirtyFlags(), stream);
	B3DCoreSyncWrite((SceneActor&)*this, stream);
	B3DCoreSyncWrite(*this, stream);

	return CoreSyncData(buffer, size);
}

void ReflectionProbe::MarkCoreDirtyInternal(ActorDirtyFlag flags)
{
	MarkCoreDirty((u32)flags);
}

RTTITypeBase* ReflectionProbe::GetRttiStatic()
{
	return ReflectionProbeRTTI::Instance();
}

RTTITypeBase* ReflectionProbe::GetRtti() const
{
	return ReflectionProbe::GetRttiStatic();
}

template class TReflectionProbe<true>;
template class TReflectionProbe<false>;

namespace bs { namespace ct
{
ReflectionProbe::ReflectionProbe(ReflectionProbeType type, float radius, const Vector3& extents, const SPtr<Texture>& filteredTexture)
	: TReflectionProbe(type, radius, extents), mRendererId(0)
{
	mFilteredTexture = filteredTexture;
}

ReflectionProbe::~ReflectionProbe()
{
	GetRenderer()->NotifyReflectionProbeRemoved(this);
}

void ReflectionProbe::Initialize()
{
	UpdateBounds();
	GetRenderer()->NotifyReflectionProbeAdded(this);

	CoreObject::Initialize();
}

void ReflectionProbe::SyncToCore(const CoreSyncData& data)
{
	Bitstream stream(data.GetBuffer(), data.GetBufferSize());

	u32 dirtyFlags = 0;
	bool oldIsActive = mActive;
	ReflectionProbeType oldType = mType;

	B3DRTTIRead(dirtyFlags, stream);
	B3DCoreSyncRead((SceneActor&)*this, stream);
	B3DCoreSyncRead(*this, stream);

	UpdateBounds();

	if(dirtyFlags == (u32)ActorDirtyFlag::Transform)
	{
		if(mActive)
			GetRenderer()->NotifyReflectionProbeUpdated(this, false);
	}
	else
	{
		if(oldIsActive != mActive)
		{
			if(mActive)
				GetRenderer()->NotifyReflectionProbeAdded(this);
			else
			{
				ReflectionProbeType newType = mType;
				mType = oldType;
				GetRenderer()->NotifyReflectionProbeRemoved(this);
				mType = newType;
			}
		}
		else
		{
			ReflectionProbeType newType = mType;
			mType = oldType;
			GetRenderer()->NotifyReflectionProbeRemoved(this);
			mType = newType;

			GetRenderer()->NotifyReflectionProbeAdded(this);
		}
	}
}
}}
