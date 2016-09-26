//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsRTTIType.h"
#include "BsCBone.h"
#include "BsGameObjectRTTI.h"

namespace BansheeEngine
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class BS_EXPORT CBoneRTTI : public RTTIType<CBone, Component, CBoneRTTI>
	{
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(mBoneName, 0)
		BS_END_RTTI_MEMBERS
	public:
		CBoneRTTI()
			:mInitMembers(this)
		{ }

		const String& getRTTIName() override
		{
			static String name = "CBone";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_CBone;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return GameObjectRTTI::createGameObject<CBone>();
		}
	};

	/** @} */
	/** @endcond */
}