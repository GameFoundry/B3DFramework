//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPhysXPrerequisites.h"
#include "BsPhysicsMaterial.h"
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
		physx::PxMaterial* _getInternal() const { return mInternal; }

	private:
		physx::PxMaterial* mInternal;
	};

	/** @} */
}