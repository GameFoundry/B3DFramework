//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/BsCLightProbeVolume.h"
#include "Private/RTTI/BsCLightProbeVolumeRTTI.h"
#include "Scene/BsSceneManager.h"

namespace bs
{
	CLightProbeVolume::CLightProbeVolume()
	{
		setFlag(ComponentFlag::AlwaysRun, true);
		setName("LightProbeVolume");
	}

	CLightProbeVolume::CLightProbeVolume(const HSceneObject& parent, const AABox& volume, const Vector3I& cellCount)
		:Component(parent), mVolume(volume), mCellCount(cellCount)
	{
		setFlag(ComponentFlag::AlwaysRun, true);
		setName("LightProbeVolume");
	}

	CLightProbeVolume::~CLightProbeVolume()
	{
		if(mInternal != nullptr)
			mInternal->Destroy();
	}

	void CLightProbeVolume::RenderProbe(UINT32 handle)
	{
		if (mInternal != nullptr && SO()->GetActive())
		{
			mInternal->_updateState(*SO());
			mInternal->RenderProbe(handle);
		}
	}

	void CLightProbeVolume::RenderProbes()
	{
		if (mInternal != nullptr && SO()->GetActive())
		{
			mInternal->_updateState(*SO());
			mInternal->RenderProbes();
		}
	}

	Vector<LightProbeInfo> CLightProbeVolume::GetProbes() const
	{
		if (mInternal != nullptr)
			return mInternal->GetProbes();

		return Vector<LightProbeInfo>();
	}

	void CLightProbeVolume::OnInitialized()
	{
		// If mInternal already exists this means this object was deserialized,
		// so all we need to do is initialize it.
		if (mInternal != nullptr)
			mInternal->Initialize();
		else
			mInternal = LightProbeVolume::create(mVolume, mCellCount);

		gSceneManager()._bindActor(mInternal, sceneObject());
	}

	void CLightProbeVolume::OnDestroyed()
	{
		gSceneManager()._unbindActor(mInternal);
	}
	
	RTTITypeBase* CLightProbeVolume::getRTTIStatic()
	{
		return CLightProbeVolumeRTTI::Instance();
	}

	RTTITypeBase* CLightProbeVolume::getRTTI() const
	{
		return CLightProbeVolume::GetRTTIStatic();
	}
}
