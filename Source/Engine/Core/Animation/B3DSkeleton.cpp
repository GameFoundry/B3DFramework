//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Animation/B3DSkeleton.h"
#include "Animation/B3DAnimationClip.h"
#include "Animation/B3DSkeletonMask.h"
#include "RTTI/B3DSkeletonRTTI.h"

using namespace b3d;

LocalSkeletonPose::LocalSkeletonPose(u32 boneCount, bool individualOverride)
	: NumBones(boneCount)
{
	const u32 overridesPerBone = individualOverride ? 3 : 1;

	u32 elementSize = sizeof(Vector3) * 2 + sizeof(Quaternion) + sizeof(bool) * overridesPerBone;
	u8* buffer = (u8*)B3DAllocate(elementSize * boneCount);

	Positions = (Vector3*)buffer;
	buffer += sizeof(Vector3) * boneCount;

	Rotations = (Quaternion*)buffer;
	buffer += sizeof(Quaternion) * boneCount;

	Scales = (Vector3*)buffer;
	buffer += sizeof(Vector3) * boneCount;

	HasOverride = (bool*)buffer;
}

LocalSkeletonPose::LocalSkeletonPose(u32 positionCount, u32 rotationCount, u32 scaleCount)
{
	u32 bufferSize = sizeof(Vector3) * positionCount + sizeof(Quaternion) * rotationCount + sizeof(Vector3) * scaleCount;
	u8* buffer = (u8*)B3DAllocate(bufferSize);

	Positions = (Vector3*)buffer;
	buffer += sizeof(Vector3) * positionCount;

	Rotations = (Quaternion*)buffer;
	buffer += sizeof(Quaternion) * rotationCount;

	Scales = (Vector3*)buffer;
}

LocalSkeletonPose::LocalSkeletonPose(LocalSkeletonPose&& other)
	: Positions{ std::exchange(other.Positions, nullptr) }
	, Rotations{ std::exchange(other.Rotations, nullptr) }
	, Scales{ std::exchange(other.Scales, nullptr) }
	, HasOverride{ std::exchange(other.HasOverride, nullptr) }
	, NumBones(std::exchange(other.NumBones, 0))
{}

LocalSkeletonPose::~LocalSkeletonPose()
{
	if(Positions != nullptr)
		B3DFree(Positions);
}

LocalSkeletonPose& LocalSkeletonPose::operator=(LocalSkeletonPose&& other)
{
	if(this != &other)
	{
		if(Positions != nullptr)
			B3DFree(Positions);

		Positions = std::exchange(other.Positions, nullptr);
		Rotations = std::exchange(other.Rotations, nullptr);
		Scales = std::exchange(other.Scales, nullptr);
		HasOverride = std::exchange(other.HasOverride, nullptr);
		NumBones = std::exchange(other.NumBones, 0);
	}

	return *this;
}

Skeleton::Skeleton(BONE_DESC* bones, u32 boneCount)
	: mNumBones(boneCount)
{
	mBoneTransforms.Resize(boneCount);
	mInvBindPoses.Resize(boneCount);
	mBoneInfo.Resize(boneCount);

	for(u32 i = 0; i < boneCount; i++)
	{
		mBoneTransforms[i] = bones[i].LocalTfrm;
		mInvBindPoses[i] = bones[i].InvBindPose;
		mBoneInfo[i].Name = bones[i].Name;
		mBoneInfo[i].Parent = bones[i].Parent;
	}
}

SPtr<Skeleton> Skeleton::Create(BONE_DESC* bones, u32 boneCount)
{
	Skeleton* rawPtr = new(B3DAllocate<Skeleton>()) Skeleton(bones, boneCount);

	return B3DMakeSharedFromExisting<Skeleton>(rawPtr);
}

