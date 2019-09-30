//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Utility/BsModule.h"
#include "CoreThread/BsCoreThread.h"
#include "Math/BsConvexVolume.h"
#include "RenderAPI/BsVertexDataDesc.h"

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
			UINT64 animId;
			UINT32 startIdx;
			UINT32 numBones;
		};

		/** Contains data about a calculated morph shape. */
		struct MorphShapeInfo
		{
			SPtr<MeshData> meshData;
			UINT32 version;
		};

		/** Contains meta-data about where calculated animation data is stored. */
		struct AnimInfo
		{
			PoseInfo poseInfo;
			MorphShapeInfo morphShapeInfo;
		};

		/**
		 * Maps animation ID to a animation information structure, which points to relevant skeletal or morph shape data.
		 */
		UnorderedMap<UINT64, AnimInfo> infos;

		/** Global joint transforms for all skeletons in the scene. */
		Vector<Matrix4> transforms;

		/** True if the animation is being evaluated asynchronously along with rendering (delayed one frame). */
		bool async = false;
	};

	/**
	 * Keeps track of all active animations, queues animation thread tasks and synchronizes data between simulation, core
	 * and animation threads.
	 */
	class BS_CORE_EXPORT AnimationManager : public Module<AnimationManager>
	{
	public:
		AnimationManager();

		/** Pauses or resumes the animation evaluation. */
		void setPaused(bool paused);

		/**
		 * Determines how often to evaluate animations. If rendering is not running at adequate framerate the animation
		 * could end up being evaluated less times than specified here.
		 *
		 * @param[in]	fps		Number of frames per second to evaluate the animation. Default is 60.
		 */
		void setUpdateRate(UINT32 fps);

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
		 *								This is enough to have one buffer be processed by the core thread, one queued
		 *								for future rendering and one that's being written to.
		 */
		const EvaluatedAnimationData* update(bool async = true);

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
		UINT64 registerAnimation(Animation* anim);

		/** Unregisters an animation with the specified ID. Must be called before an Animation is destroyed. */
		void unregisterAnimation(UINT64 id);

		/**
		 * Evaluates animation for a single object and writes the result in the currently active write buffer.
		 *
		 * @param[in]	anim		Proxy representing the animation to evaluate.
		 * @param[in]	boneIdx		Index in the output buffer in which to write evaluated bone information. This will be
		 *							automatically advanced by the number of written bone transforms.
		 */
		void evaluateAnimation(AnimationProxy* anim, UINT32& boneIdx);

		UINT64 mNextId = 1;
		UnorderedMap<UINT64, Animation*> mAnimations;
		
		float mUpdateRate = 1.0f / 60.0f;
		float mAnimationTime = 0.0f;
		float mLastAnimationUpdateTime = 0.0f;
		float mNextAnimationUpdateTime = 0.0f;
		float mLastAnimationDeltaTime = 0.0f;
		bool mPaused = false;

		SPtr<VertexDataDesc> mBlendShapeVertexDesc;

		// Animation thread
		Vector<SPtr<AnimationProxy>> mProxies;
		Vector<ConvexVolume> mCullFrustums;
		EvaluatedAnimationData mAnimData[CoreThread::NUM_SYNC_BUFFERS + 1];

		UINT32 mPoseReadBufferIdx = 2;
		UINT32 mPoseWriteBufferIdx = 0;
		
		Signal mWorkerDoneSignal;
		Mutex mMutex;

		UINT32 mNumActiveWorkers = 0;
		bool mSwapBuffers = false;
	};

	/** Provides easier access to AnimationManager. */
	BS_CORE_EXPORT AnimationManager& gAnimation();

	/** @} */
}
