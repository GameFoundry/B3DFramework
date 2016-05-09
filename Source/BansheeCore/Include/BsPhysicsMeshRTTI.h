//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsRTTIType.h"
#include "BsPhysicsMesh.h"
#include "BsPhysics.h"

namespace BansheeEngine
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
		PhysicsMeshRTTI()
			:mInitMembers(this)
		{ }

		void onDeserializationEnded(IReflectable* obj) override
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
		FPhysicsMeshRTTI()
			:mInitMembers(this)
		{ }

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