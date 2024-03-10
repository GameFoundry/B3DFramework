//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Serialization/BsManagedDiff.h"
#include "Serialization/BsManagedSerializableDiff.h"
#include "Serialization/BsBinarySerializer.h"
#include "Serialization/BsManagedSerializableObject.h"
#include "Reflection/BsRTTIType.h"
#include "Utility/BsUtility.h"
#include "Scene/BsSceneObject.h"

using namespace bs;
SPtr<SerializedObject> ManagedDiff::GenerateDiffInternal(IReflectable* orgObj, IReflectable* newObj, ObjectMap& objectMap, bool reflectableOnly)
{
	CoreSerializationContext context;

	ManagedSerializableObject* orgManSerzObj;
	SPtr<ManagedSerializableObject> orgDecodedObject;
	if(orgObj->GetTypeId() == TID_SerializedObject)
	{
		auto* orgSerzObj = static_cast<SerializedObject*>(orgObj);
		orgDecodedObject = std::static_pointer_cast<ManagedSerializableObject>(orgSerzObj->Decode(&context));

		orgManSerzObj = orgDecodedObject.get();
	}
	else
	{
		B3D_ASSERT(orgObj->GetTypeId() == TID_ScriptSerializableObject);
		orgManSerzObj = static_cast<ManagedSerializableObject*>(orgObj);
	}

	ManagedSerializableObject* newManSerzObj;
	SPtr<ManagedSerializableObject> newDecodedObject;
	if(newObj->GetTypeId() == TID_SerializedObject)
	{
		auto* newSerzObj = static_cast<SerializedObject*>(newObj);
		newDecodedObject = std::static_pointer_cast<ManagedSerializableObject>(newSerzObj->Decode(&context));

		newManSerzObj = newDecodedObject.get();
	}
	else
	{
		B3D_ASSERT(newObj->GetTypeId() == TID_ScriptSerializableObject);
		newManSerzObj = static_cast<ManagedSerializableObject*>(newObj);
	}

	SPtr<ManagedSerializableDiff> diff = ManagedSerializableDiff::Create(orgManSerzObj, newManSerzObj);
	if(diff == nullptr)
		return nullptr;

	SPtr<SerializedObject> output = B3DMakeShared<SerializedObject>();
	output->SubObjects.push_back(SerializedSubObject());

	SerializedSubObject& subObject = output->SubObjects.back();
	subObject.TypeId = ManagedSerializableObject::GetRttiStatic()->GetRttiId();

	SerializedField entry;
	entry.FieldId = 0;
	entry.Value = SerializedObject::Create(*diff);

	subObject.FieldEntries[0] = entry;

	return output;
}

void ManagedDiff::ApplyDiff(const SPtr<IReflectable>& object, const SPtr<SerializedObject>& serzDiff, FrameAllocator& alloc, DiffObjectMap& objectMap, FrameVector<DiffCommand>& diffCommands, SerializationContext* context)
{
	SPtr<SerializedObject> diffObj = std::static_pointer_cast<SerializedObject>(serzDiff->SubObjects[0].FieldEntries[0].Value);

	SPtr<ManagedSerializableDiff> diff = std::static_pointer_cast<ManagedSerializableDiff>(diffObj->Decode(context));

	if(diff != nullptr)
	{
		SPtr<ManagedSerializableObject> managedObj = std::static_pointer_cast<ManagedSerializableObject>(object);
		diff->Apply(managedObj);
	}
}
