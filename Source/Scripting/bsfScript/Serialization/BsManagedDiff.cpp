//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Serialization/BsManagedDiff.h"
#include "Serialization/BsManagedSerializableDiff.h"
#include "Serialization/BsBinarySerializer.h"
#include "Serialization/BsManagedSerializableObject.h"
#include "Reflection/BsRTTIType.h"
#include "Utility/BsUtility.h"
#include "Scene/BsSceneObject.h"

namespace bs
{
	SPtr<SerializedObject> ManagedDiff::_generateDiff(IReflectable* orgObj,
		IReflectable* newObj, ObjectMap& objectMap, bool reflectableOnly)
	{
		CoreSerializationContext context;
		context.goState = bs_shared_ptr_new<GameObjectDeserializationState>(GODM_UseOriginalIds | GODM_RestoreExternal);

		ManagedSerializableObject* orgManSerzObj;
		SPtr<ManagedSerializableObject> orgDecodedObject;
		if (orgObj->getTypeId() == TID_SerializedObject)
		{
			auto* orgSerzObj = static_cast<SerializedObject*>(orgObj);
			orgDecodedObject = std::static_pointer_cast<ManagedSerializableObject>(orgSerzObj->decode(&context));

			orgManSerzObj = orgDecodedObject.get();
		}
		else
		{
			assert(orgObj->getTypeId() == TID_ScriptSerializableObject);
			orgManSerzObj = static_cast<ManagedSerializableObject*>(orgObj);
		}

		ManagedSerializableObject* newManSerzObj;
		SPtr<ManagedSerializableObject> newDecodedObject;
		if (newObj->getTypeId() == TID_SerializedObject)
		{
			auto* newSerzObj = static_cast<SerializedObject*>(newObj);
			newDecodedObject = std::static_pointer_cast<ManagedSerializableObject>(newSerzObj->decode(&context));

			newManSerzObj = newDecodedObject.get();
		}
		else
		{
			assert(newObj->getTypeId() == TID_ScriptSerializableObject);
			newManSerzObj = static_cast<ManagedSerializableObject*>(newObj);
		}

		context.goState->resolve();

		SPtr<ManagedSerializableDiff> diff = ManagedSerializableDiff::create(orgManSerzObj, newManSerzObj);
		if (diff == nullptr)
			return nullptr;

		SPtr<SerializedObject> output = bs_shared_ptr_new<SerializedObject>();
		output->subObjects.push_back(SerializedSubObject());

		SerializedSubObject& subObject = output->subObjects.back();
		subObject.typeId = ManagedSerializableObject::getRTTIStatic()->getRTTIId();

		SerializedEntry entry;
		entry.fieldId = 0;
		entry.serialized = SerializedObject::create(*diff);

		subObject.entries[0] = entry;

		return output;
	}

	void ManagedDiff::applyDiff(const SPtr<IReflectable>& object, const SPtr<SerializedObject>& serzDiff,
		FrameAlloc& alloc, DiffObjectMap& objectMap, FrameVector<DiffCommand>& diffCommands, SerializationContext* context)
	{
		SPtr<SerializedObject> diffObj = std::static_pointer_cast<SerializedObject>(serzDiff->subObjects[0].entries[0].serialized);

		SPtr<ManagedSerializableDiff> diff = std::static_pointer_cast<ManagedSerializableDiff>(diffObj->decode(context));
		
		if (diff != nullptr)
		{
			SPtr<ManagedSerializableObject> managedObj = std::static_pointer_cast<ManagedSerializableObject>(object);
			diff->apply(managedObj);
		}
	}
}
