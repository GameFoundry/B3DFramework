//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/BsCSkybox.h"
#include "Private/RTTI/BsCSkyboxRTTI.h"
#include "Scene/BsSceneManager.h"
#include "Renderer/BsSkybox.h"

using namespace b3d;

CSkybox::CSkybox()
{
	SetFlag(ComponentFlag::AlwaysRun, true);
	SetName("Skybox");
}

CSkybox::CSkybox(const HSceneObject& parent)
	: Component(parent)
{
	SetFlag(ComponentFlag::AlwaysRun, true);
	SetName("Skybox");
}

CSkybox::~CSkybox()
{
	mInternal->Destroy();
}

void CSkybox::OnBeginPlay()
{
	const SPtr<SceneInstance>& scene = SceneObject()->GetScene();

	// If mInternal already exists this means this object was deserialized,
	// so all we need to do is initialize it.
	if(mInternal != nullptr)
	{
		mInternal->SetScene(scene);
		mInternal->Initialize();
	}
	else
	{
		mInternal = Skybox::Create(scene);
	}

	GetSceneManager().BindActorInternal(mInternal, SceneObject());
}

void CSkybox::OnDestroyed()
{
	GetSceneManager().UnbindActorInternal(mInternal);
}

RTTIType* CSkybox::GetRttiStatic()
{
	return CSkyboxRTTI::Instance();
}

RTTIType* CSkybox::GetRtti() const
{
	return CSkybox::GetRttiStatic();
}
