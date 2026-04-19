//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Reflection/B3DRTTIPlain.h"
#include "Math/B3DAABox.h"
#include "Math/B3DBounds.h"
#include "Math/B3DDegree.h"
#include "Math/B3DRadian.h"
#include "Math/B3DMatrix3.h"
#include "Math/B3DMatrix4.h"
#include "Math/B3DQuaternion.h"
#include "Math/B3DPlane.h"
#include "Math/B3DArea2.h"
#include "Math/B3DSphere.h"
#include "Math/B3DVector2.h"
#include "Math/B3DVector3.h"
#include "Math/B3DVector3I.h"
#include "Math/B3DVector4.h"
#include "Math/B3DVector4I.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Utility
	 *  @{
	 */

	B3D_ALLOW_MEMCPY_SERIALIZATION(AABox, TID_AABox);
	B3D_ALLOW_MEMCPY_SERIALIZATION(Bounds, TID_Bounds);
	B3D_ALLOW_MEMCPY_SERIALIZATION(Degree, TID_Degree);
	B3D_ALLOW_MEMCPY_SERIALIZATION(Radian, TID_Radian);
	B3D_ALLOW_MEMCPY_SERIALIZATION(Matrix3, TID_Matrix3);
	B3D_ALLOW_MEMCPY_SERIALIZATION(Matrix4, TID_Matrix4);
	B3D_ALLOW_MEMCPY_SERIALIZATION(Quaternion, TID_Quaternion);
	B3D_ALLOW_MEMCPY_SERIALIZATION(Plane, TID_Plane);
	B3D_ALLOW_MEMCPY_SERIALIZATION(Area2, TID_Area2);
	B3D_ALLOW_MEMCPY_SERIALIZATION(Sphere, TID_Sphere);
	B3D_ALLOW_MEMCPY_SERIALIZATION(Vector2, TID_Vector2);
	B3D_ALLOW_MEMCPY_SERIALIZATION(Vector2I, TID_Vector2I);
	B3D_ALLOW_MEMCPY_SERIALIZATION(Vector3, TID_Vector3);
	B3D_ALLOW_MEMCPY_SERIALIZATION(Vector3I, TID_Vector3I);
	B3D_ALLOW_MEMCPY_SERIALIZATION(Vector4, TID_Vector4);
	B3D_ALLOW_MEMCPY_SERIALIZATION(Vector4I, TID_Vector4I);
	B3D_ALLOW_MEMCPY_SERIALIZATION(Size2, TID_Size2);
	B3D_ALLOW_MEMCPY_SERIALIZATION(Size2UI, TID_Size2UI);
	B3D_ALLOW_MEMCPY_SERIALIZATION(Size2I, TID_Size2I);

	/** @} */
	/** @endcond */
} // namespace b3d
