//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Scene/BsTransform.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class BS_CORE_EXPORT TransformRTTI : public RTTIType<Transform, IReflectable, TransformRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(mPosition, 0)
			BS_RTTI_MEMBER_PLAIN(mRotation, 1)
			BS_RTTI_MEMBER_PLAIN(mScale, 2)
		BS_END_RTTI_MEMBERS

	public:
		TransformRTTI()
			:mInitMembers(this)
		{ }

		const String& getRTTIName() override
		{
			static String name = "Transform";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_Transform;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<Transform>();
		}
	};

	/** @} */
	/** @endcond */
}
