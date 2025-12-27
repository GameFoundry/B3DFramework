//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Scene/B3DComponent.h"

namespace b3d
{
	class IAudioListenerImplementation;

	/** @addtogroup Components
	 *  @{
	 */

	/**
	 * Represents a listener that hears audio sources. For spatial audio the volume and pitch of played audio is determined
	 * by the distance, orientation and velocity differences between the source and the listener.
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Audio)) AudioListener : public Component
	{
	public:
		AudioListener(const HSceneObject& parent);
		virtual ~AudioListener() = default;

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;

		void OnDestroyed() override;
		void OnDisabled() override;
		void OnEnabled() override;
		void OnTransformChanged(TransformChangedFlags flags) override;
		void Update() override;

	protected:
		/** Creates the internal representation of the AudioListener and restores the values saved by the Component. */
		void RestoreInternal();

		/** Destroys the internal AudioListener representation. */
		void DestroyInternal();

		/**
		 * Updates the transform of the internal AudioListener representation from the transform of the component's scene
		 * object.
		 */
		void UpdateTransform();

		SPtr<IAudioListenerImplementation> mImplementation;
		Vector3 mLastPosition = Vector3::kZero;
		Vector3 mVelocity = Vector3::kZero;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class AudioListenerRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const;

	protected:
		AudioListener(); // Serialization only
	};

	/** @} */

	/** @addtogroup Audio
	 *  @{
	 */

	/** Low-level interface for an audio source. Should be implemented by the audio plugin. */
	class B3D_EXPORT IAudioListenerImplementation
	{
	public:
		virtual ~IAudioListenerImplementation() = default;

		/** Sets the position and orientation of the audio source. */
		virtual void SetTransform(const Transform& transform) = 0;

		/** Sets the velocity of the listener. */
		virtual void SetVelocity(const Vector3& velocity) = 0;
	};

	/** @} */
} // namespace b3d
