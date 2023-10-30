//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Utility/BsModule.h"
#include "CoreObject/BsRenderThread.h"
#include "Math/BsConvexVolume.h"
#include "RenderAPI/BsVertexDescription.h"
#include "Threading/BsWaitGroup.h"

namespace bs
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
			u64 AnimId;
			u32 StartIdx;
			u32 NumBones;
		};

		/** Contains data about a calculated morph shape. */
		struct MorphShapeInfo
		{
			SPtr<MeshData> MeshData;
			u32 Version;
		};

		/** Contains meta-data about where calculated animation data is stored. */
		struct AnimInfo
		{
			PoseInfo PoseInfo;
			MorphShapeInfo MorphShapeInfo;
		};

		/**
		 * Maps animation ID to a animation information structure, which points to relevant skeletal or morph shape data.
		 */
		UnorderedMap<u64, AnimInfo> Infos;

		/** Global joint transforms for all skeletons in the scene. */
		Vector<Matrix4> Transforms;

		/** True if the animation is being evaluated asynchronously along with rendering (delayed one frame). */
		bool Async = false;
	};

	/**
	 * Keeps track of all active animations, queues animation thread tasks and synchronizes data between main, render
	 * and animation threads.
	 */
	class B3D_CORE_EXPORT AnimationManager : public Module<AnimationManager>
	{
	public:
		AnimationManager();

		/** Pauses or resumes the animation evaluation. */
		void SetPaused(bool paused);

		/**
		 * Determines how often to evaluate animations. If rendering is not running at adequate framerate the animation
		 * could end up being evaluated less times than specified here.
		 *
		 * @param[in]	fps		Number of frames per second to evaluate the animation. Default is 60.
		 */
		void SetUpdateRate(u32 fps);

		/**
		 * Evaluates animations for all animated objects, and returns the evaluated skeleton bone poses and morph shape
		 * meshes that can be passed along to the renderer.
		 *
		 * @param[in]		async		If true the method returns immediately while the animation gets evaluated in the
		 *								background. The returned evaluated data will be the data from the previous frame.
		 *								Therefore note that this introduces a one frame latency on the animation. If the
		 *								latency is not acceptable set this to false, at a potential performance impact.
		 * @return						Evaluated animation data for this frame (if @p async is false), or the previous
		 *								frame (if @p async is true). Note that the system re-uses the returned buffers,
		 *								and the returned buffer should stop being used after every second call to update().
		 *								This is enough to have one buffer be processed by the render thread, one queued
		 *								for future rendering and one that's being written to.
		 */
		const EvaluatedAnimationData* Update(bool async = true);

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
		u64 RegisterAnimation(Animation* anim);

		/** Unregisters an animation with the specified ID. Must be called before an Animation is destroyed. */
		void UnregisterAnimation(u64 id);

		/**
		 * Evaluates animation for a single object and writes the result in the currently active write buffer.
		 *
		 * @param[in]	anim		Proxy representing the animation to evaluate.
		 * @param[in]	boneIdx		Index in the output buffer in which to write evaluated bone information. This will be
		 *							automatically advanced by the number of written bone transforms.
		 */
		void EvaluateAnimation(AnimationProxy* anim, u32& boneIdx);

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

		u32 mPoseReadBufferIdx = 2;
		u32 mPoseWriteBufferIdx = 0;

		Mutex mMutex;
		WaitGroup mWorkerWaitGroup;

		bool mSwapBuffers = false;
	};

	/** Provides easier access to AnimationManager. */
	B3D_CORE_EXPORT AnimationManager& GetAnimationManager();

	/** @} */
} // namespace bs
