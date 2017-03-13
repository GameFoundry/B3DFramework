//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsCSkybox.h"
#include "BsCSkyboxRTTI.h"
#include "BsSceneManager.h"

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

	RTTITypeBase* CSkybox::getRTTIStatic()
	{
		return CSkyboxRTTI::instance();
	}

	RTTITypeBase* CSkybox::getRTTI() const
	{
		return CSkybox::getRTTIStatic();
	}
}
