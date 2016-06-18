//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsAnimationClip.h"
#include "BsResources.h"
#include "BsSkeleton.h"
#include "BsAnimationClipRTTI.h"

namespace BansheeEngine
{
	AnimationClip::AnimationClip()
		: Resource(false), mVersion(0), mCurves(bs_shared_ptr_new<AnimationCurves>())
	{

	}

	AnimationClip::AnimationClip(const SPtr<AnimationCurves>& curves)
		: Resource(false), mVersion(0), mCurves(curves)
	{

	}

	HAnimationClip AnimationClip::create()
	{
		SPtr<AnimationClip> newClip = createEmpty();
		newClip->initialize();

		return static_resource_cast<AnimationClip>(gResources()._createResourceHandle(newClip));
	}

	HAnimationClip AnimationClip::create(const SPtr<AnimationCurves>& curves)
	{
		return static_resource_cast<AnimationClip>(gResources()._createResourceHandle(_createPtr(curves)));
	}

	SPtr<AnimationClip> AnimationClip::createEmpty()
	{
		AnimationClip* rawPtr = new (bs_alloc<AnimationClip>()) AnimationClip();

		SPtr<AnimationClip> newClip = bs_core_ptr<AnimationClip>(rawPtr);
		newClip->_setThisPtr(newClip);

		return newClip;
	}

	SPtr<AnimationClip> AnimationClip::_createPtr(const SPtr<AnimationCurves>& curves)
	{
		AnimationClip* rawPtr = new (bs_alloc<AnimationClip>()) AnimationClip(curves);

		SPtr<AnimationClip> newClip = bs_core_ptr<AnimationClip>(rawPtr);
		newClip->_setThisPtr(newClip);
		newClip->initialize();

		return newClip;
	}

	void AnimationClip::addPositionCurve(const String& name, const TAnimationCurve<Vector3>& curve)
	{
		SPtr<AnimationCurves> newCurves = bs_shared_ptr_new<AnimationCurves>();
		newCurves->rotation = mCurves->rotation;
		newCurves->scale = mCurves->scale;
		newCurves->generic = mCurves->generic;

		for(auto& entry : mCurves->position)
		{
			if (entry.name != name)
				newCurves->position.push_back(entry);
		}

		newCurves->position.push_back({ name, curve });
		mCurves = newCurves;

		buildNameMapping();
		mVersion++;
	}

	void AnimationClip::addRotationCurve(const String& name, const TAnimationCurve<Quaternion>& curve)
	{
		SPtr<AnimationCurves> newCurves = bs_shared_ptr_new<AnimationCurves>();
		newCurves->position = mCurves->position;
		newCurves->scale = mCurves->scale;
		newCurves->generic = mCurves->generic;

		for (auto& entry : mCurves->rotation)
		{
			if (entry.name != name)
				newCurves->rotation.push_back(entry);
		}

		newCurves->rotation.push_back({ name, curve });
		mCurves = newCurves;

		buildNameMapping();
		mVersion++;
	}

	void AnimationClip::addScaleCurve(const String& name, const TAnimationCurve<Vector3>& curve)
	{
		SPtr<AnimationCurves> newCurves = bs_shared_ptr_new<AnimationCurves>();
		newCurves->position = mCurves->position;
		newCurves->rotation = mCurves->rotation;
		newCurves->generic = mCurves->generic;

		for (auto& entry : mCurves->scale)
		{
			if (entry.name != name)
				newCurves->scale.push_back(entry);
		}

		newCurves->scale.push_back({ name, curve });
		mCurves = newCurves;

		buildNameMapping();
		mVersion++;
	}

	void AnimationClip::addGenericCurve(const String& name, const TAnimationCurve<float>& curve)
	{
		SPtr<AnimationCurves> newCurves = bs_shared_ptr_new<AnimationCurves>();
		newCurves->position = mCurves->position;
		newCurves->rotation = mCurves->rotation;
		newCurves->scale = mCurves->scale;

		for (auto& entry : mCurves->generic)
		{
			if (entry.name != name)
				newCurves->generic.push_back(entry);
		}

		mCurves = newCurves;

		buildNameMapping();
		mVersion++;
	}

	void AnimationClip::removePositionCurve(const String& name)
	{
		SPtr<AnimationCurves> newCurves = bs_shared_ptr_new<AnimationCurves>();
		newCurves->rotation = mCurves->rotation;
		newCurves->scale = mCurves->scale;
		newCurves->generic = mCurves->generic;

		for (auto& entry : mCurves->position)
		{
			if (entry.name != name)
				newCurves->position.push_back(entry);
		}

		mCurves = newCurves;

		buildNameMapping();
		mVersion++;
	}

	void AnimationClip::removeRotationCurve(const String& name)
	{
		SPtr<AnimationCurves> newCurves = bs_shared_ptr_new<AnimationCurves>();
		newCurves->position = mCurves->position;
		newCurves->scale = mCurves->scale;
		newCurves->generic = mCurves->generic;

		for (auto& entry : mCurves->rotation)
		{
			if (entry.name != name)
				newCurves->rotation.push_back(entry);
		}

		mCurves = newCurves;

		buildNameMapping();
		mVersion++;
	}

	void AnimationClip::removeScaleCurve(const String& name)
	{
		SPtr<AnimationCurves> newCurves = bs_shared_ptr_new<AnimationCurves>();
		newCurves->position = mCurves->position;
		newCurves->rotation = mCurves->rotation;
		newCurves->generic = mCurves->generic;

		for (auto& entry : mCurves->scale)
		{
			if (entry.name != name)
				newCurves->scale.push_back(entry);
		}

		mCurves = newCurves;

		buildNameMapping();
		mVersion++;
	}

	void AnimationClip::removeGenericCurve(const String& name)
	{
		SPtr<AnimationCurves> newCurves = bs_shared_ptr_new<AnimationCurves>();
		newCurves->position = mCurves->position;
		newCurves->rotation = mCurves->rotation;
		newCurves->scale = mCurves->scale;

		for (auto& entry : mCurves->generic)
		{
			if (entry.name != name)
				newCurves->generic.push_back(entry);
		}

		mCurves = newCurves;

		buildNameMapping();
		mVersion++;
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
					memset(indices, -1, sizeof(UINT32) * 4);

					indices[typeIdx] = i;
				}
				else
					mNameMapping[entry.name][typeIdx] = i;
			}
		};

		registerEntries(mCurves->position, CurveType::Position);
		registerEntries(mCurves->rotation, CurveType::Rotation);
		registerEntries(mCurves->scale, CurveType::Scale);
		registerEntries(mCurves->generic, CurveType::Generic);
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

			auto iterFind = mNameMapping.find(boneInfo.name);
			if(iterFind != mNameMapping.end())
			{
				const UINT32* indices = iterFind->second;

				mapping[i].position = indices[(UINT32)CurveType::Position];
				mapping[i].rotation = indices[(UINT32)CurveType::Rotation];
				mapping[i].scale = indices[(UINT32)CurveType::Scale];
			}
			else
				mapping[i] = { (UINT32)-1, (UINT32)-1, (UINT32)-1 };
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