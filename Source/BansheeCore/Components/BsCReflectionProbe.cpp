//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Components/BsCReflectionProbe.h"
#include "Private/RTTI/BsCReflectionProbeRTTI.h"
#include "Scene/BsSceneManager.h"

namespace bs
{
	CReflectionProbe::CReflectionProbe()
	{
		setFlag(ComponentFlag::AlwaysRun, true);
		setName("ReflectionProbe");
	}

	CReflectionProbe::CReflectionProbe(const HSceneObject& parent)
		: Component(parent)
	{
		setFlag(ComponentFlag::AlwaysRun, true);
		setName("ReflectionProbe");
	}

	CReflectionProbe::~CReflectionProbe()
	{
		mInternal->destroy();
	}

	Sphere CReflectionProbe::getBounds() const
	{
		mInternal->_updateState(*SO());

		return mInternal->getBounds();
	}

	void CReflectionProbe::onInitialized()
	{
		// If mInternal already exists this means this object was deserialized,
		// so all we need to do is initialize it.
		if (mInternal != nullptr)
			mInternal->initialize();
		else
			mInternal = ReflectionProbe::createBox(Vector3::ONE);

		gSceneManager()._bindActor(mInternal, sceneObject());

		// If filtered texture doesn't exist, ensure it is generated
		SPtr<Texture> filteredTexture = mInternal->getFilteredTexture();
		if(filteredTexture == nullptr)
		{
			if (mInternal->getCustomTexture())
				mInternal->filter();
			else
				mInternal->capture();
		}
	}

	void CReflectionProbe::onDestroyed()
	{
		gSceneManager()._unbindActor(mInternal);
	}

	RTTITypeBase* CReflectionProbe::getRTTIStatic()
	{
		return CReflectionProbeRTTI::instance();
	}

	RTTITypeBase* CReflectionProbe::getRTTI() const
	{
		return CReflectionProbe::getRTTIStatic();
	}
}
