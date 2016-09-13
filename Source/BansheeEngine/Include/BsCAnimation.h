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
		/** Information about scene objects bound to a specific animation curve. */
		struct SceneObjectMappingInfo
		{
			HSceneObject sceneObject;
			bool isMappedToBone;
			HBone bone;
		};

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

		/** @copydoc Animation::blend1D */
		void blend1D(const Blend1DInfo& info, float t);

		/** @copydoc Animation::blend2D */
		void blend2D(const Blend2DInfo& info, const Vector2& t);

		/** @copydoc Animation::crossFade */
		void crossFade(const HAnimationClip& clip, float fadeLength);

		/** @copydoc Animation::sample */
		void sample(const HAnimationClip& clip, float time);

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

		/** Sets bounds that will be used for animation and mesh culling. Only relevant if setUseBounds() is set to true. */
		void setBounds(const AABox& bounds);

		/** Gets animation bounds. @see setBounds. */
		const AABox& getBounds() const { return mBounds; }

		/** 
		 * Determines should animation bounds be used for visibility determination (culling). If false the bounds of the
         * mesh attached to the relevant CRenderable component will be used instead.
		 */
		void setUseBounds(bool enable);

		/** Checks whether animation bounds are enabled. @see setUseBounds. */
		bool getUseBounds() const { return mUseBounds; }

		/** Enables or disables culling of the animation when out of view. Culled animation will not be evaluated. */
		void setEnableCull(bool enable);

		/** Checks whether the animation will be evaluated when it is out of view. */
		bool getEnableCull() const { return mEnableCull; }

		/** Triggered whenever an animation event is reached. */
		Event<void(const HAnimationClip&, const String&)> onEventTriggered;

		/** @name Internal
		 *  @{
		 */

		/** Returns the Animation implementation wrapped by this component. */
		SPtr<Animation> _getInternal() const { return mInternal; }

		/** 
		 * Registers a new bone component, creating a new transform mapping from the bone name to the scene object the
		 * component is attached to. 
		 */
		void _addBone(const HBone& bone);

		/** Unregisters a bone component, removing the bone -> scene object mapping. */
		void _removeBone(const HBone& bone);

		/** Called whenever the bone name the Bone component points to changes. */
		void _notifyBoneChanged(const HBone& bone);

		/** 
		 * Registers a Renderable component with the animation, should be called whenever a Renderable component is added
		 * to the same scene object as this component.
		 */
		void _registerRenderable(const HRenderable& renderable);

		/** 
		 * Removes renderable from the animation component. Should be called when a Renderable component is removed from
		 * this scene object.
		 */
		void _unregisterRenderable();

		/** Re-applies the bounds to the internal animation object, and the relevant renderable object if one exists. */
		void _updateBounds(bool updateRenderable = true);

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
    protected:
		using Component::destroyInternal;

		/** Creates the internal representation of the Animation and restores the values saved by the Component. */
		void restoreInternal();

		/** Destroys the internal Animation representation. */
		void destroyInternal();

		/** Callback triggered whenever an animation event is triggered. */
		void eventTriggered(const HAnimationClip& clip, const String& name);

		/** 
		 * Finds any scene objects that are mapped to bone transforms. Such object's transforms will be affected by 
		 * skeleton bone animation.
		 */
		void setBoneMappings();

		/** Searches child scene objects for Bone components and returns any found ones. */
		Vector<HBone> findChildBones();

		SPtr<Animation> mInternal;
		HRenderable mAnimatedRenderable;

		HAnimationClip mDefaultClip;
		AnimWrapMode mWrapMode;
		float mSpeed;
		bool mEnableCull;
		bool mUseBounds;
		AABox mBounds;

		Vector<SceneObjectMappingInfo> mMappingInfos;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class CAnimationRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;

	protected:
		CAnimation(); // Serialization only
     };

	 /** @} */
}