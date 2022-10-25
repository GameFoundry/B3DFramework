//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Reflection/BsRTTIPlain.h"
#include "Reflection/BsRTTIType.h"
#include "Serialization/BsManagedSerializableList.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-SEngine
	 *  @{
	 */

	class BS_SCR_BE_EXPORT ManagedSerializableListRTTI : public RTTIType<ManagedSerializableList, IReflectable, ManagedSerializableListRTTI>
	{
	private:
		SPtr<ManagedSerializableTypeInfoList> GetTypeInfo(ManagedSerializableList* obj)
		{
			return obj->mListTypeInfo;
		}

		void SetTypeInfo(ManagedSerializableList* obj, SPtr<ManagedSerializableTypeInfoList> val)
		{
			obj->mListTypeInfo = val;
		}

		u32& GetNumElements(ManagedSerializableList* obj)
		{
			return (u32&)obj->mNumElements;
		}

		void SetNumElements(ManagedSerializableList* obj, u32& numElements)
		{
			obj->mNumElements = numElements;
		}

		SPtr<ManagedSerializableFieldData> GetListEntry(ManagedSerializableList* obj, u32 arrayIdx)
		{
			return obj->GetFieldData(arrayIdx);
		}

		void SetListEntry(ManagedSerializableList* obj, u32 arrayIdx, SPtr<ManagedSerializableFieldData> val)
		{
			obj->SetFieldData(arrayIdx, val);
		}

		u32 GetNumListEntries(ManagedSerializableList* obj)
		{
			return (u32)obj->mNumElements;
		}

		void SetNumListEntries(ManagedSerializableList* obj, u32 numEntries)
		{
			obj->mCachedEntries = Vector<SPtr<ManagedSerializableFieldData>>(numEntries);
		}

	public:
		ManagedSerializableListRTTI()
		{
			AddReflectablePtrField("mListTypeInfo", 0, &ManagedSerializableListRTTI::GetTypeInfo, &ManagedSerializableListRTTI::SetTypeInfo);
			AddPlainField("mNumElements", 1, &ManagedSerializableListRTTI::GetNumElements, &ManagedSerializableListRTTI::SetNumElements);
			AddReflectablePtrArrayField("mListEntries", 2, &ManagedSerializableListRTTI::GetListEntry, &ManagedSerializableListRTTI::GetNumListEntries, &ManagedSerializableListRTTI::SetListEntry, &ManagedSerializableListRTTI::SetNumListEntries);
		}

		const String& GetRttiName()
		{
			static String name = "ScriptSerializableList";
			return name;
		}

		u32 GetRttiId()
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
} // namespace bs
