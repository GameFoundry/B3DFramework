//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/BsCReflectionProbe.h"
#include "Private/RTTI/BsCReflectionProbeRTTI.h"
#include "Scene/BsSceneInstance.h"

using namespace b3d;

CReflectionProbe::CReflectionProbe()
{
	SetFlag(ComponentFlag::AlwaysRun, true);
	SetName("ReflectionProbe");
}

CReflectionProbe::CReflectionProbe(const HSceneObject& parent)
	: Component(parent)
{
	SetFlag(ComponentFlag::AlwaysRun, true);
	SetName("ReflectionProbe");
}

CReflectionProbe::~CReflectionProbe()
{
	mInternal->Destroy();
}

Sphere CReflectionProbe::GetBounds() const
{
	mInternal->UpdateStateFromSceneObject(*SO());

	return mInternal->GetBounds();
}

void CReflectionProbe::OnBeginPlay()
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
		mInternal = ReflectionProbe::CreateBox(scene, Vector3::kOne);

	scene->BindActor(mInternal, SceneObject());

	// If filtered texture doesn't exist, ensure it is generated
	SPtr<Texture> filteredTexture = mInternal->GetFilteredTexture();
	if(filteredTexture == nullptr)
	{
		if(mInternal->GetCustomTexture())
			mInternal->Filter();
		else
			mInternal->Capture();
	}
}

void CReflectionProbe::OnDestroyed()
{
	const SPtr<SceneInstance>& scene = SceneObject()->GetScene();
	scene->UnbindActor(mInternal);
}

RTTIType* CReflectionProbe::GetRttiStatic()
{
	return CReflectionProbeRTTI::Instance();
}

RTTIType* CReflectionProbe::GetRtti() const
{
	return CReflectionProbe::GetRttiStatic();
}
