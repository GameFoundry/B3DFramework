//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
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

	void CSkybox::onInitialized()
	{
		// If mInternal already exists this means this object was deserialized,
		// so all we need to do is initialize it.
		if (mInternal != nullptr)
			mInternal->initialize();
		else
			mInternal = Skybox::create();

		gSceneManager()._bindActor(mInternal, sceneObject());
	}

	void CSkybox::onDestroyed()
	{
		gSceneManager()._unbindActor(mInternal);
	}

	RTTITypeBase* CSkybox::getRTTIStatic()
	{
		return CSkyboxRTTI::instance();
	}

	RTTITypeBase* CSkybox::getRTTI() const
	{
		return CSkybox::getRTTIStatic();
	}
}
