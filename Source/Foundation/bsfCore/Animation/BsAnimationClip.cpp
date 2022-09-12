//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Animation/BsAnimationClip.h"
#include "Resources/BsResources.h"
#include "Animation/BsSkeleton.h"
#include "Private/RTTI/BsAnimationClipRTTI.h"

namespace bs
{
	void AnimationCurves::AddPositionCurve(const String& name, const TAnimationCurve<Vector3>& curve)
	{
		auto iterFind = std::find_if(position.begin(), position.end(), [&](auto x) { return x.name == name; });

		if (iterFind != position.end())
			iterFind->curve = curve;
		else
			position.push_back({ name, AnimationCurveFlags(), curve });
	}

	void AnimationCurves::AddRotationCurve(const String& name, const TAnimationCurve<Quaternion>& curve)
	{
		auto iterFind = std::find_if(rotation.begin(), rotation.end(), [&](auto x) { return x.name == name; });

		if (iterFind != rotation.end())
			iterFind->curve = curve;
		else
			rotation.push_back({ name, AnimationCurveFlags(), curve });
	}

	void AnimationCurves::AddScaleCurve(const String& name, const TAnimationCurve<Vector3>& curve)
	{
		auto iterFind = std::find_if(scale.begin(), scale.end(), [&](auto x) { return x.name == name; });

		if (iterFind != scale.end())
			iterFind->curve = curve;
		else
			scale.push_back({ name, AnimationCurveFlags(), curve });
	}

	void AnimationCurves::AddGenericCurve(const String& name, const TAnimationCurve<float>& curve)
	{
		auto iterFind = std::find_if(generic.begin(), generic.end(), [&](auto x) { return x.name == name; });

		if (iterFind != generic.end())
			iterFind->curve = curve;
		else
			generic.push_back({ name, AnimationCurveFlags(), curve });
	}

	void AnimationCurves::RemovePositionCurve(const String& name)
	{
		auto iterFind = std::find_if(position.begin(), position.end(), [&](auto x) { return x.name == name; });

		if (iterFind != position.end())
			position.erase(iterFind);
	}

	void AnimationCurves::RemoveRotationCurve(const String& name)
	{
		auto iterFind = std::find_if(rotation.begin(), rotation.end(), [&](auto x) { return x.name == name; });

		if (iterFind != rotation.end())
			rotation.erase(iterFind);
	}

	void AnimationCurves::RemoveScaleCurve(const String& name)
	{
		auto iterFind = std::find_if(scale.begin(), scale.end(), [&](auto x) { return x.name == name; });

		if (iterFind != scale.end())
			scale.erase(iterFind);
	}

	void AnimationCurves::RemoveGenericCurve(const String& name)
	{
		auto iterFind = std::find_if(generic.begin(), generic.end(), [&](auto x) { return x.name == name; });

		if (iterFind != generic.end())
			generic.erase(iterFind);
	}

	AnimationClip::AnimationClip()
		: Resource(false), mVersion(0), mCurves(bs_shared_ptr_new<AnimationCurves>())
		, MRootMotion(bs_shared_ptr_new<RootMotion>()), mIsAdditive(false), mLength(0.0f), mSampleRate(1)
	{

	}

	AnimationClip::AnimationClip(const SPtr<AnimationCurves>& curves, bool isAdditive, UINT32 sampleRate,
		const SPtr<RootMotion>& rootMotion)
		: Resource(false), mVersion(0), mCurves(curves), mRootMotion(rootMotion), mIsAdditive(isAdditive), mLength(0.0f)
		, MSampleRate(sampleRate)
	{
		if (mCurves == nullptr)
			mCurves = bs_shared_ptr_new<AnimationCurves>();

		if (mRootMotion == nullptr)
			mRootMotion = bs_shared_ptr_new<RootMotion>();

		buildNameMapping();
		calculateLength();
	}

	HAnimationClip AnimationClip::Create(bool isAdditive)
	{
		return static_resource_cast<AnimationClip>(gResources()._createResourceHandle(
			_createPtr(bs_shared_ptr_new<AnimationCurves>(), isAdditive)));
	}

	HAnimationClip AnimationClip::create(const SPtr<AnimationCurves>& curves, bool isAdditive, UINT32 sampleRate,
		const SPtr<RootMotion>& rootMotion)
	{
		return static_resource_cast<AnimationClip>(gResources()._createResourceHandle(
			_createPtr(curves, isAdditive, sampleRate, rootMotion)));
	}

	SPtr<AnimationClip> AnimationClip::CreateEmpty()
	{
		AnimationClip* rawPtr = new (bs_alloc<AnimationClip>()) AnimationClip();

		SPtr<AnimationClip> newClip = bs_core_ptr<AnimationClip>(rawPtr);
		newClip->_setThisPtr(newClip);

		return newClip;
	}

