//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
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
		mInternal->Destroy();
	}

	Sphere CReflectionProbe::GetBounds() const
	{
		mInternal->_updateState(*SO());

		return mInternal->GetBounds();
	}

	void CReflectionProbe::OnInitialized()
	{
		// If mInternal already exists this means this object was deserialized,
		// so all we need to do is initialize it.
		if (mInternal != nullptr)
			mInternal->Initialize();
		else
			mInternal = ReflectionProbe::createBox(Vector3::ONE);

		gSceneManager()._bindActor(mInternal, sceneObject());

		// If filtered texture doesn't exist, ensure it is generated
		SPtr<Texture> filteredTexture = mInternal->GetFilteredTexture();
		if(filteredTexture == nullptr)
		{
			if (mInternal->GetCustomTexture())
				mInternal->Filter();
			else
				mInternal->Capture();
		}
	}

	void CReflectionProbe::OnDestroyed()
	{
		gSceneManager()._unbindActor(mInternal);
	}

	RTTITypeBase* CReflectionProbe::getRTTIStatic()
	{
		return CReflectionProbeRTTI::Instance();
	}

	RTTITypeBase* CReflectionProbe::getRTTI() const
	{
		return CReflectionProbe::GetRTTIStatic();
	}
}
