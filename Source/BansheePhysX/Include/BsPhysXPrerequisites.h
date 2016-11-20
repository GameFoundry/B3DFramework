//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "foundation\PxVec3.h"
#include "foundation\PxVec4.h"
#include "foundation\PxQuat.h"
#include "foundation\PxTransform.h"

namespace bs
{
#if (BS_PLATFORM == BS_PLATFORM_WIN32) && !defined(BS_STATIC_LIB)
#	ifdef BS_PHYSX_EXPORTS
#		define BS_PHYSX_EXPORT __declspec(dllexport)
#	else
#       if defined( __MINGW32__ )
#           define BS_PHYSX_EXPORT
#       else
#    		define BS_PHYSX_EXPORT __declspec(dllimport)
#       endif
#	endif
#else
#	define BS_PHYSX_EXPORT
#endif

/** @addtogroup Plugins
 *  @{
 */

/** @defgroup PhysX BansheePhysX
 *	NVIDIA %PhysX implementation of Banshee's physics.
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
	enum TypeID_BansheeEditor
	{
		TID_FPhysXMesh = 100000,
	};

	/** Converts a Banshee vector to a PhysX vector. */
	inline const physx::PxVec3& toPxVector(const Vector3& input)
	{
		return *(physx::PxVec3*)&input;
	}

	/** Converts a Banshee vector to a PhysX vector. */
	inline const physx::PxVec4& toPxVector(const Vector4& input)
	{
		return *(physx::PxVec4*)&input;
	}

	/** Converts a Banshee quaternion to a PhysX quaternion. */
	inline const physx::PxQuat& toPxQuaternion(const Quaternion& input)
	{
		return *(physx::PxQuat*)&input;
	}

	/** Converts a Banshee position/rotation pair to a PhysX transform. */
	inline physx::PxTransform toPxTransform(const Vector3& pos, const Quaternion& rot)
	{
		return physx::PxTransform(toPxVector(pos), toPxQuaternion(rot));
	}

	/** Converts a PhysX vector to a Banshee vector. */
	inline const Vector3& fromPxVector(const physx::PxVec3& input)
	{
		return *(Vector3*)&input;
	}

	/** Converts a PhysX vector to a Banshee vector. */
	inline const Vector4& fromPxVector(const physx::PxVec4& input)
	{
		return *(Vector4*)&input;
	}

	/** Converts a PhysX quaternion to a Banshee quaternion. */
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