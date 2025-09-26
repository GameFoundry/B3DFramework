//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "Reflection/B3DRTTIPlain.h"
#include "Scene/B3DTransform.h"
#include "RTTI/B3DMathRTTI.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT TransformRTTI : public TRTTIType<Transform, IReflectable, TransformRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(mPosition, 0)
			B3D_RTTI_MEMBER(mRotation, 1)
			B3D_RTTI_MEMBER(mScale, 2)
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
} // namespace b3d
