//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
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
#include "Scene/BsSceneActor.h"

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
		HAnimationClip BottomLeftClip;
		HAnimationClip BottomRightClip;
	};

	/** Contains a mapping between a scene object and an animation curve it is animated with. */
	struct AnimatedSceneObject
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
		SPtr<MorphShape> Shape;
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
		 * @param	skeleton		New skeleton to assign to the proxy.
		 * @param	mask			Mask that filters which skeleton bones are enabled or disabled.
		 * @param	inOutClipInfos	Potentially new clip infos that will be used for rebuilding the proxy. Once the
		 *							method completes clip info layout and state indices will be populated for
		 *							further use in the Update*() methods.
		 * @param	sceneObjects	A list of scene objects that are influenced by specific animation curves.
		 * @param	morphShapes		Morph shapes used for per-vertex animation.
		 *
		 * @note	Should be called from the main thread when the caller is sure the animation thread is not using it.
		 */
		void RebuildFull(const SPtr<Skeleton>& skeleton, const SkeletonMask& mask, Vector<AnimationClipInfo>& inOutClipInfos, const Vector<AnimatedSceneObject>& sceneObjects, const SPtr<MorphShapes>& morphShapes);

		/**
		 * Rebuilds the internal proxy data according to the newly clips. This should be called whenever clips are added
		 * or removed, or clip layout indices change.
		 *
		 * @param	inOutClipInfos	New clip infos that will be used for rebuilding the proxy. Once the method
		 *							completes clip info layout and state indices will be populated for further use
		 *							in the Update*() methods.
		 * @param	sceneObjects	A list of scene objects that are influenced by specific animation curves.
		 * @param	morphShapes		Morph shapes used for per-vertex animation.
		 *
		 * @note	Should be called from the main thread when the caller is sure the animation thread is not using it.
		 */
		void RebuildClips(Vector<AnimationClipInfo>& inOutClipInfos, const Vector<AnimatedSceneObject>& sceneObjects, const SPtr<MorphShapes>& morphShapes);

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
		void UpdateTransforms(const Vector<AnimatedSceneObject>& sceneObjects);

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
		SPtr<Skeleton> Skeleton;
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

	class B3D_CORE_EXPORT Animation : public CoreObject, public IResourceListener, public SceneActor
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

		void SetMorphChannelWeight(u32 index, float weight);

		/**
		 * Sets a mask that allows certain bones from the skeleton to be disabled. Caller must ensure that the mask matches
		 * the skeleton assigned to the animation.
		 */
		void SetMask(const SkeletonMask& mask);

		void SetWrapMode(AnimationWrapMode wrapMode);
		void SetSpeed(float speed);

		void SetBounds(const AABox& bounds);
		const AABox& GetBounds() const { return mBounds; }

		void SetCulling(bool cull);
		bool GetCulling() const { return mCull; }

		void Play(const HAnimationClip& clip);
		void BlendAdditive(const HAnimationClip& clip, float weight, float fadeLength = 0.0f, u32 layer = 0);
		void Blend1D(const Blend1DInfo& info, float alpha);
		void Blend2D(const Blend2DInfo& info, const Vector2& alpha);
		void CrossFade(const HAnimationClip& clip, float fadeLength);
		void Sample(const HAnimationClip& clip, float time);
		void Stop(u32 layer);
		void StopAll();
		bool IsPlaying() const;

		u32 GetClipCount() const;
		HAnimationClip GetClip(u32 index) const;

		bool GetState(const HAnimationClip& clip, AnimationClipState& state);
		void SetState(const HAnimationClip& clip, AnimationClipState state);

		void MapCurveToSceneObject(const String& curve, const HSceneObject& sceneObject);
		void UnmapSceneObject(const HSceneObject& sceneObject);

		bool GetGenericCurveValue(u32 curveIndex, float& outValue);

		/** Creates a new empty Animation object. */
		static SPtr<Animation> Create(const SPtr<SceneInstance>& scene);

		/** Triggered whenever an animation event is reached. */
		Event<void(const HAnimationClip&, const String&)> OnEventTriggered;

		/** @name Internal
		 *  @{
		 */

		/** Returns the unique ID for this animation object. */
		u64 GetAnimationId() const { return mAnimationId; }

		/** Checks if any currently set animation clips perform animation of the root bone. */
		bool GetAnimatesRoot() const;

		/** @} */
	private:
		friend class AnimationScene;

		Animation(const SPtr<SceneInstance>& scene);

		/**
		 * Triggers any events between the last frame and current one.
		 *
		 * @param	delta			Time elapsed since the last call to this method.
		 */
		void TriggerEvents(float delta);

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

		u64 mAnimationId;
		AnimationWrapMode mDefaultWrapMode = AnimationWrapMode::Loop;
		float mDefaultSpeed = 1.0f;
		AABox mBounds;
		bool mCull = true;
		AnimDirtyState mDirty = AnimationDirtyStateFlag::All;

		SPtr<Skeleton> mSkeleton;
		SkeletonMask mSkeletonMask;
		SPtr<MorphShapes> mMorphShapes;
		Vector<float> mMorphChannelWeights;
		Vector<AnimationClipInfo> mClipInfos;
		UnorderedMap<UUID, AnimatedSceneObject> mAnimatedSceneObjects;
		Vector<float> mGenericCurveOutputs;
		bool mGenericCurveValuesValid = false;
		AnimationSampleStep mSampleStep = AnimationSampleStep::None;

		// Animation thread only
		SPtr<AnimationProxy> mAnimationProxy;
	};

	/** @} */
} // namespace b3d
