//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Reflection/BsRTTIPlain.h"
#include "Scene/BsGameObjectManager.h"
#include "Serialization/BsManagedSerializableArray.h"
#include "Serialization/BsScriptAssemblyManager.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-SEngine
	 *  @{
	 */

	class BS_SCR_BE_EXPORT ManagedSerializableArrayRTTI : public RTTIType<ManagedSerializableArray, IReflectable, ManagedSerializableArrayRTTI>
	{
	private:
		SPtr<ManagedSerializableTypeInfoArray> GetTypeInfo(ManagedSerializableArray* obj)
		{
			return obj->mArrayTypeInfo;
		}

		void SetTypeInfo(ManagedSerializableArray* obj, SPtr<ManagedSerializableTypeInfoArray> val)
		{
			obj->mArrayTypeInfo = val;
		}

		u32& GetElementSize(ManagedSerializableArray* obj)
		{
			return (u32&)obj->mElemSize;
		}

		void SetElementSize(ManagedSerializableArray* obj, u32& numElements)
		{
			obj->mElemSize = numElements;
		}

		u32& GetNumElements(ManagedSerializableArray* obj, u32 arrayIdx)
		{
			return (u32&)obj->mNumElements[arrayIdx];
		}

		void SetNumElements(ManagedSerializableArray* obj, u32 arrayIdx, u32& numElements)
		{
			obj->mNumElements[arrayIdx] = numElements;
		}

		u32 GetNumElementsNumEntries(ManagedSerializableArray* obj)
		{
			return (u32)obj->mNumElements.size();
		}

		void SetNumElementsNumEntries(ManagedSerializableArray* obj, u32 numEntries)
		{
			obj->mNumElements.resize(numEntries);
		}

		SPtr<ManagedSerializableFieldData> GetArrayEntry(ManagedSerializableArray* obj, u32 arrayIdx)
		{
			return obj->GetFieldData(arrayIdx);
		}

		void SetArrayEntry(ManagedSerializableArray* obj, u32 arrayIdx, SPtr<ManagedSerializableFieldData> val)
		{
			obj->SetFieldData(arrayIdx, val);
		}

		u32 GetNumArrayEntries(ManagedSerializableArray* obj)
		{
			return obj->GetTotalLength();
		}

		void SetNumArrayEntries(ManagedSerializableArray* obj, u32 numEntries)
		{
			obj->mCachedEntries = Vector<SPtr<ManagedSerializableFieldData>>(numEntries);
		}

	public:
		ManagedSerializableArrayRTTI()
		{
			AddReflectablePtrField("mArrayTypeInfo", 0, &ManagedSerializableArrayRTTI::GetTypeInfo, &ManagedSerializableArrayRTTI::SetTypeInfo);
			AddPlainField("mElementSize", 1, &ManagedSerializableArrayRTTI::GetElementSize, &ManagedSerializableArrayRTTI::SetElementSize);
			AddPlainArrayField("mNumElements", 2, &ManagedSerializableArrayRTTI::GetNumElements, &ManagedSerializableArrayRTTI::GetNumElementsNumEntries, &ManagedSerializableArrayRTTI::SetNumElements, &ManagedSerializableArrayRTTI::SetNumElementsNumEntries);
			AddReflectablePtrArrayField("mArrayEntries", 3, &ManagedSerializableArrayRTTI::GetArrayEntry, &ManagedSerializableArrayRTTI::GetNumArrayEntries, &ManagedSerializableArrayRTTI::SetArrayEntry, &ManagedSerializableArrayRTTI::SetNumArrayEntries);
		}

		const String& GetRttiName()
		{
			static String name = "ScriptSerializableArray";
			return name;
		}

		u32 GetRttiId()
		{
			return TID_ScriptSerializableArray;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return ManagedSerializableArray::CreateNew();
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
