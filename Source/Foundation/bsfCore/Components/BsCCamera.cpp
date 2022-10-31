//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/BsCCamera.h"
#include "Private/RTTI/BsCCameraRTTI.h"
#include "Scene/BsSceneObject.h"
#include "Scene/BsSceneManager.h"
#include "BsCoreApplication.h"

using namespace bs;

CCamera::CCamera()
{
	SetFlag(ComponentFlag::AlwaysRun, true);
	SetName("Camera");
}

CCamera::CCamera(const HSceneObject& parent)
	: Component(parent)
{
	SetFlag(ComponentFlag::AlwaysRun, true);
	SetName("Camera");
}

ConvexVolume CCamera::GetWorldFrustum() const
{
	const Vector<Plane>& frustumPlanes = GetFrustum().GetPlanes();
	Matrix4 worldMatrix = SO()->GetWorldMatrix();

	Vector<Plane> worldPlanes(frustumPlanes.size());
	u32 i = 0;
	for(auto& plane : frustumPlanes)
	{
		worldPlanes[i] = worldMatrix.MultiplyAffine(plane);
		i++;
	}

	return ConvexVolume(worldPlanes);
}

void CCamera::UpdateView() const
{
	mInternal->UpdateStateInternal(*SO());
}

void CCamera::SetMain(bool main)
{
	mInternal->SetMain(main);
}

void CCamera::InstantiateInternal()
{
	// If mInternal already exists this means this object was deserialized,
	// so all we need to do is initialize it.
	if(mInternal != nullptr)
		mInternal->Initialize();
	else
		mInternal = Camera::Create();
}

void CCamera::OnInitialized()
{
	GetSceneManager().BindActorInternal(mInternal, SO());

	// Make sure primary RT gets applied if camera gets deserialized with main camera state
	GetSceneManager().NotifyMainCameraStateChangedInternal(mInternal);
}

void CCamera::OnDestroyed()
{
	GetSceneManager().UnbindActorInternal(mInternal);

	mInternal->Destroy();
}

RTTITypeBase* CCamera::GetRttiStatic()
{
	return CCameraRTTI::Instance();
}

RTTITypeBase* CCamera::GetRtti() const
{
	return CCamera::GetRttiStatic();
}
