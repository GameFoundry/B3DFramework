//********************************* B3D Framework - Copyright 2026 Marko Pintera *****************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "Serialization/B3DScriptAssemblyManager.h"
#include "Serialization/B3DManagedSerializableObject.h"
#include "Serialization/B3DManagedSerializableField.h"
#include "Serialization/B3DManagedDelta.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-SEngine
	 *  @{
	 */

	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableObjectRTTI : public TRTTIType<ManagedSerializableObject, IReflectable, ManagedSerializableObjectRTTI>
	{
		TArray<SPtr<ManagedSerializableFieldDataEntry>> mFieldEntries;

		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(mObjInfo, 0)
			B3D_RTTI_GENERATED_MEMBER_CONTAINER(mFieldEntries, 1)
		B3D_RTTI_END_MEMBERS

	public:
		void OnOperationStarted(ManagedSerializableObject& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::ReadBit))
			{
				SPtr<ManagedObjectInfo> currentTypeObjectInfo = object.mObjInfo;
				while(currentTypeObjectInfo != nullptr)
				{
					for(const auto& memberInfo : currentTypeObjectInfo->Members)
					{
						if(memberInfo->IsSerializable())
						{
							const SPtr<ManagedSerializableFieldKey> fieldKey = ManagedSerializableFieldKey::Create((u16)memberInfo->ParentTypeId, (u16)memberInfo->FieldId);
							const SPtr<ManagedSerializableFieldData> fieldData = object.GetFieldData(memberInfo);

							mFieldEntries.Add(ManagedSerializableFieldDataEntry::Create(fieldKey, fieldData));
						}
					}

					currentTypeObjectInfo = currentTypeObjectInfo->BaseClass;
				}
			}
		}

		void OnOperationEnded(ManagedSerializableObject& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::WriteBit))
			{
				const u32 entryCount = (u32)mFieldEntries.size();

				for(u32 entryIndex = 0; entryIndex < entryCount; ++entryIndex)
					object.mCachedData[*mFieldEntries[entryIndex]->MKey] = mFieldEntries[entryIndex]->MValue;
			}
		}

		IDeltaHandler& GetDeltaHandler() const
		{
			static ManagedDeltaHandler managedDiffHandler;
			return managedDiffHandler;
		}

		const String& GetRttiName()
		{
			static String name = "ScriptSerializableObject";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ScriptSerializableObject;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return ManagedSerializableObject::CreateEmpty();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
