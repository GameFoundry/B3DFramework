//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsAudioSource.h"
#include "BsComponent.h"

namespace bs 
{
	/** @addtogroup Components-Core
	 *  @{
	 */

	/**
	 * @copydoc	AudioSource
	 *
	 * Wraps AudioSource as a Component.
	 */
    class BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Audio,n:AudioSource) CAudioSource : public Component
    {
    public:
		CAudioSource(const HSceneObject& parent);
		virtual ~CAudioSource() {}
		
		/** @copydoc AudioSource::setClip */
		BS_SCRIPT_EXPORT(n:Clip,pr:setter)
		void setClip(const HAudioClip& clip);

		/** @copydoc AudioSource::getClip */
		BS_SCRIPT_EXPORT(n:Clip,pr:getter)
		HAudioClip getClip() const { return mAudioClip; }

		/** @copydoc AudioSource::setVolume */
		BS_SCRIPT_EXPORT(n:Volume,pr:setter)
		void setVolume(float volume);

		/** @copydoc AudioSource::getVolume */
		BS_SCRIPT_EXPORT(n:Volume,pr:getter)
		float getVolume() const { return mVolume; }

		/** @copydoc AudioSource::setPitch */
		BS_SCRIPT_EXPORT(n:Pitch,pr:setter)
		void setPitch(float pitch);

		/** @copydoc AudioSource::getPitch */
		BS_SCRIPT_EXPORT(n:Pitch,pr:getter)
		float getPitch() const { return mPitch; }

		/** @copydoc AudioSource::setIsLooping */
		BS_SCRIPT_EXPORT(n:Loop,pr:setter)
		void setIsLooping(bool loop);

		/** @copydoc AudioSource::getIsLooping */
		BS_SCRIPT_EXPORT(n:Loop,pr:getter)
		bool getIsLooping() const { return mLoop; }

		/** @copydoc AudioSource::setPriority */
		BS_SCRIPT_EXPORT(n:Priority,pr:setter)
		void setPriority(UINT32 priority);

		/** @copydoc AudioSource::getPriority */
		BS_SCRIPT_EXPORT(n:Priority,pr:getter)
		UINT32 getPriority() const { return mPriority; }

		/** @copydoc AudioSource::setMinDistance */
		BS_SCRIPT_EXPORT(n:MinDistance,pr:setter)
		void setMinDistance(float distance);

		/** @copydoc AudioSource::getMinDistance */
		BS_SCRIPT_EXPORT(n:MinDistance,pr:getter)
		float getMinDistance() const { return mMinDistance; }

		/** @copydoc AudioSource::setAttenuation */
		BS_SCRIPT_EXPORT(n:Attenuation,pr:setter)
		void setAttenuation(float attenuation);

		/** @copydoc AudioSource::getAttenuation */
		BS_SCRIPT_EXPORT(n:Attenuation,pr:getter)
		float getAttenuation() const { return mAttenuation; }

		/** @copydoc AudioSource::setTime */
		BS_SCRIPT_EXPORT(n:Time,pr:setter)
		void setTime(float time);

		/** @copydoc AudioSource::getTime */
		BS_SCRIPT_EXPORT(n:Time,pr:getter)
		float getTime() const;

		/** Sets whether playback should start as soon as the component is enabled. */
		BS_SCRIPT_EXPORT(n:PlayOnStart,pr:setter)
		void setPlayOnStart(bool enable) { mPlayOnStart = enable; }

		/** Determines should playback start as soon as the component is enabled. */
		BS_SCRIPT_EXPORT(n:PlayOnStart,pr:getter)
		bool getPlayOnStart() const { return mPlayOnStart; }

		/** @copydoc AudioSource::play */
		BS_SCRIPT_EXPORT(n:Play)
		void play();

		/** @copydoc AudioSource::pause */
		BS_SCRIPT_EXPORT(n:Pause)
		void pause();

		/** @copydoc AudioSource::stop */
		BS_SCRIPT_EXPORT(n:Stop)
		void stop();

		/** @copydoc AudioSource::getState */
		BS_SCRIPT_EXPORT(n:State,pr:getter)
		AudioSourceState getState() const;

		/** @name Internal
		 *  @{
		 */

		/** Returns the AudioSource implementation wrapped by this component. */
		AudioSource* _getInternal() const { return mInternal.get(); }

		/** @} */

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;

		/** @copydoc Component::onInitialized() */
		void onInitialized() override;

		/** @copydoc Component::onDestroyed() */
		void onDestroyed() override;

		/** @copydoc Component::onDisabled() */
		void onDisabled() override;

		/** @copydoc Component::onEnabled() */
		void onEnabled() override;

		/** @copydoc Component::onTransformChanged() */
		void onTransformChanged(TransformChangedFlags flags) override;

		/** @copydoc Component::update() */
		void update() override;
    protected:
		using Component::destroyInternal;

		/** Creates the internal representation of the AudioSource and restores the values saved by the Component. */
		void restoreInternal();

		/** Destroys the internal AudioSource representation. */
		void destroyInternal();

		/** 
		 * Updates the transform of the internal AudioSource representation from the transform of the component's scene
		 * object.
		 */
		void updateTransform();

		SPtr<AudioSource> mInternal;
		Vector3 mLastPosition = Vector3::ZERO;
		Vector3 mVelocity = Vector3::ZERO;

		HAudioClip mAudioClip;
		float mVolume;
		float mPitch;
		bool mLoop;
		UINT32 mPriority;
		float mMinDistance;
		float mAttenuation;
		bool mPlayOnStart;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class CAudioSourceRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;

	protected:
		CAudioSource() {} // Serialization only
     };

	 /** @} */
}