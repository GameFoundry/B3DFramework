//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Animation/BsSkeleton.h"
#include "Animation/BsAnimationClip.h"
#include "Animation/BsSkeletonMask.h"
#include "Private/RTTI/BsSkeletonRTTI.h"

namespace bs
{
	LocalSkeletonPose::LocalSkeletonPose()
		: positions(nullptr), rotations(nullptr), scales(nullptr), hasOverride(nullptr), numBones(0)
	{ }

	LocalSkeletonPose::LocalSkeletonPose(UINT32 numBones)
		: numBones(numBones)
	{
		UINT32 elementSize = sizeof(Vector3) * 2 + sizeof(Quaternion) + sizeof(bool);
		UINT8* buffer = (UINT8*)bs_alloc(elementSize * numBones);

		positions = (Vector3*)buffer;
		buffer += sizeof(Vector3) * numBones;

		rotations = (Quaternion*)buffer;
		buffer += sizeof(Quaternion) * numBones;

		scales = (Vector3*)buffer;
		buffer += sizeof(Vector3) * numBones;

		hasOverride = (bool*)buffer;
	}

	LocalSkeletonPose::LocalSkeletonPose(UINT32 numPos, UINT32 numRot, UINT32 numScale)
		: hasOverride(nullptr), numBones(0)
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
		: positions(other.positions), rotations(other.rotations), scales(other.scales), hasOverride(other.hasOverride)
		, numBones(other.numBones)
	{
		other.positions = nullptr;
		other.rotations = nullptr;
		other.scales = nullptr;
		other.hasOverride = nullptr;
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
			hasOverride = other.hasOverride;
			numBones = other.numBones;

			other.positions = nullptr;
			other.rotations = nullptr;
			other.scales = nullptr;
			other.hasOverride = nullptr;
			other.numBones = 0;
		}

