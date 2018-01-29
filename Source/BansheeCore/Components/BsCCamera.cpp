//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
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

	CCamera::~CCamera()
	{
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
		mInternal->_updateState(*SO());
	}

	void CCamera::setMain(bool main)
	{
		mInternal->setMain(main);

		gSceneManager()._notifyMainCameraStateChanged(mInternal);
	}

	void CCamera::_instantiate()
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
		gSceneManager()._bindActor(mInternal, SO());

		// Make sure primary RT gets applied if camera gets deserialized with main camera state
		gSceneManager()._notifyMainCameraStateChanged(mInternal);
	}

	void CCamera::onDestroyed()
	{
		gSceneManager()._unbindActor(mInternal);

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
