//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Serialization/BsBinaryDiff.h"
#include "Serialization/BsSerializedObject.h"
#include "Serialization/BsBinarySerializer.h"
#include "Serialization/BsBinaryCloner.h"
#include "Reflection/BsRTTIType.h"
#include "FileSystem/BsDataStream.h"

namespace bs
{
	SPtr<SerializedObject> IDiff::generateDiff(const SPtr<SerializedObject>& orgObj,
		const SPtr<SerializedObject>& newObj)
	{
		ObjectMap objectMap;
		return generateDiff(orgObj, newObj, objectMap);
	}

	SPtr<SerializedInstance> IDiff::generateDiff(RTTITypeBase* rtti, UINT32 fieldType, const SPtr<SerializedInstance>& orgData,
		const SPtr<SerializedInstance>& newData, ObjectMap& objectMap)
	{
		SPtr<SerializedInstance> modification;
		switch (fieldType)
		{
		case SerializableFT_ReflectablePtr:
		case SerializableFT_Reflectable:
		{
			SPtr<SerializedObject> orgObjData = std::static_pointer_cast<SerializedObject>(orgData);
			SPtr<SerializedObject> newObjData = std::static_pointer_cast<SerializedObject>(newData);

			auto iterFind = objectMap.find(newObjData);
			if (iterFind != objectMap.end())
				modification = iterFind->second;
			else
			{
				RTTITypeBase* childRtti = nullptr;
				if (orgObjData->getRootTypeId() == newObjData->getRootTypeId())
					childRtti = IReflectable::_getRTTIfromTypeId(newObjData->getRootTypeId());

				SPtr<SerializedObject> objectDiff;
				if (childRtti != nullptr)
				{
					IDiff& handler = childRtti->getDiffHandler();
					objectDiff = handler.generateDiff(orgObjData, newObjData, objectMap);
				}

				if (objectDiff != nullptr)
					objectMap[newObjData] = objectDiff;

				modification = objectDiff;
			}
		}
			break;
		case SerializableFT_Plain:
		{
			SPtr<SerializedField> orgFieldData = std::static_pointer_cast<SerializedField>(orgData);
			SPtr<SerializedField> newFieldData = std::static_pointer_cast<SerializedField>(newData);

			bool isModified = orgFieldData->size != newFieldData->size;
			if (!isModified)
				isModified = memcmp(orgFieldData->value, newFieldData->value, newFieldData->size) != 0;

			if (isModified)
				modification = newFieldData->clone();
		}
			break;
		case SerializableFT_DataBlock:
		{
			SPtr<SerializedDataBlock> orgFieldData = std::static_pointer_cast<SerializedDataBlock>(orgData);
			SPtr<SerializedDataBlock> newFieldData = std::static_pointer_cast<SerializedDataBlock>(newData);

			bool isModified = orgFieldData->size != newFieldData->size;
			if (!isModified)
			{
				UINT8* orgStreamData = nullptr;
				if(orgFieldData->stream->isFile())
				{
					orgStreamData = (UINT8*)bs_stack_alloc(orgFieldData->size);
					orgFieldData->stream->seek(orgFieldData->offset);
					orgFieldData->stream->read(orgStreamData, orgFieldData->size);
				}
				else
				{
					SPtr<MemoryDataStream> orgMemStream = std::static_pointer_cast<MemoryDataStream>(orgFieldData->stream);
					orgStreamData = orgMemStream->getCurrentPtr();
				}

				UINT8* newStreamData = nullptr;
				if (newFieldData->stream->isFile())
				{
					newStreamData = (UINT8*)bs_stack_alloc(newFieldData->size);
					newFieldData->stream->seek(newFieldData->offset);
					newFieldData->stream->read(newStreamData, newFieldData->size);
				}
				else
				{
					SPtr<MemoryDataStream> newMemStream = std::static_pointer_cast<MemoryDataStream>(newFieldData->stream);
					newStreamData = newMemStream->getCurrentPtr();
				}

				isModified = memcmp(orgStreamData, newStreamData, newFieldData->size) != 0;

				if (newFieldData->stream->isFile())
					bs_stack_free(newStreamData);

				if (orgFieldData->stream->isFile())
					bs_stack_free(orgStreamData);
			}

			if (isModified)
				modification = newFieldData->clone();
		}
		break;
		}

		return modification;
	}

