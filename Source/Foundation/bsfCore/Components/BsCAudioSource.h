//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Audio/BsAudioSource.h"
#include "Scene/BsComponent.h"

namespace bs
{
	/** @addtogroup Components-Core
	 *  @{
	 */

	/**
	 * @copydoc	AudioSource
	 *
	 * @note Wraps AudioSource as a Component.
	 */
	class B3D_CORE_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Audio), ExportName(AudioSource)) CAudioSource : public Component
	{
	public:
		CAudioSource(const HSceneObject& parent);
		virtual ~CAudioSource() = default;

		/** @copydoc AudioSource::SetClip */
		B3D_SCRIPT_EXPORT(ExportName(Clip), Property(Setter))
		void SetClip(const HAudioClip& clip);

		/** @copydoc AudioSource::GetClip */
		B3D_SCRIPT_EXPORT(ExportName(Clip), Property(Getter))
		HAudioClip GetClip() const { return mAudioClip; }

		/** @copydoc AudioSource::SetVolume */
		B3D_SCRIPT_EXPORT(ExportName(Volume), Property(Setter), UIValueRange([ 0, 1 ]), UI(AsSlider))
		void SetVolume(float volume);

		/** @copydoc AudioSource::GetVolume */
		B3D_SCRIPT_EXPORT(ExportName(Volume), Property(Getter))
		float GetVolume() const { return mVolume; }

		/** @copydoc AudioSource::SetPitch */
		B3D_SCRIPT_EXPORT(ExportName(Pitch), Property(Setter))
		void SetPitch(float pitch);

		/** @copydoc AudioSource::GetPitch */
		B3D_SCRIPT_EXPORT(ExportName(Pitch), Property(Getter))
		float GetPitch() const { return mPitch; }

		/** @copydoc AudioSource::SetIsLooping */
		B3D_SCRIPT_EXPORT(ExportName(Loop), Property(Setter))
		void SetIsLooping(bool loop);

		/** @copydoc AudioSource::GetIsLooping */
		B3D_SCRIPT_EXPORT(ExportName(Loop), Property(Getter))
		bool GetIsLooping() const { return mLoop; }

		/** @copydoc AudioSource::SetPriority */
		B3D_SCRIPT_EXPORT(ExportName(Priority), Property(Setter))
		void SetPriority(u32 priority);

		/** @copydoc AudioSource::GetPriority */
		B3D_SCRIPT_EXPORT(ExportName(Priority), Property(Getter))
		u32 GetPriority() const { return mPriority; }

		/** @copydoc AudioSource::SetMinDistance */
		B3D_SCRIPT_EXPORT(ExportName(MinDistance), Property(Setter))
		void SetMinDistance(float distance);

		/** @copydoc AudioSource::GetMinDistance */
		B3D_SCRIPT_EXPORT(ExportName(MinDistance), Property(Getter))

		float GetMinDistance() const { return mMinDistance; }

		/** @copydoc AudioSource::SetAttenuation */
		B3D_SCRIPT_EXPORT(ExportName(Attenuation), Property(Setter))
		void SetAttenuation(float attenuation);

		/** @copydoc AudioSource::GetAttenuation */
		B3D_SCRIPT_EXPORT(ExportName(Attenuation), Property(Getter))
		float GetAttenuation() const { return mAttenuation; }

		/** @copydoc AudioSource::SetTime */
		B3D_SCRIPT_EXPORT(ExportName(Time), Property(Setter), UI(Hide))
		void SetTime(float time);

		/** @copydoc AudioSource::GetTime */
		B3D_SCRIPT_EXPORT(ExportName(Time), Property(Getter), UI(Hide))
		float GetTime() const;

		/** Sets whether playback should start as soon as the component is enabled. */
		B3D_SCRIPT_EXPORT(ExportName(PlayOnStart), Property(Setter))
		void SetPlayOnStart(bool enable) { mPlayOnStart = enable; }

		/** Determines should playback start as soon as the component is enabled. */
		B3D_SCRIPT_EXPORT(ExportName(PlayOnStart), Property(Getter))

		bool GetPlayOnStart() const { return mPlayOnStart; }

		/** @copydoc AudioSource::Play */
		B3D_SCRIPT_EXPORT(ExportName(Play))
		void Play();

		/** @copydoc AudioSource::Pause */
		B3D_SCRIPT_EXPORT(ExportName(Pause))
		void Pause();

		/** @copydoc AudioSource::Stop */
		B3D_SCRIPT_EXPORT(ExportName(Stop))
		void Stop();

		/** @copydoc AudioSource::GetState */
		B3D_SCRIPT_EXPORT(ExportName(State), Property(Getter))
		AudioSourceState GetState() const;

		/** @name Internal
		 *  @{
		 */

		/** Returns the AudioSource implementation wrapped by this component. */
		AudioSource* GetInternalInternal() const { return mInternal.get(); }

		/** @} */

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;

		void OnBeginPlay() override;
		void OnDestroyed() override;
		void OnDisabled() override;
		void OnEnabled() override;
		void OnTransformChanged(TransformChangedFlags flags) override;
		void Update() override;

	protected:
		/** Creates the internal representation of the AudioSource and restores the values saved by the Component. */
		void RestoreInternal();

		/** Destroys the internal AudioSource representation. */
		void DestroyInternal();

		/**
		 * Updates the transform of the internal AudioSource representation from the transform of the component's scene
		 * object.
		 */
		void UpdateTransform();

		SPtr<AudioSource> mInternal;
		Vector3 mLastPosition = Vector3::kZero;
		Vector3 mVelocity = Vector3::kZero;

		HAudioClip mAudioClip;
		float mVolume = 1.0f;
		float mPitch = 1.0f;
		bool mLoop = false;
		u32 mPriority = 0;
		float mMinDistance = 1.0f;
		float mAttenuation = 1.0f;
		bool mPlayOnStart = true;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class CAudioSourceRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const;

	protected:
		CAudioSource(); // Serialization only
	};

	/** @} */
} // namespace bs
