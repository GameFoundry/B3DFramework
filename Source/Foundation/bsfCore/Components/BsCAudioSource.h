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

		/** @copydoc AudioSource::setClip */
		BS_SCRIPT_EXPORT(ExportName(Clip), Property(Setter))
		void SetClip(const HAudioClip& clip);

		/** @copydoc AudioSource::getClip */
		BS_SCRIPT_EXPORT(ExportName(Clip), Property(Getter))

		HAudioClip GetClip() const { return mAudioClip; }

		/** @copydoc AudioSource::setVolume */
		BS_SCRIPT_EXPORT(ExportName(Volume), Property(Setter), UIValueRange([ 0, 1 ]), UI(AsSlider))
		void SetVolume(float volume);

		/** @copydoc AudioSource::getVolume */
		BS_SCRIPT_EXPORT(ExportName(Volume), Property(Getter))

		float GetVolume() const { return mVolume; }

		/** @copydoc AudioSource::setPitch */
		BS_SCRIPT_EXPORT(ExportName(Pitch), Property(Setter))
		void SetPitch(float pitch);

		/** @copydoc AudioSource::getPitch */
		BS_SCRIPT_EXPORT(ExportName(Pitch), Property(Getter))

		float GetPitch() const { return mPitch; }

		/** @copydoc AudioSource::setIsLooping */
		BS_SCRIPT_EXPORT(ExportName(Loop), Property(Setter))
		void SetIsLooping(bool loop);

		/** @copydoc AudioSource::getIsLooping */
		BS_SCRIPT_EXPORT(ExportName(Loop), Property(Getter))

		bool GetIsLooping() const { return mLoop; }

		/** @copydoc AudioSource::setPriority */
		BS_SCRIPT_EXPORT(ExportName(Priority), Property(Setter))
		void SetPriority(u32 priority);

		/** @copydoc AudioSource::getPriority */
		BS_SCRIPT_EXPORT(ExportName(Priority), Property(Getter))

		u32 GetPriority() const { return mPriority; }

		/** @copydoc AudioSource::setMinDistance */
		BS_SCRIPT_EXPORT(ExportName(MinDistance), Property(Setter))
		void SetMinDistance(float distance);

		/** @copydoc AudioSource::getMinDistance */
		BS_SCRIPT_EXPORT(ExportName(MinDistance), Property(Getter))

		float GetMinDistance() const { return mMinDistance; }

		/** @copydoc AudioSource::setAttenuation */
		BS_SCRIPT_EXPORT(ExportName(Attenuation), Property(Setter))
		void SetAttenuation(float attenuation);

		/** @copydoc AudioSource::getAttenuation */
		BS_SCRIPT_EXPORT(ExportName(Attenuation), Property(Getter))

		float GetAttenuation() const { return mAttenuation; }

		/** @copydoc AudioSource::setTime */
		BS_SCRIPT_EXPORT(ExportName(Time), Property(Setter), UI(Hide))
		void SetTime(float time);

		/** @copydoc AudioSource::getTime */
		BS_SCRIPT_EXPORT(ExportName(Time), Property(Getter), UI(Hide))
		float GetTime() const;

		/** Sets whether playback should start as soon as the component is enabled. */
		BS_SCRIPT_EXPORT(ExportName(PlayOnStart), Property(Setter))

		void SetPlayOnStart(bool enable) { mPlayOnStart = enable; }

		/** Determines should playback start as soon as the component is enabled. */
		BS_SCRIPT_EXPORT(ExportName(PlayOnStart), Property(Getter))

		bool GetPlayOnStart() const { return mPlayOnStart; }

		/** @copydoc AudioSource::play */
		BS_SCRIPT_EXPORT(ExportName(Play))
		void Play();

		/** @copydoc AudioSource::pause */
		BS_SCRIPT_EXPORT(ExportName(Pause))
		void Pause();

		/** @copydoc AudioSource::stop */
		BS_SCRIPT_EXPORT(ExportName(Stop))
		void Stop();

		/** @copydoc AudioSource::getState */
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

		/** @copydoc Component::onInitialized() */
		void OnInitialized() override;

		/** @copydoc Component::onDestroyed() */
		void OnDestroyed() override;

		/** @copydoc Component::onDisabled() */
		void OnDisabled() override;

		/** @copydoc Component::onEnabled() */
		void OnEnabled() override;

		/** @copydoc Component::onTransformChanged() */
		void OnTransformChanged(TransformChangedFlags flags) override;

		/** @copydoc Component::update() */
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