void Skeleton::GetPose(Matrix4* outPose, LocalSkeletonPose& outLocalPose, const SkeletonMask& mask, const AnimationClip& clip, float time, bool loop)
{
	B3DMarkAllocatorFrame();
	{
		FrameVector<AnimationCurveMapping> boneToCurveMapping(mNumBones);

		AnimationState state;
		state.Curves = clip.GetCurves();
		state.Length = clip.GetLength();
		state.BoneToCurveMapping = boneToCurveMapping.data();
		state.Loop = loop;
		state.Weight = 1.0f;
		state.Time = time;

		FrameVector<TCurveCache<Vector3>> positionCache(state.Curves->Position.size());
		FrameVector<TCurveCache<Quaternion>> rotationCache(state.Curves->Rotation.size());
		FrameVector<TCurveCache<Vector3>> scaleCache(state.Curves->Scale.size());

		state.PositionCaches = positionCache.data();
		state.RotationCaches = rotationCache.data();
		state.ScaleCaches = scaleCache.data();
		state.GenericCaches = nullptr;
		state.Disabled = false;

		AnimationStateLayer layer;
		layer.Index = 0;
		layer.Additive = false;
		layer.States = &state;
		layer.StateCount = 1;

		clip.GetBoneMapping(*this, state.BoneToCurveMapping);

		GetPose(outPose, outLocalPose, mask, &layer, 1);
	}
	B3DClearAllocatorFrame();
}

