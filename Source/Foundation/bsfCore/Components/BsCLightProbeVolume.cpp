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
			mInternal->destroy();
	}

	void CLightProbeVolume::renderProbe(UINT32 handle)
	{
		if (mInternal != nullptr && SO()->getActive())
		{
			mInternal->UpdateStateInternal(*SO());
			mInternal->renderProbe(handle);
		}
	}

	void CLightProbeVolume::renderProbes()
	{
		if (mInternal != nullptr && SO()->getActive())
		{
			mInternal->UpdateStateInternal(*SO());
			mInternal->renderProbes();
		}
	}

	Vector<LightProbeInfo> CLightProbeVolume::getProbes() const
	{
		if (mInternal != nullptr)
			return mInternal->getProbes();

		return Vector<LightProbeInfo>();
	}

	void CLightProbeVolume::onInitialized()
	{
		// If mInternal already exists this means this object was deserialized,
		// so all we need to do is initialize it.
		if (mInternal != nullptr)
			mInternal->initialize();
		else
			mInternal = LightProbeVolume::create(mVolume, mCellCount);

		gSceneManager().BindActorInternal(mInternal, sceneObject());
	}

	void CLightProbeVolume::onDestroyed()
	{
		gSceneManager().UnbindActorInternal(mInternal);
	}
	
	RTTITypeBase* CLightProbeVolume::getRTTIStatic()
	{
		return CLightProbeVolumeRTTI::instance();
	}

	RTTITypeBase* CLightProbeVolume::getRTTI() const
	{
		return CLightProbeVolume::getRTTIStatic();
	}
}
