//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "Reflection/B3DRTTIPlain.h"
#include "RTTI/B3DUUIDRTTI.h"
#include "RTTI/B3DStringRTTI.h"
#include "Resources/B3DResource.h"
#include "Resources/B3DResourceMetaData.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT ResourceRTTI : public TRTTIType<Resource, IReflectable, ResourceRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			//B3D_RTTI_MEMBER(mSize, 0)
			B3D_RTTI_MEMBER(mMetaData, 1)
			B3D_RTTI_MEMBER(mId, 2)
			B3D_RTTI_MEMBER(mName, 3)
		B3D_RTTI_END_MEMBERS

	public:
		void OnOperationStarted(Resource& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::WriteBit))
			{
				object.mKeepSourceData = (context.Flags & SF_KeepResourceSourceData) != 0;
			}
		}

		const String& GetRttiName() override
		{
			static String name = "Resource";
			return name;
		}

		u32 GetRttiId() const override
		{
			return 100;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			B3D_ASSERT(false && "Cannot instantiate an abstract class.");
			return nullptr;
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