	void IDiff::applyDiff(const SPtr<IReflectable>& object, const SPtr<SerializedObject>& diff)
	{
		static const UnorderedMap<String, UINT64> dummyParams;

		FrameAlloc& alloc = gFrameAlloc();
		alloc.markFrame();

		FrameVector<DiffCommand> commands;

		DiffObjectMap objectMap;
		applyDiff(object, diff, alloc, objectMap, commands);

		IReflectable* destObject = nullptr;
		RTTITypeBase* rttiInstance = nullptr;

		Stack<IReflectable*> objectStack;
		Vector<std::pair<RTTITypeBase*, IReflectable*>> rttiInstances;

		for (auto& command : commands)
		{
			bool isArray = (command.type & Diff_ArrayFlag) != 0;
			DiffCommandType type = (DiffCommandType)(command.type & 0xF);

			switch (type)
			{
			case Diff_ArraySize:
				command.field->setArraySize(rttiInstance, destObject, command.arraySize);
				break;
			case Diff_ObjectStart:
			{
				destObject = command.object.get();
				objectStack.push(destObject);

				FrameStack<RTTITypeBase*> rttiTypes;
				RTTITypeBase* curRtti = destObject->getRTTI();
				while (curRtti != nullptr)
				{
					rttiTypes.push(curRtti);
					curRtti = curRtti->getBaseClass();
				}

				// Call base class first, followed by derived classes
				while(!rttiTypes.empty())
				{
					RTTITypeBase* curRtti = rttiTypes.top();
					RTTITypeBase* rttiInstance = curRtti->_clone(alloc);

					rttiInstances.push_back(std::make_pair(rttiInstance, destObject));
					rttiInstance->onDeserializationStarted(destObject, dummyParams);

					rttiTypes.pop();
				}
			}
				break;
			case Diff_SubObjectStart:
				{
					// Find the instance
					rttiInstance = nullptr;
					for(auto iter = rttiInstances.rbegin(); iter != rttiInstances.rend(); ++iter)
					{
						if(iter->second != destObject)
							break;

						if(iter->first->getRTTIId() == command.rttiType->getRTTIId())
							rttiInstance = iter->first;
					}

					assert(rttiInstance);
				}
				break;
			case Diff_ObjectEnd:
			{
				while (!rttiInstances.empty())
				{
					if(rttiInstances.back().second != destObject)
						break;

					RTTITypeBase* rttiInstance = rttiInstances.back().first;

					rttiInstance->onDeserializationEnded(destObject, dummyParams);
					alloc.destruct(rttiInstance);

					rttiInstances.erase(rttiInstances.end() - 1);
				}

				objectStack.pop();

				if (!objectStack.empty())
					destObject = objectStack.top();
				else
					destObject = nullptr;
			}
				break;
			default:
				break;
			}

			if (isArray)
			{
				switch (type)
				{
				case Diff_ReflectablePtr:
				{
					RTTIReflectablePtrFieldBase* field = static_cast<RTTIReflectablePtrFieldBase*>(command.field);
					field->setArrayValue(rttiInstance, destObject, command.arrayIdx, command.object);
				}
					break;
				case Diff_Reflectable:
				{
					RTTIReflectableFieldBase* field = static_cast<RTTIReflectableFieldBase*>(command.field);
					field->setArrayValue(rttiInstance, destObject, command.arrayIdx, *command.object);
				}
					break;
				case Diff_Plain:
				{
					RTTIPlainFieldBase* field = static_cast<RTTIPlainFieldBase*>(command.field);
					field->arrayElemFromBuffer(rttiInstance, destObject, command.arrayIdx, command.value);
				}
					break;
				default:
					break;
				}
			}
			else
			{
				switch (type)
				{
				case Diff_ReflectablePtr:
				{
					RTTIReflectablePtrFieldBase* field = static_cast<RTTIReflectablePtrFieldBase*>(command.field);
					field->setValue(rttiInstance, destObject, command.object);
				}
					break;
				case Diff_Reflectable:
				{
					RTTIReflectableFieldBase* field = static_cast<RTTIReflectableFieldBase*>(command.field);
					field->setValue(rttiInstance, destObject, *command.object);
				}
					break;
				case Diff_Plain:
				{
					RTTIPlainFieldBase* field = static_cast<RTTIPlainFieldBase*>(command.field);
					field->fromBuffer(rttiInstance, destObject, command.value);
				}
					break;
				case Diff_DataBlock:
				{
					RTTIManagedDataBlockFieldBase* field = static_cast<RTTIManagedDataBlockFieldBase*>(command.field);
					field->setValue(rttiInstance, destObject, command.streamValue, command.size);
				}
					break;
				default:
					break;
				}
			}
		}

		alloc.clear();
	}

