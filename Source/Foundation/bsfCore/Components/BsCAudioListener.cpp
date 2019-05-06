//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/BsCAudioListener.h"
#include "Scene/BsSceneObject.h"
#include "Utility/BsTime.h"
#include "Private/RTTI/BsCAudioListenerRTTI.h"

using namespace std::placeholders;

namespace bs
{
	CAudioListener::CAudioListener()
	{
		setName("AudioListener");

		mNotifyFlags = TCF_Transform;
	}

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
		const Vector3 worldPos = SO()->getTransform().getPosition();

		const float frameDelta = gTime().getFrameDelta();
		if(frameDelta > 0.0f)
			mVelocity = (worldPos - mLastPosition) / frameDelta;
		else
			mVelocity = Vector3::ZERO;

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
		const Transform& tfrm = SO()->getTransform();

		mInternal->setTransform(tfrm);
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