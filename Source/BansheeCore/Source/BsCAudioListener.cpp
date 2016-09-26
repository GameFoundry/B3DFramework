//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsCAudioListener.h"
#include "BsSceneObject.h"
#include "BsTime.h"
#include "BsCAudioListenerRTTI.h"

using namespace std::placeholders;

namespace BansheeEngine
{
	CAudioListener::CAudioListener(const HSceneObject& parent)
		: Component(parent)
	{
		setName("AudioListener");

		mNotifyFlags = TCF_Transform;
	}
	
	void CAudioListener::onInitialized()
	{

	}

	void CAudioListener::onDestroyed()
	{
		destroyInternal();
	}

	void CAudioListener::onDisabled()
	{
		destroyInternal();
	}

	void CAudioListener::onEnabled()
	{
		restoreInternal();
	}

	void CAudioListener::onTransformChanged(TransformChangedFlags flags)
	{
		if (!SO()->getActive())
			return;

		if ((flags & (TCF_Parent | TCF_Transform)) != 0)
			updateTransform();
	}

	void CAudioListener::update()
	{
		Vector3 worldPos = SO()->getWorldPosition();
		mVelocity = (worldPos - mLastPosition) / gTime().getFrameDelta();
		mLastPosition = worldPos;
	}

	void CAudioListener::restoreInternal()
	{
		if (mInternal == nullptr)
			mInternal = AudioListener::create();

		updateTransform();
	}

	void CAudioListener::destroyInternal()
	{
		// This should release the last reference and destroy the internal listener
		mInternal = nullptr;
	}
	
	void CAudioListener::updateTransform()
	{
		mInternal->setPosition(SO()->getWorldPosition());
		mInternal->setDirection(SO()->getForward());
		mInternal->setUp(SO()->getUp());
		mInternal->setVelocity(mVelocity);
	}
	
	RTTITypeBase* CAudioListener::getRTTIStatic()
	{
		return CAudioListenerRTTI::instance();
	}

	RTTITypeBase* CAudioListener::getRTTI() const
	{
		return CAudioListener::getRTTIStatic();
	}
}