	void IDiff::applyDiff(RTTITypeBase* rtti, const SPtr<IReflectable>& object, const SPtr<SerializedObject>& diff, 
		FrameAlloc& alloc, DiffObjectMap& objectMap, FrameVector<DiffCommand>& diffCommands)
	{
		IDiff& diffHandler = rtti->getDiffHandler();
		diffHandler.applyDiff(object, diff, alloc, objectMap, diffCommands);
	}

	SPtr<SerializedObject> BinaryDiff::generateDiff(const SPtr<SerializedObject>& orgObj, 
		const SPtr<SerializedObject>& newObj, ObjectMap& objectMap)
	{
		SPtr<SerializedObject> output;
		for (auto& subObject : newObj->subObjects)
		{
			RTTITypeBase* rtti = IReflectable::_getRTTIfromTypeId(subObject.typeId);
			if (rtti == nullptr)
				continue;

			SerializedSubObject* orgSubObject = nullptr;
			for (auto& curSubObject : orgObj->subObjects)
			{
				if (curSubObject.typeId == subObject.typeId)
				{
					orgSubObject = &curSubObject;
					break;
				}
			}
			
			SerializedSubObject* diffSubObject = nullptr;
			for (auto& newEntry : subObject.entries)
			{
				RTTIField* genericField = rtti->findField(newEntry.first);
				if (genericField == nullptr)
					continue;

				SPtr<SerializedInstance> newEntryData = newEntry.second.serialized;
				SPtr<SerializedInstance> orgEntryData;

				if (orgSubObject != nullptr)
				{
					auto orgEntryFind = orgSubObject->entries.find(newEntry.first);
					if (orgEntryFind != orgSubObject->entries.end())
						orgEntryData = orgEntryFind->second.serialized;
				}

				SPtr<SerializedInstance> modification;
				bool hasModification = false;
				if (genericField->isArray())
				{
					SPtr<SerializedArray> orgArrayData = std::static_pointer_cast<SerializedArray>(orgEntryData);
					SPtr<SerializedArray> newArrayData = std::static_pointer_cast<SerializedArray>(newEntryData);

					SPtr<SerializedArray> serializedArray;

					if (newEntryData != nullptr && orgEntryData != nullptr)
					{
						for (auto& arrayEntryPair : newArrayData->entries)
						{
							SPtr<SerializedInstance> arrayModification;

							auto iterFind = orgArrayData->entries.find(arrayEntryPair.first);
							if (iterFind == orgArrayData->entries.end())
								arrayModification = arrayEntryPair.second.serialized->clone();
							else
							{
								arrayModification = IDiff::generateDiff(rtti, genericField->mType, iterFind->second.serialized,
									arrayEntryPair.second.serialized, objectMap);
							}

							if (arrayModification != nullptr)
							{
								if (serializedArray == nullptr)
								{
									serializedArray = bs_shared_ptr_new<SerializedArray>();
									serializedArray->numElements = newArrayData->numElements;
								}

								SerializedArrayEntry arrayEntry;
								arrayEntry.index = arrayEntryPair.first;
								arrayEntry.serialized = arrayModification;
								serializedArray->entries[arrayEntryPair.first] = arrayEntry;
							}
						}
					}
					else if (newEntryData == nullptr)
					{
						serializedArray = bs_shared_ptr_new<SerializedArray>();
					}
					else if (orgEntryData == nullptr)
					{
						serializedArray = std::static_pointer_cast<SerializedArray>(newArrayData->clone());
					}

					modification = serializedArray;
					hasModification = modification != nullptr;
				}
				else
				{
					if (newEntryData != nullptr && orgEntryData != nullptr)
					{
						modification = IDiff::generateDiff(rtti, genericField->mType, orgEntryData, newEntryData, objectMap);
						hasModification = modification != nullptr;
					}
					else if (newEntryData == nullptr)
					{
						switch (genericField->mType)
						{
						case SerializableFT_Plain:
							modification = bs_shared_ptr_new<SerializedField>();
							break;
						case SerializableFT_DataBlock:
							modification = bs_shared_ptr_new<SerializedDataBlock>();
							break;
						default:
							break;
						}

						hasModification = true;
					}
					else if (orgEntryData == nullptr)
					{
						modification = newEntryData->clone();
						hasModification = modification != nullptr;
					}
				}

				if (hasModification)
				{
					if (output == nullptr)
						output = bs_shared_ptr_new<SerializedObject>();

					if (diffSubObject == nullptr)
					{
						output->subObjects.push_back(SerializedSubObject());
						diffSubObject = &output->subObjects.back();
						diffSubObject->typeId = rtti->getRTTIId();
					}

					SerializedEntry modificationEntry;
					modificationEntry.fieldId = genericField->mUniqueId;
					modificationEntry.serialized = modification;
					diffSubObject->entries[genericField->mUniqueId] = modificationEntry;
				}
			}
		}

		return output;
	}

