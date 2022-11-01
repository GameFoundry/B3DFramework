//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Animation/BsAnimationManager.h"
#include "Animation/BsAnimation.h"
#include "Animation/BsAnimationClip.h"
#include "Threading/BsTaskScheduler.h"
#include "Utility/BsTime.h"
#include "Scene/BsSceneManager.h"
#include "Renderer/BsCamera.h"
#include "Animation/BsMorphShapes.h"
#include "Mesh/BsMeshData.h"
#include "Mesh/BsMeshUtility.h"

using namespace bs;

AnimationManager::AnimationManager()
{
	mBlendShapeVertexDesc = VertexDataDesc::Create();
	mBlendShapeVertexDesc->AddVertElem(VET_FLOAT3, VES_POSITION, 1, 1);
	mBlendShapeVertexDesc->AddVertElem(VET_UBYTE4_NORM, VES_NORMAL, 1, 1);
}

void AnimationManager::SetPaused(bool paused)
{
	mPaused = paused;
}

void AnimationManager::SetUpdateRate(u32 fps)
{
	if(fps == 0)
		fps = 1;

	mUpdateRate = 1.0f / fps;
}

const EvaluatedAnimationData* AnimationManager::Update(bool async)
{
	// Wait for any workers to complete
	{
		Lock lock(mMutex);

		while(mNumActiveWorkers > 0)
			mWorkerDoneSignal.wait(lock);

		// Advance the buffers (last write buffer becomes read buffer)
		if(mSwapBuffers)
		{
			mPoseReadBufferIdx = (mPoseReadBufferIdx + 1) % (CoreThread::kNumSyncBuffers + 1);
			mPoseWriteBufferIdx = (mPoseWriteBufferIdx + 1) % (CoreThread::kNumSyncBuffers + 1);

			mSwapBuffers = false;
		}
	}

	if(mPaused)
		return &mAnimData[mPoseReadBufferIdx];

	mAnimationTime += GetTime().GetFrameDelta();
	if(mAnimationTime < mNextAnimationUpdateTime)
		return &mAnimData[mPoseReadBufferIdx];

	mNextAnimationUpdateTime = Math::Floor(mAnimationTime / mUpdateRate) * mUpdateRate + mUpdateRate;

	float timeDelta = mAnimationTime - mLastAnimationUpdateTime;
	mLastAnimationUpdateTime = mAnimationTime;

	// Trigger events and update attachments (for the data from the last frame)
	if(async)
	{
		for(auto& anim : mAnimations)
		{
			anim.second->UpdateFromProxy();
			anim.second->TriggerEvents(mLastAnimationDeltaTime);
		}
	}

	mLastAnimationDeltaTime = timeDelta;

	// Update animation proxies from the latest data
	mProxies.clear();
	for(auto& anim : mAnimations)
	{
		anim.second->UpdateAnimProxy(timeDelta);
		mProxies.push_back(anim.second->mAnimProxy);
	}

	// Build frustums for culling
	mCullFrustums.clear();

	auto& allCameras = GetSceneManager().GetAllCameras();
	for(auto& entry : allCameras)
	{
		// Note: This should also check on-demand cameras as there's no point in updating them if they wont render this frame
		bool isOverlayCamera = entry.second->GetRenderSettings()->OverlayOnly;
		if(isOverlayCamera)
			continue;

		// TODO: Not checking if camera and animation renderable's layers match. If we checked more animations could
		// be culled.
		mCullFrustums.push_back(entry.second->GetWorldFrustum());
	}

	// Prepare the write buffer
	u32 totalNumBones = 0;
	for(auto& anim : mProxies)
	{
		if(anim->Skeleton != nullptr)
			totalNumBones += anim->Skeleton->GetNumBones();
	}

	// Prepare the write buffer
	EvaluatedAnimationData& renderData = mAnimData[mPoseWriteBufferIdx];
	renderData.Transforms.resize(totalNumBones);
	renderData.Infos.clear();

	// Queue animation evaluation tasks
	{
		Lock lock(mMutex);
		mNumActiveWorkers = (u32)mProxies.size();
	}

	u32 curBoneIdx = 0;
	for(auto& anim : mProxies)
	{
		auto evaluateAnimWorker = [this, anim, curBoneIdx]()
		{
			u32 boneIdx = curBoneIdx;
			EvaluateAnimation(anim.get(), boneIdx);

			Lock lock(mMutex);
			{
				assert(mNumActiveWorkers > 0);
				mNumActiveWorkers--;
			}

			mWorkerDoneSignal.notify_one();
		};

		SPtr<Task> task = Task::Create("AnimWorker", evaluateAnimWorker);
		TaskScheduler::Instance().AddTask(task);

		if(anim->Skeleton != nullptr)
			curBoneIdx += anim->Skeleton->GetNumBones();
	}

	// Wait for tasks to complete
	if(!async)
	{
		{
			Lock lock(mMutex);

			while(mNumActiveWorkers > 0)
				mWorkerDoneSignal.wait(lock);
		}

		// Trigger events and update attachments (for the data we just evaluated)
		for(auto& anim : mAnimations)
		{
			anim.second->UpdateFromProxy();
			anim.second->TriggerEvents(timeDelta);
		}
	}

	mSwapBuffers = true;

	EvaluatedAnimationData* output;
	if(!async)
		output = &mAnimData[mPoseWriteBufferIdx];
	else
		output = &mAnimData[mPoseReadBufferIdx];

	output->Async = async;
	return output;
}

