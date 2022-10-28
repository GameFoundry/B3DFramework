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
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(DocumentationGroup(Audio), ExportName(AudioSource)) CAudioSource : public Component
	{
	public:
		CAudioSource(const HSceneObject& parent);
		virtual ~CAudioSource() = default;

		/** @copydoc AudioSource::SetClip */
		BS_SCRIPT_EXPORT(ExportName(Clip), Property(Setter))
		void SetClip(const HAudioClip& clip);

		/** @copydoc AudioSource::GetClip */
		BS_SCRIPT_EXPORT(ExportName(Clip), Property(Getter))
		HAudioClip GetClip() const { return mAudioClip; }

		/** @copydoc AudioSource::SetVolume */
		BS_SCRIPT_EXPORT(ExportName(Volume), Property(Setter), UIValueRange([ 0, 1 ]), UI(AsSlider))
		void SetVolume(float volume);

		/** @copydoc AudioSource::GetVolume */
		BS_SCRIPT_EXPORT(ExportName(Volume), Property(Getter))
		float GetVolume() const { return mVolume; }

		/** @copydoc AudioSource::SetPitch */
		BS_SCRIPT_EXPORT(ExportName(Pitch), Property(Setter))
		void SetPitch(float pitch);

		/** @copydoc AudioSource::GetPitch */
		BS_SCRIPT_EXPORT(ExportName(Pitch), Property(Getter))
		float GetPitch() const { return mPitch; }

		/** @copydoc AudioSource::SetIsLooping */
		BS_SCRIPT_EXPORT(ExportName(Loop), Property(Setter))
		void SetIsLooping(bool loop);

		/** @copydoc AudioSource::GetIsLooping */
		BS_SCRIPT_EXPORT(ExportName(Loop), Property(Getter))
		bool GetIsLooping() const { return mLoop; }

		/** @copydoc AudioSource::SetPriority */
		BS_SCRIPT_EXPORT(ExportName(Priority), Property(Setter))
		void SetPriority(u32 priority);

		/** @copydoc AudioSource::GetPriority */
		BS_SCRIPT_EXPORT(ExportName(Priority), Property(Getter))
		u32 GetPriority() const { return mPriority; }

		/** @copydoc AudioSource::SetMinDistance */
		BS_SCRIPT_EXPORT(ExportName(MinDistance), Property(Setter))
		void SetMinDistance(float distance);

		/** @copydoc AudioSource::GetMinDistance */
		BS_SCRIPT_EXPORT(ExportName(MinDistance), Property(Getter))

		float GetMinDistance() const { return mMinDistance; }

		/** @copydoc AudioSource::SetAttenuation */
		BS_SCRIPT_EXPORT(ExportName(Attenuation), Property(Setter))
		void SetAttenuation(float attenuation);

		/** @copydoc AudioSource::GetAttenuation */
		BS_SCRIPT_EXPORT(ExportName(Attenuation), Property(Getter))
		float GetAttenuation() const { return mAttenuation; }

		/** @copydoc AudioSource::SetTime */
		BS_SCRIPT_EXPORT(ExportName(Time), Property(Setter), UI(Hide))
		void SetTime(float time);

		/** @copydoc AudioSource::GetTime */
		BS_SCRIPT_EXPORT(ExportName(Time), Property(Getter), UI(Hide))
		float GetTime() const;

		/** Sets whether playback should start as soon as the component is enabled. */
		BS_SCRIPT_EXPORT(ExportName(PlayOnStart), Property(Setter))
		void SetPlayOnStart(bool enable) { mPlayOnStart = enable; }

		/** Determines should playback start as soon as the component is enabled. */
		BS_SCRIPT_EXPORT(ExportName(PlayOnStart), Property(Getter))

		bool GetPlayOnStart() const { return mPlayOnStart; }

		/** @copydoc AudioSource::Play */
		BS_SCRIPT_EXPORT(ExportName(Play))
		void Play();

		/** @copydoc AudioSource::Pause */
		BS_SCRIPT_EXPORT(ExportName(Pause))
		void Pause();

		/** @copydoc AudioSource::Stop */
		BS_SCRIPT_EXPORT(ExportName(Stop))
		void Stop();

		/** @copydoc AudioSource::GetState */
		BS_SCRIPT_EXPORT(ExportName(State), Property(Getter))
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

		void OnInitialized() override;
		void OnDestroyed() override;
		void OnDisabled() override;
		void OnEnabled() override;
		void OnTransformChanged(TransformChangedFlags flags) override;
		void Update() override;

	protected:
		using Component::DestroyInternal;

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
