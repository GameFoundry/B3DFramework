//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsCLightProbeVolume.h"
#include "BsCLightProbeVolumeRTTI.h"
#include "BsSceneManager.h"

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
		setName("Light");
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
			mInternal->_updateTransform(SO());
			mInternal->renderProbe(handle);
		}
	}

	void CLightProbeVolume::renderProbes()
	{
		if (mInternal != nullptr && SO()->getActive())
		{
			mInternal->_updateTransform(SO());
			mInternal->renderProbes();
		}
	}

	void CLightProbeVolume::onInitialized()
	{
		// If mInternal already exists this means this object was deserialized,
		// so all we need to do is initialize it.
		if (mInternal != nullptr)
			mInternal->initialize();
		else
			mInternal = LightProbeVolume::create(mVolume, mCellCount);

		gSceneManager()._registerLightProbeVolume(mInternal, sceneObject());
	}

	void CLightProbeVolume::onDestroyed()
	{
		gSceneManager()._unregisterLightProbeVolume(mInternal);
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
