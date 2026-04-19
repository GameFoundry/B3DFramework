//********************************* B3D Framework - Copyright 2026 Marko Pintera *****************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "Reflection/B3DRTTIPlain.h"
#include "Reflection/B3DRTTIType.h"
#include "Serialization/B3DManagedSerializableList.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-SEngine
	 *  @{
	 */

	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableListRTTI : public TRTTIType<ManagedSerializableList, IReflectable, ManagedSerializableListRTTI>
	{
		TArray<SPtr<ManagedSerializableFieldData>> mListEntries;

		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(mListTypeInfo, 0)
			B3D_RTTI_MEMBER(mNumElements, 1)
			B3D_RTTI_GENERATED_MEMBER_CONTAINER(mListEntries, 2)
		B3D_RTTI_END_MEMBERS

	public:
		void OnOperationStarted(ManagedSerializableList& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::ReadBit))
			{
				const u32 entryCount = object.mNumElements;
				mListEntries.reserve(entryCount);

				for(u32 entryIndex = 0; entryIndex < entryCount; ++entryIndex)
					mListEntries.Add(object.GetFieldData(entryIndex));
			}
		}

		void OnOperationEnded(ManagedSerializableList& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::WriteBit))
			{
				const u32 entryCount = (u32)mListEntries.size();
				object.mCachedEntries.resize(entryCount);

				for(u32 entryIndex = 0; entryIndex < entryCount; ++entryIndex)
					object.SetFieldData(entryIndex, mListEntries[entryIndex]);
			}
		}

		const String& GetRttiName()
		{
			static String name = "ScriptSerializableList";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ScriptSerializableList;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return ManagedSerializableList::CreateEmpty();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
