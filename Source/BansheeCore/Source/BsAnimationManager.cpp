//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsAnimationManager.h"
#include "BsAnimation.h"
#include "BsAnimationClip.h"
#include "BsTaskScheduler.h"
#include "BsTime.h"
#include "BsSceneManager.h"
#include "BsCamera.h"
#include "BsMorphShapes.h"
#include "BsMeshData.h"
#include "BsMeshUtility.h"

namespace bs
{
	AnimationManager::AnimationManager()
		: mNextId(1), mUpdateRate(1.0f / 60.0f), mAnimationTime(0.0f), mLastAnimationUpdateTime(0.0f)
		, mNextAnimationUpdateTime(0.0f), mPaused(false), mWorkerStarted(false), mPoseReadBufferIdx(1)
		, mPoseWriteBufferIdx(0), mDataReady(false)
	{
		mAnimationWorker = Task::create("Animation", std::bind(&AnimationManager::evaluateAnimation, this));

		mDataReadyCount.store(0, std::memory_order_release);
		mWorkerState.store(WorkerState::Inactive, std::memory_order_release);

		mBlendShapeVertexDesc = VertexDataDesc::create();
		mBlendShapeVertexDesc->addVertElem(VET_FLOAT3, VES_POSITION, 1, 1);
		mBlendShapeVertexDesc->addVertElem(VET_UBYTE4_NORM, VES_NORMAL, 1, 1);
	}

	void AnimationManager::setPaused(bool paused)
	{
		mPaused = paused;
	}

	void AnimationManager::setUpdateRate(UINT32 fps)
	{
		if (fps == 0)
			fps = 1;

		mUpdateRate = 1.0f / fps;
	}

	void AnimationManager::preUpdate()
	{
		if (mPaused || !mWorkerStarted)
			return;

		mAnimationWorker->wait();
		
		WorkerState state = mWorkerState.load(std::memory_order_acquire);
		assert(state == WorkerState::DataReady);

		// Trigger events
		for (auto& anim : mAnimations)
		{
			anim.second->updateFromProxy();
			anim.second->triggerEvents(mAnimationTime, gTime().getFrameDelta());
		}
	}

	void AnimationManager::postUpdate()
	{
		if (mPaused)
			return;

		mAnimationTime += gTime().getFrameDelta();
		if (mAnimationTime < mNextAnimationUpdateTime)
			return;

		mNextAnimationUpdateTime = Math::floor(mAnimationTime / mUpdateRate) * mUpdateRate + mUpdateRate;

		float timeDelta = mAnimationTime - mLastAnimationUpdateTime;
		mLastAnimationUpdateTime = mAnimationTime;

		// Update poses in the currently active buffer. Multi-buffering allows the core thread to safely read the
		// poses without worrying about them being overwritten by another call to postUpdate, as long as the simulation
		// thread doesn't go more than (CoreThread::NUM_SYNC_BUFFERS - 1) frames ahead.
		mProxies.clear();
		for (auto& anim : mAnimations)
		{
			anim.second->updateAnimProxy(timeDelta);
			mProxies.push_back(anim.second->mAnimProxy);
		}

		mCullFrustums.clear();

		auto& allCameras = gSceneManager().getAllCameras();
		for(auto& entry : allCameras)
		{
			bool isOverlayCamera = entry.second.camera->getRenderSettings()->overlayOnly;
			if (isOverlayCamera)
				continue;

			// TODO: Not checking if camera and animation renderable's layers match. If we checked more animations could
			// be culled.
			mCullFrustums.push_back(entry.second.camera->getWorldFrustum());
		}

		// Make sure thread finishes writing all changes to the anim proxies as they will be read by the animation thread
		mWorkerStarted = true;
		mWorkerState.store(WorkerState::Started, std::memory_order_release);

		// Note: Animation thread will trigger about the same time as the core thread. The core thread will need to wait
		// until animation thread finishes, which might end up blocking it (and losing the multi-threading performance). 
		// Consider delaying displayed animation for a single frame or pre-calculating animations (by advancing time the
		// previous frame) for non-dirty animations.
		TaskScheduler::instance().addTask(mAnimationWorker);
	}

