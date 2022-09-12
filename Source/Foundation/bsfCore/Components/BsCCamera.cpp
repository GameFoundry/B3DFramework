//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/BsCCamera.h"
#include "Private/RTTI/BsCCameraRTTI.h"
#include "Scene/BsSceneObject.h"
#include "Scene/BsSceneManager.h"
#include "BsCoreApplication.h"

namespace bs
{
	CCamera::CCamera()
	{
		setFlag(ComponentFlag::AlwaysRun, true);
		setName("Camera");
	}

	CCamera::CCamera(const HSceneObject& parent)
		: Component(parent)
	{
		setFlag(ComponentFlag::AlwaysRun, true);
		setName("Camera");
	}

	ConvexVolume CCamera::GetWorldFrustum() const
	{
		const Vector<Plane>& frustumPlanes = getFrustum().GetPlanes();
		Matrix4 worldMatrix = SO()->GetWorldMatrix();

		Vector<Plane> WorldPlanes(frustumPlanes.Size());
		UINT32 i = 0;
		for (auto& plane : frustumPlanes)
		{
			worldPlanes[i] = worldMatrix.MultiplyAffine(plane);
			i++;
		}

		return ConvexVolume(worldPlanes);
	}

	void CCamera::UpdateView() const
	{
		mInternal->_updateState(*SO());
	}

	void CCamera::SetMain(bool main)
	{
		mInternal->SetMain(main);
	}

	void CCamera::_instantiate()
	{
		// If mInternal already exists this means this object was deserialized,
		// so all we need to do is initialize it.
		if (mInternal != nullptr)
			mInternal->Initialize();
		else
			mInternal = Camera::create();
	}

	void CCamera::OnInitialized()
	{
		gSceneManager()._bindActor(mInternal, SO());

		// Make sure primary RT gets applied if camera gets deserialized with main camera state
		gSceneManager()._notifyMainCameraStateChanged(mInternal);
	}

	void CCamera::OnDestroyed()
	{
		gSceneManager()._unbindActor(mInternal);

		mInternal->Destroy();
	}

	RTTITypeBase* CCamera::getRTTIStatic()
	{
		return CCameraRTTI::Instance();
	}

	RTTITypeBase* CCamera::getRTTI() const
	{
		return CCamera::GetRTTIStatic();
	}
}
