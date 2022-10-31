//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/BsCSkybox.h"
#include "Private/RTTI/BsCSkyboxRTTI.h"
#include "Scene/BsSceneManager.h"
#include "Renderer/BsSkybox.h"

using namespace bs;

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

void CSkybox::OnInitialized()
{
	// If mInternal already exists this means this object was deserialized,
	// so all we need to do is initialize it.
	if(mInternal != nullptr)
		mInternal->Initialize();
	else
		mInternal = Skybox::Create();

	GetSceneManager().BindActorInternal(mInternal, SceneObject());
}

void CSkybox::OnDestroyed()
{
	GetSceneManager().UnbindActorInternal(mInternal);
}

RTTITypeBase* CSkybox::GetRttiStatic()
{
	return CSkyboxRTTI::Instance();
}

RTTITypeBase* CSkybox::GetRtti() const
{
	return CSkybox::GetRttiStatic();
}
