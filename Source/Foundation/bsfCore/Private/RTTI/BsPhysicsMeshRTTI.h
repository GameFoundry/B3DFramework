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
		void OnDeserializationEnded(IReflectable* obj, SerializationContext* context)
		{
			PhysicsMesh* mesh = static_cast<PhysicsMesh*>(obj);
			mesh->Initialize();
		}

		const String& GetRttiName()
		{
			static String name = "PhysicsMesh";
			return name;
		}

		u32 GetRttiId()
		{
			return TID_PhysicsMesh;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			SPtr<PhysicsMesh> mesh = GetPhysics().CreateMesh(nullptr, PhysicsMeshType::Convex);
			mesh->SetThisPtrInternal(mesh);

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
		const String& GetRttiName()
		{
			static String name = "FPhysicsMesh";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_FPhysicsMesh;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			BS_EXCEPT(InternalErrorException, "Cannot instantiate an abstract class.");
			return nullptr;
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
