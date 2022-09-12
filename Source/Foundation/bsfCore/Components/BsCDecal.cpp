//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/BsCDecal.h"
#include "Private/RTTI/BsCDecalRTTI.h"
#include "Scene/BsSceneManager.h"

namespace bs
{
	CDecal::CDecal()
	{
		setFlag(ComponentFlag::AlwaysRun, true);
		setName("Decal");
	}

	CDecal::CDecal(const HSceneObject& parent)
		: Component(parent)
	{
		setFlag(ComponentFlag::AlwaysRun, true);
		setName("Decal");
	}

	CDecal::~CDecal()
	{
		mInternal->Destroy();
	}

	void CDecal::OnInitialized()
	{
		// If mInternal already exists this means this object was deserialized,
		// so all we need to do is initialize it.
		if (mInternal != nullptr)
			mInternal->Initialize();
		else
			mInternal = Decal::create(HMaterial());

		gSceneManager()._bindActor(mInternal, sceneObject());
	}

	void CDecal::OnDestroyed()
	{
		gSceneManager()._unbindActor(mInternal);
	}

	RTTITypeBase* CDecal::getRTTIStatic()
	{
		return CDecalRTTI::Instance();
	}

	RTTITypeBase* CDecal::getRTTI() const
	{
		return CDecal::GetRTTIStatic();
	}
}
