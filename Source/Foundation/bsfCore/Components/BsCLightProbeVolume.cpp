//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/BsCLightProbeVolume.h"
#include "Private/RTTI/BsCLightProbeVolumeRTTI.h"
#include "Scene/BsSceneInstance.h"

using namespace b3d;

CLightProbeVolume::CLightProbeVolume()
{
	SetFlag(ComponentFlag::AlwaysRun, true);
	SetName("LightProbeVolume");
}

CLightProbeVolume::CLightProbeVolume(const HSceneObject& parent, const AABox& volume, const Vector3I& cellCount)
	: Component(parent), mVolume(volume), mCellCount(cellCount)
{
	SetFlag(ComponentFlag::AlwaysRun, true);
	SetName("LightProbeVolume");
}

CLightProbeVolume::~CLightProbeVolume()
{
	if(mInternal != nullptr)
		mInternal->Destroy();
}

void CLightProbeVolume::RenderProbe(u32 handle)
{
	if(mInternal != nullptr && GetEnabled())
	{
		mInternal->UpdateStateFromSceneObject(*SO());
		mInternal->RenderProbe(handle);
	}
}

void CLightProbeVolume::RenderProbes()
{
	if(mInternal != nullptr && GetEnabled())
	{
		mInternal->UpdateStateFromSceneObject(*SO());
		mInternal->RenderProbes();
	}
}

Vector<LightProbeInfo> CLightProbeVolume::GetProbes() const
{
	if(mInternal != nullptr)
		return mInternal->GetProbes();

	return Vector<LightProbeInfo>();
}

void CLightProbeVolume::OnBeginPlay()
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
		mInternal = LightProbeVolume::Create(scene, mVolume, mCellCount);

	scene->BindActor(mInternal, SceneObject());
}

void CLightProbeVolume::OnDestroyed()
{
	const SPtr<SceneInstance>& scene = SceneObject()->GetScene();
	scene->UnbindActor(mInternal);
}

RTTIType* CLightProbeVolume::GetRttiStatic()
{
	return CLightProbeVolumeRTTI::Instance();
}

RTTIType* CLightProbeVolume::GetRtti() const
{
	return CLightProbeVolume::GetRttiStatic();
}
