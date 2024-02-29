//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Reflection/BsRTTIField.h"
#include "Reflection/BsRTTIPlain.h"
#include "RTTI/BsRTTISchemaRTTI.h"
#include "Error/BsException.h"

using namespace bs;

RTTITypeBase* RTTIFieldTypeSchema::GetRttiStatic()
{
	return RTTIFieldTypeSchemaRTTI::Instance();
}

RTTITypeBase* RTTIFieldTypeSchema::GetRtti() const
{
	return GetRttiStatic();
}

RTTITypeBase* RTTIFieldSchema::GetRttiStatic()
{
	return RTTIFieldSchemaRTTI::Instance();
}

RTTITypeBase* RTTIFieldSchema::GetRtti() const
{
	return GetRttiStatic();
}

RTTIFieldInfo RTTIFieldInfo::DEFAULT;

void RTTIField::CheckIsArray(bool array) const
{
	if(array && !Schema.IsArray)
	{
		B3D_EXCEPT(InternalErrorException, "Invalid field type. Needed an array type but got a single type.");
	}

	if(!array && Schema.IsArray)
	{
		B3D_EXCEPT(InternalErrorException, "Invalid field type. Needed a single type but got an array type.");
	}
}
