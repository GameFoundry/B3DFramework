//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Reflection/BsRTTIPlain.h"
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
