//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsRTTIType.h"
#include "BsPhysicsMaterial.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class BS_CORE_EXPORT PhysicsMaterialRTTI : public RTTIType<PhysicsMaterial, Resource, PhysicsMaterialRTTI>
	{
		struct PhysicsMaterialData
		{
			float staticFriction;
			float dynamicFriction;
			float restitutionCoefficient;
		};

	private:
		float& getStaticFriction(PhysicsMaterial* obj)
		{
			PhysicsMaterialData& materialData = any_cast_ref<PhysicsMaterialData>(obj->mRTTIData);

			return materialData.staticFriction;
		}

		void setStaticFriction(PhysicsMaterial* obj, float& size) { obj->setStaticFriction(size); }

		float& getDynamicFriction(PhysicsMaterial* obj)
		{
			PhysicsMaterialData& materialData = any_cast_ref<PhysicsMaterialData>(obj->mRTTIData);

			return materialData.dynamicFriction;
		}

		void setDynamicFriction(PhysicsMaterial* obj, float& size) { obj->setDynamicFriction(size); }

		float& getRestitutionCoefficient(PhysicsMaterial* obj)
		{
			PhysicsMaterialData& materialData = any_cast_ref<PhysicsMaterialData>(obj->mRTTIData);

			return materialData.restitutionCoefficient;
		}

		void setRestitutionCoefficient(PhysicsMaterial* obj, float& size) { obj->setRestitutionCoefficient(size); }

	public:
		PhysicsMaterialRTTI()
		{
			addPlainField("staticFriction", 0, &PhysicsMaterialRTTI::getStaticFriction, &PhysicsMaterialRTTI::setStaticFriction);
			addPlainField("dynamicFriction", 1, &PhysicsMaterialRTTI::getDynamicFriction, &PhysicsMaterialRTTI::setDynamicFriction);
			addPlainField("restitutionCoefficient", 2, &PhysicsMaterialRTTI::getRestitutionCoefficient, &PhysicsMaterialRTTI::setRestitutionCoefficient);
		}

		void onSerializationStarted(IReflectable* obj, const UnorderedMap<String, UINT64>& params) override
		{
			PhysicsMaterial* material = static_cast<PhysicsMaterial*>(obj);

			PhysicsMaterialData materialData = 
			{ 
				material->getStaticFriction(), 
				material->getDynamicFriction(), 
				material->getRestitutionCoefficient() 
			};

			material->mRTTIData = materialData;
		}

		void onSerializationEnded(IReflectable* obj, const UnorderedMap<String, UINT64>& params) override
		{
			PhysicsMaterial* material = static_cast<PhysicsMaterial*>(obj);
			material->mRTTIData = nullptr;
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
	};

	/** @} */
	/** @endcond */
}