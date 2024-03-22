//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Reflection/BsRTTIPlain.h"
#include "Scene/BsTransform.h"
#include "RTTI/BsMathRTTI.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class B3D_CORE_EXPORT TransformRTTI : public RTTIType<Transform, IReflectable, TransformRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN(mPosition, 0)
			B3D_RTTI_MEMBER_PLAIN(mRotation, 1)
			B3D_RTTI_MEMBER_PLAIN(mScale, 2)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName()
		{
			static String name = "Transform";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_Transform;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<Transform>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