	SPtr<AnimationClip> AnimationClip::_createPtr(const SPtr<AnimationCurves>& curves, bool isAdditive, UINT32 sampleRate,
		const SPtr<RootMotion>& rootMotion)
	{
		AnimationClip* rawPtr = new (bs_alloc<AnimationClip>()) AnimationClip(curves, isAdditive, sampleRate, rootMotion);

		SPtr<AnimationClip> newClip = bs_core_ptr<AnimationClip>(rawPtr);
		newClip->_setThisPtr(newClip);
		newClip->Initialize();

		return newClip;
	}

	void AnimationClip::SetCurves(const AnimationCurves& curves)
	{
		*mCurves = curves;

		buildNameMapping();
		calculateLength();
		mVersion++;
	}

	bool AnimationClip::HasRootMotion() const
	{
		return mRootMotion != nullptr &&
			(mRootMotion->position.GetNumKeyFrames() > 0 || mRootMotion->rotation.getNumKeyFrames() > 0);
	}

	void AnimationClip::CalculateLength()
	{
		mLength = 0.0f;

		for (auto& entry : mCurves->position)
			mLength = std::max(mLength, entry.curve.GetLength());

		for (auto& entry : mCurves->rotation)
			mLength = std::max(mLength, entry.curve.GetLength());

		for (auto& entry : mCurves->scale)
			mLength = std::max(mLength, entry.curve.GetLength());

		for (auto& entry : mCurves->generic)
			mLength = std::max(mLength, entry.curve.GetLength());
	}

	void AnimationClip::BuildNameMapping()
	{
		mNameMapping.Clear();

		auto registerEntries = [&](auto& curve, CurveType type)
		{
			UINT32 typeIdx = (UINT32)type;

			for (UINT32 i = 0; i < (UINT32)curve.size(); i++)
			{
				auto& entry = curve[i];

				auto iterFind = mNameMapping.find(entry.name);
				if (iterFind == mNameMapping.end())
				{
					UINT32* indices = mNameMapping[entry.name].Data();
					memset(indices, -1, sizeof(UINT32) * (int)CurveType::Count);

					indices[typeIdx] = i;
				}
				else
					mNameMapping[entry.name][typeIdx] = i;
			}
		};

		registerEntries(mCurves->position, CurveType::Position);
		registerEntries(mCurves->rotation, CurveType::Rotation);
		registerEntries(mCurves->scale, CurveType::Scale);

		// Generic and morph curves
		{
			Vector<TNamedAnimationCurve<float>>& curve = mCurves->generic;
			for (UINT32 i = 0; i < (UINT32)curve.size(); i++)
			{
				auto& entry = curve[i];

				UINT32 typeIdx;
				if (entry.flags.IsSet(AnimationCurveFlag::MorphFrame))
					typeIdx = (UINT32)CurveType::MorphFrame;
				else if (entry.flags.IsSet(AnimationCurveFlag::MorphWeight))
					typeIdx = (UINT32)CurveType::MorphWeight;
				else
					typeIdx = (UINT32)CurveType::Generic;

				auto iterFind = mNameMapping.find(entry.name);
				if (iterFind == mNameMapping.end())
				{
					UINT32* indices = mNameMapping[entry.name].Data();
					memset(indices, -1, sizeof(UINT32) * (int)CurveType::Count);

					indices[typeIdx] = i;
				}
				else
					mNameMapping[entry.name][typeIdx] = i;
			}
		}
	}

	void AnimationClip::Initialize()
	{
		buildNameMapping();

		Resource::initialize();
	}

	void AnimationClip::GetBoneMapping(const Skeleton& skeleton, AnimationCurveMapping* mapping) const
	{
		UINT32 numBones = skeleton.GetNumBones();
		for(UINT32 i = 0; i < numBones; i++)
		{
			const SkeletonBoneInfo& boneInfo = skeleton.GetBoneInfo(i);

			getCurveMapping(boneInfo.name, mapping[i]);
		}
	}

	void AnimationClip::GetCurveMapping(const String& name, AnimationCurveMapping& mapping) const
	{
		auto iterFind = mNameMapping.find(name);
		if (iterFind != mNameMapping.end())
		{
			const UINT32* indices = iterFind->second.Data();

			mapping.position = indices[(UINT32)CurveType::Position];
			mapping.rotation = indices[(UINT32)CurveType::Rotation];
			mapping.scale = indices[(UINT32)CurveType::Scale];
		}
		else
			mapping = { (UINT32)-1, (UINT32)-1, (UINT32)-1 };
	}

	void AnimationClip::GetMorphMapping(const String& name, UINT32& frameIdx, UINT32& weightIdx) const
	{
		auto iterFind = mNameMapping.find(name);
		if (iterFind != mNameMapping.end())
		{
			const UINT32* indices = iterFind->second.Data();

			frameIdx = indices[(UINT32)CurveType::MorphFrame];
			weightIdx = indices[(UINT32)CurveType::MorphWeight];
		}
		else
		{
			frameIdx = (UINT32)-1;
			weightIdx = (UINT32)-1;
		}
	}

	RTTITypeBase* AnimationClip::getRTTIStatic()
	{
		return AnimationClipRTTI::Instance();
	}

	RTTITypeBase* AnimationClip::getRTTI() const
	{
		return GetRTTIStatic();
	}
}
