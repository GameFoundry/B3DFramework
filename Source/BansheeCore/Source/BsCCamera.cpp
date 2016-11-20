//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsCCamera.h"
#include "BsCCameraRTTI.h"
#include "BsSceneObject.h"
#include "BsCoreSceneManager.h"

namespace bs 
{
	CCamera::CCamera(const HSceneObject& parent, SPtr<RenderTarget> target, float left, float top, float width, float height)
		: Component(parent), mTarget(target), mLeft(left), mTop(top), mWidth(width), mHeight(height)
    {
		setName("Camera");
    }

    CCamera::~CCamera()
    {
		mInternal->destroy();
    }

	ConvexVolume CCamera::getWorldFrustum() const
	{
		const Vector<Plane>& frustumPlanes = getFrustum().getPlanes();
		Matrix4 worldMatrix = SO()->getWorldTfrm();

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
		UINT32 curHash = SO()->getTransformHash();
		if (curHash != mInternal->_getLastModifiedHash())
		{
			mInternal->setPosition(SO()->getWorldPosition());
			mInternal->setRotation(SO()->getWorldRotation());

			mInternal->_setLastModifiedHash(curHash);
		}
	}

	void CCamera::setMain(bool main)
	{
		mInternal->setMain(main);

		gCoreSceneManager()._notifyMainCameraStateChanged(mInternal);
	}

	void CCamera::update() 
	{

	}

	void CCamera::onInitialized()
	{
		// If mInternal already exists this means this object was deserialized,
		// so all we need to do is initialize it.
		if (mInternal != nullptr)
			mInternal->initialize();
		else
		{
			mInternal = Camera::create(mTarget, mLeft, mTop, mWidth, mHeight);
			mTarget = nullptr;
		}

		gCoreSceneManager()._registerCamera(mInternal, SO());
	}

	void CCamera::onDestroyed()
	{
		gCoreSceneManager()._unregisterCamera(mInternal);
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
