//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Reflection/B3DRTTIField.h"
#include "Reflection/B3DRTTIPlain.h"
#include "RTTI/B3DRTTISchemaRTTI.h"

using namespace b3d;

RTTIType* RTTIFieldDataTypeSchema::GetRttiStatic()
{
	return RTTIFieldDataTypeSchemaRTTI::Instance();
}

RTTIType* RTTIFieldDataTypeSchema::GetRtti() const
{
	return GetRttiStatic();
}

RTTIType* RTTIFieldSchema::GetRttiStatic()
{
	return RTTIFieldSchemaRTTI::Instance();
}

RTTIType* RTTIFieldSchema::GetRtti() const
{
	return GetRttiStatic();
}

RTTIFieldInfo RTTIFieldInfo::DEFAULT;

void RTTIField::CheckIsArray(bool array) const
{
	if(array && !Schema.IsContainer)
		B3D_ASSERT(false && "Invalid field type. Needed an array type but got a single type.");

	if(!array && Schema.IsContainer)
		B3D_ASSERT(false && "Invalid field type. Needed a single type but got an array type.");
}