void Skeleton::GetPose(Matrix4* outPose, LocalSkeletonPose& outLocalPose, const SkeletonMask& mask, const AnimationStateLayer* layers, u32 layerCount)
{
	// Note: If more performance is required this method could be optimized with vector instructions

	B3D_ASSERT(outLocalPose.NumBones == mNumBones);

	for(u32 boneIndex = 0; boneIndex < mNumBones; boneIndex++)
	{
		outLocalPose.Positions[boneIndex] = Vector3::kZero;
		outLocalPose.Rotations[boneIndex] = Quaternion::kZero;
		outLocalPose.Scales[boneIndex] = Vector3::kOne;
	}

	bool* hasAnimCurve = B3DStackAllocate<bool>(mNumBones);
	B3DZeroOut(hasAnimCurve, mNumBones);

	// Note: For a possible performance improvement consider keeping an array of only active (non-disabled) bones and
	// just iterate over them without mask checks. Possibly also a list of active curve mappings to avoid those checks
	// as well.
	for(u32 layerIndex = 0; layerIndex < layerCount; layerIndex++)
	{
		const AnimationStateLayer& layer = layers[layerIndex];

		float invLayerWeight;
		if(layer.Additive)
		{
			float weightSum = 0.0f;
			for(u32 j = 0; j < layer.StateCount; j++)
				weightSum += layer.States[j].Weight;

			invLayerWeight = 1.0f / weightSum;
		}
		else
			invLayerWeight = 1.0f;

		for(u32 j = 0; j < layer.StateCount; j++)
		{
			const AnimationState& state = layer.States[j];
			if(state.Disabled)
				continue;

			float normWeight = state.Weight * invLayerWeight;

			// Early exit for clips that don't contribute (which there could be plenty especially for sequential blends)
			if(Math::ApproxEquals(normWeight, 0.0f))
				continue;

			for(u32 k = 0; k < mNumBones; k++)
			{
				if(!mask.IsEnabled(k))
					continue;

				const AnimationCurveMapping& mapping = state.BoneToCurveMapping[k];
				u32 curveIdx = mapping.Position;
				if(curveIdx != (u32)-1)
				{
					const TAnimationCurve<Vector3>& curve = state.Curves->Position[curveIdx].Curve;
					outLocalPose.Positions[k] += curve.Evaluate(state.Time, state.PositionCaches[curveIdx], false) * normWeight;

					outLocalPose.HasOverride[k] = false;
					hasAnimCurve[k] = true;
				}

				curveIdx = mapping.Scale;
				if(curveIdx != (u32)-1)
				{
					const TAnimationCurve<Vector3>& curve = state.Curves->Scale[curveIdx].Curve;
					outLocalPose.Scales[k] *= curve.Evaluate(state.Time, state.ScaleCaches[curveIdx], false) * normWeight;

					outLocalPose.HasOverride[k] = false;
					hasAnimCurve[k] = true;
				}

				if(layer.Additive)
				{
					curveIdx = mapping.Rotation;
					if(curveIdx != (u32)-1)
					{
						bool isAssigned = outLocalPose.Rotations[k].W != 0.0f;
						if(!isAssigned)
							outLocalPose.Rotations[k] = Quaternion::kIdentity;

						const TAnimationCurve<Quaternion>& curve = state.Curves->Rotation[curveIdx].Curve;

						Quaternion value = curve.Evaluate(state.Time, state.RotationCaches[curveIdx], false);
						value = Quaternion::Lerp(normWeight, Quaternion::kIdentity, value);

						outLocalPose.Rotations[k] *= value;
						outLocalPose.HasOverride[k] = false;
						hasAnimCurve[k] = true;
					}
				}
				else
				{
					curveIdx = mapping.Rotation;
					if(curveIdx != (u32)-1)
					{
						const TAnimationCurve<Quaternion>& curve = state.Curves->Rotation[curveIdx].Curve;
						Quaternion value = curve.Evaluate(state.Time, state.RotationCaches[curveIdx], false) * normWeight;

						if(value.Dot(outLocalPose.Rotations[k]) < 0.0f)
							value = -value;

						outLocalPose.Rotations[k] += value;
						outLocalPose.HasOverride[k] = false;
						hasAnimCurve[k] = true;
					}
				}
			}
		}
	}

	// Apply default local tranform to non-animated bones (so that any potential child bones are transformed properly)
	for(u32 i = 0; i < mNumBones; i++)
	{
		if(hasAnimCurve[i])
			continue;

		outLocalPose.Positions[i] = mBoneTransforms[i].GetPosition();
		outLocalPose.Rotations[i] = mBoneTransforms[i].GetRotation();
		outLocalPose.Scales[i] = mBoneTransforms[i].GetScale();
	}

	// Calculate local pose matrices
	u32 isGlobalBytes = sizeof(bool) * mNumBones;
	bool* isGlobal = (bool*)B3DStackAllocate(isGlobalBytes);
	memset(isGlobal, 0, isGlobalBytes);

	for(u32 i = 0; i < mNumBones; i++)
	{
		bool isAssigned = outLocalPose.Rotations[i].W != 0.0f;
		if(!isAssigned)
			outLocalPose.Rotations[i] = Quaternion::kIdentity;
		else
			outLocalPose.Rotations[i].Normalize();

		if(outLocalPose.HasOverride[i])
		{
			isGlobal[i] = true;
			continue;
		}

		outPose[i] = Matrix4::TRS(outLocalPose.Positions[i], outLocalPose.Rotations[i], outLocalPose.Scales[i]);
	}

	// Calculate global poses
	// Note: For a possible performance improvement consider sorting bones in such order so that parents (and overrides)
	// always come before children, we no isGlobal check is needed.
	std::function<void(u32)> fnCalcGlobal = [&](u32 boneIndex)
	{
		u32 parentBoneIndex = mBoneInfo[boneIndex].Parent;
		if(parentBoneIndex == (u32)-1)
		{
			isGlobal[boneIndex] = true;
			return;
		}

		if(!isGlobal[parentBoneIndex])
			fnCalcGlobal(parentBoneIndex);

		outPose[boneIndex] = outPose[parentBoneIndex] * outPose[boneIndex];
		isGlobal[boneIndex] = true;
	};

	for(u32 i = 0; i < mNumBones; i++)
	{
		if(!isGlobal[i])
			fnCalcGlobal(i);
	}

	for(u32 i = 0; i < mNumBones; i++)
		outPose[i] = outPose[i] * mInvBindPoses[i];

	B3DStackFree(isGlobal);
	B3DStackFree(hasAnimCurve);
}

Transform Skeleton::CalcBoneTransform(u32 index) const
{
	if(index >= mNumBones)
		return Transform::kIdentity;

	Transform output = mBoneTransforms[index];

	u32 parentIndex = mBoneInfo[index].Parent;
	while(parentIndex != (u32)-1)
	{
		output.MakeWorld(mBoneTransforms[parentIndex]);

		parentIndex = mBoneInfo[parentIndex].Parent;
	}

	return output;
}

u32 Skeleton::GetRootBoneIndex() const
{
	for(u32 i = 0; i < mNumBones; i++)
	{
		if(mBoneInfo[i].Parent == (u32)-1)
			return i;
	}

	return (u32)-1;
}

SPtr<Skeleton> Skeleton::CreateEmpty()
{
	Skeleton* rawPtr = new(B3DAllocate<Skeleton>()) Skeleton();

	SPtr<Skeleton> newSkeleton = B3DMakeSharedFromExisting<Skeleton>(rawPtr);
	return newSkeleton;
}

RTTIType* Skeleton::GetRttiStatic()
{
	return SkeletonRTTI::Instance();
}

RTTIType* Skeleton::GetRtti() const
{
	return GetRttiStatic();
}
