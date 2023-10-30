//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "CoreObject/BsCoreObject.h"
#include "Resources/BsIResourceListener.h"
#include "Utility/BsFlags.h"
#include "Animation/BsSkeleton.h"
#include "Animation/BsSkeletonMask.h"
#include "Math/BsVector2.h"
#include "Math/BsAABox.h"

namespace bs
{
	/** @addtogroup Animation
	 *  @{
	 */

	/** Determines how an animation clip behaves when it reaches the end. */
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(Animation)) AnimWrapMode
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
		AnimWrapMode WrapMode = AnimWrapMode::Loop;
		/**
		 * Determines should the time be advanced automatically. Certain type of animation clips don't involve playback
		 * (e.g. for blending where animation weight controls the animation).
		 */
		bool Stopped = false;
	};

	/** @} */

	/** @addtogroup Animation-Internal
	 *  @{
	 */

	/** Flags that determine which portion of Animation was changed and needs to be updated. */
	enum class AnimDirtyStateFlag
	{
		Clean = 0,
		Value = 1 << 0,
		Layout = 1 << 1,
		All = 1 << 2,
		Culling = 1 << 3,
		MorphWeights = 1 << 4
	};

	typedef Flags<AnimDirtyStateFlag> AnimDirtyState;
	B3D_FLAGS_OPERATORS(AnimDirtyStateFlag)

	/** Type of playback for animation clips. */
	enum class AnimPlaybackType
	{
		/** Play back the animation normally by advancing time. */
		Normal,
		/** Sample only a single frame from the animation. */
		Sampled,
		/** Do not play the animation. */
		None
	};

	/** Steps used for progressing through the animation when it is being sampled a single frame. */
	enum class AnimSampleStep
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
		AnimPlaybackType PlaybackType = AnimPlaybackType::Normal;

		float FadeDirection = 0.0f;
		float FadeTime = 0.0f;
		float FadeLength = 0.0f;

		/**
		 * Version of the animation curves used by the AnimationProxy. Used to detecting the internal animation curves
		 * changed.
		 */
		u64 CurveVersion = 0;
		u32 LayerIdx = (u32)-1; /**< Layer index this clip belongs to in AnimationProxy structure. */
		u32 StateIdx = (u32)-1; /**< State index this clip belongs to in AnimationProxy structure. */
	};

	/** Represents an animation clip used in 1D blending. Each clip has a position on the number line. */
	struct B3D_CORE_EXPORT B3D_SCRIPT_EXPORT(ExportAsStruct(true), DocumentationGroup(Animation)) BlendClipInfo
	{
		BlendClipInfo() = default;

		HAnimationClip Clip;
		float Position = 0.0f;
	};

	/** Defines a 1D blend where multiple animation clips are blended between each other using linear interpolation. */
	struct B3D_CORE_EXPORT B3D_SCRIPT_EXPORT(ExportAsStruct(true), DocumentationGroup(Animation)) Blend1DInfo
	{
		Vector<BlendClipInfo> Clips;
	};

	/** Defines a 2D blend where two animation clips are blended between each other using bilinear interpolation. */
	struct B3D_SCRIPT_EXPORT(ExportAsStruct(true), DocumentationGroup(Animation)) Blend2DInfo
	{
		HAnimationClip TopLeftClip;
		HAnimationClip TopRightClip;
		HAnimationClip BotLeftClip;
		HAnimationClip BotRightClip;
	};

	/** Contains a mapping between a scene object and an animation curve it is animated with. */
	struct AnimatedSceneObject
	{
		HSceneObject So;
		String CurveName;
	};

	/** Information about a set of morph shapes blended sequentially. */
	struct MorphChannelInfo
	{
		float Weight;
		u32 ShapeStart;
		u32 ShapeCount;

		u32 FrameCurveIdx;
		u32 WeightCurveIdx;
	};

	/** Morph shape and its contribution to the final shape. */
	struct MorphShapeInfo
	{
		SPtr<MorphShape> Shape;
		float FrameWeight;
		float FinalWeight;
	};

	/** Contains information about a scene object that is animated by a specific animation curve. */
	struct AnimatedSceneObjectInfo
	{
		u64 Id; /**< Instance ID of the scene object. */
		i32 BoneIdx; /**< Bone from which to access the transform. If -1 then no bone mapping is present. */
		i32 LayerIdx; /**< If no bone mapping, layer on which the animation containing the referenced curve is in. */
		i32 StateIdx; /**< If no bone mapping, animation state containing the referenced curve. */
		AnimationCurveMapping CurveIndices; /**< Indices of the curves used for the transform. */
		u32 Hash; /**< Hash value of the scene object's transform. */
	};

	/** Represents a copy of the Animation data for use specifically on the animation thread. */
	struct AnimationProxy
	{
		AnimationProxy(u64 id);
		AnimationProxy(const AnimationProxy&) = delete;
		~AnimationProxy();

		AnimationProxy& operator=(const AnimationProxy&) = delete;

		/**
		 * Rebuilds the internal proxy data according to the newly assigned skeleton and clips. This should be called
		 * whenever the animation skeleton changes.
		 *
		 * @param[in]		skeleton		New skeleton to assign to the proxy.
		 * @param[in]		mask			Mask that filters which skeleton bones are enabled or disabled.
		 * @param[in, out]	clipInfos		Potentially new clip infos that will be used for rebuilding the proxy. Once the
		 *									method completes clip info layout and state indices will be populated for
		 *									further use in the update*() methods.
		 * @param[in]		sceneObjects	A list of scene objects that are influenced by specific animation curves.
		 * @param[in]		morphShapes		Morph shapes used for per-vertex animation.
		 *
		 * @note	Should be called from the main thread when the caller is sure the animation thread is not using it.
		 */
		void Rebuild(const SPtr<Skeleton>& skeleton, const SkeletonMask& mask, Vector<AnimationClipInfo>& clipInfos, const Vector<AnimatedSceneObject>& sceneObjects, const SPtr<MorphShapes>& morphShapes);

		/**
		 * Rebuilds the internal proxy data according to the newly clips. This should be called whenever clips are added
		 * or removed, or clip layout indices change.
		 *
		 * @param[in, out]	clipInfos		New clip infos that will be used for rebuilding the proxy. Once the method
		 *									completes clip info layout and state indices will be populated for further use
		 *									in the update*() methods.
		 * @param[in]		sceneObjects	A list of scene objects that are influenced by specific animation curves.
		 * * @param[in]		morphShapes		Morph shapes used for per-vertex animation.
		 *
		 * @note	Should be called from the main thread when the caller is sure the animation thread is not using it.
		 */
		void Rebuild(Vector<AnimationClipInfo>& clipInfos, const Vector<AnimatedSceneObject>& sceneObjects, const SPtr<MorphShapes>& morphShapes);

		/**
		 * Updates the proxy data with new information about the clips. Caller must guarantee that clip layout didn't
		 * change since the last call to rebuild().
		 *
		 * @note	Should be called from the main thread when the caller is sure the animation thread is not using it.
		 */
		void UpdateClipInfos(const Vector<AnimationClipInfo>& clipInfos);

		/**
		 * Updates the proxy data with new weights used for morph shapes. Caller must ensure the weights are ordered so
		 * they match with the morph shapes provided to the last rebuild() call.
		 */
		void UpdateMorphChannelWeights(const Vector<float>& weights);

		/**
		 * Updates the proxy data with new scene object transforms. Caller must guarantee that clip layout didn't
		 * change since the last call to rebuild().
		 *
		 * @note	Should be called from the main thread when the caller is sure the animation thread is not using it.
		 */
		void UpdateTransforms(const Vector<AnimatedSceneObject>& sceneObjects);

		/**
		 * Updates the proxy data with new clip times. Caller must guarantee that clip layout didn't change since the last
		 * call to rebuild().
		 *
		 * @note	Should be called from the main thread when the caller is sure the animation thread is not using it.
		 */
		void UpdateTime(const Vector<AnimationClipInfo>& clipInfos);

		/** Destroys all dynamically allocated objects. */
		void Clear();

		u64 Id;

		// Skeletal animation
		AnimationStateLayer* Layers = nullptr;
		u32 NumLayers = 0;
		SPtr<Skeleton> Skeleton;
		SkeletonMask SkeletonMask;
		u32 NumSceneObjects = 0;
		AnimatedSceneObjectInfo* SceneObjectInfos = nullptr;
		Matrix4* SceneObjectTransforms = nullptr;

		// Morph shape animation
		MorphChannelInfo* MorphChannelInfos = nullptr;
		MorphShapeInfo* MorphShapeInfos = nullptr;
		u32 NumMorphChannels = 0;
		u32 NumMorphShapes = 0;
		u32 NumMorphVertices = 0;
		bool MorphChannelWeightsDirty = false;

		// Culling
		AABox MBounds;
		bool MCullEnabled = true;

		// Single frame sample
		AnimSampleStep SampleStep = AnimSampleStep::None;

		// Evaluation results
		LocalSkeletonPose SkeletonPose;
		LocalSkeletonPose SceneObjectPose;
		u32 NumGenericCurves = 0;
		float* GenericCurveOutputs = nullptr;
		bool WasCulled = false;
	};

	/**
	 * Handles animation playback. Takes one or multiple animation clips as input and evaluates them every animation update
	 * tick depending on set properties. The evaluated data is used by the render thread for skeletal animation, by the main
	 * thread for updating attached scene objects and bones (if skeleton is attached), or the data is made available for
	 * manual queries in the case of generic animation.
	 */
	class B3D_CORE_EXPORT Animation : public CoreObject, public IResourceListener
	{
	public:
		~Animation();

		/**
		 * Changes the skeleton which will the translation/rotation/scale animation values manipulate. If no skeleton is set
		 * the animation will only evaluate the generic curves, and the root translation/rotation/scale curves.
		 */
		void SetSkeleton(const SPtr<Skeleton>& skeleton);

		/**
		 * Sets morph shapes that can be used for per-vertex blending animation. After they're set call
		 * setMorphShapeWeight() to apply morph shapes.
		 */
		void SetMorphShapes(const SPtr<MorphShapes>& morphShapes);

		/**
		 * Changes a weight of a single morph channel, determining how much of it to apply on top of the base mesh.
		 *
		 * @param idx		Index of the morph channel to modify. This must match the channels contained in the object
		 *					provided to setMorphShapes().
		 * @param weight	Weight that determines how much of the channel to apply to the mesh, in range [0, 1].
		 */
		void SetMorphChannelWeight(u32 idx, float weight);

		/**
		 * Sets a mask that allows certain bones from the skeleton to be disabled. Caller must ensure that the mask matches
		 * the skeleton assigned to the animation.
		 */
		void SetMask(const SkeletonMask& mask);

		/**
		 * Determines the wrap mode for all active animations. Wrap mode determines what happens when animation reaches the
		 * first or last frame.
		 */
		void SetWrapMode(AnimWrapMode wrapMode);

		/** Determines the speed for all animations. The default value is 1.0f. Use negative values to play-back in reverse. */
		void SetSpeed(float speed);

		/** Determines bounds that will be used for animation culling, if enabled. Bounds must be in world space. */
		void SetBounds(const AABox& bounds);

		/** @copydoc SetBounds */
		const AABox& GetBounds() const { return mBounds; }

		/**
		 * When enabled, animation that is not in a view of any camera will not be evaluated. View determination is done by
		 * checking the bounds provided in setBounds().
		 */
		void SetCulling(bool cull);

		/** @copydoc SetCulling */
		bool GetCulling() const { return mCull; }

		/**
		 * Plays the specified animation clip.
		 *
		 * @param[in]	clip		Clip to play.
		 */
		void Play(const HAnimationClip& clip);

		/**
		 * Plays the specified animation clip on top of the animation currently playing in the main layer. Multiple
		 * such clips can be playing at once, as long as you ensure each is given its own layer. Each animation can
		 * also have a weight that determines how much it influences the main animation.
		 *
		 * @param[in]	clip		Clip to additively blend. Must contain additive animation curves.
		 * @param[in]	weight		Determines how much of an effect will the blended animation have on the final output.
		 *							In range [0, 1].
		 * @param[in]	fadeLength	Applies the blend over a specified time period, increasing the weight as the time
		 *							passes. Set to zero to blend immediately. In seconds.
		 * @param[in]	layer		Layer to play the clip in. Multiple additive clips can be playing at once in separate
		 *							layers and each layer has its own weight.
		 */
		void BlendAdditive(const HAnimationClip& clip, float weight, float fadeLength = 0.0f, u32 layer = 0);

		/**
		 * Blend multiple animation clips between each other using linear interpolation. Unlike normal animations these
		 * animations are not advanced with the progress of time, and is instead expected the user manually changes the
		 * @p t parameter.
		 *
		 * @param[in]	info	Information about the clips to blend. Clip positions must be sorted from lowest to highest.
		 * @param[in]	t		Parameter that controls the blending. Range depends on the positions of the provided
		 *						animation clips.
		 */
		void Blend1D(const Blend1DInfo& info, float t);

		/**
		 * Blend four animation clips between each other using bilinear interpolation. Unlike normal animations these
		 * animations are not advanced with the progress of time, and is instead expected the user manually changes the
		 * @p t parameter.
		 *
		 * @param[in]	info	Information about the clips to blend.
		 * @param[in]	t		Parameter that controls the blending, in range [(0, 0), (1, 1)]. t = (0, 0) means top left
		 *						animation has full influence, t = (1, 0) means top right animation has full influence,
		 *						t = (0, 1) means bottom left animation has full influence, t = (1, 1) means bottom right
		 *						animation has full influence.
		 */
		void Blend2D(const Blend2DInfo& info, const Vector2& t);

		/**
		 * Fades the specified animation clip in, while fading other playing animation out, over the specified time
		 * period.
		 *
		 * @param[in]	clip		Clip to fade in.
		 * @param[in]	fadeLength	Determines the time period over which the fade occurs. In seconds.
		 */
		void CrossFade(const HAnimationClip& clip, float fadeLength);

		/**
		 * Samples an animation clip at the specified time, displaying only that particular frame without further playback.
		 *
		 * @param[in] clip	Animation clip to sample.
		 * @param[in] time	Time to sample the clip at.
		 */
		void Sample(const HAnimationClip& clip, float time);

		/**
		 * Stops playing all animations on the provided layer. Specify -1 to stop animation on the main layer
		 * (non-additive animations).
		 */
		void Stop(u32 layer);

		/** Stops playing all animations. */
		void StopAll();

		/** Checks if any animation clips are currently playing. */
		bool IsPlaying() const;

		/** Returns the total number of animation clips influencing this animation. */
		u32 GetNumClips() const;

		/**
		 * Returns one of the animation clips influencing this animation.
		 *
		 * @param[in]	idx		Sequential index of the animation clip to retrieve. In range [0, getNumClips()].
		 * @return				Animation clip at the specified index, or null if the index is out of range.
		 */
		HAnimationClip GetClip(u32 idx) const;

		/**
		 * Retrieves detailed information about a currently playing animation clip.
		 *
		 * @param[in]	clip	Clip to retrieve the information for.
		 * @param[out]	state	Animation clip state containing the requested information. Only valid if the method returns
		 *						true.
		 * @return				True if the state was found (animation clip is playing), false otherwise.
		 */
		bool GetState(const HAnimationClip& clip, AnimationClipState& state);

		/**
		 * Changes the state of a playing animation clip. If animation clip is not currently playing the playback is started
		 * for the clip.
		 *
		 * @param[in]	clip	Clip to change the state for.
		 * @param[in]	state	New state of the animation (e.g. changing the time for seeking).
		 */
		void SetState(const HAnimationClip& clip, AnimationClipState state);

		/**
		 * Ensures that any position/rotation/scale animation of a specific animation curve is transfered to the
		 * the provided scene object. Also allow the opposite operation which can allow scene object transform changes
		 * to manipulate object bones.
		 *
		 * @param[in]	curve	Name of the curve (bone) to connect the scene object with. Use empty string to map to the
		 *						root bone, regardless of the bone name.
		 * @param[in]	so		Scene object to influence by the curve modifications, and vice versa.
		 */
		void MapCurveToSceneObject(const String& curve, const HSceneObject& so);

		/** Removes the curve <-> scene object mapping that was set via mapCurveToSceneObject(). */
		void UnmapSceneObject(const HSceneObject& so);

		/**
		 * Retrieves an evaluated value for a generic curve with the specified index.
		 *
		 * @param[in]	curveIdx	The curve index referencing a set of curves from the first playing animation clip.
		 *							Generic curves from all other clips are ignored.
		 * @param[out]	value		Value of the generic curve. Only valid if the method return true.
		 * @return					True if the value was retrieved successfully. The method might fail if animation update
		 *							didn't yet have a chance to execute and values are not yet available, or if the
		 *							animation clip changed since the last frame (the last problem can be avoided by ensuring
		 *							to read the curve values before changing the clip).
		 */
		bool GetGenericCurveValue(u32 curveIdx, float& value);

		/** Creates a new empty Animation object. */
		static SPtr<Animation> Create();

		/** Triggered whenever an animation event is reached. */
		Event<void(const HAnimationClip&, const String&)> OnEventTriggered;

		/** @name Internal
		 *  @{
		 */

		/** Returns the unique ID for this animation object. */
		u64 GetIdInternal() const { return mId; }

		/** Checks if any currently set animation clips perform animation of the root bone. */
		bool GetAnimatesRootInternal() const;

		/** @} */
	private:
		friend class AnimationManager;

		Animation();

		/**
		 * Triggers any events between the last frame and current one.
		 *
		 * @param[in]	delta			Time elapsed since the last call to this method.
		 */
		void TriggerEvents(float delta);

		/**
		 * Updates the animation proxy object based on the currently set skeleton, playing clips and dirty flags.
		 *
		 * @param[in]	timeDelta	Seconds passed since the last call to this method.
		 */
		void UpdateAnimProxy(float timeDelta);

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
		AnimationClipInfo* AddClip(const HAnimationClip& clip, u32 layer, bool stopExisting = true);

		void GetListenerResources(Vector<HResource>& resources) override;
		void NotifyResourceLoaded(const HResource& resource) override;
		void NotifyResourceChanged(const HResource& resource) override;

		u64 mId;
		AnimWrapMode mDefaultWrapMode = AnimWrapMode::Loop;
		float mDefaultSpeed = 1.0f;
		AABox mBounds;
		bool mCull = true;
		AnimDirtyState mDirty = AnimDirtyStateFlag::All;

		SPtr<Skeleton> mSkeleton;
		SkeletonMask mSkeletonMask;
		SPtr<MorphShapes> mMorphShapes;
		Vector<float> mMorphChannelWeights;
		Vector<AnimationClipInfo> mClipInfos;
		UnorderedMap<u64, AnimatedSceneObject> mSceneObjects;
		Vector<float> mGenericCurveOutputs;
		bool mGenericCurveValuesValid = false;
		AnimSampleStep mSampleStep = AnimSampleStep::None;

		// Animation thread only
		SPtr<AnimationProxy> mAnimProxy;
	};

	/** @} */
} // namespace bs
