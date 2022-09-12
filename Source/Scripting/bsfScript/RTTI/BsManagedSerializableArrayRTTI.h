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

		UINT32& GetElementSize(ManagedSerializableArray* obj)
		{
			return (UINT32&)obj->mElemSize;
		}

		void SetElementSize(ManagedSerializableArray* obj, UINT32& numElements)
		{
			obj->mElemSize = numElements;
		}

		UINT32& GetNumElements(ManagedSerializableArray* obj, UINT32 arrayIdx)
		{
			return (UINT32&)obj->mNumElements[arrayIdx];
		}

		void SetNumElements(ManagedSerializableArray* obj, UINT32 arrayIdx, UINT32& numElements)
		{
			obj->mNumElements[arrayIdx] = numElements;
		}

		UINT32 GetNumElementsNumEntries(ManagedSerializableArray* obj)
		{
			return (UINT32)obj->mNumElements.Size();
		}

		void SetNumElementsNumEntries(ManagedSerializableArray* obj, UINT32 numEntries)
		{
			obj->mNumElements.Resize(numEntries);
		}

		SPtr<ManagedSerializableFieldData> GetArrayEntry(ManagedSerializableArray* obj, UINT32 arrayIdx)
		{
			return obj->GetFieldData(arrayIdx);
		}

		void SetArrayEntry(ManagedSerializableArray* obj, UINT32 arrayIdx, SPtr<ManagedSerializableFieldData> val)
		{
			obj->SetFieldData(arrayIdx, val);
		}

		UINT32 GetNumArrayEntries(ManagedSerializableArray* obj)
		{
			return obj->GetTotalLength();
		}

		void SetNumArrayEntries(ManagedSerializableArray* obj, UINT32 numEntries)
		{
			obj->mCachedEntries = Vector<SPtr<ManagedSerializableFieldData>>(numEntries);
		}

	public:
		ManagedSerializableArrayRTTI()
		{
			addReflectablePtrField("mArrayTypeInfo", 0, &ManagedSerializableArrayRTTI::getTypeInfo, &ManagedSerializableArrayRTTI::setTypeInfo);
			addPlainField("mElementSize", 1, &ManagedSerializableArrayRTTI::getElementSize, &ManagedSerializableArrayRTTI::setElementSize);
			addPlainArrayField("mNumElements", 2, &ManagedSerializableArrayRTTI::getNumElements, &ManagedSerializableArrayRTTI::getNumElementsNumEntries,
				&ManagedSerializableArrayRTTI::setNumElements, &ManagedSerializableArrayRTTI::setNumElementsNumEntries);
			addReflectablePtrArrayField("mArrayEntries", 3, &ManagedSerializableArrayRTTI::getArrayEntry, &ManagedSerializableArrayRTTI::getNumArrayEntries,
				&ManagedSerializableArrayRTTI::setArrayEntry, &ManagedSerializableArrayRTTI::setNumArrayEntries);
		}

		const String& GetRTTIName() override
		{
			static String name = "ScriptSerializableArray";
			return name;
		}

		UINT32 GetRTTIId() override
		{
			return TID_ScriptSerializableArray;
		}

		SPtr<IReflectable> NewRTTIObject() override
		{
			return ManagedSerializableArray::CreateNew();
		}
	};

	/** @} */
	/** @endcond */
}
