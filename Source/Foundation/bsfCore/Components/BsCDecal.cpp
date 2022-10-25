//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/BsCDecal.h"
#include "Private/RTTI/BsCDecalRTTI.h"
#include "Scene/BsSceneManager.h"

using namespace bs;

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

void CDecal::OnInitialized()
{
	// If mInternal already exists this means this object was deserialized,
	// so all we need to do is initialize it.
	if(mInternal != nullptr)
		mInternal->Initialize();
	else
		mInternal = Decal::Create(HMaterial());

	gSceneManager().BindActorInternal(mInternal, SceneObject());
}

void CDecal::OnDestroyed()
{
	gSceneManager().UnbindActorInternal(mInternal);
}

RTTITypeBase* CDecal::GetRttiStatic()
{
	return CDecalRTTI::Instance();
}

RTTITypeBase* CDecal::GetRtti() const
{
	return CDecal::GetRttiStatic();
}
