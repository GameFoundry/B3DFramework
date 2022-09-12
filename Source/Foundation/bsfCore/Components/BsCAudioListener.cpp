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
	
	void CAudioListener::OnInitialized()
	{

	}

	void CAudioListener::OnDestroyed()
	{
		destroyInternal();
	}

	void CAudioListener::OnDisabled()
	{
		destroyInternal();
	}

	void CAudioListener::OnEnabled()
	{
		restoreInternal();
	}

	void CAudioListener::OnTransformChanged(TransformChangedFlags flags)
	{
		if (!SO()->GetActive())
			return;

		if ((flags & (TCF_Parent | TCF_Transform)) != 0)
			updateTransform();
	}

	void CAudioListener::Update()
	{
		const Vector3 worldPos = SO()->GetTransform().GetPosition();

		const float frameDelta = gTime().GetFrameDelta();
		if(frameDelta > 0.0f)
			mVelocity = (worldPos - mLastPosition) / frameDelta;
		else
			mVelocity = Vector3::ZERO;

		mLastPosition = worldPos;
	}

	void CAudioListener::RestoreInternal()
	{
		if (mInternal == nullptr)
			mInternal = AudioListener::create();

		updateTransform();
	}

	void CAudioListener::DestroyInternal()
	{
		// This should release the last reference and destroy the internal listener
		mInternal = nullptr;
	}
	
	void CAudioListener::UpdateTransform()
	{
		const Transform& tfrm = SO()->GetTransform();

		mInternal->SetTransform(tfrm);
		mInternal->SetVelocity(mVelocity);
	}
	
	RTTITypeBase* CAudioListener::getRTTIStatic()
	{
		return CAudioListenerRTTI::Instance();
	}

	RTTITypeBase* CAudioListener::getRTTI() const
	{
		return CAudioListener::GetRTTIStatic();
	}
}