		return *this;
	}

	Skeleton::Skeleton()
	{ }

	Skeleton::Skeleton(BONE_DESC* bones, UINT32 numBones)
		: mNumBones(numBones), mBoneTransforms(bs_newN<Matrix4>(numBones)), mInvBindPoses(bs_newN<Matrix4>(numBones))
		, mBoneInfo(bs_newN<SkeletonBoneInfo>(numBones))
	{
		for(UINT32 i = 0; i < numBones; i++)
		{
			mBoneTransforms[i] = bones[i].localTfrm;
			mInvBindPoses[i] = bones[i].invBindPose;
			mBoneInfo[i].name = bones[i].name;
			mBoneInfo[i].parent = bones[i].parent;
		}
	}

	Skeleton::~Skeleton()
	{
		if(mBoneTransforms != nullptr)
			bs_deleteN(mBoneTransforms, mNumBones);

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

	void Skeleton::getPose(Matrix4* pose, LocalSkeletonPose& localPose, const SkeletonMask& mask, 
		const AnimationClip& clip, float time, bool loop)
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
			state.disabled = false;

			AnimationStateLayer layer;
			layer.index = 0;
			layer.additive = false;
			layer.states = &state;
			layer.numStates = 1;

			clip.getBoneMapping(*this, state.boneToCurveMapping);

			getPose(pose, localPose, mask, &layer, 1);
		}
		bs_frame_clear();
	}

	void Skeleton::getPose(Matrix4* pose, LocalSkeletonPose& localPose, const SkeletonMask& mask, 
		const AnimationStateLayer* layers, UINT32 numLayers)
	{
		// Note: If more performance is required this method could be optimized with vector instructions

		assert(localPose.numBones == mNumBones);

		for(UINT32 i = 0; i < mNumBones; i++)
		{
			localPose.positions[i] = Vector3::ZERO;
			localPose.rotations[i] = Quaternion::ZERO;
			localPose.scales[i] = Vector3::ONE;
		}

		bool* hasAnimCurve = bs_stack_alloc<bool>(mNumBones);
		bs_zero_out(hasAnimCurve, mNumBones);

		// Note: For a possible performance improvement consider keeping an array of only active (non-disabled) bones and
		// just iterate over them without mask checks. Possibly also a list of active curve mappings to avoid those checks
		// as well.
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
				if (state.disabled)
					continue;

				float normWeight = state.weight * invLayerWeight;

				// Early exit for clips that don't contribute (which there could be plenty especially for sequential blends)
				if (Math::approxEquals(normWeight, 0.0f))
					continue;

				for (UINT32 k = 0; k < mNumBones; k++)
				{
					if (!mask.isEnabled(k))
						continue;

					const AnimationCurveMapping& mapping = state.boneToCurveMapping[k];
					UINT32 curveIdx = mapping.position;
					if (curveIdx != (UINT32)-1)
					{
						const TAnimationCurve<Vector3>& curve = state.curves->position[curveIdx].curve;
						localPose.positions[k] += curve.evaluate(state.time, state.positionCaches[curveIdx], state.loop) * normWeight;

						localPose.hasOverride[k] = false;
						hasAnimCurve[k] = true;
					}

					curveIdx = mapping.scale;
					if (curveIdx != (UINT32)-1)
					{
						const TAnimationCurve<Vector3>& curve = state.curves->scale[curveIdx].curve;
						localPose.scales[k] *= curve.evaluate(state.time, state.scaleCaches[curveIdx], state.loop) * normWeight;

						localPose.hasOverride[k] = false;
						hasAnimCurve[k] = true;
					}

					if (layer.additive)
					{
						curveIdx = mapping.rotation;
						if (curveIdx != (UINT32)-1)
						{
							bool isAssigned = localPose.rotations[k].w != 0.0f;
							if (!isAssigned)
								localPose.rotations[k] = Quaternion::IDENTITY;

							const TAnimationCurve<Quaternion>& curve = state.curves->rotation[curveIdx].curve;

							Quaternion value = curve.evaluate(state.time, state.rotationCaches[curveIdx], state.loop);
							value = Quaternion::lerp(normWeight, Quaternion::IDENTITY, value);

							localPose.rotations[k] *= value;
							localPose.hasOverride[k] = false;
							hasAnimCurve[k] = true;
						}
					}
					else
					{
						curveIdx = mapping.rotation;
						if (curveIdx != (UINT32)-1)
						{
							const TAnimationCurve<Quaternion>& curve = state.curves->rotation[curveIdx].curve;
							Quaternion value = curve.evaluate(state.time, state.rotationCaches[curveIdx], state.loop) * normWeight;

							if (value.dot(localPose.rotations[k]) < 0.0f)
								value = -value;

							localPose.rotations[k] += value;
							localPose.hasOverride[k] = false;
							hasAnimCurve[k] = true;
						}
					}
				}
			}
		}

		// Calculate local pose matrices
		UINT32 isGlobalBytes = sizeof(bool) * mNumBones;
		bool* isGlobal = (bool*)bs_stack_alloc(isGlobalBytes);
		memset(isGlobal, 0, isGlobalBytes);

		for(UINT32 i = 0; i < mNumBones; i++)
		{
			bool isAssigned = localPose.rotations[i].w != 0.0f;
			if (!isAssigned)
				localPose.rotations[i] = Quaternion::IDENTITY;
			else
				localPose.rotations[i].normalize();

			if (localPose.hasOverride[i])
			{
				isGlobal[i] = true;
				continue;
			}

			pose[i] = Matrix4::TRS(localPose.positions[i], localPose.rotations[i], localPose.scales[i]);
		}

		// Apply default local tranform to non-animated bones (so that any potential child bones are transformed properly)
		for(UINT32 i = 0; i < mNumBones; i++)
		{
			if(hasAnimCurve[i])
				continue;

			pose[i] = mBoneTransforms[i] * pose[i];
		}

		// Calculate global poses
		// Note: For a possible performance improvement consider sorting bones in such order so that parents (and overrides)
		// always come before children, we no isGlobal check is needed.
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

		for (UINT32 i = 0; i < mNumBones; i++)
			pose[i] = pose[i] * mInvBindPoses[i];

		bs_stack_free(isGlobal);
		bs_stack_free(hasAnimCurve);
	}

	UINT32 Skeleton::getRootBoneIndex() const
	{
		for (UINT32 i = 0; i < mNumBones; i++)
		{
			if (mBoneInfo[i].parent == (UINT32)-1)
				return i;
		}

		return (UINT32)-1;
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