	void BinaryDiff::applyDiff(const SPtr<IReflectable>& object, const SPtr<SerializedObject>& diff,
		FrameAlloc& alloc, DiffObjectMap& objectMap, FrameVector<DiffCommand>& diffCommands)
	{
		static const UnorderedMap<String, UINT64> dummyParams;

		if (object == nullptr || diff == nullptr || object->getTypeId() != diff->getRootTypeId())
			return;

		// Generate a list of commands per sub-object
		FrameVector<FrameVector<DiffCommand>> commandsPerSubObj;

		Stack<RTTITypeBase*> rttiInstances;
		for (auto& subObject : diff->subObjects)
		{
			RTTITypeBase* rtti = IReflectable::_getRTTIfromTypeId(subObject.typeId);
			if (rtti == nullptr)
				continue;

			if (!object->isDerivedFrom(rtti))
				continue;

			RTTITypeBase* rttiInstance = rtti->_clone(alloc);
			rttiInstance->onSerializationStarted(object.get(), dummyParams);
			rttiInstances.push(rttiInstance);

			FrameVector<DiffCommand> commands;

			DiffCommand subObjStartCommand;
			subObjStartCommand.rttiType = rtti;
			subObjStartCommand.field = nullptr;
			subObjStartCommand.type = Diff_SubObjectStart;

			commands.push_back(subObjStartCommand);

			for (auto& diffEntry : subObject.entries)
			{
				RTTIField* genericField = rtti->findField(diffEntry.first);
				if (genericField == nullptr)
					continue;

				SPtr<SerializedInstance> diffData = diffEntry.second.serialized;

				if (genericField->isArray())
				{
					SPtr<SerializedArray> diffArray = std::static_pointer_cast<SerializedArray>(diffData);

					UINT32 numArrayElements = (UINT32)diffArray->numElements;

					DiffCommand arraySizeCommand;
					arraySizeCommand.field = genericField;
					arraySizeCommand.type = Diff_ArraySize | Diff_ArrayFlag;
					arraySizeCommand.arraySize = numArrayElements;

					commands.push_back(arraySizeCommand);

					switch (genericField->mType)
					{
					case SerializableFT_ReflectablePtr:
					{
						RTTIReflectablePtrFieldBase* field = static_cast<RTTIReflectablePtrFieldBase*>(genericField);

						UINT32 orgArraySize = genericField->getArraySize(rttiInstance, object.get());
						for (auto& arrayElem : diffArray->entries)
						{
							SPtr<SerializedObject> arrayElemData = std::static_pointer_cast<SerializedObject>(arrayElem.second.serialized);

							DiffCommand command;
							command.field = genericField;
							command.type = Diff_ReflectablePtr | Diff_ArrayFlag;
							command.arrayIdx = arrayElem.first;

							if (arrayElemData == nullptr)
							{
								command.object = nullptr;
								commands.push_back(command);
							}
							else
							{
								bool needsNewObject = arrayElem.first >= orgArraySize;

								if (!needsNewObject)
								{
									SPtr<IReflectable> childObj = field->getArrayValue(rttiInstance, object.get(), arrayElem.first);
									if (childObj != nullptr)
									{
										IDiff::applyDiff(childObj->getRTTI(), childObj, arrayElemData, alloc, objectMap, 
											commands);
										command.object = childObj;
									}
									else
										needsNewObject = true;
								}

								if (needsNewObject)
								{
									RTTITypeBase* childRtti = IReflectable::_getRTTIfromTypeId(arrayElemData->getRootTypeId());
									if (childRtti != nullptr)
									{
										auto findObj = objectMap.find(arrayElemData);
										if (findObj == objectMap.end())
										{
											SPtr<IReflectable> newObject = childRtti->newRTTIObject();
											findObj = objectMap.insert(std::make_pair(arrayElemData, newObject)).first;
										}

										IDiff::applyDiff(childRtti, findObj->second, arrayElemData, alloc, objectMap, 
											commands);
										command.object = findObj->second;
										commands.push_back(command);
									}
									else
									{
										command.object = nullptr;
										commands.push_back(command);
									}
								}
							}
						}
					}
						break;
					case SerializableFT_Reflectable:
					{
						RTTIReflectableFieldBase* field = static_cast<RTTIReflectableFieldBase*>(genericField);

						UINT32 orgArraySize = genericField->getArraySize(rttiInstance, object.get());

						Vector<SPtr<IReflectable>> newArrayElements(numArrayElements);
						UINT32 minArrayLength = std::min(orgArraySize, numArrayElements);
						for (UINT32 i = 0; i < minArrayLength; i++)
						{
							IReflectable& childObj = field->getArrayValue(rttiInstance, object.get(), i);
							newArrayElements[i] = BinaryCloner::clone(&childObj, true);
						}

						for (auto& arrayElem : diffArray->entries)
						{
							SPtr<SerializedObject> arrayElemData = std::static_pointer_cast<SerializedObject>(arrayElem.second.serialized);

							if (arrayElem.first < orgArraySize)
							{
								SPtr<IReflectable> childObj = newArrayElements[arrayElem.first];
								IDiff::applyDiff(childObj->getRTTI(), childObj, arrayElemData, alloc, objectMap, 
									commands);
							}
							else
							{
								RTTITypeBase* childRtti = IReflectable::_getRTTIfromTypeId(arrayElemData->getRootTypeId());
								if (childRtti != nullptr)
								{
									SPtr<IReflectable> newObject = childRtti->newRTTIObject();
									IDiff::applyDiff(childRtti, newObject, arrayElemData, alloc, objectMap, commands);

									newArrayElements[arrayElem.first] = newObject;
								}
							}
						}

						for (UINT32 i = 0; i < numArrayElements; i++)
						{
							DiffCommand command;
							command.field = genericField;
							command.type = Diff_Reflectable | Diff_ArrayFlag;
							command.arrayIdx = i;
							command.object = newArrayElements[i];

							commands.push_back(command);
						}
					}
						break;
					case SerializableFT_Plain:
					{
						for (auto& arrayElem : diffArray->entries)
						{
							SPtr<SerializedField> fieldData = std::static_pointer_cast<SerializedField>(arrayElem.second.serialized);
							if (fieldData != nullptr)
							{
								DiffCommand command;
								command.field = genericField;
								command.type = Diff_Plain | Diff_ArrayFlag;
								command.value = fieldData->value;
								command.size = fieldData->size;
								command.arrayIdx = arrayElem.first;

								commands.push_back(command);
							}
						}
					}
						break;
					default:
						break;
					}
				}
				else
				{
					switch (genericField->mType)
					{
					case SerializableFT_ReflectablePtr:
					{
						RTTIReflectablePtrFieldBase* field = static_cast<RTTIReflectablePtrFieldBase*>(genericField);
						SPtr<SerializedObject> fieldObjectData = std::static_pointer_cast<SerializedObject>(diffData);

						DiffCommand command;
						command.field = genericField;
						command.type = Diff_ReflectablePtr;

						if (fieldObjectData == nullptr)
							command.object = nullptr;
						else
						{
							SPtr<IReflectable> childObj = field->getValue(rttiInstance, object.get());
							if (childObj == nullptr)
							{
								RTTITypeBase* childRtti = IReflectable::_getRTTIfromTypeId(fieldObjectData->getRootTypeId());
								if (childRtti != nullptr)
								{
									auto findObj = objectMap.find(fieldObjectData);
									if (findObj == objectMap.end())
									{
										SPtr<IReflectable> newObject = childRtti->newRTTIObject();
										findObj = objectMap.insert(std::make_pair(fieldObjectData, newObject)).first;
									}

									IDiff::applyDiff(childRtti, findObj->second, fieldObjectData, alloc, objectMap, 
										commands);
									command.object = findObj->second;
								}
								else
								{
									command.object = nullptr;
								}
							}
							else
							{
								IDiff::applyDiff(childObj->getRTTI(), childObj, fieldObjectData, alloc, objectMap, 
									commands);
								command.object = childObj;
							}
						}

						commands.push_back(command);
					}
						break;
					case SerializableFT_Reflectable:
					{
						RTTIReflectableFieldBase* field = static_cast<RTTIReflectableFieldBase*>(genericField);
						SPtr<SerializedObject> fieldObjectData = std::static_pointer_cast<SerializedObject>(diffData);

						IReflectable& childObj = field->getValue(rttiInstance, object.get());
						SPtr<IReflectable> clonedObj = BinaryCloner::clone(&childObj, true);

						IDiff::applyDiff(clonedObj->getRTTI(), clonedObj, fieldObjectData, alloc, objectMap, commands);

						DiffCommand command;
						command.field = genericField;
						command.type = Diff_Reflectable;
						command.object = clonedObj;

						commands.push_back(command);
					}
						break;
					case SerializableFT_Plain:
					{
						SPtr<SerializedField> diffFieldData = std::static_pointer_cast<SerializedField>(diffData);

						if (diffFieldData->size > 0)
						{
							DiffCommand command;
							command.field = genericField;
							command.type = Diff_Plain;
							command.value = diffFieldData->value;
							command.size = diffFieldData->size;

							commands.push_back(command);
						}
					}
						break;
					case SerializableFT_DataBlock:
					{
						SPtr<SerializedDataBlock> diffFieldData = std::static_pointer_cast<SerializedDataBlock>(diffData);

						DiffCommand command;
						command.field = genericField;
						command.type = Diff_DataBlock;
						command.streamValue = diffFieldData->stream;
						command.value = nullptr;
						command.size = diffFieldData->size;

						commands.push_back(command);
					}
						break;
					}
				}
			}
		}

		DiffCommand objStartCommand;
		objStartCommand.field = nullptr;
		objStartCommand.type = Diff_ObjectStart;
		objStartCommand.object = object;

		diffCommands.push_back(objStartCommand);

		// Go in reverse because when deserializing we want to deserialize base first, and then derived types
		for(auto iter = commandsPerSubObj.rbegin(); iter != commandsPerSubObj.rend(); ++iter)
			diffCommands.insert(diffCommands.end(), iter->begin(), iter->end());

		DiffCommand objEndCommand;
		objEndCommand.field = nullptr;
		objEndCommand.type = Diff_ObjectEnd;
		objEndCommand.object = object;

		diffCommands.push_back(objEndCommand);

		while (!rttiInstances.empty())
		{
			RTTITypeBase* rttiInstance = rttiInstances.top();
			rttiInstance->onSerializationEnded(object.get(), dummyParams);
			alloc.destruct(rttiInstance);

			rttiInstances.pop();
		}
	}
}