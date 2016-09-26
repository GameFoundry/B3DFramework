//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsBinaryDiff.h"
#include "BsSerializedObject.h"
#include "BsBinarySerializer.h"
#include "BsBinaryCloner.h"
#include "BsRTTIType.h"
#include "BsDataStream.h"

namespace BansheeEngine
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

		Vector<DiffCommand> commands;

		DiffObjectMap objectMap;
		applyDiff(object, diff, objectMap, commands);

		IReflectable* destObject = nullptr;
		Stack<IReflectable*> objectStack;
		Vector<RTTITypeBase*> rttiTypes;

		for (auto& command : commands)
		{
			bool isArray = (command.type & Diff_ArrayFlag) != 0;
			DiffCommandType type = (DiffCommandType)(command.type & 0xF);

			switch (type)
			{
			case Diff_ArraySize:
				command.field->setArraySize(destObject, command.arraySize);
				break;
			case Diff_ObjectStart:
			{
				destObject = command.object.get();
				objectStack.push(destObject);

				RTTITypeBase* curRtti = destObject->getRTTI();
				while (curRtti != nullptr)
				{
					rttiTypes.push_back(curRtti);
					curRtti = curRtti->getBaseClass();
				}

				// Call base class first, followed by derived classes
				for(auto iter = rttiTypes.rbegin(); iter != rttiTypes.rend(); ++iter)
					(*iter)->onDeserializationStarted(destObject, dummyParams);
			}
				break;
			case Diff_ObjectEnd:
			{
				Stack<RTTITypeBase*> rttiTypes;
				RTTITypeBase* curRtti = destObject->getRTTI();
				while (curRtti != nullptr)
				{
					rttiTypes.push(curRtti);
					curRtti = curRtti->getBaseClass();
				}

				while (!rttiTypes.empty())
				{
					rttiTypes.top()->onDeserializationEnded(destObject, dummyParams);
					rttiTypes.pop();
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
					field->setArrayValue(destObject, command.arrayIdx, command.object);
				}
					break;
				case Diff_Reflectable:
				{
					RTTIReflectableFieldBase* field = static_cast<RTTIReflectableFieldBase*>(command.field);
					field->setArrayValue(destObject, command.arrayIdx, *command.object);
				}
					break;
				case Diff_Plain:
				{
					RTTIPlainFieldBase* field = static_cast<RTTIPlainFieldBase*>(command.field);
					field->arrayElemFromBuffer(destObject, command.arrayIdx, command.value);
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
					field->setValue(destObject, command.object);
				}
					break;
				case Diff_Reflectable:
				{
					RTTIReflectableFieldBase* field = static_cast<RTTIReflectableFieldBase*>(command.field);
					field->setValue(destObject, *command.object);
				}
					break;
				case Diff_Plain:
				{
					RTTIPlainFieldBase* field = static_cast<RTTIPlainFieldBase*>(command.field);
					field->fromBuffer(destObject, command.value);
				}
					break;
				case Diff_DataBlock:
				{
					RTTIManagedDataBlockFieldBase* field = static_cast<RTTIManagedDataBlockFieldBase*>(command.field);
					field->setValue(destObject, command.streamValue, command.size);
				}
					break;
				default:
					break;
				}
			}
		}
	}

	void IDiff::applyDiff(RTTITypeBase* rtti, const SPtr<IReflectable>& object, const SPtr<SerializedObject>& diff, 
		DiffObjectMap& objectMap, Vector<DiffCommand>& diffCommands)
	{
		IDiff& diffHandler = rtti->getDiffHandler();
		diffHandler.applyDiff(object, diff, objectMap, diffCommands);
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
		DiffObjectMap& objectMap, Vector<DiffCommand>& diffCommands)
	{
		static const UnorderedMap<String, UINT64> dummyParams;

		if (object == nullptr || diff == nullptr || object->getTypeId() != diff->getRootTypeId())
			return;

		DiffCommand objStartCommand;
		objStartCommand.field = nullptr;
		objStartCommand.type = Diff_ObjectStart;
		objStartCommand.object = object;

		diffCommands.push_back(objStartCommand);

		Stack<RTTITypeBase*> rttiTypes;
		for (auto& subObject : diff->subObjects)
		{
			for (auto& diffEntry : subObject.entries)
			{
				RTTITypeBase* rtti = IReflectable::_getRTTIfromTypeId(subObject.typeId);
				if (rtti == nullptr)
					continue;

				if (!object->isDerivedFrom(rtti))
					continue;

				rtti->onSerializationStarted(object.get(), dummyParams);
				rttiTypes.push(rtti);

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

					diffCommands.push_back(arraySizeCommand);

					switch (genericField->mType)
					{
					case SerializableFT_ReflectablePtr:
					{
						RTTIReflectablePtrFieldBase* field = static_cast<RTTIReflectablePtrFieldBase*>(genericField);

						UINT32 orgArraySize = genericField->getArraySize(object.get());
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
								diffCommands.push_back(command);
							}
							else
							{
								bool needsNewObject = arrayElem.first >= orgArraySize;

								if (!needsNewObject)
								{
									SPtr<IReflectable> childObj = field->getArrayValue(object.get(), arrayElem.first);
									if (childObj != nullptr)
									{
										IDiff::applyDiff(childObj->getRTTI(), childObj, arrayElemData, objectMap, diffCommands);
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

										IDiff::applyDiff(childRtti, findObj->second, arrayElemData, objectMap, diffCommands);
										command.object = findObj->second;
										diffCommands.push_back(command);
									}
									else
									{
										command.object = nullptr;
										diffCommands.push_back(command);
									}
								}
							}
						}
					}
						break;
					case SerializableFT_Reflectable:
					{
						RTTIReflectableFieldBase* field = static_cast<RTTIReflectableFieldBase*>(genericField);

						UINT32 orgArraySize = genericField->getArraySize(object.get());

						Vector<SPtr<IReflectable>> newArrayElements(numArrayElements);
						UINT32 minArrayLength = std::min(orgArraySize, numArrayElements);
						for (UINT32 i = 0; i < minArrayLength; i++)
						{
							IReflectable& childObj = field->getArrayValue(object.get(), i);
							newArrayElements[i] = BinaryCloner::clone(&childObj, true);
						}

						for (auto& arrayElem : diffArray->entries)
						{
							SPtr<SerializedObject> arrayElemData = std::static_pointer_cast<SerializedObject>(arrayElem.second.serialized);

							if (arrayElem.first < orgArraySize)
							{
								SPtr<IReflectable> childObj = newArrayElements[arrayElem.first];
								IDiff::applyDiff(childObj->getRTTI(), childObj, arrayElemData, objectMap, diffCommands);
							}
							else
							{
								RTTITypeBase* childRtti = IReflectable::_getRTTIfromTypeId(arrayElemData->getRootTypeId());
								if (childRtti != nullptr)
								{
									SPtr<IReflectable> newObject = childRtti->newRTTIObject();
									IDiff::applyDiff(childRtti, newObject, arrayElemData, objectMap, diffCommands);

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

							diffCommands.push_back(command);
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

								diffCommands.push_back(command);
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
							SPtr<IReflectable> childObj = field->getValue(object.get());
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

									IDiff::applyDiff(childRtti, findObj->second, fieldObjectData, objectMap, diffCommands);
									command.object = findObj->second;
								}
								else
								{
									command.object = nullptr;
								}
							}
							else
							{
								IDiff::applyDiff(childObj->getRTTI(), childObj, fieldObjectData, objectMap, diffCommands);
								command.object = childObj;
							}
						}

						diffCommands.push_back(command);
					}
						break;
					case SerializableFT_Reflectable:
					{
						RTTIReflectableFieldBase* field = static_cast<RTTIReflectableFieldBase*>(genericField);
						SPtr<SerializedObject> fieldObjectData = std::static_pointer_cast<SerializedObject>(diffData);

						IReflectable& childObj = field->getValue(object.get());
						SPtr<IReflectable> clonedObj = BinaryCloner::clone(&childObj, true);

						IDiff::applyDiff(clonedObj->getRTTI(), clonedObj, fieldObjectData, objectMap, diffCommands);

						DiffCommand command;
						command.field = genericField;
						command.type = Diff_Reflectable;
						command.object = clonedObj;

						diffCommands.push_back(command);
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

							diffCommands.push_back(command);
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

						diffCommands.push_back(command);
					}
						break;
					}
				}
			}
		}

		DiffCommand objEndCommand;
		objEndCommand.field = nullptr;
		objEndCommand.type = Diff_ObjectEnd;
		objEndCommand.object = object;

		diffCommands.push_back(objEndCommand);

		while (!rttiTypes.empty())
		{
			rttiTypes.top()->onSerializationEnded(object.get(), dummyParams);
			rttiTypes.pop();
		}
	}
}