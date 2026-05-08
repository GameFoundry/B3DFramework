//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Serialization/B3DManagedDelta.h"
#include "Serialization/B3DManagedSerializableDelta.h"
#include "Serialization/B3DBinarySerializer.h"
#include "Serialization/B3DManagedSerializableObject.h"
#include "Reflection/B3DRTTIType.h"
#include "Utility/B3DUtility.h"
#include "Scene/B3DSceneObject.h"

using namespace b3d;
TShared<SerializedObject> ManagedDeltaHandler::GenerateDeltaRecursive(IReflectable* original, IReflectable* modified, ObjectMap& objectMap, RTTIOperationContext& context, bool reflectableOnly)
{
	ManagedSerializableObject* orgManSerzObj;
	TShared<ManagedSerializableObject> orgDecodedObject;
	if(original->GetTypeId() == TID_SerializedObject)
	{
		auto* orgSerzObj = static_cast<SerializedObject*>(original);
		orgDecodedObject = std::static_pointer_cast<ManagedSerializableObject>(orgSerzObj->Decode(context));

		orgManSerzObj = orgDecodedObject.get();
	}
	else
	{
		B3D_ASSERT(original->GetTypeId() == TID_ScriptSerializableObject);
		orgManSerzObj = static_cast<ManagedSerializableObject*>(original);
	}

	ManagedSerializableObject* newManSerzObj;
	TShared<ManagedSerializableObject> newDecodedObject;
	if(modified->GetTypeId() == TID_SerializedObject)
	{
		auto* newSerzObj = static_cast<SerializedObject*>(modified);
		newDecodedObject = std::static_pointer_cast<ManagedSerializableObject>(newSerzObj->Decode(context));

		newManSerzObj = newDecodedObject.get();
	}
	else
	{
		B3D_ASSERT(modified->GetTypeId() == TID_ScriptSerializableObject);
		newManSerzObj = static_cast<ManagedSerializableObject*>(modified);
	}

	TShared<ManagedSerializableDelta> diff = ManagedSerializableDelta::Create(orgManSerzObj, newManSerzObj, &context);
	if(diff == nullptr)
		return nullptr;

	TShared<SerializedObject> output = B3DMakeShared<SerializedObject>();
	output->SubObjects.push_back(SerializedSubObject());

	SerializedSubObject& subObject = output->SubObjects.back();
	subObject.TypeId = ManagedSerializableObject::GetRttiStatic()->GetRttiId();

	SerializedField entry;
	entry.FieldId = 0;
	entry.Value = SerializedObject::Create(*diff);

	subObject.FieldEntries[0] = entry;

	return output;
}

void ManagedDeltaHandler::GenerateDeltaApplyCommands(const TShared<IReflectable>& object, const TShared<SerializedObject>& delta, FrameAllocator& allocator, DeltaObjectMap& objectMap, FrameVector<DeltaCommand>& inOutDeltaCommands, RTTIOperationContext& context)
{
	TShared<SerializedObject> diffObj = std::static_pointer_cast<SerializedObject>(delta->SubObjects[0].FieldEntries[0].Value);

	TShared<ManagedSerializableDelta> diff = std::static_pointer_cast<ManagedSerializableDelta>(diffObj->Decode(context));

	if(diff != nullptr)
	{
		TShared<ManagedSerializableObject> managedObj = std::static_pointer_cast<ManagedSerializableObject>(object);
		diff->Apply(managedObj);
	}
}
