//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsAnimation.h"
#include "BsComponent.h"

namespace BansheeEngine 
{
	/** @addtogroup Components
	 *  @{
	 */

	/**
	 * @copydoc	Animation
	 *
	 * Wraps Animation as a Component.
	 */
    class BS_EXPORT CAnimation : public Component
    {
    public:
		CAnimation(const HSceneObject& parent);
		virtual ~CAnimation() {}
		
		/** 
		 * Sets the default clip to play as soon as the component is enabled. If more control over playing clips is needed
		 * use the play(), blend() and crossFade() methods to queue clips for playback manually, and setState() method for
		 * modify their states individually.
		 */
		void setDefaultClip(const HAnimationClip& clip);

		/** @copydoc Animation::setWrapMode */
		void setWrapMode(AnimWrapMode wrapMode);

		/** @copydoc Animation::setSpeed */
		void setSpeed(float speed);

		/** @copydoc Animation::play */
		void play(const HAnimationClip& clip);

		/** @copydoc Animation::blendAdditive */
		void blendAdditive(const HAnimationClip& clip, float weight, float fadeLength = 0.0f, UINT32 layer = 0);

		/** @copydoc Animation::blendSequential */
		void blendSequential(const BlendSequentialInfo& info);

		/** @copydoc Animation::blend1D */
		void blend1D(const Blend1DInfo& info, float t);

		/** @copydoc Animation::blend2D */
		void blend2D(const Blend2DInfo& info, const Vector2& t);

		/** @copydoc Animation::crossFade */
		void crossFade(const HAnimationClip& clip, float fadeLength);

		/** @copydoc Animation::stop */
		void stop(UINT32 layer);

		/** @copydoc Animation::stopAll */
		void stopAll();
		
		/** @copydoc Animation::isPlaying */
		bool isPlaying() const;

		/** @copydoc Animation::getState */
		bool getState(const HAnimationClip& clip, AnimationClipState& state);

		/** @copydoc Animation::setState */
		void setState(const HAnimationClip& clip, AnimationClipState state);

		/** @name Internal
		 *  @{
		 */

		/** Returns the Animation implementation wrapped by this component. */
		SPtr<Animation> _getInternal() const { return mInternal; }

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
    protected:
		using Component::destroyInternal;

		/** Creates the internal representation of the Animation and restores the values saved by the Component. */
		void restoreInternal();

		/** Destroys the internal Animation representation. */
		void destroyInternal();

		SPtr<Animation> mInternal;

		HAnimationClip mDefaultClip;
		AnimWrapMode mWrapMode;
		float mSpeed;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class CAnimationRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;

	protected:
		CAnimation() {} // Serialization only
     };

	 /** @} */
}