//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsRTTIType.h"
#include "BsCAnimation.h"
#include "BsGameObjectRTTI.h"

namespace BansheeEngine
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class BS_EXPORT CAnimationRTTI : public RTTIType<CAnimation, Component, CAnimationRTTI>
	{
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_REFL(mDefaultClip, 0)
			BS_RTTI_MEMBER_PLAIN(mWrapMode, 1)
			BS_RTTI_MEMBER_PLAIN(mSpeed, 2)
			BS_RTTI_MEMBER_PLAIN(mEnableCull, 3)
			BS_RTTI_MEMBER_PLAIN(mUseBounds, 4)
			BS_RTTI_MEMBER_PLAIN(mBounds, 5)
		BS_END_RTTI_MEMBERS
	public:
		CAnimationRTTI()
			:mInitMembers(this)
		{ }

		const String& getRTTIName() override
		{
			static String name = "CAnimation";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_CAnimation;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return GameObjectRTTI::createGameObject<CAnimation>();
		}
	};

	/** @} */
	/** @endcond */
}