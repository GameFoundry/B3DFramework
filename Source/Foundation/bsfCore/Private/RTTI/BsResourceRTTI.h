//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Reflection/BsRTTIPlain.h"
#include "RTTI/BsUUIDRTTI.h"
#include "Resources/BsResource.h"
#include "Resources/BsResourceMetaData.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class B3D_CORE_EXPORT ResourceRTTI : public RTTIType<Resource, IReflectable, ResourceRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(mSize, 0)
			B3D_RTTI_MEMBER(mMetaData, 1)
			B3D_RTTI_MEMBER(mId, 2)
		B3D_RTTI_END_MEMBERS

	public:
		void OnDeserializationStarted(IReflectable* obj, SerializationContext* context)
		{
			Resource* resource = static_cast<Resource*>(obj);

			if(context)
				resource->mKeepSourceData = (context->Flags & SF_KeepResourceSourceData) != 0;
			else
				resource->mKeepSourceData = false;
		}

		const String& GetRttiName()
		{
			static String name = "Resource";
			return name;
		}

		u32 GetRttiId() const override
		{
			return 100;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			B3D_EXCEPT(InternalErrorException, "Cannot instantiate an abstract class.");
			return nullptr;
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