void AnimationManager::EvaluateAnimation(AnimationProxy* anim, u32& curBoneIdx)
{
	// Culling
	if(anim->MCullEnabled)
	{
		bool isVisible = false;
		for(auto& frustum : mCullFrustums)
		{
			if(frustum.Intersects(anim->MBounds))
			{
				isVisible = true;
				break;
			}
		}

		if(!isVisible)
		{
			anim->WasCulled = true;
			return;
		}
	}

	anim->WasCulled = false;

	// Evaluation
	EvaluatedAnimationData& renderData = mAnimData[mPoseWriteBufferIdx];

	u32 prevPoseBufferIdx = (mPoseWriteBufferIdx + CoreThread::kNumSyncBuffers) % (CoreThread::kNumSyncBuffers + 1);
	EvaluatedAnimationData& prevRenderData = mAnimData[prevPoseBufferIdx];

	EvaluatedAnimationData::AnimInfo animInfo;
	bool hasAnimInfo = false;

	// Evaluate skeletal animation
	if(anim->Skeleton != nullptr)
	{
		u32 numBones = anim->Skeleton->GetNumBones();

		EvaluatedAnimationData::PoseInfo& poseInfo = animInfo.PoseInfo;
		poseInfo.AnimId = anim->Id;
		poseInfo.StartIdx = curBoneIdx;
		poseInfo.NumBones = numBones;

		memset(anim->SkeletonPose.HasOverride, 0, sizeof(bool) * anim->SkeletonPose.NumBones);
		Matrix4* boneDst = renderData.Transforms.data() + curBoneIdx;

		// Copy transforms from mapped scene objects
		u32 boneTfrmIdx = 0;
		for(u32 i = 0; i < anim->NumSceneObjects; i++)
		{
			const AnimatedSceneObjectInfo& soInfo = anim->SceneObjectInfos[i];

			if(soInfo.BoneIdx == -1)
				continue;

			boneDst[soInfo.BoneIdx] = anim->SceneObjectTransforms[boneTfrmIdx];
			anim->SkeletonPose.HasOverride[soInfo.BoneIdx] = true;
			boneTfrmIdx++;
		}

		// Animate bones
		anim->Skeleton->GetPose(boneDst, anim->SkeletonPose, anim->SkeletonMask, anim->Layers, anim->NumLayers);

		curBoneIdx += numBones;
		hasAnimInfo = true;
	}
	else
	{
		EvaluatedAnimationData::PoseInfo& poseInfo = animInfo.PoseInfo;
		poseInfo.AnimId = anim->Id;
		poseInfo.StartIdx = 0;
		poseInfo.NumBones = 0;
	}

	// Reset mapped SO transform
	for(u32 i = 0; i < anim->SceneObjectPose.NumBones; i++)
	{
		anim->SceneObjectPose.Positions[i] = Vector3::kZero;
		anim->SceneObjectPose.Rotations[i] = Quaternion::kIdentity;
		anim->SceneObjectPose.Scales[i] = Vector3::kOne;
	}

	// Update mapped scene objects
	memset(anim->SceneObjectPose.HasOverride, 1, sizeof(bool) * 3 * anim->NumSceneObjects);

	// Update scene object transforms
	for(u32 i = 0; i < anim->NumSceneObjects; i++)
	{
		const AnimatedSceneObjectInfo& soInfo = anim->SceneObjectInfos[i];

		// We already evaluated bones
		if(soInfo.BoneIdx != -1)
			continue;

		if(soInfo.LayerIdx == -1 || soInfo.StateIdx == -1)
			continue;

		const AnimationState& state = anim->Layers[soInfo.LayerIdx].States[soInfo.StateIdx];
		if(state.Disabled)
			continue;

		{
			u32 curveIdx = soInfo.CurveIndices.Position;
			if(curveIdx != (u32)-1)
			{
				const TAnimationCurve<Vector3>& curve = state.Curves->Position[curveIdx].Curve;
				anim->SceneObjectPose.Positions[curveIdx] = curve.Evaluate(state.Time, state.PositionCaches[curveIdx], false);
				anim->SceneObjectPose.HasOverride[i * 3 + 0] = false;
			}
		}

		{
			u32 curveIdx = soInfo.CurveIndices.Rotation;
			if(curveIdx != (u32)-1)
			{
				const TAnimationCurve<Quaternion>& curve = state.Curves->Rotation[curveIdx].Curve;
				anim->SceneObjectPose.Rotations[curveIdx] = curve.Evaluate(state.Time, state.RotationCaches[curveIdx], false);
				anim->SceneObjectPose.Rotations[curveIdx].Normalize();
				anim->SceneObjectPose.HasOverride[i * 3 + 1] = false;
			}
		}

		{
			u32 curveIdx = soInfo.CurveIndices.Scale;
			if(curveIdx != (u32)-1)
			{
				const TAnimationCurve<Vector3>& curve = state.Curves->Scale[curveIdx].Curve;
				anim->SceneObjectPose.Scales[curveIdx] = curve.Evaluate(state.Time, state.ScaleCaches[curveIdx], false);
				anim->SceneObjectPose.HasOverride[i * 3 + 2] = false;
			}
		}
	}

	// Update generic curves
	// Note: No blending for generic animations, just use first animation
	if(anim->NumLayers > 0 && anim->Layers[0].NumStates > 0)
	{
		const AnimationState& state = anim->Layers[0].States[0];
		if(!state.Disabled)
		{
			u32 numCurves = (u32)state.Curves->Generic.size();
			for(u32 i = 0; i < numCurves; i++)
			{
				const TAnimationCurve<float>& curve = state.Curves->Generic[i].Curve;
				anim->GenericCurveOutputs[i] = curve.Evaluate(state.Time, state.GenericCaches[i], false);
			}
		}
	}

	// Update morph shapes
	if(anim->NumMorphShapes > 0)
	{
		auto iterFind = prevRenderData.Infos.find(anim->Id);
		if(iterFind != prevRenderData.Infos.end())
			animInfo.MorphShapeInfo = iterFind->second.MorphShapeInfo;
		else
			animInfo.MorphShapeInfo.Version = 1; // 0 is considered invalid version

		// Recalculate weights if curves are present
		bool hasMorphCurves = false;
		for(u32 i = 0; i < anim->NumMorphChannels; i++)
		{
			MorphChannelInfo& channelInfo = anim->MorphChannelInfos[i];
			if(channelInfo.WeightCurveIdx != (u32)-1)
			{
				channelInfo.Weight = Math::Clamp01(anim->GenericCurveOutputs[channelInfo.WeightCurveIdx]);
				hasMorphCurves = true;
			}

			float frameWeight;
			if(channelInfo.FrameCurveIdx != (u32)-1)
			{
				frameWeight = Math::Clamp01(anim->GenericCurveOutputs[channelInfo.FrameCurveIdx]);
				hasMorphCurves = true;
			}
			else
				frameWeight = 0.0f;

			if(channelInfo.ShapeCount == 1)
			{
				MorphShapeInfo& shapeInfo = anim->MorphShapeInfos[channelInfo.ShapeStart];

				// Blend between base shape and the only available frame
				float relative = frameWeight - shapeInfo.FrameWeight;
				if(relative <= 0.0f)
				{
					float diff = shapeInfo.FrameWeight;
					if(diff > 0.0f)
					{
						float t = -relative / diff;
						shapeInfo.FinalWeight = 1.0f - std::min(t, 1.0f);
					}
					else
						shapeInfo.FinalWeight = 1.0f;
				}
				else // If past the final frame we clamp
					shapeInfo.FinalWeight = 1.0f;
			}
			else if(channelInfo.ShapeCount > 1)
			{
				for(u32 j = 0; j < channelInfo.ShapeCount - 1; j++)
				{
					float prevShapeWeight;
					if(j > 0)
						prevShapeWeight = anim->MorphShapeInfos[j - 1].FrameWeight;
					else
						prevShapeWeight = 0.0f; // Base shape, blend between it and the first frame

					float nextShapeWeight = anim->MorphShapeInfos[j + 1].FrameWeight;
					MorphShapeInfo& shapeInfo = anim->MorphShapeInfos[j];

					float relative = frameWeight - shapeInfo.FrameWeight;
					if(relative <= 0.0f)
					{
						float diff = shapeInfo.FrameWeight - prevShapeWeight;
						if(diff > 0.0f)
						{
							float t = -relative / diff;
							shapeInfo.FinalWeight = 1.0f - std::min(t, 1.0f);
						}
						else
							shapeInfo.FinalWeight = 1.0f;
					}
					else
					{
						float diff = nextShapeWeight - shapeInfo.FrameWeight;
						if(diff > 0.0f)
						{
							float t = relative / diff;
							shapeInfo.FinalWeight = std::min(t, 1.0f);
						}
						else
							shapeInfo.FinalWeight = 0.0f;
					}
				}

				// Last frame
				{
					u32 lastFrame = channelInfo.ShapeStart + channelInfo.ShapeCount - 1;
					MorphShapeInfo& prevShapeInfo = anim->MorphShapeInfos[lastFrame - 1];
					MorphShapeInfo& shapeInfo = anim->MorphShapeInfos[lastFrame];

					float relative = frameWeight - shapeInfo.FrameWeight;
					if(relative <= 0.0f)
					{
						float diff = shapeInfo.FrameWeight - prevShapeInfo.FrameWeight;
						if(diff > 0.0f)
						{
							float t = -relative / diff;
							shapeInfo.FinalWeight = 1.0f - std::min(t, 1.0f);
						}
						else
							shapeInfo.FinalWeight = 1.0f;
					}
					else // If past the final frame we clamp
						shapeInfo.FinalWeight = 1.0f;
				}
			}

			for(u32 j = 0; j < channelInfo.ShapeCount; j++)
			{
				MorphShapeInfo& shapeInfo = anim->MorphShapeInfos[channelInfo.ShapeStart + j];
				shapeInfo.FinalWeight *= channelInfo.Weight;
			}
		}

		// Generate morph shape vertices
		if(anim->MorphChannelWeightsDirty || hasMorphCurves)
		{
			SPtr<MeshData> meshData = B3DMakeShared<MeshData>(anim->NumMorphVertices, 0, mBlendShapeVertexDesc);

			u8* bufferData = meshData->GetData();
			memset(bufferData, 0, meshData->GetSize());

			u32 tempDataSize = (sizeof(Vector3) + sizeof(float)) * anim->NumMorphVertices;
			u8* tempData = (u8*)B3DStackAllocate(tempDataSize);
			memset(tempData, 0, tempDataSize);

			Vector3* tempNormals = (Vector3*)tempData;
			float* accumulatedWeight = (float*)(tempData + sizeof(Vector3) * anim->NumMorphVertices);

			u8* positions = meshData->GetElementData(VES_POSITION, 1, 1);
			u8* normals = meshData->GetElementData(VES_NORMAL, 1, 1);

			u32 stride = mBlendShapeVertexDesc->GetVertexStride(1);

			for(u32 i = 0; i < anim->NumMorphShapes; i++)
			{
				const MorphShapeInfo& info = anim->MorphShapeInfos[i];
				float absWeight = Math::Abs(info.FinalWeight);

				if(absWeight < 0.0001f)
					continue;

				const Vector<MorphVertex>& morphVertices = info.Shape->GetVertices();
				u32 numVertices = (u32)morphVertices.size();
				for(u32 j = 0; j < numVertices; j++)
				{
					const MorphVertex& vertex = morphVertices[j];

					Vector3* destPos = (Vector3*)(positions + vertex.SourceIdx * stride);
					*destPos += vertex.DeltaPosition * info.FinalWeight;

					tempNormals[vertex.SourceIdx] += vertex.DeltaNormal * info.FinalWeight;
					accumulatedWeight[vertex.SourceIdx] += absWeight;
				}
			}

			for(u32 i = 0; i < anim->NumMorphVertices; i++)
			{
				PackedNormal* destNrm = (PackedNormal*)(normals + i * stride);

				if(accumulatedWeight[i] > 0.0001f)
				{
					Vector3 normal = tempNormals[i] / accumulatedWeight[i];
					normal /= 2.0f; // Accumulated normal is in range [-2, 2] but our normal packing method assumes [-1, 1] range

					MeshUtility::PackNormals(&normal, (u8*)destNrm, 1, sizeof(Vector3), stride);
					destNrm->W = (u8)(std::min(1.0f, accumulatedWeight[i]) * 255.999f);
				}
				else
				{
					*destNrm = { { 127, 127, 127, 0 } };
				}
			}

			B3DStackFree(tempData);

			animInfo.MorphShapeInfo.MeshData = meshData;

			animInfo.MorphShapeInfo.Version++;
			anim->MorphChannelWeightsDirty = false;
		}

		hasAnimInfo = true;
	}
	else
		animInfo.MorphShapeInfo.Version = 1;

	if(hasAnimInfo)
	{
		Lock lock(mMutex);
		renderData.Infos[anim->Id] = animInfo;
	}
}

u64 AnimationManager::RegisterAnimation(Animation* anim)
{
	mAnimations[mNextId] = anim;
	return mNextId++;
}

void AnimationManager::UnregisterAnimation(u64 animId)
{
	mAnimations.erase(animId);
}

namespace bs
{
AnimationManager& GetAnimationManager()
{
	return AnimationManager::Instance();
}
} // namespace bs
