//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPhysXPrerequisites.h"
#include "Physics/B3DPhysicsMaterial.h"
#include "PxMaterial.h"

namespace b3d
{
	/** @addtogroup PhysX
	 *  @{
	 */

	/** PhysX implementation of a PhysicsMaterial. */
	class PhysXMaterial : public PhysicsMaterial
	{
	public:
		PhysXMaterial(physx::PxPhysics* physx, float staFric, float dynFriction, float restitution);
		~PhysXMaterial() override;

		void SetStaticFriction(float value) override;
		float GetStaticFriction() const override;
		void SetDynamicFriction(float value) override;
		float GetDynamicFriction() const override;
		void SetRestitutionCoefficient(float value) override;
		float GetRestitutionCoefficient() const override;

		/** Returns the internal PhysX material. */
		physx::PxMaterial* GetPxMaterial() const { return mInternal; }

	private:
		physx::PxMaterial* mInternal;
	};

	/** @} */
} // namespace b3d
