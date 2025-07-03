//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/BsCDecal.h"
#include "Private/RTTI/BsCDecalRTTI.h"
#include "Scene/BsSceneInstance.h"

using namespace b3d;

CDecal::CDecal()
{
	SetFlag(ComponentFlag::AlwaysRun, true);
	SetName("Decal");
}

CDecal::CDecal(const HSceneObject& parent)
	: Component(parent)
{
	SetFlag(ComponentFlag::AlwaysRun, true);
	SetName("Decal");
}

CDecal::~CDecal()
{
	mInternal->Destroy();
}

void CDecal::OnBeginPlay()
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
		mInternal = Decal::Create(scene, HMaterial());

	scene->BindActor(mInternal, SceneObject());
}

void CDecal::OnDestroyed()
{
	const SPtr<SceneInstance>& scene = SceneObject()->GetScene();
	scene->UnbindActor(mInternal);
}

RTTIType* CDecal::GetRttiStatic()
{
	return CDecalRTTI::Instance();
}

RTTIType* CDecal::GetRtti() const
{
	return CDecal::GetRttiStatic();
}
