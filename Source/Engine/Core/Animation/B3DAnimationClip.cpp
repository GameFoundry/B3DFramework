//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Animation/B3DAnimationClip.h"
#include "Resources/B3DResources.h"
#include "Animation/B3DSkeleton.h"
#include "RTTI/B3DAnimationClipRTTI.h"

using namespace b3d;

void AnimationCurves::AddPositionCurve(const String& name, const TAnimationCurve<Vector3>& curve)
{
	auto found = std::find_if(Position.begin(), Position.end(), [&](auto x)
								 { return x.Name == name; });

	if(found != Position.end())
		found->Curve = curve;
	else
		Position.push_back({ name, AnimationCurveFlags(), curve });
}

void AnimationCurves::AddRotationCurve(const String& name, const TAnimationCurve<Quaternion>& curve)
{
	auto found = std::find_if(Rotation.begin(), Rotation.end(), [&](auto x)
								 { return x.Name == name; });

	if(found != Rotation.end())
		found->Curve = curve;
	else
		Rotation.push_back({ name, AnimationCurveFlags(), curve });
}

void AnimationCurves::AddScaleCurve(const String& name, const TAnimationCurve<Vector3>& curve)
{
	auto found = std::find_if(Scale.begin(), Scale.end(), [&](auto x)
								 { return x.Name == name; });

	if(found != Scale.end())
		found->Curve = curve;
	else
		Scale.push_back({ name, AnimationCurveFlags(), curve });
}

void AnimationCurves::AddGenericCurve(const String& name, const TAnimationCurve<float>& curve)
{
	auto found = std::find_if(Generic.begin(), Generic.end(), [&](auto x)
								 { return x.Name == name; });

	if(found != Generic.end())
		found->Curve = curve;
	else
		Generic.push_back({ name, AnimationCurveFlags(), curve });
}

void AnimationCurves::RemovePositionCurve(const String& name)
{
	auto found = std::find_if(Position.begin(), Position.end(), [&](auto x)
								 { return x.Name == name; });

	if(found != Position.end())
		Position.erase(found);
}

void AnimationCurves::RemoveRotationCurve(const String& name)
{
	auto found = std::find_if(Rotation.begin(), Rotation.end(), [&](auto x)
								 { return x.Name == name; });

	if(found != Rotation.end())
		Rotation.erase(found);
}

void AnimationCurves::RemoveScaleCurve(const String& name)
{
	auto found = std::find_if(Scale.begin(), Scale.end(), [&](auto x)
								 { return x.Name == name; });

	if(found != Scale.end())
		Scale.erase(found);
}

void AnimationCurves::RemoveGenericCurve(const String& name)
{
	auto found = std::find_if(Generic.begin(), Generic.end(), [&](auto x)
								 { return x.Name == name; });

	if(found != Generic.end())
		Generic.erase(found);
}

AnimationClip::AnimationClip()
	: Resource(false), mVersion(0), mCurves(B3DMakeShared<AnimationCurves>()), mRootMotion(B3DMakeShared<RootMotion>()), mIsAdditive(false), mLength(0.0f), mSampleRate(1)
{
}

AnimationClip::AnimationClip(const TShared<AnimationCurves>& curves, bool isAdditive, u32 sampleRate, const TShared<RootMotion>& rootMotion)
	: Resource(false), mVersion(0), mCurves(curves), mRootMotion(rootMotion), mIsAdditive(isAdditive), mLength(0.0f), mSampleRate(sampleRate)
{
	if(mCurves == nullptr)
		mCurves = B3DMakeShared<AnimationCurves>();

	if(mRootMotion == nullptr)
		mRootMotion = B3DMakeShared<RootMotion>();

	BuildNameMapping();
	CalculateLength();
}

HAnimationClip AnimationClip::Create(bool isAdditive)
{
	return B3DStaticResourceCast<AnimationClip>(GetResources().CreateResourceHandle(
		CreatePtrInternal(B3DMakeShared<AnimationCurves>(), isAdditive)));
}

HAnimationClip AnimationClip::Create(const TShared<AnimationCurves>& curves, bool isAdditive, u32 sampleRate, const TShared<RootMotion>& rootMotion)
{
	return B3DStaticResourceCast<AnimationClip>(GetResources().CreateResourceHandle(
		CreatePtrInternal(curves, isAdditive, sampleRate, rootMotion)));
}

TShared<AnimationClip> AnimationClip::CreateEmpty()
{
	AnimationClip* rawPtr = new(B3DAllocate<AnimationClip>()) AnimationClip();

	TShared<AnimationClip> newClip = B3DMakeSharedFromExisting<AnimationClip>(rawPtr);
	newClip->SetShared(newClip);

	return newClip;
}

TShared<AnimationClip> AnimationClip::CreatePtrInternal(const TShared<AnimationCurves>& curves, bool isAdditive, u32 sampleRate, const TShared<RootMotion>& rootMotion)
{
	AnimationClip* rawPtr = new(B3DAllocate<AnimationClip>()) AnimationClip(curves, isAdditive, sampleRate, rootMotion);

	TShared<AnimationClip> newClip = B3DMakeSharedFromExisting<AnimationClip>(rawPtr);
	newClip->SetShared(newClip);
	newClip->Initialize();

	return newClip;
}

