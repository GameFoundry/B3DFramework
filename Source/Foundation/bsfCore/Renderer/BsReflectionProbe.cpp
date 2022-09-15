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

namespace bs
{
	ReflectionProbeBase::ReflectionProbeBase(ReflectionProbeType type, float radius, const Vector3& extents)
		: mType(type), mRadius(radius), mExtents(extents)
	{ }

	float ReflectionProbeBase::GetRadius() const
	{
		Vector3 scale = mTransform.GetScale();
		return mRadius * std::max(std::max(scale.x, scale.y), scale.z);
	}

	void ReflectionProbeBase::UpdateBounds()
	{
		Vector3 position = mTransform.GetPosition();
		Vector3 scale = mTransform.GetScale();

		switch (mType)
		{
		case ReflectionProbeType::Sphere:
			mBounds = Sphere(position, mRadius * std::max(std::max(scale.x, scale.y), scale.z));
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
		if (mRendererTask)
			mRendererTask->Cancel();
	}

	void ReflectionProbe::Capture()
	{
		if (mCustomTexture != nullptr)
			return;

		CaptureAndFilter();
	}

	void ReflectionProbe::Filter()
	{
		if (mCustomTexture == nullptr)
			return;

		CaptureAndFilter();
	}

	void ReflectionProbe::CaptureAndFilter()
	{
		// If previous rendering task exists, cancel it
		if (mRendererTask != nullptr)
			mRendererTask->Cancel();

		TEXTURE_DESC cubemapDesc;
		cubemapDesc.type = TEX_TYPE_CUBE_MAP;
		cubemapDesc.format = PF_RG11B10F;
		cubemapDesc.width = ct::IBLUtility::REFLECTION_CUBEMAP_SIZE;
		cubemapDesc.height = ct::IBLUtility::REFLECTION_CUBEMAP_SIZE;
		cubemapDesc.numMips = PixelUtil::GetMaxMipmaps(cubemapDesc.width, cubemapDesc.height, 1, cubemapDesc.format);
		cubemapDesc.usage = TU_STATIC | TU_RENDERTARGET;

		mFilteredTexture = Texture::CreatePtrInternal(cubemapDesc);

		auto renderComplete = [this]()
		{
			mRendererTask = nullptr;
		};

		SPtr<ct::ReflectionProbe> coreProbe = GetCore();
		SPtr<ct::Texture> coreTexture = mFilteredTexture->GetCore();

		if (mCustomTexture == nullptr)
		{
			auto renderReflProbe = [coreTexture, coreProbe]()
			{
				float radius = coreProbe->mType == ReflectionProbeType::Sphere ? coreProbe->mRadius :
					coreProbe->mExtents.Length();

				ct::CaptureSettings settings;
				settings.encodeDepth = true;
				settings.depthEncodeNear = radius;
				settings.depthEncodeFar = radius + 1; // + 1 arbitrary, make it a customizable value?

				ct::gRenderer()->CaptureSceneCubeMap(coreTexture, coreProbe->GetTransform().GetPosition(), settings);
				ct::gIBLUtility().FilterCubemapForSpecular(coreTexture, nullptr);

				coreProbe->mFilteredTexture = coreTexture;
				ct::gRenderer()->NotifyReflectionProbeUpdated(coreProbe.get(), true);

				return true;
			};

			mRendererTask = ct::RendererTask::Create("ReflProbeRender", renderReflProbe);
		}
		else
		{
			SPtr<ct::Texture> coreCustomTex = mCustomTexture->GetCore();
			auto filterReflProbe = [coreCustomTex, coreTexture, coreProbe]()
			{
				ct::gIBLUtility().ScaleCubemap(coreCustomTex, 0, coreTexture, 0);
				ct::gIBLUtility().FilterCubemapForSpecular(coreTexture, nullptr);

				coreProbe->mFilteredTexture = coreTexture;
				ct::gRenderer()->NotifyReflectionProbeUpdated(coreProbe.get(), true);

				return true;
			};

			mRendererTask = ct::RendererTask::Create("ReflProbeRender", filterReflProbe);
		}

		mRendererTask->onComplete.Connect(renderComplete);
		ct::gRenderer()->AddTask(mRendererTask);
	}

	SPtr<ct::ReflectionProbe> ReflectionProbe::GetCore() const
	{
		return std::static_pointer_cast<ct::ReflectionProbe>(mCoreSpecific);
	}

	SPtr<ReflectionProbe> ReflectionProbe::CreateSphere(float radius)
	{
		ReflectionProbe* probe = new (bs_alloc<ReflectionProbe>()) ReflectionProbe(ReflectionProbeType::Sphere, radius, Vector3::ZERO);
		SPtr<ReflectionProbe> probePtr = bs_core_ptr<ReflectionProbe>(probe);
		probePtr->SetThisPtrInternal(probePtr);
		probePtr->Initialize();

		return probePtr;
	}

	SPtr<ReflectionProbe> ReflectionProbe::CreateBox(const Vector3& extents)
	{
		ReflectionProbe* probe = new (bs_alloc<ReflectionProbe>()) ReflectionProbe(ReflectionProbeType::Box, 1.0f, extents);
		SPtr<ReflectionProbe> probePtr = bs_core_ptr<ReflectionProbe>(probe);
		probePtr->SetThisPtrInternal(probePtr);
		probePtr->Initialize();

		return probePtr;
	}

	SPtr<ReflectionProbe> ReflectionProbe::CreateEmpty()
	{
		ReflectionProbe* probe = new (bs_alloc<ReflectionProbe>()) ReflectionProbe();
		SPtr<ReflectionProbe> probePtr = bs_core_ptr<ReflectionProbe>(probe);
		probePtr->SetThisPtrInternal(probePtr);

		return probePtr;
	}

	SPtr<ct::CoreObject> ReflectionProbe::CreateCore() const
	{
		SPtr<ct::Texture> filteredTexture;
		if (mFilteredTexture != nullptr)
			filteredTexture = mFilteredTexture->GetCore();

		ct::ReflectionProbe* probe = new (bs_alloc<ct::ReflectionProbe>()) ct::ReflectionProbe(mType, mRadius, mExtents,
			filteredTexture);
		SPtr<ct::ReflectionProbe> probePtr = bs_shared_ptr<ct::ReflectionProbe>(probe);
		probePtr->SetThisPtrInternal(probePtr);

		return probePtr;
	}

	CoreSyncData ReflectionProbe::SyncToCore(FrameAlloc* allocator)
	{
		UINT32 size = 0;
		size += rtti_size(GetCoreDirtyFlags()).bytes;
		size += csync_size((SceneActor&)*this);
		size += csync_size(*this);

		UINT8* buffer = allocator->Alloc(size);

		Bitstream stream(buffer, size);
		rtti_write(GetCoreDirtyFlags(), stream);
		csync_write((SceneActor&)*this, stream);
		csync_write(*this, stream);

		return CoreSyncData(buffer, size);
	}

	void ReflectionProbe::MarkCoreDirtyInternal(ActorDirtyFlag flags)
	{
		MarkCoreDirty((UINT32)flags);
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

	namespace ct
	{
	ReflectionProbe::ReflectionProbe(ReflectionProbeType type, float radius, const Vector3& extents,
		const SPtr<Texture>& filteredTexture)
		: TReflectionProbe(type, radius, extents), mRendererId(0)
	{
		mFilteredTexture = filteredTexture;
	}

	ReflectionProbe::~ReflectionProbe()
	{
		gRenderer()->NotifyReflectionProbeRemoved(this);
	}

	void ReflectionProbe::Initialize()
	{
		UpdateBounds();
		gRenderer()->NotifyReflectionProbeAdded(this);

		CoreObject::Initialize();
	}

	void ReflectionProbe::SyncToCore(const CoreSyncData& data)
	{
		Bitstream stream(data.GetBuffer(), data.GetBufferSize());

		UINT32 dirtyFlags = 0;
		bool oldIsActive = mActive;
		ReflectionProbeType oldType = mType;

		rtti_read(dirtyFlags, stream);
		csync_read((SceneActor&)*this, stream);
		csync_read(*this, stream);

		UpdateBounds();

		if (dirtyFlags == (UINT32)ActorDirtyFlag::Transform)
		{
			if (mActive)
				gRenderer()->NotifyReflectionProbeUpdated(this, false);
		}
		else
		{
			if (oldIsActive != mActive)
			{
				if (mActive)
					gRenderer()->NotifyReflectionProbeAdded(this);
				else
				{
					ReflectionProbeType newType = mType;
					mType = oldType;
					gRenderer()->NotifyReflectionProbeRemoved(this);
					mType = newType;
				}
			}
			else
			{
				ReflectionProbeType newType = mType;
				mType = oldType;
				gRenderer()->NotifyReflectionProbeRemoved(this);
				mType = newType;

				gRenderer()->NotifyReflectionProbeAdded(this);
			}
		}
	}
}}
