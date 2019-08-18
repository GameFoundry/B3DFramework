//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Reflection/BsRTTIPlain.h"
#include "Physics/BsPhysicsMaterial.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class BS_CORE_EXPORT PhysicsMaterialRTTI : public RTTIType<PhysicsMaterial, Resource, PhysicsMaterialRTTI>
	{
	private:
		float& getStaticFriction(PhysicsMaterial* obj)
		{
			return mStaticFriction;
		}

		void setStaticFriction(PhysicsMaterial* obj, float& size) { obj->setStaticFriction(size); }

		float& getDynamicFriction(PhysicsMaterial* obj)
		{
			return mDynamicFriction;
		}

		void setDynamicFriction(PhysicsMaterial* obj, float& size) { obj->setDynamicFriction(size); }

		float& getRestitutionCoefficient(PhysicsMaterial* obj)
		{
			return mRestitutionCoefficient;
		}

		void setRestitutionCoefficient(PhysicsMaterial* obj, float& size) { obj->setRestitutionCoefficient(size); }

	public:
		PhysicsMaterialRTTI()
		{
			addPlainField("staticFriction", 0, &PhysicsMaterialRTTI::getStaticFriction, &PhysicsMaterialRTTI::setStaticFriction);
			addPlainField("dynamicFriction", 1, &PhysicsMaterialRTTI::getDynamicFriction, &PhysicsMaterialRTTI::setDynamicFriction);
			addPlainField("restitutionCoefficient", 2, &PhysicsMaterialRTTI::getRestitutionCoefficient, &PhysicsMaterialRTTI::setRestitutionCoefficient);
		}

		void onSerializationStarted(IReflectable* obj, SerializationContext* context) override
		{
			PhysicsMaterial* material = static_cast<PhysicsMaterial*>(obj);

			mStaticFriction = material->getStaticFriction();
			mDynamicFriction = material->getDynamicFriction();
			mRestitutionCoefficient = material->getRestitutionCoefficient();
		}

		const String& getRTTIName() override
		{
			static String name = "PhysicsMaterial";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_PhysicsMaterial;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return PhysicsMaterial::_createPtr();
		}

	private:
		float mStaticFriction;
		float mDynamicFriction;
		float mRestitutionCoefficient;
	};

	/** @} */
	/** @endcond */
}
