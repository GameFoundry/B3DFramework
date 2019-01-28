//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "foundation/PxVec3.h"
#include "foundation/PxVec4.h"
#include "foundation/PxQuat.h"
#include "foundation/PxTransform.h"

namespace bs
{
/** @addtogroup Plugins
 *  @{
 */

/** @defgroup PhysX bsfPhysX
 *	NVIDIA %PhysX implementation of framework's physics.
 *  @{
 */

/** @cond RTTI */
/** @defgroup RTTI-Impl-PhysX RTTI types
 *  Types containing RTTI for specific classes.
 */
/** @endcond */

/** @} */
/** @} */

	class PhysXRigidbody;
	class PhsyXMaterial;
	class FPhysXCollider;

	/** @addtogroup PhysX
	 *  @{
	 */

	/**	Type IDs used by the RTTI system for the PhysX library. */
	enum TypeID_PhysX
	{
		TID_FPhysXMesh = 100000,
	};

	/** Converts a framework vector to a PhysX vector. */
	inline const physx::PxVec3& toPxVector(const Vector3& input)
	{
		return *(physx::PxVec3*)&input;
	}

	/** Converts a framework vector to a PhysX vector. */
	inline const physx::PxVec4& toPxVector(const Vector4& input)
	{
		return *(physx::PxVec4*)&input;
	}

	/** Converts a framework quaternion to a PhysX quaternion. */
	inline const physx::PxQuat& toPxQuaternion(const Quaternion& input)
	{
		return *(physx::PxQuat*)&input;
	}

	/** Converts a framework position/rotation pair to a PhysX transform. */
	inline physx::PxTransform toPxTransform(const Vector3& pos, const Quaternion& rot)
	{
		return physx::PxTransform(toPxVector(pos), toPxQuaternion(rot));
	}

	/** Converts a PhysX vector to framework's vector. */
	inline const Vector3& fromPxVector(const physx::PxVec3& input)
	{
		return *(Vector3*)&input;
	}

	/** Converts a PhysX vector to framework's vector. */
	inline const Vector4& fromPxVector(const physx::PxVec4& input)
	{
		return *(Vector4*)&input;
	}

	/** Converts a PhysX quaternion to framework's quaternion. */
	inline const Quaternion& fromPxQuaternion(const physx::PxQuat& input)
	{
		return *(Quaternion*)&input;
	}

	/** Flags used on PhysX shape filters. */
	enum class PhysXObjectFilterFlag
	{
		NoReport = 1 << 0, /**< Don't report collision events. */
		ReportBasic = 1 << 1, /**< Report start/begin collision events. */
		ReportAll = 1 << 2, /**< Report start/begin, as well as persistant collision events. */
		CCD = 1 << 3 /**< Use continous collision detection for this shape. */
	};

	/** @copydoc PhysXObjectFilterFlag */
	typedef Flags<PhysXObjectFilterFlag> PhysXObjectFilterFlags;
	BS_FLAGS_OPERATORS(PhysXObjectFilterFlag)

	/** @} */
}