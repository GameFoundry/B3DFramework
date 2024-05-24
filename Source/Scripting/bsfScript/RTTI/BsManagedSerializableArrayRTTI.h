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

	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableArrayRTTI : public RTTIType<ManagedSerializableArray, IReflectable, ManagedSerializableArrayRTTI>
	{
		Vector<SPtr<ManagedSerializableFieldData>> mArrayEntries;

		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(mArrayTypeInfo, 0)
			B3D_RTTI_MEMBER(mElemSize, 1)
			B3D_RTTI_MEMBER_CONTAINER(mNumElements, 2)
			B3D_RTTI_GENERATED_MEMBER_CONTAINER(mArrayEntries, 3)
		B3D_RTTI_END_MEMBERS

	public:
		void OnOperationStarted(ManagedSerializableArray& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::ReadBit))
			{
				const u32 arrayLength = object.GetTotalLength();
				mArrayEntries.reserve(arrayLength);

				for(u32 arrayElementIndex = 0; arrayElementIndex < arrayLength; ++arrayElementIndex)
				{
					mArrayEntries.emplace_back(object.GetFieldData(arrayElementIndex));
				}
			}
		}

		void OnOperationEnded(ManagedSerializableArray& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::WriteBit))
			{
				const u32 arrayLength = (u32)mArrayEntries.size();
				object.mCachedEntries.resize(arrayLength);

				for(u32 arrayElementIndex = 0; arrayElementIndex < arrayLength; ++arrayElementIndex)
					object.SetFieldData(arrayElementIndex, mArrayEntries[arrayElementIndex]);
			}
		}

		const String& GetRttiName()
		{
			static String name = "ScriptSerializableArray";
			return name;
		}

		u32 GetRttiId() const override
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
