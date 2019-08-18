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
#include "Math/BsVector2I.h"
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

	BS_ALLOW_MEMCPY_SERIALIZATION(AABox);
	BS_ALLOW_MEMCPY_SERIALIZATION(Bounds);
	BS_ALLOW_MEMCPY_SERIALIZATION(Degree);
	BS_ALLOW_MEMCPY_SERIALIZATION(Radian);
	BS_ALLOW_MEMCPY_SERIALIZATION(Matrix3);
	BS_ALLOW_MEMCPY_SERIALIZATION(Matrix4);
	BS_ALLOW_MEMCPY_SERIALIZATION(Quaternion);
	BS_ALLOW_MEMCPY_SERIALIZATION(Plane);
	BS_ALLOW_MEMCPY_SERIALIZATION(Rect2);
	BS_ALLOW_MEMCPY_SERIALIZATION(Sphere);
	BS_ALLOW_MEMCPY_SERIALIZATION(Vector2);
	BS_ALLOW_MEMCPY_SERIALIZATION(Vector2I);
	BS_ALLOW_MEMCPY_SERIALIZATION(Vector3);
	BS_ALLOW_MEMCPY_SERIALIZATION(Vector3I);
	BS_ALLOW_MEMCPY_SERIALIZATION(Vector4);
	BS_ALLOW_MEMCPY_SERIALIZATION(Vector4I);

	/** @} */
	/** @endcond */
}
