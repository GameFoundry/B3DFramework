//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"

namespace b3d
{
	/** @addtogroup Plugins
	 *  @{
	 */

	/** @defgroup NullPhysics bsfNullPhysics
	 *	Null implementation of framework's physics.
	 *  @{
	 */

	/** @cond RTTI */
	/** @defgroup RTTI-Impl-NullPhysics RTTI types
	 *  Types containing RTTI for specific classes.
	 */
	/** @endcond */

	/** @} */
	/** @} */

	class NullPhysicsRigidbody;
	class NullPhysicsMaterial;
	class NullPhysicsCollider;

	/** @addtogroup NullPhysics
	 *  @{
	 */

	/**	Type IDs used by the RTTI system for the null physics library. */
	enum TypeID_NullPhysics
	{
		TID_FNullPhysicsMesh = 110000,
	};

	/** @} */
} // namespace b3d
