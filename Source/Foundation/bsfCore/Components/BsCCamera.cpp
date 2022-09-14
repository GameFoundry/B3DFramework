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

	ConvexVolume CCamera::getWorldFrustum() const
	{
		const Vector<Plane>& frustumPlanes = getFrustum().getPlanes();
		Matrix4 worldMatrix = SO()->getWorldMatrix();

		Vector<Plane> worldPlanes(frustumPlanes.size());
		UINT32 i = 0;
		for (auto& plane : frustumPlanes)
		{
			worldPlanes[i] = worldMatrix.multiplyAffine(plane);
			i++;
		}

		return ConvexVolume(worldPlanes);
	}

	void CCamera::updateView() const
	{
		mInternal->UpdateStateInternal(*SO());
	}

	void CCamera::setMain(bool main)
	{
		mInternal->setMain(main);
	}

	void CCamera::InstantiateInternal()
	{
		// If mInternal already exists this means this object was deserialized,
		// so all we need to do is initialize it.
		if (mInternal != nullptr)
			mInternal->initialize();
		else
			mInternal = Camera::create();
	}

	void CCamera::onInitialized()
	{
		gSceneManager().BindActorInternal(mInternal, SO());

		// Make sure primary RT gets applied if camera gets deserialized with main camera state
		gSceneManager().NotifyMainCameraStateChangedInternal(mInternal);
	}

	void CCamera::onDestroyed()
	{
		gSceneManager().UnbindActorInternal(mInternal);

		mInternal->destroy();
	}

	RTTITypeBase* CCamera::getRTTIStatic()
	{
		return CCameraRTTI::instance();
	}

	RTTITypeBase* CCamera::getRTTI() const
	{
		return CCamera::getRTTIStatic();
	}
}
