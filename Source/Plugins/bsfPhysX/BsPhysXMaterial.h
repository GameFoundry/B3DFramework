//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPhysXPrerequisites.h"
#include "Physics/BsPhysicsMaterial.h"
#include "PxMaterial.h"

namespace bs
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
		physx::PxMaterial* GetInternalInternal() const { return mInternal; }

	private:
		physx::PxMaterial* mInternal;
	};

	/** @} */
} // namespace bs
