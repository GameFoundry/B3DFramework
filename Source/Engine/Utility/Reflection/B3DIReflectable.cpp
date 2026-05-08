//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Reflection/B3DIReflectable.h"
#include "Reflection/B3DRTTIType.h"
#include "RTTI/B3DIReflectableRTTI.h"

using namespace b3d;

void IReflectable::RegisterRTTITypeInternal(RTTIType* rttiType)
{
	if(!B3D_CHECK_LOG(!IsTypeIdDuplicateInternal(rttiType->GetRttiId()), "RTTI type has a duplicate ID."))
		return;

	GetAllRttiTypes()[rttiType->GetRttiId()] = rttiType;
}

TShared<IReflectable> IReflectable::CreateInstanceFromTypeId(u32 rttiTypeId)
{
	RTTIType* type = GetRTTITypeFromTypeId(rttiTypeId);

	TShared<IReflectable> output;
	if(type != nullptr)
		output = type->NewRttiObject();

	return output;
}

RTTIType* IReflectable::GetRTTITypeFromTypeId(u32 rttiTypeId)
{
	const auto iterFind = GetAllRttiTypes().find(rttiTypeId);
	if(iterFind != GetAllRttiTypes().end())
		return iterFind->second;

	return nullptr;
}

bool IReflectable::IsTypeIdDuplicateInternal(u32 typeId)
{
	if(typeId == TID_Abstract)
		return false;

	return IReflectable::GetRTTITypeFromTypeId(typeId) != nullptr;
}

bool IReflectable::IsDerivedFrom(const RTTIType* base) const
{
	return GetRtti()->IsDerivedFrom(base);
}

u32 IReflectable::GetTypeId() const
{
	return GetRtti()->GetRttiId();
}

const String& IReflectable::GetTypeName() const
{
	return GetRtti()->GetRttiName();
}

RTTIType* IReflectable::GetRttiStatic()
{
	return IReflectableRTTI::Instance();
}
