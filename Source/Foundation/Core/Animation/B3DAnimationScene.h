//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Utility/B3DModule.h"
#include "CoreObject/B3DRenderThread.h"
#include "Math/B3DConvexVolume.h"
#include "RenderAPI/B3DVertexDescription.h"
#include "Threading/B3DWaitGroup.h"
#include "Math/B3DMatrix4.h"

namespace b3d
{
	struct AnimationProxy;

	/** @addtogroup Animation-Internal
	 *  @{
	 */

	/** Contains skeleton poses for all animations evaluated on a single frame. */
	struct EvaluatedAnimationData
	{
		/** Contains meta-data about a calculated skeleton pose. Actual data maps to the @p transforms buffer. */
		struct PoseInfo
		{
			u64 AnimationId;
			u32 BoneStartIndex;
			u32 BoneCount;
		};

		/** Contains data about a calculated morph shape. */
		struct MorphShapeInfo
		{
			SPtr<MeshData> MeshData;
			u32 Version;
		};

		/** Contains meta-data about where calculated animation data is stored. */
		struct AnimationInfo
		{
			PoseInfo PoseInfo;
			MorphShapeInfo MorphShapeInfo;
		};

		/**
		 * Maps animation ID to a animation information structure, which points to relevant skeletal or morph shape data.
		 */
		UnorderedMap<u64, AnimationInfo> Infos;

		/** Global joint transforms for all skeletons in the scene. */
		Vector<Matrix4> Transforms;

		/** True if the animation is being evaluated asynchronously along with rendering (delayed one frame). */
		bool AsynchronousEvaluation = false;
	};

	/**
	 * Keeps track of all active animations in a single scene instance. Queues animation thread tasks and synchronizes data
	 * between main, render and animation threads.
	 */
	class B3D_EXPORT AnimationScene
	{
	public:
		AnimationScene();

		/** Pauses or resumes the animation evaluation. */
		void SetPaused(bool paused);

		/**
		 * Determines how often to evaluate animations. If rendering is not running at adequate framerate the animation
		 * could end up being evaluated less times than specified here.
		 *
		 * @param	fps		Number of frames per second to evaluate the animation. Default is 60.
		 */
		void SetUpdateRate(u32 fps);

		/**
		 * Evaluates animations for all animated objects, and returns the evaluated skeleton bone poses and morph shape
		 * meshes that can be passed along to the renderer.
		 *
		 * @param	async	If true the method returns immediately while the animation gets evaluated in the
		 *					background. The returned evaluated data will be the data from the previous frame.
		 *					Therefore note that this introduces a one frame latency on the animation. If the
		 *					latency is not acceptable set this to false, at a potential performance impact.
		 * @return			Evaluated animation data for this frame (if @p async is false), or the previous
		 *					frame (if @p async is true). Note that the system re-uses the returned buffers,
		 *					and the returned buffer should stop being used after every second call to Update().
		 *					This is enough to have one buffer be processed by the render thread, one queued
		 *					for future rendering and one that's being written to.
		 */
		const EvaluatedAnimationData* Update(bool async = true);

		/** Creates a new empty animation scene. */
		static SPtr<AnimationScene> Create() { return B3DMakeShared<AnimationScene>(); }

		/**
		 * @name Internal
		 * @{
		 */

		/** Scene instance that owns this animation scene. */
		void SetOwner(const SPtr<SceneInstance>& scene) { mOwner = scene; }

		/** @} */

	private:
		friend class Animation;

		/** Possible states the worker thread can be in, used for synchronization. */
		enum class WorkerState
		{
			Inactive,
			Started,
			DataReady
		};

		/**
		 * Registers a new animation and returns a unique ID for it. Must be called whenever an Animation is constructed.
		 */
		u64 RegisterAnimation(Animation* animation);

		/** Unregisters an animation with the specified ID. Must be called before an Animation is destroyed. */
		void UnregisterAnimation(u64 id);

		/**
		 * Evaluates animation for a single object and writes the result in the currently active write buffer.
		 *
		 * @param	anim			Proxy representing the animation to evaluate.
		 * @param	outBoneIndex	Index in the output buffer in which to write evaluated bone information. This will be
		 *							automatically advanced by the number of written bone transforms.
		 */
		void EvaluateAnimation(AnimationProxy* anim, u32& outBoneIndex);

		u64 mNextId = 1;
		UnorderedMap<u64, Animation*> mAnimations;

		float mUpdateRate = 1.0f / 60.0f;
		float mAnimationTime = 0.0f;
		float mLastAnimationUpdateTime = 0.0f;
		float mNextAnimationUpdateTime = 0.0f;
		float mLastAnimationDeltaTime = 0.0f;
		bool mPaused = false;

		SPtr<VertexDescription> mBlendShapeVertexDescription;

		// Animation thread
		Vector<SPtr<AnimationProxy>> mProxies;
		Vector<ConvexVolume> mCullFrustums;
		EvaluatedAnimationData mAnimData[RenderThread::kSyncBufferCount + 1];

		u32 mPoseReadBufferIndex = 2;
		u32 mPoseWriteBufferIndex = 0;

		Mutex mMutex;
		WaitGroup mWorkerWaitGroup;

		bool mSwapBuffers = false;
		WeakSPtr<SceneInstance> mOwner;
	};

	/** @} */
} // namespace b3d
