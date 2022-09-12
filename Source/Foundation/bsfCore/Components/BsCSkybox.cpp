//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/BsCSkybox.h"
#include "Private/RTTI/BsCSkyboxRTTI.h"
#include "Scene/BsSceneManager.h"
#include "Renderer/BsSkybox.h"

namespace bs
{
	CSkybox::CSkybox()
	{
		setFlag(ComponentFlag::AlwaysRun, true);
		setName("Skybox");
	}

	CSkybox::CSkybox(const HSceneObject& parent)
		: Component(parent)
	{
		setFlag(ComponentFlag::AlwaysRun, true);
		setName("Skybox");
	}

	CSkybox::~CSkybox()
	{
		mInternal->destroy();
	}

	void CSkybox::OnInitialized()
	{
		// If mInternal already exists this means this object was deserialized,
		// so all we need to do is initialize it.
		if (mInternal != nullptr)
			mInternal->initialize();
		else
			mInternal = Skybox::create();

		gSceneManager()._bindActor(mInternal, sceneObject());
	}

	void CSkybox::OnDestroyed()
	{
		gSceneManager()._unbindActor(mInternal);
	}

	RTTITypeBase* CSkybox::getRTTIStatic()
	{
		return CSkyboxRTTI::Instance();
	}

	RTTITypeBase* CSkybox::getRTTI() const
	{
		return CSkybox::GetRTTIStatic();
	}
}
