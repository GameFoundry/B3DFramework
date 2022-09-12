//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Reflection/BsRTTIField.h"
#include "Reflection/BsRTTIPlain.h"
#include "RTTI/BsRTTISchemaRTTI.h"
#include "Error/BsException.h"

namespace bs
{
	RTTITypeBase* RTTIFieldSchema::getRTTIStatic()
	{
		return RTTIFieldSchemaRTTI::Instance();
	}

	RTTITypeBase* RTTIFieldSchema::getRTTI() const
	{
		return GetRTTIStatic();
	}
	
	RTTIFieldInfo RTTIFieldInfo::DEFAULT;

	void RTTIField::CheckIsArray(bool array) const
	{
		if(array && !schema.isArray)
		{
			BS_EXCEPT(InternalErrorException,
				"Invalid field type. Needed an array type but got a single type.");
		}

		if(!array && schema.isArray)
		{
			BS_EXCEPT(InternalErrorException,
				"Invalid field type. Needed a single type but got an array type.");
		}
	}
}
