//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsRTTIType.h"
#include "BsAnimationClip.h"
#include "BsAnimationCurveRTTI.h"

namespace BansheeEngine
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class BS_CORE_EXPORT AnimationClipRTTI : public RTTIType <AnimationClip, Resource, AnimationClipRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN_NAMED(positionCurves, mCurves->position, 0)
			BS_RTTI_MEMBER_PLAIN_NAMED(rotationCurves, mCurves->rotation, 1)
			BS_RTTI_MEMBER_PLAIN_NAMED(scaleCurves, mCurves->scale, 2)
			BS_RTTI_MEMBER_PLAIN_NAMED(genericCurves, mCurves->generic, 3)
			BS_RTTI_MEMBER_PLAIN(mIsAdditive, 4)
		BS_END_RTTI_MEMBERS
	public:
		AnimationClipRTTI()
			:mInitMembers(this)
		{
			
		}

		void onDeserializationEnded(IReflectable* obj, const UnorderedMap<String, UINT64>& params) override
		{
			AnimationClip* clip = static_cast<AnimationClip*>(obj);
			clip->initialize();
		}

		const String& getRTTIName() override
		{
			static String name = "AnimationClip";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_AnimationClip;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return AnimationClip::createEmpty();
		}
	};

	/** @} */
	/** @endcond */
}