void AnimationClip::SetCurves(const AnimationCurves& curves)
{
	*mCurves = curves;

	BuildNameMapping();
	CalculateLength();
	mVersion++;
}

bool AnimationClip::HasRootMotion() const
{
	return mRootMotion != nullptr &&
		(mRootMotion->Position.GetNumKeyFrames() > 0 || mRootMotion->Rotation.GetNumKeyFrames() > 0);
}

void AnimationClip::CalculateLength()
{
	mLength = 0.0f;

	for(auto& entry : mCurves->Position)
		mLength = std::max(mLength, entry.Curve.GetLength());

	for(auto& entry : mCurves->Rotation)
		mLength = std::max(mLength, entry.Curve.GetLength());

	for(auto& entry : mCurves->Scale)
		mLength = std::max(mLength, entry.Curve.GetLength());

	for(auto& entry : mCurves->Generic)
		mLength = std::max(mLength, entry.Curve.GetLength());
}

void AnimationClip::BuildNameMapping()
{
	mNameMapping.clear();

	auto fnRegisterEntries = [&](auto& curve, CurveType type)
	{
		u32 typeIndex = (u32)type;

		for(u32 curveIndex = 0; curveIndex < (u32)curve.size(); curveIndex++)
		{
			auto& entry = curve[curveIndex];

			auto found = mNameMapping.find(entry.Name);
			if(found == mNameMapping.end())
			{
				u32* indices = mNameMapping[entry.Name].data();
				memset(indices, -1, sizeof(u32) * (int)CurveType::Count);

				indices[typeIndex] = curveIndex;
			}
			else
				mNameMapping[entry.Name][typeIndex] = curveIndex;
		}
	};

	fnRegisterEntries(mCurves->Position, CurveType::Position);
	fnRegisterEntries(mCurves->Rotation, CurveType::Rotation);
	fnRegisterEntries(mCurves->Scale, CurveType::Scale);

	// Generic and morph curves
	{
		Vector<TNamedAnimationCurve<float>>& curve = mCurves->Generic;
		for(u32 curveIndex = 0; curveIndex < (u32)curve.size(); curveIndex++)
		{
			auto& entry = curve[curveIndex];

			u32 typeIndex;
			if(entry.Flags.IsSet(AnimationCurveFlag::MorphFrame))
				typeIndex = (u32)CurveType::MorphFrame;
			else if(entry.Flags.IsSet(AnimationCurveFlag::MorphWeight))
				typeIndex = (u32)CurveType::MorphWeight;
			else
				typeIndex = (u32)CurveType::Generic;

			auto found = mNameMapping.find(entry.Name);
			if(found == mNameMapping.end())
			{
				u32* indices = mNameMapping[entry.Name].data();
				memset(indices, -1, sizeof(u32) * (int)CurveType::Count);

				indices[typeIndex] = curveIndex;
			}
			else
				mNameMapping[entry.Name][typeIndex] = curveIndex;
		}
	}
}

void AnimationClip::Initialize()
{
	BuildNameMapping();

	Resource::Initialize();
}

void AnimationClip::GetBoneMapping(const Skeleton& skeleton, AnimationCurveMapping* outMapping) const
{
	u32 boneCount = skeleton.GetBoneCount();
	for(u32 boneIndex = 0; boneIndex < boneCount; boneIndex++)
	{
		const SkeletonBoneInfo& boneInfo = skeleton.GetBoneInfo(boneIndex);

		GetCurveMapping(boneInfo.Name, outMapping[boneIndex]);
	}
}

void AnimationClip::GetCurveMapping(const String& name, AnimationCurveMapping& outMapping) const
{
	auto found = mNameMapping.find(name);
	if(found != mNameMapping.end())
	{
		const u32* indices = found->second.data();

		outMapping.Position = indices[(u32)CurveType::Position];
		outMapping.Rotation = indices[(u32)CurveType::Rotation];
		outMapping.Scale = indices[(u32)CurveType::Scale];
	}
	else
		outMapping = { (u32)-1, (u32)-1, (u32)-1 };
}

void AnimationClip::GetMorphMapping(const String& name, u32& outFrameIndex, u32& outWeightIndex) const
{
	auto found = mNameMapping.find(name);
	if(found != mNameMapping.end())
	{
		const u32* indices = found->second.data();

		outFrameIndex = indices[(u32)CurveType::MorphFrame];
		outWeightIndex = indices[(u32)CurveType::MorphWeight];
	}
	else
	{
		outFrameIndex = (u32)-1;
		outWeightIndex = (u32)-1;
	}
}

RTTIType* AnimationClip::GetRttiStatic()
{
	return AnimationClipRTTI::Instance();
}

RTTIType* AnimationClip::GetRtti() const
{
	return GetRttiStatic();
}
