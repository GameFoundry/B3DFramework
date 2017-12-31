//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "Animation/BsAnimation.h"
#include "Scene/BsComponent.h"

namespace bs 
{
	/** @addtogroup Components
	 *  @{
	 */

	/**
	 * @copydoc	Animation
	 *
	 * @note	Wraps Animation as a Component.
	 */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Animation,n:Animation) CAnimation : public Component
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
		 * Determines the default clip to play as soon as the component is enabled. If more control over playing clips is needed
		 * use the play(), blend() and crossFade() methods to queue clips for playback manually, and setState() method for
		 * modify their states individually.
		 */
		BS_SCRIPT_EXPORT(n:DefaultClip,pr:setter)
		void setDefaultClip(const HAnimationClip& clip);

		/** @copydoc setDefaultClip */
		BS_SCRIPT_EXPORT(n:DefaultClip,pr:getter)
		HAnimationClip getDefaultClip() const { return mDefaultClip; }

		/** @copydoc Animation::setWrapMode */
		BS_SCRIPT_EXPORT(n:WrapMode,pr:setter)
		void setWrapMode(AnimWrapMode wrapMode);

		/** @copydoc setWrapMode */
		BS_SCRIPT_EXPORT(n:WrapMode,pr:getter)
		AnimWrapMode getWrapMode() const { return mWrapMode; }

		/** @copydoc Animation::setSpeed */
		BS_SCRIPT_EXPORT(n:Speed,pr:setter)
		void setSpeed(float speed);

		/** @copydoc setSpeed */
		BS_SCRIPT_EXPORT(n:Speed,pr:getter)
		float getSpeed() const { return mSpeed; }

		/** @copydoc Animation::play */
		BS_SCRIPT_EXPORT(n:Play)
		void play(const HAnimationClip& clip);

		/** @copydoc Animation::blendAdditive */
		BS_SCRIPT_EXPORT(n:BlendAdditive)
		void blendAdditive(const HAnimationClip& clip, float weight, float fadeLength = 0.0f, UINT32 layer = 0);

		/** @copydoc Animation::blend1D */
		BS_SCRIPT_EXPORT(n:Blend1D)
		void blend1D(const Blend1DInfo& info, float t);

		/** @copydoc Animation::blend2D */
		BS_SCRIPT_EXPORT(n:Blend2D)
		void blend2D(const Blend2DInfo& info, const Vector2& t);

		/** @copydoc Animation::crossFade */
		BS_SCRIPT_EXPORT(n:CrossFade)
		void crossFade(const HAnimationClip& clip, float fadeLength);

		/** @copydoc Animation::sample */
		BS_SCRIPT_EXPORT(n:Sample)
		void sample(const HAnimationClip& clip, float time);

		/** @copydoc Animation::stop */
		BS_SCRIPT_EXPORT(n:Stop)
		void stop(UINT32 layer);

		/** @copydoc Animation::stopAll */
		BS_SCRIPT_EXPORT(n:StopAll)
		void stopAll();
		
		/** @copydoc Animation::isPlaying */
		BS_SCRIPT_EXPORT(n:IsPlaying,pr:getter)
		bool isPlaying() const;

		/** @copydoc Animation::getState */
		BS_SCRIPT_EXPORT(n:GetState)
		bool getState(const HAnimationClip& clip, AnimationClipState& state);

		/** @copydoc Animation::setState */
		BS_SCRIPT_EXPORT(n:SetState)
		void setState(const HAnimationClip& clip, AnimationClipState state);

		/**
		 * Changes a weight of a single morph channel, determining how much of it to apply on top of the base mesh.
		 *
		 * @param name		Name of the morph channel to modify. This depends on the mesh the animation is currently
		 *					animating. 
		 * @param weight	Weight that determines how much of the channel to apply to the mesh, in range [0, 1]. 	
		 */
		BS_SCRIPT_EXPORT(n:SetMorphChannelWeight)
		void setMorphChannelWeight(const String& name, float weight);

		/** Determines bounds that will be used for animation and mesh culling. Only relevant if setUseBounds() is set to true. */
		BS_SCRIPT_EXPORT(n:Bounds,pr:setter)
		void setBounds(const AABox& bounds);

		/** @copydoc setBounds */
		BS_SCRIPT_EXPORT(n:Bounds,pr:getter)
		const AABox& getBounds() const { return mBounds; }

		/** 
		 * Determines should animation bounds be used for visibility determination (culling). If false the bounds of the
		 * mesh attached to the relevant CRenderable component will be used instead.
		 */
		BS_SCRIPT_EXPORT(n:UseBounds,pr:setter)
		void setUseBounds(bool enable);

		/** @copydoc setUseBounds */
		BS_SCRIPT_EXPORT(n:UseBounds,pr:getter)
		bool getUseBounds() const { return mUseBounds; }

		/** Enables or disables culling of the animation when out of view. Culled animation will not be evaluated. */
		BS_SCRIPT_EXPORT(n:Cull,pr:setter)
		void setEnableCull(bool enable);

		/** Checks whether the animation will be evaluated when it is out of view. */
		BS_SCRIPT_EXPORT(n:Cull,pr:getter)
		bool getEnableCull() const { return mEnableCull; }

		/** @copydoc Animation::getNumClips */
		BS_SCRIPT_EXPORT(in:true)
		UINT32 getNumClips() const;

		/** @copydoc Animation::getClip */
		BS_SCRIPT_EXPORT(in:true)
		HAnimationClip getClip(UINT32 idx) const;

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

		/**
		 * Rebuilds internal curve -> property mapping about the currently playing animation clip. This mapping allows the
		 * animation component to know which property to assign which values from an animation curve. This should be called
		 * whenever playback for a new clip starts, or when clip curves change.
		 */
		BS_SCRIPT_EXPORT(in:true)
		void _refreshClipMappings();

		/** @copydoc Animation::getGenericCurveValue */
		BS_SCRIPT_EXPORT(in:true)
		bool _getGenericCurveValue(UINT32 curveIdx, float& value);

		/** 
		 * Preview mode allows certain operations on the component to be allowed (like basic animation playback), 
		 * even when the component is not actively running. This is intended for use primarily by the animation editor. 
		 * Preview mode ends automatically when the component is enabled (i.e. starts running normally), or when 
		 * explicitly disabled. Returns true if the preview mode was enabled (which could fail if the component is
		 * currently running).
		 */
		BS_SCRIPT_EXPORT(in:true)
		bool _togglePreviewMode(bool enabled);

		/** Triggered when the list of properties animated via generic animation curves needs to be recreated (script only). */
		BS_SCRIPT_EXPORT(n:RebuildFloatProperties)
		std::function<void(const HAnimationClip&)> _scriptRebuildFloatProperties;

		/** Triggered when generic animation curves values need be applied to the properties they effect (script only). */
		BS_SCRIPT_EXPORT(n:_UpdateFloatProperties)
		std::function<void()> _scriptUpdateFloatProperties;

		/** Triggers a callback in script code when animation event is triggered (script only). */
		BS_SCRIPT_EXPORT(n:EventTriggered)
		std::function<void(const HAnimationClip&, const String&)> _scriptOnEventTriggered;

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

		/** @copydoc Component::update() */
		void update() override;

		/** @copydoc Component::onDisabled() */
		void onDisabled() override;

		/** @copydoc Component::onEnabled() */
		void onEnabled() override;

		/** @copydoc Component::onTransformChanged() */
		void onTransformChanged(TransformChangedFlags flags) override;
	protected:
		using Component::destroyInternal;

		/** Creates the internal representation of the Animation and restores the values saved by the Component. */
		void restoreInternal(bool previewMode);

		/** Destroys the internal Animation representation. */
		void destroyInternal();

		/** Callback triggered whenever an animation event is triggered. */
		void eventTriggered(const HAnimationClip& clip, const String& name);

		/** 
		 * Finds any scene objects that are mapped to bone transforms. Such object's transforms will be affected by 
		 * skeleton bone animation.
		 */
		void setBoneMappings();

		/**
		 * Finds any curves that affect a transform of a specific scene object, and ensures that animation properly updates
		 * those transforms. This does not include curves referencing bones.
		 */
		void updateSceneObjectMapping();

		/** @copydoc Animation::mapCurveToSceneObject */
		void mapCurveToSceneObject(const String& curve, const HSceneObject& so);

		/** @copydoc Animation::unmapSceneObject */
		void unmapSceneObject(const HSceneObject& so);
		
		/** Searches child scene objects for Bone components and returns any found ones. */
		Vector<HBone> findChildBones();

		SPtr<Animation> mInternal;
		HRenderable mAnimatedRenderable;

		HAnimationClip mDefaultClip;
		HAnimationClip mPrimaryPlayingClip;
		AnimWrapMode mWrapMode;
		float mSpeed;
		bool mEnableCull;
		bool mUseBounds;
		bool mPreviewMode;
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