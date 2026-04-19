//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DNullPhysicsPrerequisites.h"
#include "Physics/B3DPhysicsMaterial.h"

namespace b3d
{
	/** @addtogroup NullPhysics
	 *  @{
	 */

	/** Null implementation of a PhysicsMaterial. */
	class NullPhysicsMaterial : public PhysicsMaterial
	{
	public:
		NullPhysicsMaterial(float staFric, float dynFriction, float restitution);
		~NullPhysicsMaterial() override = default;

		void SetStaticFriction(float value) override { mStaticFriction = value; }
		float GetStaticFriction() const override { return mStaticFriction; }
		void SetDynamicFriction(float value) override { mDynamicFriction = value; }
		float GetDynamicFriction() const override { return mDynamicFriction; }
		void SetRestitutionCoefficient(float value) override { mRestitutionCoefficient = value; }
		float GetRestitutionCoefficient() const override { return mRestitutionCoefficient; }

	private:
		float mStaticFriction;
		float mDynamicFriction;
		float mRestitutionCoefficient;
	};

	/** @} */
} // namespace b3d