	void AnimationManager::evaluateAnimation()
	{
		// Make sure we don't load obsolete anim proxy data written by the simulation thread
		WorkerState state = mWorkerState.load(std::memory_order_acquire);
		assert(state == WorkerState::Started);

		// No need for locking, as we are sure that only postUpdate() writes to the proxy buffer, and increments the write
		// buffer index. And it's called sequentially ensuring previous call to evaluate finishes.

		UINT32 totalNumBones = 0;
		for (auto& anim : mProxies)
		{
			if (anim->skeleton != nullptr)
				totalNumBones += anim->skeleton->getNumBones();
		}

		RendererAnimationData& renderData = mAnimData[mPoseWriteBufferIdx];
		
		UINT32 prevPoseBufferIdx = (mPoseWriteBufferIdx + CoreThread::NUM_SYNC_BUFFERS - 1) % CoreThread::NUM_SYNC_BUFFERS;
		RendererAnimationData& prevRenderData = mAnimData[prevPoseBufferIdx];
		
		mPoseWriteBufferIdx = (mPoseWriteBufferIdx + 1) % CoreThread::NUM_SYNC_BUFFERS;

		renderData.transforms.resize(totalNumBones);
		renderData.infos.clear();

		UINT32 curBoneIdx = 0;
		for(auto& anim : mProxies)
		{
			if(anim->mCullEnabled)
			{
				bool isVisible = false;
				for(auto& frustum : mCullFrustums)
				{
					if(frustum.intersects(anim->mBounds))
					{
						isVisible = true;
						break;
					}
				}

				if (!isVisible)
					continue;
			}

			RendererAnimationData::AnimInfo animInfo;
			bool hasAnimInfo = false;

			// Evaluate skeletal animation
			if (anim->skeleton != nullptr)
			{
				UINT32 numBones = anim->skeleton->getNumBones();

				RendererAnimationData::PoseInfo& poseInfo = animInfo.poseInfo;
				poseInfo.animId = anim->id;
				poseInfo.startIdx = curBoneIdx;
				poseInfo.numBones = numBones;

				memset(anim->skeletonPose.hasOverride, 0, sizeof(bool) * anim->skeletonPose.numBones);
				Matrix4* boneDst = renderData.transforms.data() + curBoneIdx;

				// Copy transforms from mapped scene objects
				UINT32 boneTfrmIdx = 0;
				for(UINT32 i = 0; i < anim->numSceneObjects; i++)
				{
					const AnimatedSceneObjectInfo& soInfo = anim->sceneObjectInfos[i];

					if (soInfo.boneIdx == -1)
						continue;

					boneDst[soInfo.boneIdx] = anim->sceneObjectTransforms[boneTfrmIdx];
					anim->skeletonPose.hasOverride[soInfo.boneIdx] = true;
					boneTfrmIdx++;
				}

				// Animate bones
				anim->skeleton->getPose(boneDst, anim->skeletonPose, anim->skeletonMask, anim->layers, anim->numLayers);

				curBoneIdx += numBones;
				hasAnimInfo = true;
			}
			else
			{
				RendererAnimationData::PoseInfo& poseInfo = animInfo.poseInfo;
				poseInfo.animId = anim->id;
				poseInfo.startIdx = 0;
				poseInfo.numBones = 0;
			}

			// Reset mapped SO transform
			for (UINT32 i = 0; i < anim->sceneObjectPose.numBones; i++)
			{
				anim->sceneObjectPose.positions[i] = Vector3::ZERO;
				anim->sceneObjectPose.rotations[i] = Quaternion::IDENTITY;
				anim->sceneObjectPose.scales[i] = Vector3::ONE;
			}

			// Update mapped scene objects
			memset(anim->sceneObjectPose.hasOverride, 1, sizeof(bool) * anim->numSceneObjects);

			// Update scene object transforms
			for(UINT32 i = 0; i < anim->numSceneObjects; i++)
			{
				const AnimatedSceneObjectInfo& soInfo = anim->sceneObjectInfos[i];

				// We already evaluated bones
				if (soInfo.boneIdx != -1)
					continue;

				if (soInfo.layerIdx == (UINT32)-1 || soInfo.stateIdx == (UINT32)-1)
					continue;

				const AnimationState& state = anim->layers[soInfo.layerIdx].states[soInfo.stateIdx];
				if (state.disabled)
					continue;

				{
					UINT32 curveIdx = soInfo.curveIndices.position;
					if (curveIdx != (UINT32)-1)
					{
						const TAnimationCurve<Vector3>& curve = state.curves->position[curveIdx].curve;
						anim->sceneObjectPose.positions[curveIdx] = curve.evaluate(state.time, state.positionCaches[curveIdx], state.loop);
						anim->sceneObjectPose.hasOverride[curveIdx] = false;
					}
				}

				{
					UINT32 curveIdx = soInfo.curveIndices.rotation;
					if (curveIdx != (UINT32)-1)
					{
						const TAnimationCurve<Quaternion>& curve = state.curves->rotation[curveIdx].curve;
						anim->sceneObjectPose.rotations[curveIdx] = curve.evaluate(state.time, state.rotationCaches[curveIdx], state.loop);
						anim->sceneObjectPose.rotations[curveIdx].normalize();
						anim->sceneObjectPose.hasOverride[curveIdx] = false;
					}
				}

				{
					UINT32 curveIdx = soInfo.curveIndices.scale;
					if (curveIdx != (UINT32)-1)
					{
						const TAnimationCurve<Vector3>& curve = state.curves->scale[curveIdx].curve;
						anim->sceneObjectPose.scales[curveIdx] = curve.evaluate(state.time, state.scaleCaches[curveIdx], state.loop);
						anim->sceneObjectPose.hasOverride[curveIdx] = false;
					}
				}
			}

			// Update generic curves
			// Note: No blending for generic animations, just use first animation
			if (anim->numLayers > 0 && anim->layers[0].numStates > 0)
			{
				const AnimationState& state = anim->layers[0].states[0];
				if (!state.disabled)
				{
					UINT32 numCurves = (UINT32)state.curves->generic.size();
					for (UINT32 i = 0; i < numCurves; i++)
					{
						const TAnimationCurve<float>& curve = state.curves->generic[i].curve;
						anim->genericCurveOutputs[i] = curve.evaluate(state.time, state.genericCaches[i], state.loop);
					}
				}
			}

			// Update morph shapes
			if(anim->numMorphShapes > 0)
			{
				auto iterFind = prevRenderData.infos.find(anim->id);
				if (iterFind != prevRenderData.infos.end())
					animInfo.morphShapeInfo = iterFind->second.morphShapeInfo;
				else
					animInfo.morphShapeInfo.version = 1; // 0 is considered invalid version

				// Recalculate weights if curves are present
				bool hasMorphCurves = false;
				for(UINT32 i = 0; i < anim->numMorphChannels; i++)
				{
					MorphChannelInfo& channelInfo = anim->morphChannelInfos[i];
					if(channelInfo.weightCurveIdx != (UINT32)-1)
					{
						channelInfo.weight = Math::clamp01(anim->genericCurveOutputs[channelInfo.weightCurveIdx]);
						hasMorphCurves = true;
					}

					float frameWeight;
					if (channelInfo.frameCurveIdx != (UINT32)-1)
					{
						frameWeight = Math::clamp01(anim->genericCurveOutputs[channelInfo.frameCurveIdx]);
						hasMorphCurves = true;
					}
					else
						frameWeight = 0.0f;

					if(channelInfo.shapeCount == 1)
					{
						MorphShapeInfo& shapeInfo = anim->morphShapeInfos[channelInfo.shapeStart];

						// Blend between base shape and the only available frame
						float relative = frameWeight - shapeInfo.frameWeight;
						if (relative <= 0.0f)
						{
							float diff = shapeInfo.frameWeight;
							if (diff > 0.0f)
							{
								float t = -relative / diff;
								shapeInfo.finalWeight = 1.0f - std::min(t, 1.0f);
							}
							else
								shapeInfo.finalWeight = 1.0f;
						}
						else // If past the final frame we clamp
							shapeInfo.finalWeight = 1.0f;
					}
					else if(channelInfo.shapeCount > 1)
					{
						for(UINT32 j = 0; j < channelInfo.shapeCount - 1; j++)
						{
							float prevShapeWeight;
							if (j > 0)
								prevShapeWeight = anim->morphShapeInfos[j - 1].frameWeight;
							else
								prevShapeWeight = 0.0f; // Base shape, blend between it and the first frame

							float nextShapeWeight = anim->morphShapeInfos[j + 1].frameWeight;
							MorphShapeInfo& shapeInfo = anim->morphShapeInfos[j];

							float relative = frameWeight - shapeInfo.frameWeight;
							if (relative <= 0.0f)
							{
								float diff = shapeInfo.frameWeight - prevShapeWeight;
								if (diff > 0.0f)
								{
									float t = -relative / diff;
									shapeInfo.finalWeight = 1.0f - std::min(t, 1.0f);
								}
								else
									shapeInfo.finalWeight = 1.0f;
							}
							else
							{
								float diff = nextShapeWeight - shapeInfo.frameWeight;
								if (diff > 0.0f)
								{
									float t = relative / diff;
									shapeInfo.finalWeight = std::min(t, 1.0f);
								}
								else
									shapeInfo.finalWeight = 0.0f;
							}
						}

						// Last frame
						{
							UINT32 lastFrame = channelInfo.shapeStart + channelInfo.shapeCount - 1;
							MorphShapeInfo& prevShapeInfo = anim->morphShapeInfos[lastFrame - 1];
							MorphShapeInfo& shapeInfo = anim->morphShapeInfos[lastFrame];

							float relative = frameWeight - shapeInfo.frameWeight;
							if (relative <= 0.0f)
							{
								float diff = shapeInfo.frameWeight - prevShapeInfo.frameWeight;
								if (diff > 0.0f)
								{
									float t = -relative / diff;
									shapeInfo.finalWeight = 1.0f - std::min(t, 1.0f);
								}
								else
									shapeInfo.finalWeight = 1.0f;
							}
							else // If past the final frame we clamp
								shapeInfo.finalWeight = 1.0f;
						}
					}

					for(UINT32 j = 0; j < channelInfo.shapeCount; j++)
					{
						MorphShapeInfo& shapeInfo = anim->morphShapeInfos[channelInfo.shapeStart + j];
						shapeInfo.finalWeight *= channelInfo.weight;
					}
				}

				// Generate morph shape vertices
				if(anim->morphChannelWeightsDirty || hasMorphCurves)
				{
					SPtr<MeshData> meshData = bs_shared_ptr_new<MeshData>(anim->numMorphVertices, 0, mBlendShapeVertexDesc);

					UINT8* bufferData = meshData->getData();
					memset(bufferData, 0, meshData->getSize());

					UINT32 tempDataSize = (sizeof(Vector3) + sizeof(float)) * anim->numMorphVertices;
					UINT8* tempData = (UINT8*)bs_stack_alloc(tempDataSize);
					memset(tempData, 0, tempDataSize);

					Vector3* tempNormals = (Vector3*)tempData;
					float* accumulatedWeight = (float*)(tempData + sizeof(Vector3) * anim->numMorphVertices);

					UINT8* positions = meshData->getElementData(VES_POSITION, 1, 1);
					UINT8* normals = meshData->getElementData(VES_NORMAL, 1, 1);

					UINT32 stride = mBlendShapeVertexDesc->getVertexStride(1);

					for(UINT32 i = 0; i < anim->numMorphShapes; i++)
					{
						const MorphShapeInfo& info = anim->morphShapeInfos[i];
						float absWeight = Math::abs(info.finalWeight);

						if (absWeight < 0.0001f)
							continue;

						const Vector<MorphVertex>& morphVertices = info.shape->getVertices();
						UINT32 numVertices = (UINT32)morphVertices.size();
						for(UINT32 j = 0; j < numVertices; j++)
						{
							const MorphVertex& vertex = morphVertices[j];

							Vector3* destPos = (Vector3*)(positions + vertex.sourceIdx * stride);
							*destPos += vertex.deltaPosition * info.finalWeight;

							tempNormals[vertex.sourceIdx] += vertex.deltaNormal * info.finalWeight;
							accumulatedWeight[vertex.sourceIdx] += absWeight;
						}
					}

					for(UINT32 i = 0; i < anim->numMorphVertices; i++)
					{
						PackedNormal* destNrm = (PackedNormal*)(normals + i * stride);

						if (accumulatedWeight[i] > 0.0001f)
						{
							Vector3 normal = tempNormals[i] / accumulatedWeight[i];
							normal /= 2.0f; // Accumulated normal is in range [-2, 2] but our normal packing method assumes [-1, 1] range

							MeshUtility::packNormals(&normal, (UINT8*)destNrm, 1, sizeof(Vector3), stride);
							destNrm->w = (UINT8)(std::min(1.0f, accumulatedWeight[i]) * 255.999f);
						}
						else
						{
							*destNrm = { 127, 127, 127, 0 };
						}
					}

					bs_stack_free(tempData);

					animInfo.morphShapeInfo.meshData = meshData;

					animInfo.morphShapeInfo.version++;
					anim->morphChannelWeightsDirty = false;
				}

				hasAnimInfo = true;
			}
			else
				animInfo.morphShapeInfo.version = 1;

			if (hasAnimInfo)
				renderData.infos[anim->id] = animInfo;
		}

		// Increments counter and ensures all writes are recorded
		mWorkerState.store(WorkerState::DataReady, std::memory_order_release);
		mDataReadyCount.fetch_add(1, std::memory_order_acq_rel);
	}

