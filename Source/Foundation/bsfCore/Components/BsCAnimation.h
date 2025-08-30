//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Animation/BsAnimation.h"
#include "Scene/BsComponent.h"

namespace b3d
{
	/** @addtogroup Components-Core
	 *  @{
	 */

	/**
	 * Handles animation playback. Takes one or multiple animation clips as input and evaluates them every animation update
	 * tick depending on set properties. The evaluated data is used by the render thread for skeletal animation, by the main
	 * thread for updating attached scene objects and bones (if skeleton is attached), or the data is made available for
	 * manual queries in the case of generic animation.
	 */
	class B3D_CORE_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Animation), ExportName(Animation)) CAnimation : public Component
	{
		/** Information about scene objects bound to a specific animation curve. */
		struct SceneObjectMappingInfo
		{
			HSceneObject SceneObject;
			bool IsMappedToBone;
			HBone Bone;
		};

	public:
		CAnimation(const HSceneObject& parent);

		/**
		 * Determines the default clip to play as soon as the component is enabled. If more control over playing clips is needed
		 * use the Play(), Blend*(), CrossFade() methods to queue clips for playback manually, and SetState() method to
		 * modify their states individually.
		 */
		B3D_SCRIPT_EXPORT(ExportName(DefaultClip), Property(Setter))
		void SetDefaultClip(const HAnimationClip& clip);

		/** @copydoc SetDefaultClip */
		B3D_SCRIPT_EXPORT(ExportName(DefaultClip), Property(Getter))
		HAnimationClip GetDefaultClip() const { return mDefaultClip; }

		/**
		 * Determines the wrap mode for all active animations. Wrap mode determines what happens when animation reaches the
		 * first or last frame.
		 */
		B3D_SCRIPT_EXPORT(ExportName(WrapMode), Property(Setter))
		void SetWrapMode(AnimationWrapMode wrapMode);

		/** @copydoc SetWrapMode */
		B3D_SCRIPT_EXPORT(ExportName(WrapMode), Property(Getter))
		AnimationWrapMode GetWrapMode() const { return mWrapMode; }

		/** Determines the speed for all animations. The default value is 1.0f. Use negative values to play-back in reverse. */
		B3D_SCRIPT_EXPORT(ExportName(Speed), Property(Setter))
		void SetSpeed(float speed);

		/** @copydoc SetSpeed */
		B3D_SCRIPT_EXPORT(ExportName(Speed), Property(Getter))
		float GetSpeed() const { return mSpeed; }

		/** Plays the specified animation clip. */
		B3D_SCRIPT_EXPORT(ExportName(Play))
		void Play(const HAnimationClip& clip);

		/**
		 * Plays the specified animation clip on top of the animation currently playing in the main layer. Multiple
		 * such clips can be playing at once, as long as you ensure each is given its own layer. Each animation can
		 * also have a weight that determines how much it influences the main animation.
		 *
		 * @param	clip		Clip to additively blend. Must contain additive animation curves.
		 * @param	weight		Determines how much of an effect will the blended animation have on the final output. In range [0, 1].
		 * @param	fadeLength	Applies the blend over a specified time period, increasing the weight as the time
		 *						passes. Set to zero to blend immediately. In seconds.
		 * @param	layer		Layer to play the clip in. Multiple additive clips can be playing at once in separate
		 *						layers and each layer has its own weight.
		 */
		B3D_SCRIPT_EXPORT(ExportName(BlendAdditive))
		void BlendAdditive(const HAnimationClip& clip, float weight, float fadeLength = 0.0f, u32 layer = 0);

		/**
		 * Blend multiple animation clips between each other using linear interpolation. Unlike normal animations these
		 * animations are not advanced with the progress of time, and is instead expected the user manually changes the
		 * @p alpha parameter.
		 *
		 * @param	info	Information about the clips to blend. Clip positions must be sorted from lowest to highest.
		 * @param	alpha	Parameter that controls the blending. Range depends on the positions of the provided
		 *					animation clips.
		 */
		B3D_SCRIPT_EXPORT(ExportName(Blend1D))
		void Blend1D(const Blend1DInfo& info, float alpha);

		/**
		 * Blend four animation clips between each other using bilinear interpolation. Unlike normal animations these
		 * animations are not advanced with the progress of time, and is instead expected the user manually changes the
		 * @p alpha parameter.
		 *
		 * @param	info	Information about the clips to blend.
		 * @param	alpha	Parameter that controls the blending, in range [(0, 0), (1, 1)]. alpha = (0, 0) means top left
		 *					animation has full influence, alpha = (1, 0) means top right animation has full influence,
		 *					alpha = (0, 1) means bottom left animation has full influence, alpha = (1, 1) means bottom right
		 *					animation has full influence.
		 */
		B3D_SCRIPT_EXPORT(ExportName(Blend2D))
		void Blend2D(const Blend2DInfo& info, const Vector2& alpha);

		/**
		 * Fades the specified animation clip in, while fading other playing animation out, over the specified time period.
		 *
		 * @param	clip		Clip to fade in.
		 * @param	fadeLength	Determines the time period over which the fade occurs. In seconds.
		 */
		B3D_SCRIPT_EXPORT(ExportName(CrossFade))
		void CrossFade(const HAnimationClip& clip, float fadeLength);

		/**
		 * Samples an animation clip at the specified time, displaying only that particular frame without further playback.
		 *
		 * @param clip	Animation clip to sample.
		 * @param time	Time to sample the clip at.
		 */
		B3D_SCRIPT_EXPORT(ExportName(Sample))
		void Sample(const HAnimationClip& clip, float time);

		/**
		 * Stops playing all animations on the provided layer. Specify ~0u to stop animation on the main layer
		 * (non-additive animations).
		 */
		B3D_SCRIPT_EXPORT(ExportName(Stop))
		void Stop(u32 layer);

		/** Stops playing all animations. */
		B3D_SCRIPT_EXPORT(ExportName(StopAll))
		void StopAll();

		/** Checks if any animation clips are currently playing. */
		B3D_SCRIPT_EXPORT(ExportName(IsPlaying), Property(Getter))
		bool IsPlaying() const;

		/**
		 * Retrieves detailed information about a currently playing animation clip.
		 *
		 * @param	clip	Clip to retrieve the information for.
		 * @param	state	Animation clip state containing the requested information. Only valid if the method returns
		 *					true.
		 * @return			True if the state was found (animation clip is playing), false otherwise.
		 */
		B3D_SCRIPT_EXPORT(ExportName(GetState))
		bool GetState(const HAnimationClip& clip, AnimationClipState& state);

		/**
		 * Changes the state of a playing animation clip. If animation clip is not currently playing the playback is started
		 * for the clip.
		 *
		 * @param	clip	Clip to change the state for.
		 * @param	state	New state of the animation (e.g. changing the time for seeking).
		 */
		B3D_SCRIPT_EXPORT(ExportName(SetState))
		void SetState(const HAnimationClip& clip, AnimationClipState state);

		/**
		 * Changes a weight of a single morph channel, determining how much of it to apply on top of the base mesh.
		 *
		 * @param name		Name of the morph channel to modify. This depends on the mesh the animation is currently
		 *					animating.
		 * @param weight	Weight that determines how much of the channel to apply to the mesh, in range [0, 1].
		 */
		B3D_SCRIPT_EXPORT(ExportName(SetMorphChannelWeight))
		void SetMorphChannelWeight(const String& name, float weight);

		/** Determines bounds that will be used for animation and mesh culling. Only relevant if SetUseCustomBounds() is set to true. */
		B3D_SCRIPT_EXPORT(ExportName(CustomBounds), Property(Setter))
		void SetCustomBounds(const AABox& bounds);

		/** @copydoc SetCustomBounds */
		B3D_SCRIPT_EXPORT(ExportName(CustomBounds), Property(Getter))
		const AABox& GetCustomBounds() const { return mCustomBounds; }

		/**
		 * Determines should animation bounds be used for visibility determination (culling). If false the bounds of the
		 * mesh attached to the relevant Renderable component will be used instead.
		 */
		B3D_SCRIPT_EXPORT(ExportName(UseCustomBounds), Property(Setter))
		void SetUseCustomBounds(bool enable);

		/** @copydoc SetUseCustomBounds */
		B3D_SCRIPT_EXPORT(ExportName(UseCustomBounds), Property(Getter))
		bool GetUseCustomBounds() const { return mUseCustomBounds; }

		/** Enables or disables culling of the animation when out of view. Culled animation will not be evaluated. */
		B3D_SCRIPT_EXPORT(ExportName(Cull), Property(Setter))
		void SetEnableCull(bool enable);

		/** Checks whether the animation will be evaluated when it is out of view. */
		B3D_SCRIPT_EXPORT(ExportName(Cull), Property(Getter))
		bool GetEnableCull() const { return mEnableCull; }

		/** Returns the total number of animation clips influencing this animation. */
		B3D_SCRIPT_EXPORT(InteropOnly(true))
		u32 GetClipCount() const;

		/**
		 * Returns one of the animation clips influencing this animation.
		 *
		 * @param	index	Sequential index of the animation clip to retrieve.
		 * @return			Animation clip at the specified index, or null if the index is out of range.
		 */
		B3D_SCRIPT_EXPORT(InteropOnly(true))
		HAnimationClip GetClip(u32 index) const;

		/** Triggered whenever an animation event is reached. */
		Event<void(const HAnimationClip&, const String&)> OnEventTriggered;

		/** @name Internal
		 *  @{
		 */

		/** Returns the Animation implementation wrapped by this component. */
		SPtr<Animation> GetInternalInternal() const { return mInternal; }

		/**
		 * Registers a new bone component, creating a new transform mapping from the bone name to the scene object the
		 * component is attached to.
		 */
		void AddBone(const HBone& bone);

		/** Unregisters a bone component, removing the bone -> scene object mapping. */
		void RemoveBone(const HBone& bone);

		/** Called whenever the bone name the Bone component points to changes. */
		void NotifyBoneChanged(const HBone& bone);

		/**
		 * Registers a Renderable component with the animation, should be called whenever a Renderable component is added
		 * to the same scene object as this component.
		 */
		void RegisterRenderable(const HRenderable& renderable);

		/**
		 * Removes renderable from the animation component. Should be called when a Renderable component is removed from
		 * this scene object.
		 */
		void UnregisterRenderable();

		/** Re-applies the bounds to the internal animation object, and the relevant renderable object if one exists. */
		void UpdateBounds(bool updateRenderable = true);

		/**
		 * Rebuilds internal curve -> property mapping about the currently playing animation clip. This mapping allows the
		 * animation component to know which property to assign which values from an animation curve. This should be called
		 * whenever playback for a new clip starts, or when clip curves change.
		 */
		B3D_SCRIPT_EXPORT(InteropOnly(true))
		void RefreshClipMappingsInternal();

		/**
		 * Retrieves an evaluated value for a generic curve with the specified index.
		 *
		 * @param	index		The curve index referencing a set of curves from the first playing animation clip.
		 *						Generic curves from all other clips are ignored.
		 * @param	outValue	Value of the generic curve. Only valid if the method return true.
		 * @return				True if the value was retrieved successfully. The method might fail if animation update
		 *						didn't yet have a chance to execute and values are not yet available, or if the
		 *						animation clip changed since the last frame (the last problem can be avoided by ensuring
		 *						to read the curve values before changing the clip).
		 */
		B3D_SCRIPT_EXPORT(InteropOnly(true))
		bool GetGenericCurveValueInternal(u32 index, float& outValue);

		/**
		 * Preview mode allows certain operations on the component to be allowed (like basic animation playback),
		 * even when the component is not actively running. This is intended for use primarily by the animation editor.
		 * Preview mode ends automatically when the component is enabled (i.e. starts running normally), or when
		 * explicitly disabled. Returns true if the preview mode was enabled (which could fail if the component is
		 * currently running).
		 */
		B3D_SCRIPT_EXPORT(InteropOnly(true))
		bool TogglePreviewModeInternal(bool enabled);

		/** Triggered when the list of properties animated via generic animation curves needs to be recreated (script only). */
		B3D_SCRIPT_EXPORT(ExportName(RebuildFloatProperties))
		std::function<void(const HAnimationClip&)> ScriptRebuildFloatPropertiesInternal;

		/** Triggered when generic animation curves values need be applied to the properties they effect (script only). */
		B3D_SCRIPT_EXPORT(ExportName(_UpdateFloatProperties))
		std::function<void()> ScriptUpdateFloatPropertiesInternal;

		/** Triggers a callback in script code when animation event is triggered (script only). */
		B3D_SCRIPT_EXPORT(ExportName(EventTriggered))
		std::function<void(const HAnimationClip&, const String&)> ScriptOnEventTriggeredInternal;

		/** @} */

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;

		void OnDestroyed() override;
		void Update() override;
		void OnDisabled() override;
		void OnEnabled() override;
		void OnTransformChanged(TransformChangedFlags flags) override;

	protected:
		/** Creates the internal representation of the Animation and restores the values saved by the Component. */
		void RestoreInternal(bool previewMode);

		/** Destroys the internal Animation representation. */
		void DestroyInternal();

		/** Callback triggered whenever an animation event is triggered. */
		void EventTriggered(const HAnimationClip& clip, const String& name);

		/**
		 * Finds any scene objects that are mapped to bone transforms. Such object's transforms will be affected by
		 * skeleton bone animation.
		 */
		void SetBoneMappings();

		/**
		 * Finds any curves that affect a transform of a specific scene object, and ensures that animation properly updates
		 * those transforms. This does not include curves referencing bones.
		 */
		void UpdateSceneObjectMapping();

		/**
		 * Ensures that any position/rotation/scale animation of a specific animation curve is transferred to the
		 * provided scene object. Also allow the opposite operation which can allow scene object transform changes
		 * to manipulate object bones.
		 *
		 * @param	curve			Name of the curve (bone) to connect the scene object with. Use empty string to map to the
		 *							root bone, regardless of the bone name.
		 * @param	sceneObject		Scene object to influence by the curve modifications, and vice versa.
		 */
		void MapCurveToSceneObject(const String& curve, const HSceneObject& sceneObject);

		/** Removes the curve <-> scene object mapping that was set via MapCurveToSceneObject(). */
		void UnmapSceneObject(const HSceneObject& sceneObject);

		/** Searches child scene objects for Bone components and returns any found ones. */
		Vector<HBone> FindChildBones();

		SPtr<Animation> mInternal;
		HRenderable mAnimatedRenderable;

		HAnimationClip mDefaultClip;
		HAnimationClip mPrimaryPlayingClip;
		AnimationWrapMode mWrapMode = AnimationWrapMode::Loop;
		float mSpeed = 1.0f;
		bool mEnableCull = true;
		bool mUseCustomBounds = false;
		bool mPreviewMode = false;
		AABox mCustomBounds;

		Vector<SceneObjectMappingInfo> mMappedSceneObjectInfos;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class CAnimationRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;

	protected:
		CAnimation(); // Serialization only
	};

	/** @} */
} // namespace b3d
