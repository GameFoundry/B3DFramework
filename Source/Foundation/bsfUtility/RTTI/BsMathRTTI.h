//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Reflection/BsRTTIPlain.h"
#include "Math/BsAABox.h"
#include "Math/BsBounds.h"
#include "Math/BsDegree.h"
#include "Math/BsRadian.h"
#include "Math/BsMatrix3.h"
#include "Math/BsMatrix4.h"
#include "Math/BsQuaternion.h"
#include "Math/BsPlane.h"
#include "Math/BsRect2.h"
#include "Math/BsSphere.h"
#include "Math/BsVector2.h"
#include "Math/BsVector3.h"
#include "Math/BsVector3I.h"
#include "Math/BsVector4.h"
#include "Math/BsVector4I.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Utility
	 *  @{
	 */

	B3D_ALLOW_MEMCPY_SERIALIZATION(AABox);
	B3D_ALLOW_MEMCPY_SERIALIZATION(Bounds);
	B3D_ALLOW_MEMCPY_SERIALIZATION(Degree);
	B3D_ALLOW_MEMCPY_SERIALIZATION(Radian);
	B3D_ALLOW_MEMCPY_SERIALIZATION(Matrix3);
	B3D_ALLOW_MEMCPY_SERIALIZATION(Matrix4);
	B3D_ALLOW_MEMCPY_SERIALIZATION(Quaternion);
	B3D_ALLOW_MEMCPY_SERIALIZATION(Plane);
	B3D_ALLOW_MEMCPY_SERIALIZATION(Rect2);
	B3D_ALLOW_MEMCPY_SERIALIZATION(Sphere);
	B3D_ALLOW_MEMCPY_SERIALIZATION(Vector2);
	B3D_ALLOW_MEMCPY_SERIALIZATION(Vector2I);
	B3D_ALLOW_MEMCPY_SERIALIZATION(Vector3);
	B3D_ALLOW_MEMCPY_SERIALIZATION(Vector3I);
	B3D_ALLOW_MEMCPY_SERIALIZATION(Vector4);
	B3D_ALLOW_MEMCPY_SERIALIZATION(Vector4I);
	B3D_ALLOW_MEMCPY_SERIALIZATION_WITH_ID(Size2, TID_Size2);
	B3D_ALLOW_MEMCPY_SERIALIZATION_WITH_ID(Size2UI, TID_Size2UI);

	/** @} */
	/** @endcond */
} // namespace bs
