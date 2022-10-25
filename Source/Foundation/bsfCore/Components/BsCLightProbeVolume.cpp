//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/BsCLightProbeVolume.h"
#include "Private/RTTI/BsCLightProbeVolumeRTTI.h"
#include "Scene/BsSceneManager.h"

using namespace bs;

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
	if(mInternal != nullptr && SO()->GetActive())
	{
		mInternal->UpdateStateInternal(*SO());
		mInternal->RenderProbe(handle);
	}
}

void CLightProbeVolume::RenderProbes()
{
	if(mInternal != nullptr && SO()->GetActive())
	{
		mInternal->UpdateStateInternal(*SO());
		mInternal->RenderProbes();
	}
}

Vector<LightProbeInfo> CLightProbeVolume::GetProbes() const
{
	if(mInternal != nullptr)
		return mInternal->GetProbes();

	return Vector<LightProbeInfo>();
}

void CLightProbeVolume::OnInitialized()
{
	// If mInternal already exists this means this object was deserialized,
	// so all we need to do is initialize it.
	if(mInternal != nullptr)
		mInternal->Initialize();
	else
		mInternal = LightProbeVolume::Create(mVolume, mCellCount);

	gSceneManager().BindActorInternal(mInternal, SceneObject());
}

void CLightProbeVolume::OnDestroyed()
{
	gSceneManager().UnbindActorInternal(mInternal);
}

RTTITypeBase* CLightProbeVolume::GetRttiStatic()
{
	return CLightProbeVolumeRTTI::Instance();
}

RTTITypeBase* CLightProbeVolume::GetRtti() const
{
	return CLightProbeVolume::GetRttiStatic();
}
