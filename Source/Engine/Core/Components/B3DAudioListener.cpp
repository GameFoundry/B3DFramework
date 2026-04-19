//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/B3DAudioListener.h"

#include "Audio/B3DAudio.h"
#include "Scene/B3DSceneObject.h"
#include "Utility/B3DTime.h"
#include "RTTI/B3DAudioListenerRTTI.h"

using namespace b3d;

AudioListener::AudioListener(const HSceneObject& parent)
	: Component(parent)
{
	SetName("AudioListener");

	mNotifyFlags = TCF_Transform;
}

AudioListener::AudioListener()
	: AudioListener(nullptr)
{ }

void AudioListener::OnDestroyed()
{
	DestroyInternal();
}

void AudioListener::OnDisabled()
{
	DestroyInternal();
}

void AudioListener::OnEnabled()
{
	RestoreInternal();
}

void AudioListener::OnTransformChanged(TransformChangedFlags flags)
{
	if(!GetEnabled())
		return;

	if((flags & (TCF_Parent | TCF_Transform)) != 0)
		UpdateTransform();
}

void AudioListener::Update()
{
	const Vector3 worldPos = SO()->GetTransform().GetPosition();

	const float frameDelta = GetTime().GetFrameDelta();
	if(frameDelta > 0.0f)
		mVelocity = (worldPos - mLastPosition) / frameDelta;
	else
		mVelocity = Vector3::kZero;

	mLastPosition = worldPos;
}

void AudioListener::RestoreInternal()
{
	if(mImplementation == nullptr)
		mImplementation = GetAudio().CreateListener();

	UpdateTransform();
}

void AudioListener::DestroyInternal()
{
	// This should release the last reference and destroy the internal listener
	mImplementation = nullptr;
}

void AudioListener::UpdateTransform()
{
	const Transform& transform = SO()->GetTransform();

	mImplementation->SetTransform(transform);
	mImplementation->SetVelocity(mVelocity);
}

RTTIType* AudioListener::GetRttiStatic()
{
	return AudioListenerRTTI::Instance();
}

RTTIType* AudioListener::GetRtti() const
{
	return AudioListener::GetRttiStatic();
}
