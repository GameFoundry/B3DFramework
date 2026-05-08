//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Animation/B3DSkeleton.h"
#include "Animation/B3DSkeletonMask.h"
#include "Resources/B3DIResourceListener.h"
#include "Scene/B3DComponent.h"

namespace b3d
{
	/** @addtogroup Animation
	 *  @{
	 */

	/** Determines how an animation clip behaves when it reaches the end. */
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(Animation)) AnimationWrapMode
	{
		Loop, /**< Loop around to the beginning/end when the last/first frame is reached. */
		Clamp /**< Clamp to end/beginning, keeping the last/first frame active. */
	};

	/** Contains information about a currently playing animation clip. */
	struct B3D_SCRIPT_EXPORT(ExportAsStruct(true), DocumentationGroup(Animation)) AnimationClipState
	{
		AnimationClipState() = default;

		/** Layer the clip is playing on. Multiple clips can be played simulatenously on different layers. */
		u32 Layer = 0;
		float Time = 0.0f; /**< Current time the animation is playing from. */
		float Speed = 1.0f; /**< Speed at which the animation is playing. */
		float Weight = 1.0f; /**< Determines how much of an influence does the clip have on the final pose. */
		/** Determines what happens to other animation clips when a new clip starts playing. */
		AnimationWrapMode WrapMode = AnimationWrapMode::Loop;
		/**
		 * Determines should the time be advanced automatically. Certain type of animation clips don't involve playback
		 * (e.g. for blending where animation weight controls the animation).
		 */
		bool Stopped = false;
	};

	/** Represents an animation clip used in 1D blending. Each clip has a position on the number line. */
	struct B3D_EXPORT B3D_SCRIPT_EXPORT(ExportAsStruct(true), DocumentationGroup(Animation)) BlendClipInfo
	{
		BlendClipInfo() = default;

		HAnimationClip Clip;
		float Position = 0.0f;
	};

	/** Defines a 1D blend where multiple animation clips are blended between each other using linear interpolation. */
	struct B3D_EXPORT B3D_SCRIPT_EXPORT(ExportAsStruct(true), DocumentationGroup(Animation)) Blend1DInfo
	{
		Vector<BlendClipInfo> Clips;
	};

	/** Defines a 2D blend where two animation clips are blended between each other using bilinear interpolation. */
	struct B3D_SCRIPT_EXPORT(ExportAsStruct(true), DocumentationGroup(Animation)) Blend2DInfo
	{
		HAnimationClip TopLeftClip;
		HAnimationClip TopRightClip;
		HAnimationClip BottomLeftClip;
		HAnimationClip BottomRightClip;
	};

	/** @} */

	/** @addtogroup Animation-Internal
	 *  @{
	 */

	/** Flags that determine which portion of Animation was changed and needs to be updated. */
	enum class AnimationDirtyStateFlag
	{
		Clean = 0,
		Value = 1 << 0,
		Layout = 1 << 1,
		All = 1 << 2,
		Culling = 1 << 3,
		MorphWeights = 1 << 4
	};

	typedef Flags<AnimationDirtyStateFlag> AnimDirtyState;
	B3D_FLAGS_OPERATORS(AnimationDirtyStateFlag)

	/** Type of playback for animation clips. */
	enum class AnimationPlaybackType
	{
		/** Play back the animation normally by advancing time. */
		Normal,
		/** Sample only a single frame from the animation. */
		Sampled,
		/** Do not play the animation. */
		None
	};

	/** Steps used for progressing through the animation when it is being sampled a single frame. */
	enum class AnimationSampleStep
	{
		/** No sample. Either no playback at all or normal playback. */
		None,

		/** Sample is being done this frame. */
		Frame,

		/** Sample has been performed some previous frame. */
		Done
	};

	/** Internal information about a single playing animation clip within Animation. */
	struct AnimationClipInfo
	{
		AnimationClipInfo() = default;
		AnimationClipInfo(const HAnimationClip& clip);

		HAnimationClip Clip;
		AnimationClipState State;
		AnimationPlaybackType PlaybackType = AnimationPlaybackType::Normal;

		float FadeDirection = 0.0f;
		float FadeTime = 0.0f;
		float FadeLength = 0.0f;

		/**
		 * Version of the animation curves used by the AnimationProxy. Used to detecting the internal animation curves
		 * changed.
		 */
		u64 CurveVersion = 0;
		u32 LayerIndex = ~0u; /**< Layer index this clip belongs to in AnimationProxy structure. */
		u32 StateIndex = ~0u; /**< State index this clip belongs to in AnimationProxy structure. */
	};

	/** Contains a mapping between a scene object and an animation curve it is animated with. */
	struct SceneObjectMappingCurveInfo
	{
		HSceneObject Object;
		String CurveName;
	};

	/** Information about a set of morph shapes blended sequentially. */
	struct MorphChannelInfo
	{
		float Weight;
		u32 ShapeStart;
		u32 ShapeCount;

		u32 FrameCurveIndex;
		u32 WeightCurveIdx;
	};

	/** Morph shape and its contribution to the final shape. */
	struct MorphShapeInfo
	{
		TShared<MorphShape> Shape;
		float FrameWeight;
		float FinalWeight;
	};

	/** Contains information about a scene object that is animated by a specific animation curve. */
	struct AnimatedSceneObjectInfo
	{
		UUID SceneObjectId; /**< ID of the scene object. */
		i32 BoneIndex; /**< Bone from which to access the transform. If -1 then no bone mapping is present. */
		i32 LayerIndex; /**< If no bone mapping, layer on which the animation containing the referenced curve is in. */
		i32 StateIndex; /**< If no bone mapping, animation state containing the referenced curve. */
		AnimationCurveMapping CurveIndices; /**< Indices of the curves used for the transform. */
		u32 Hash; /**< Hash value of the scene object's transform. */
	};

	/** Represents a copy of the Animation data for use specifically on the animation thread. */
	struct AnimationProxy
	{
		AnimationProxy(u64 animationId);
		AnimationProxy(const AnimationProxy&) = delete;
		~AnimationProxy();

		AnimationProxy& operator=(const AnimationProxy&) = delete;

		/**
		 * Rebuilds the internal proxy data according to the newly assigned skeleton and clips. This should be called
		 * whenever the animation skeleton changes.
		 *
		 * @param	skeleton			New skeleton to assign to the proxy.
		 * @param	mask				Mask that filters which skeleton bones are enabled or disabled.
		 * @param	inOutClipInfos		Potentially new clip infos that will be used for rebuilding the proxy. Once the
		 *									method completes clip info layout and state indices will be populated for
		 *									further use in the Update*() methods.
		 * @param	sceneObjects		A list of scene objects that are influenced by specific animation curves.
		 * @param	morphShapes			Morph shapes used for per-vertex animation.
		 *
		 * @note Should be called from the main thread when the caller is sure the animation thread is not using it.
		 */
		void RebuildFull(const TShared<Skeleton>& skeleton, const SkeletonMask& mask, Vector<AnimationClipInfo>& inOutClipInfos, const Vector<SceneObjectMappingCurveInfo>& sceneObjects, const TShared<MorphShapes>& morphShapes);

		/**
		 * Rebuilds the internal proxy data according to the newly clips. This should be called whenever clips are added
		 * or removed, or clip layout indices change.
		 *
		 * @param	inOutClipInfos		New clip infos that will be used for rebuilding the proxy. Once the method
		 *									completes clip info layout and state indices will be populated for further use
		 *									in the Update*() methods.
		 * @param	sceneObjects		A list of scene objects that are influenced by specific animation curves.
		 * @param	morphShapes			Morph shapes used for per-vertex animation.
		 *
		 * @note Should be called from the main thread when the caller is sure the animation thread is not using it.
		 */
		void RebuildClips(Vector<AnimationClipInfo>& inOutClipInfos, const Vector<SceneObjectMappingCurveInfo>& sceneObjects, const TShared<MorphShapes>& morphShapes);

		/**
		 * Updates the proxy data with new information about the clips. Caller must guarantee that clip layout didn't
		 * change since the last call to Rebuild*().
		 *
		 * @note	Should be called from the main thread when the caller is sure the animation thread is not using it.
		 */
		void UpdateClipInfos(const Vector<AnimationClipInfo>& clipInfos);

		/**
		 * Updates the proxy data with new weights used for morph shapes. Caller must ensure the weights are ordered so
		 * they match with the morph shapes provided to the last Rebuild*() call.
		 */
		void UpdateMorphChannelWeights(const Vector<float>& weights);

		/**
		 * Updates the proxy data with new scene object transforms. Caller must guarantee that clip layout didn't
		 * change since the last call to Rebuild*().
		 *
		 * @note	Should be called from the main thread when the caller is sure the animation thread is not using it.
		 */
		void UpdateTransforms(const Vector<SceneObjectMappingCurveInfo>& sceneObjects);

		/**
		 * Updates the proxy data with new clip times. Caller must guarantee that clip layout didn't change since the last
		 * call to Rebuild*().
		 *
		 * @note	Should be called from the main thread when the caller is sure the animation thread is not using it.
		 */
		void UpdateTime(const Vector<AnimationClipInfo>& clipInfos);

		/** Destroys all dynamically allocated objects. */
		void Clear();

		u64 AnimationId;

		// Skeletal animation
		AnimationStateLayer* Layers = nullptr;
		u32 LayerCount = 0;
		TShared<Skeleton> Skeleton;
		SkeletonMask SkeletonMask;
		u32 SceneObjectCount = 0;
		AnimatedSceneObjectInfo* SceneObjectInfos = nullptr;
		Matrix4* SceneObjectTransforms = nullptr;

		// Morph shape animation
		MorphChannelInfo* MorphChannelInfos = nullptr;
		MorphShapeInfo* MorphShapeInfos = nullptr;
		u32 MorphChannelCount = 0;
		u32 MorphShapeCount = 0;
		u32 MorphVertexCount = 0;
		bool MorphChannelWeightsDirty = false;

		// Culling
		AABox Bounds;
		bool CullEnabled = true;

		// Single frame sample
		AnimationSampleStep SampleStep = AnimationSampleStep::None;

		// Evaluation results
		LocalSkeletonPose SkeletonPose;
		LocalSkeletonPose SceneObjectPose;
		u32 GenericCurveCount = 0;
		float* GenericCurveOutputs = nullptr;
		bool WasCulled = false;
	};

	/** @} */

	/** @addtogroup Animation
	 *  @{
	 */

	/**
	 * Handles animation playback. Takes one or multiple animation clips as input and evaluates them every animation update
	 * tick depending on set properties. The evaluated data is used by the render thread for skeletal animation, by the main
	 * thread for updating attached scene objects and bones (if skeleton is attached), or the data is made available for
	 * manual queries in the case of generic animation.
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Animation)) Animation : public Component, public IResourceListener
	{
		/** Information about scene objects bound to a specific animation curve. */
		struct SceneObjectMappingInfo
		{
			HSceneObject SceneObject;
			bool IsMappedToBone;
			HBone Bone;
		};

	public:
		Animation(const HSceneObject& parent);

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
		 * @param	clip		Clip to retrieve the information for.
		 * @param	outState	Animation clip state containing the requested information. Only valid if the method returns true.
		 * @return				True if the state was found (animation clip is playing), false otherwise.
		 */
		B3D_SCRIPT_EXPORT(ExportName(GetState))
		bool GetState(const HAnimationClip& clip, AnimationClipState& outState);

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
		 * @param	name	Name of the morph channel to modify. This depends on the mesh the animation is currently
		 *					animating.
		 * @param	weight	Weight that determines how much of the channel to apply to the mesh, in range [0, 1].
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

		/** Returns the unique ID for this animation object. */
		u64 GetAnimationId() const { return mAnimationId; }

		/**
		 * Changes the skeleton which will the translation/rotation/scale animation values manipulate. If no skeleton is set
		 * the animation will only evaluate the generic curves, and the root translation/rotation/scale curves.
		 */
		void SetSkeleton(const TShared<Skeleton>& skeleton);

		/**
		 * Sets morph shapes that can be used for per-vertex blending animation. After they're set call
		 * setMorphShapeWeight() to apply morph shapes.
		 */
		void SetMorphShapes(const TShared<MorphShapes>& morphShapes);

		/**
		 * Sets a mask that allows certain bones from the skeleton to be disabled. Caller must ensure that the mask matches
		 * the skeleton assigned to the animation.
		 */
		void SetMask(const SkeletonMask& mask);

		/**
		 * Registers a new bone component, creating a new transform mapping from the bone name to the scene object the
		 * component is attached to.
		 */
		void AddBone(const HBone& bone);

		/** Unregisters a bone component, removing the bone -> scene object mapping. */
		void RemoveBone(const HBone& bone);

		/** Called whenever the bone name the Bone component points to changes. */
		void NotifyBoneNameChanged(const HBone& bone);

		/** Checks if any currently set animation clips perform animation of the root bone. */
		bool GetAnimatesRoot() const;

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

		/** Returns bounds used for culling the animation. */
		AABox GetCullingBounds() const { return mCullingBounds; }

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
		 * @param	curveIndex	The curve index referencing a set of curves from the first playing animation clip.
		 *						Generic curves from all other clips are ignored.
		 * @param	outValue	Value of the generic curve. Only valid if the method return true.
		 * @return				True if the value was retrieved successfully. The method might fail if animation update
		 *						didn't yet have a chance to execute and values are not yet available, or if the
		 *						animation clip changed since the last frame (the last problem can be avoided by ensuring
		 *						to read the curve values before changing the clip).
		 */
		B3D_SCRIPT_EXPORT(InteropOnly(true))
		bool GetGenericCurveValueInternal(u32 curveIndex, float& outValue);

		/**
		 * Preview mode allows certain operations on the component to be allowed (like basic animation playback),
		 * even when the component is not actively running. This is intended for use primarily by the animation editor.
		 * Preview mode ends automatically when the component is enabled (i.e. starts running normally), or when
		 * explicitly disabled. Returns true if the preview mode was enabled (which could fail if the component is
		 * currently running). When previewing, you can only run the animation using Sample() or directly by setting
		 * state, all other playback commands will be ignored.
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
		friend class AnimationScene;

		/** Creates the internal animation proxy and registers it with the animation scene. Animation without a proxy cannot be played.  */
		void CreateAnimationProxy(bool previewMode);

		/** Destroys the internal animation proxy. Animation without a proxy cannot be played. */
		void DestroyAnimationProxy();

		/** Returns true if the animation proxy is created and registered with the animation scene. If false, animation cannot play. */
		bool IsAnimationProxyValid() const { return mAnimationId != ~0ull; }

		/** Callback triggered whenever an animation event is triggered. */
		void NotifyAnimationEventTriggered(const HAnimationClip& clip, const String& name);

		/**
		 * Finds any scene objects that are mapped to bone transforms. Such object's transforms will be affected by
		 * skeleton bone animation.
		 */
		void RebuildBoneMappings();

		/**
		 * Finds any curves that affect a transform of a specific scene object, and ensures that animation properly updates
		 * those transforms. This does not include curves referencing bones.
		 */
		void RebuildGenericMappings();

		/** Searches child scene objects for Bone components and returns any found ones. */
		Vector<HBone> FindChildBones();
		/**
		 * Triggers any events between the last frame and current one.
		 *
		 * @param	timeDelta	Time elapsed since the last call to this method.
		 */
		void TriggerEvents(float timeDelta);

		/**
		 * Updates the animation proxy object based on the currently set skeleton, playing clips and dirty flags.
		 *
		 * @param	timeDelta	Seconds passed since the last call to this method.
		 */
		void UpdateAnimationProxy(float timeDelta);

		/**
		 * Applies any outputs stored in the animation proxy (as written by the animation thread), and uses them to update
		 * the animation state on the main thread. Caller must ensure that the animation thread has finished
		 * with the animation proxy.
		 */
		void UpdateFromProxy();

		/**
		 * Registers a new animation in the specified layer, or returns an existing animation clip info if the animation is
		 * already registered. If @p stopExisting is true any existing animations in the layer will be stopped. Layout
		 * will be marked as dirty if any changes were made.
		 */
		AnimationClipInfo* GetOrCreateClipInfo(const HAnimationClip& clip, u32 layer, bool stopExisting = true);

		void GetListenerResources(Vector<HResource>& resources) override;
		void NotifyResourceLoaded(const HResource& resource) override;
		void NotifyResourceChanged(const HResource& resource) override;

		u64 mAnimationId = ~0ull;
		AnimDirtyState mDirty = AnimationDirtyStateFlag::All;

		HRenderable mAnimatedRenderable;

		HAnimationClip mDefaultClip;
		HAnimationClip mPrimaryPlayingClip;
		AnimationWrapMode mWrapMode = AnimationWrapMode::Loop;
		float mSpeed = 1.0f;
		bool mEnableCull = true;
		bool mUseCustomBounds = false;
		bool mPreviewMode = false;
		AABox mCustomBounds;
		AABox mCullingBounds;
		
		TShared<Skeleton> mSkeleton;
		SkeletonMask mSkeletonMask;
		TShared<MorphShapes> mMorphShapes;
		Vector<float> mMorphChannelWeights;
		Vector<AnimationClipInfo> mClipInfos;
		Vector<SceneObjectMappingInfo> mMappedSceneObjectInfos;
		UnorderedMap<UUID, SceneObjectMappingCurveInfo> mMappedSceneObjectsById;
		Vector<float> mGenericCurveOutputs;
		bool mGenericCurveValuesValid = false;
		AnimationSampleStep mSampleStep = AnimationSampleStep::None;

		// Animation thread only
		TShared<AnimationProxy> mAnimationProxy;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class AnimationRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;

	protected:
		Animation(); // Serialization only
	};

	/** @} */
} // namespace b3d
