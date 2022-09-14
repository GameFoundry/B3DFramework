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
		~PhysXMaterial();

		/** @copydoc PhysicsMaterial::setStaticFriction */
		void setStaticFriction(float value) override;

		/** @copydoc PhysicsMaterial::getStaticFriction */
		float getStaticFriction() const override;

		/** @copydoc PhysicsMaterial::setDynamicFriction */
		void setDynamicFriction(float value) override;

		/** @copydoc PhysicsMaterial::getDynamicFriction */
		float getDynamicFriction() const override;

		/** @copydoc PhysicsMaterial::setRestitutionCoefficient */
		void setRestitutionCoefficient(float value) override;

		/** @copydoc PhysicsMaterial::getRestitutionCoefficient */
		float getRestitutionCoefficient() const override;

		/** Returns the internal PhysX material. */
		physx::PxMaterial* GetInternalInternal() const { return mInternal; }

	private:
		physx::PxMaterial* mInternal;
	};

	/** @} */
}
