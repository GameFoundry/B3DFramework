//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Animation/BsAnimationClip.h"
#include "Resources/BsResources.h"
#include "Animation/BsSkeleton.h"
#include "Private/RTTI/BsAnimationClipRTTI.h"

namespace bs
{
	void AnimationCurves::addPositionCurve(const String& name, const TAnimationCurve<Vector3>& curve)
	{
		auto iterFind = std::find_if(position.begin(), position.end(), [&](auto x) { return x.name == name; });

		if (iterFind != position.end())
			iterFind->curve = curve;
		else
			position.push_back({ name, AnimationCurveFlags(), curve });
	}

	void AnimationCurves::addRotationCurve(const String& name, const TAnimationCurve<Quaternion>& curve)
	{
		auto iterFind = std::find_if(rotation.begin(), rotation.end(), [&](auto x) { return x.name == name; });

		if (iterFind != rotation.end())
			iterFind->curve = curve;
		else
			rotation.push_back({ name, AnimationCurveFlags(), curve });
	}

	void AnimationCurves::addScaleCurve(const String& name, const TAnimationCurve<Vector3>& curve)
	{
		auto iterFind = std::find_if(scale.begin(), scale.end(), [&](auto x) { return x.name == name; });

		if (iterFind != scale.end())
			iterFind->curve = curve;
		else
			scale.push_back({ name, AnimationCurveFlags(), curve });
	}

	void AnimationCurves::addGenericCurve(const String& name, const TAnimationCurve<float>& curve)
	{
		auto iterFind = std::find_if(generic.begin(), generic.end(), [&](auto x) { return x.name == name; });

		if (iterFind != generic.end())
			iterFind->curve = curve;
		else
			generic.push_back({ name, AnimationCurveFlags(), curve });
	}

	void AnimationCurves::removePositionCurve(const String& name)
	{
		auto iterFind = std::find_if(position.begin(), position.end(), [&](auto x) { return x.name == name; });

		if (iterFind != position.end())
			position.erase(iterFind);
	}

	void AnimationCurves::removeRotationCurve(const String& name)
	{
		auto iterFind = std::find_if(rotation.begin(), rotation.end(), [&](auto x) { return x.name == name; });

		if (iterFind != rotation.end())
			rotation.erase(iterFind);
	}

	void AnimationCurves::removeScaleCurve(const String& name)
	{
		auto iterFind = std::find_if(scale.begin(), scale.end(), [&](auto x) { return x.name == name; });

		if (iterFind != scale.end())
			scale.erase(iterFind);
	}

	void AnimationCurves::removeGenericCurve(const String& name)
	{
		auto iterFind = std::find_if(generic.begin(), generic.end(), [&](auto x) { return x.name == name; });

		if (iterFind != generic.end())
			generic.erase(iterFind);
	}

	AnimationClip::AnimationClip()
		: Resource(false), mVersion(0), mCurves(bs_shared_ptr_new<AnimationCurves>())
		, mRootMotion(bs_shared_ptr_new<RootMotion>()), mIsAdditive(false), mLength(0.0f), mSampleRate(1)
	{

	}

	AnimationClip::AnimationClip(const SPtr<AnimationCurves>& curves, bool isAdditive, UINT32 sampleRate, 
		const SPtr<RootMotion>& rootMotion)
		: Resource(false), mVersion(0), mCurves(curves), mRootMotion(rootMotion), mIsAdditive(isAdditive), mLength(0.0f)
		, mSampleRate(sampleRate)
	{
		if (mCurves == nullptr)
			mCurves = bs_shared_ptr_new<AnimationCurves>();

		if (mRootMotion == nullptr)
			mRootMotion = bs_shared_ptr_new<RootMotion>();

		buildNameMapping();
		calculateLength();
	}

	HAnimationClip AnimationClip::create(bool isAdditive)
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

	SPtr<AnimationClip> AnimationClip::createEmpty()
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
		newClip->initialize();

		return newClip;
	}

	void AnimationClip::setCurves(const AnimationCurves& curves)
	{
		*mCurves = curves;

		buildNameMapping();
		calculateLength();
		mVersion++;
	}

	bool AnimationClip::hasRootMotion() const
	{
		return mRootMotion != nullptr && 
			(mRootMotion->position.getNumKeyFrames() > 0 || mRootMotion->rotation.getNumKeyFrames() > 0);
	}

	void AnimationClip::calculateLength()
	{
		mLength = 0.0f;

		for (auto& entry : mCurves->position)
			mLength = std::max(mLength, entry.curve.getLength());

		for (auto& entry : mCurves->rotation)
			mLength = std::max(mLength, entry.curve.getLength());

		for (auto& entry : mCurves->scale)
			mLength = std::max(mLength, entry.curve.getLength());

		for (auto& entry : mCurves->generic)
			mLength = std::max(mLength, entry.curve.getLength());
	}

	void AnimationClip::buildNameMapping()
	{
		mNameMapping.clear();

		auto registerEntries = [&](auto& curve, CurveType type)
		{
			UINT32 typeIdx = (UINT32)type;

			for (UINT32 i = 0; i < (UINT32)curve.size(); i++)
			{
				auto& entry = curve[i];

				auto iterFind = mNameMapping.find(entry.name);
				if (iterFind == mNameMapping.end())
				{
					UINT32* indices = mNameMapping[entry.name];
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
				if (entry.flags.isSet(AnimationCurveFlag::MorphFrame))
					typeIdx = (UINT32)CurveType::MorphFrame;
				else if (entry.flags.isSet(AnimationCurveFlag::MorphWeight))
					typeIdx = (UINT32)CurveType::MorphWeight;
				else
					typeIdx = (UINT32)CurveType::Generic;

				auto iterFind = mNameMapping.find(entry.name);
				if (iterFind == mNameMapping.end())
				{
					UINT32* indices = mNameMapping[entry.name];
					memset(indices, -1, sizeof(UINT32) * (int)CurveType::Count);

					indices[typeIdx] = i;
				}
				else
					mNameMapping[entry.name][typeIdx] = i;
			}
		}
	}

	void AnimationClip::initialize()
	{
		buildNameMapping();

		Resource::initialize();
	}

	void AnimationClip::getBoneMapping(const Skeleton& skeleton, AnimationCurveMapping* mapping) const
	{
		UINT32 numBones = skeleton.getNumBones();
		for(UINT32 i = 0; i < numBones; i++)
		{
			const SkeletonBoneInfo& boneInfo = skeleton.getBoneInfo(i);

			getCurveMapping(boneInfo.name, mapping[i]);
		}
	}

	void AnimationClip::getCurveMapping(const String& name, AnimationCurveMapping& mapping) const
	{
		auto iterFind = mNameMapping.find(name);
		if (iterFind != mNameMapping.end())
		{
			const UINT32* indices = iterFind->second;

			mapping.position = indices[(UINT32)CurveType::Position];
			mapping.rotation = indices[(UINT32)CurveType::Rotation];
			mapping.scale = indices[(UINT32)CurveType::Scale];
		}
		else
			mapping = { (UINT32)-1, (UINT32)-1, (UINT32)-1 };
	}

	void AnimationClip::getMorphMapping(const String& name, UINT32& frameIdx, UINT32& weightIdx) const
	{
		auto iterFind = mNameMapping.find(name);
		if (iterFind != mNameMapping.end())
		{
			const UINT32* indices = iterFind->second;

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
		return AnimationClipRTTI::instance();
	}

	RTTITypeBase* AnimationClip::getRTTI() const
	{
		return getRTTIStatic();
	}
}