//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/BsCCamera.h"
#include "Private/RTTI/BsCCameraRTTI.h"
#include "Scene/BsSceneObject.h"
#include "Scene/BsSceneManager.h"
#include "BsCoreApplication.h"

using namespace b3d;

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
	mInternal->UpdateStateFromSceneObject(*SO());
}

void CCamera::SetMain(bool main)
{
	mInternal->SetMain(main);
}

void CCamera::Initialize()
{
	Component::Initialize();

	const SPtr<SceneInstance>& scene = SceneObject()->GetScene();

	// If mInternal already exists this means this object was deserialized,
	// so all we need to do is initialize it.
	if(mInternal != nullptr)
	{
		mInternal->SetScene(scene);
		mInternal->Initialize();
	}
	else
		mInternal = Camera::Create(scene);
}

void CCamera::OnBeginPlay()
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

RTTIType* CCamera::GetRttiStatic()
{
	return CCameraRTTI::Instance();
}

RTTIType* CCamera::GetRtti() const
{
	return CCamera::GetRttiStatic();
}
