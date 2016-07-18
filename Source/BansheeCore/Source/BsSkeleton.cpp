//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsSkeleton.h"
#include "BsAnimationClip.h"
#include "BsSkeletonRTTI.h"

namespace BansheeEngine
{
	LocalSkeletonPose::LocalSkeletonPose()
		: positions(nullptr), rotations(nullptr), scales(nullptr), numBones(0)
	{ }

	LocalSkeletonPose::LocalSkeletonPose(UINT32 numBones)
		: numBones(numBones)
	{
		UINT32 elementSize = sizeof(Vector3) * 2 + sizeof(Quaternion);
		UINT8* buffer = (UINT8*)bs_alloc(elementSize * numBones);

		positions = (Vector3*)buffer;
		buffer += sizeof(Vector3) * numBones;

		rotations = (Quaternion*)buffer;
		buffer += sizeof(Quaternion) * numBones;

		scales = (Vector3*)buffer;
	}

	LocalSkeletonPose::LocalSkeletonPose(UINT32 numPos, UINT32 numRot, UINT32 numScale)
		: numBones(0)
	{
		UINT32 bufferSize = sizeof(Vector3) * numPos + sizeof(Quaternion) * numRot + sizeof(Vector3) * numScale;
		UINT8* buffer = (UINT8*)bs_alloc(bufferSize);

		positions = (Vector3*)buffer;
		buffer += sizeof(Vector3) * numPos;

		rotations = (Quaternion*)buffer;
		buffer += sizeof(Quaternion) * numRot;

		scales = (Vector3*)buffer;
	}

	LocalSkeletonPose::LocalSkeletonPose(LocalSkeletonPose&& other)
		: positions(other.positions), rotations(other.rotations), scales(other.scales), numBones(other.numBones)
	{
		other.positions = nullptr;
		other.rotations = nullptr;
		other.scales = nullptr;
		other.numBones = 0;
	}

	LocalSkeletonPose::~LocalSkeletonPose()
	{
		if (positions != nullptr)
			bs_free(positions);
	}

	LocalSkeletonPose& LocalSkeletonPose::operator=(LocalSkeletonPose&& other)
	{
		if (this != &other)
		{
			if (positions != nullptr)
				bs_free(positions);

			positions = other.positions;
			rotations = other.rotations;
			scales = other.scales;
			numBones = other.numBones;

			other.positions = nullptr;
			other.rotations = nullptr;
			other.scales = nullptr;
			other.numBones = 0;
		}

		return *this;
	}

	Skeleton::Skeleton()
		:mInvBindPoses(nullptr), mBoneInfo(nullptr), mNumBones(0)
	{ }

	Skeleton::Skeleton(BONE_DESC* bones, UINT32 numBones)
		:mInvBindPoses(bs_newN<Matrix4>(numBones)), mBoneInfo(bs_newN<SkeletonBoneInfo>(numBones)), mNumBones(numBones)
	{
		for(UINT32 i = 0; i < numBones; i++)
		{
			mInvBindPoses[i] = bones[i].invBindPose;
			mBoneInfo[i].name = bones[i].name;
			mBoneInfo[i].parent = bones[i].parent;
		}
	}

	Skeleton::~Skeleton()
	{
		if(mInvBindPoses != nullptr)
			bs_deleteN(mInvBindPoses, mNumBones);

		if (mBoneInfo != nullptr)
			bs_deleteN(mBoneInfo, mNumBones);
	}

	SPtr<Skeleton> Skeleton::create(BONE_DESC* bones, UINT32 numBones)
	{
		Skeleton* rawPtr = new (bs_alloc<Skeleton>()) Skeleton(bones, numBones);

		return bs_shared_ptr<Skeleton>(rawPtr);
	}

	void Skeleton::getPose(Matrix4* pose, LocalSkeletonPose& localPose, const AnimationClip& clip, float time,
		bool loop)
	{
		bs_frame_mark();
		{
			FrameVector<AnimationCurveMapping> boneToCurveMapping(mNumBones);

			AnimationState state;
			state.curves = clip.getCurves();
			state.boneToCurveMapping = boneToCurveMapping.data();
			state.loop = loop;
			state.weight = 1.0f;
			state.time = time;

			FrameVector<TCurveCache<Vector3>> positionCache(state.curves->position.size());
			FrameVector<TCurveCache<Quaternion>> rotationCache(state.curves->rotation.size());
			FrameVector<TCurveCache<Vector3>> scaleCache(state.curves->scale.size());

			state.positionCaches = positionCache.data();
			state.rotationCaches = rotationCache.data();
			state.scaleCaches = scaleCache.data();
			state.genericCaches = nullptr;

			AnimationStateLayer layer;
			layer.index = 0;
			layer.additive = false;
			layer.states = &state;
			layer.numStates = 1;

			clip.getBoneMapping(*this, state.boneToCurveMapping);

			getPose(pose, localPose, &layer, 1);
		}
		bs_frame_clear();
	}

