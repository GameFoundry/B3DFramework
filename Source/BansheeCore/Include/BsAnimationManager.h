//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsModule.h"
#include "BsCoreThread.h"
#include "BsConvexVolume.h"

namespace BansheeEngine
{
	struct AnimationProxy;

	/** @addtogroup Animation-Internal
	 *  @{
	 */
	
	/** Contains skeleton poses for all animations evaluated on a single frame. */
	struct RendererAnimationData
	{
		/** Contains data about a calculated skeleton pose. */
		struct PoseInfo
		{
			UINT64 animId;
			UINT32 startIdx;
			UINT32 numBones;
		};

		/** Maps animation ID to a pose information structure, containing its global joint transforms. */
		UnorderedMap<UINT64, PoseInfo> poseInfos;

		/** Global joint transforms for all skeletons in the scene. */
		Vector<Matrix4> transforms;
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
		 * Synchronizes animation data from the animation thread with the scene objects. Should be called before component
		 * updates are sent. 
		 */
		void preUpdate();

		/**
		 * Synchronizes animation data to the animation thread, advances animation time and queues new animation evaluation
		 * task.
		 */
		void postUpdate(const Vector<ConvexVolume>& cullFrustums);

		/** 
		 * Gets skeleton poses required by the renderer to display all the animations. This will block the animation thread
		 * if it has not yet finished, and it will also advance the read buffer index, meaning this shouldn't be called more
		 * than once per frame. The returned data can be referenced, and is guaranteed to be valid for a single core-thread
		 * frame.
		 *
		 * @note	Core thread only.
		 */
		const RendererAnimationData& getRendererData();

	private:
		friend class Animation;

		/** 
		 * Registers a new animation and returns a unique ID for it. Must be called whenever an Animation is constructed. 
		 */
		UINT64 registerAnimation(Animation* anim);

		/** Unregisters an animation with the specified ID. Must be called before an Animation is destroyed. */
		void unregisterAnimation(UINT64 id);

		/** Worker method ran on the animation thread that evaluates all animation at the provided time. */
		void evaluateAnimation();

		UINT64 mNextId;
		UnorderedMap<UINT64, Animation*> mAnimations;
		
		float mUpdateRate;
		float mAnimationTime;
		float mLastAnimationUpdateTime;
		float mNextAnimationUpdateTime;
		bool mPaused;

		bool mWorkerRunning;
		SPtr<Task> mAnimationWorker;

		// Animation thread
		Vector<SPtr<AnimationProxy>> mProxies;
		Vector<ConvexVolume> mCullFrustums;
		RendererAnimationData mAnimData[CoreThread::NUM_SYNC_BUFFERS];

		UINT32 mPoseReadBufferIdx;
		UINT32 mPoseWriteBufferIdx;
		std::atomic<INT32> mDataReadyCount;
	};

	/** Provides easier access to AnimationManager. */
	BS_CORE_EXPORT AnimationManager& gAnimation();

	/** @} */
}