//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsAnimationClip.h"
#include "BsResources.h"
#include "BsAnimationClipRTTI.h"

namespace BansheeEngine
{
	AnimationClip::AnimationClip()
		: Resource(false)
	{
		
	}

	HAnimationClip AnimationClip::create()
	{
		return static_resource_cast<AnimationClip>(gResources()._createResourceHandle(_createPtr()));
	}

	SPtr<AnimationClip> AnimationClip::_createPtr()
	{
		SPtr<AnimationClip> newClip = createEmpty();
		newClip->initialize();

		return newClip;
	}

	SPtr<AnimationClip> AnimationClip::createEmpty()
	{
		AnimationClip* rawPtr = new (bs_alloc<AnimationClip>()) AnimationClip();

		SPtr<AnimationClip> newClip = bs_core_ptr<AnimationClip>(rawPtr);
		newClip->_setThisPtr(newClip);

		return newClip;
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