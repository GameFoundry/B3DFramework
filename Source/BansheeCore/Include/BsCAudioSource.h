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
    class BS_CORE_EXPORT CAudioSource : public Component
    {
    public:
		CAudioSource(const HSceneObject& parent);
		virtual ~CAudioSource() {}
		
		/** @copydoc AudioSource::setClip */
		void setClip(const HAudioClip& clip);

		/** @copydoc AudioSource::getClip */
		HAudioClip getClip() const { return mAudioClip; }

		/** @copydoc AudioSource::setVolume */
		void setVolume(float volume);

		/** @copydoc AudioSource::getVolume */
		float getVolume() const { return mVolume; }

		/** @copydoc AudioSource::setPitch */
		void setPitch(float pitch);

		/** @copydoc AudioSource::getPitch */
		float getPitch() const { return mPitch; }

		/** @copydoc AudioSource::setIsLooping */
		void setIsLooping(bool loop);

		/** @copydoc AudioSource::getIsLooping */
		bool getIsLooping() const { return mLoop; }

		/** @copydoc AudioSource::setPriority */
		void setPriority(UINT32 priority);

		/** @copydoc AudioSource::getPriority */
		UINT32 getPriority() const { return mPriority; }

		/** @copydoc AudioSource::setMinDistance */
		void setMinDistance(float distance);

		/** @copydoc AudioSource::getMinDistance */
		float getMinDistance() const { return mMinDistance; }

		/** @copydoc AudioSource::setAttenuation */
		void setAttenuation(float attenuation);

		/** @copydoc AudioSource::getAttenuation */
		float getAttenuation() const { return mAttenuation; }

		/** @copydoc AudioSource::setTime */
		void setTime(float time);

		/** @copydoc AudioSource::getTime */
		float getTime() const;

		/** Sets whether playback should start as soon as the component is enabled. */
		void setPlayOnStart(bool enable) { mPlayOnStart = enable; }

		/** Determines should playback start as soon as the component is enabled. */
		bool getPlayOnStart() const { return mPlayOnStart; }

		/** @copydoc AudioSource::play */
		void play();

		/** @copydoc AudioSource::pause */
		void pause();

		/** @copydoc AudioSource::stop */
		void stop();

		/** @copydoc AudioSource::getState */
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