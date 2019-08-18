//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Reflection/BsRTTIPlain.h"
#include "Physics/BsPhysicsMesh.h"
#include "Physics/BsPhysics.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class BS_CORE_EXPORT PhysicsMeshRTTI : public RTTIType<PhysicsMesh, Resource, PhysicsMeshRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_REFLPTR(mInternal, 0)
		BS_END_RTTI_MEMBERS

	public:
		void onDeserializationEnded(IReflectable* obj, SerializationContext* context) override
		{
			PhysicsMesh* mesh = static_cast<PhysicsMesh*>(obj);
			mesh->initialize();
		}

		const String& getRTTIName() override
		{
			static String name = "PhysicsMesh";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_PhysicsMesh;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			SPtr<PhysicsMesh> mesh = gPhysics().createMesh(nullptr, PhysicsMeshType::Convex);
			mesh->_setThisPtr(mesh);

			return mesh;
		}
	};

	class BS_CORE_EXPORT FPhysicsMeshRTTI : public RTTIType<FPhysicsMesh, IReflectable, FPhysicsMeshRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(mType, 0)
		BS_END_RTTI_MEMBERS
	public:
		const String& getRTTIName() override
		{
			static String name = "FPhysicsMesh";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_FPhysicsMesh;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			BS_EXCEPT(InternalErrorException, "Cannot instantiate an abstract class.");
			return nullptr;
		}
	};

	/** @} */
	/** @endcond */
}