	void Skeleton::getPose(Matrix4* pose, LocalSkeletonPose& localPose, const AnimationStateLayer* layers,
		UINT32 numLayers)
	{
		// Note: If more performance is required this method could be optimized with vector instructions

		assert(localPose.numBones == mNumBones);

		for(UINT32 i = 0; i < mNumBones; i++)
		{
			localPose.positions[i] = Vector3::ZERO;
			localPose.rotations[i] = Quaternion::IDENTITY;
			localPose.scales[i] = Vector3::ONE;
		}

		for(UINT32 i = 0; i < numLayers; i++)
		{
			const AnimationStateLayer& layer = layers[i];

			float invLayerWeight;
			if (layer.additive)
			{
				float weightSum = 0.0f;
				for (UINT32 j = 0; j < layer.numStates; j++)
					weightSum += layer.states[j].weight;

				invLayerWeight = 1.0f / weightSum;
			}
			else
				invLayerWeight = 1.0f;

			for (UINT32 j = 0; j < layer.numStates; j++)
			{
				const AnimationState& state = layer.states[j];

				float normWeight = state.weight * invLayerWeight;

				// Early exit for clips that don't contribute (which there could be plenty especially for sequential blends)
				if (Math::approxEquals(normWeight, 0.0f))
					continue;

				for (UINT32 k = 0; k < mNumBones; k++)
				{
					const AnimationCurveMapping& mapping = state.boneToCurveMapping[k];

					if (mapping.position != (UINT32)-1)
					{
						const TAnimationCurve<Vector3>& curve = state.curves->position[mapping.position].curve;
						localPose.positions[k] += curve.evaluate(state.time, state.positionCaches[k], state.loop) * normWeight;
					}
				}

				for (UINT32 k = 0; k < mNumBones; k++)
				{
					const AnimationCurveMapping& mapping = state.boneToCurveMapping[k];

					if (mapping.scale != (UINT32)-1)
					{
						const TAnimationCurve<Vector3>& curve = state.curves->scale[mapping.scale].curve;
						localPose.scales[k] *= curve.evaluate(state.time, state.scaleCaches[k], state.loop) * normWeight;
					}
				}

				if(layer.additive)
				{
					for (UINT32 k = 0; k < mNumBones; k++)
					{
						const AnimationCurveMapping& mapping = state.boneToCurveMapping[k];

						if (mapping.rotation != (UINT32)-1)
						{
							const TAnimationCurve<Quaternion>& curve = state.curves->rotation[mapping.rotation].curve;

							Quaternion value = curve.evaluate(state.time, state.rotationCaches[k], state.loop);
							value = Quaternion::lerp(normWeight, Quaternion::IDENTITY, value);

							localPose.rotations[k] *= value;
						}
					}
				}
				else
				{
					for (UINT32 k = 0; k < mNumBones; k++)
					{
						const AnimationCurveMapping& mapping = state.boneToCurveMapping[k];

						if (mapping.rotation != (UINT32)-1)
						{
							const TAnimationCurve<Quaternion>& curve = state.curves->rotation[mapping.rotation].curve;
							localPose.rotations[k] += curve.evaluate(state.time, state.rotationCaches[k], state.loop) * normWeight;
						}
					}
				}
			}
		}

		// Calculate local pose matrices
		for(UINT32 i = 0; i < mNumBones; i++)
		{
			localPose.rotations[i].normalize();

			pose[i] = Matrix4::TRS(localPose.positions[i], localPose.rotations[i], localPose.scales[i]);
			pose[i] = pose[i] * mInvBindPoses[i];
		}

		// Calculate global poses
		UINT32 isGlobalBytes = sizeof(bool) * mNumBones;
		bool* isGlobal = (bool*)bs_stack_alloc(isGlobalBytes);
		memset(isGlobal, 0, isGlobalBytes);

		std::function<void(UINT32)> calcGlobal = [&](UINT32 boneIdx)
		{
			UINT32 parentBoneIdx = mBoneInfo[boneIdx].parent;
			if (parentBoneIdx == (UINT32)-1)
			{
				isGlobal[boneIdx] = true;
				return;
			}

			if (!isGlobal[parentBoneIdx])
				calcGlobal(parentBoneIdx);

			pose[boneIdx] = pose[parentBoneIdx] * pose[boneIdx];
			isGlobal[boneIdx] = true;
		};

		for (UINT32 i = 0; i < mNumBones; i++)
		{
			if (!isGlobal[i])
				calcGlobal(i);
		}

		bs_stack_free(isGlobal);
	}

	SPtr<Skeleton> Skeleton::createEmpty()
	{
		Skeleton* rawPtr = new (bs_alloc<Skeleton>()) Skeleton();

		SPtr<Skeleton> newSkeleton = bs_shared_ptr<Skeleton>(rawPtr);
		return newSkeleton;
	}

	RTTITypeBase* Skeleton::getRTTIStatic()
	{
		return SkeletonRTTI::instance();
	}

	RTTITypeBase* Skeleton::getRTTI() const
	{
		return getRTTIStatic();
	}
}