	void AnimationManager::waitUntilComplete()
	{
		mAnimationWorker->wait();

		// Read counter, and ensure all reads are done after writes on anim thread complete
		INT32 dataReadyCount = mDataReadyCount.load(std::memory_order_acquire);

		if (dataReadyCount > CoreThread::NUM_SYNC_BUFFERS)
		{
			LOGERR("Animation manager threading issue. Too many entries in queue: " + toString(dataReadyCount));
			assert(dataReadyCount <= CoreThread::NUM_SYNC_BUFFERS);
		}

		mDataReady = dataReadyCount > 0;
		if (!mDataReady)
			return;

		mDataReadyCount.fetch_sub(1, std::memory_order_release);
		mPoseReadBufferIdx = (mPoseReadBufferIdx + 1) % CoreThread::NUM_SYNC_BUFFERS;
	}

	const RendererAnimationData& AnimationManager::getRendererData()
	{
		if (!mDataReady)
		{
			static RendererAnimationData dummy;
			return dummy;
		}

		return mAnimData[mPoseReadBufferIdx];
	}

	UINT64 AnimationManager::registerAnimation(Animation* anim)
	{
		mAnimations[mNextId] = anim;
		return mNextId++;
	}

	void AnimationManager::unregisterAnimation(UINT64 animId)
	{
		mAnimations.erase(animId);
	}

	AnimationManager& gAnimation()
	{
		return AnimationManager::instance();
	}
}