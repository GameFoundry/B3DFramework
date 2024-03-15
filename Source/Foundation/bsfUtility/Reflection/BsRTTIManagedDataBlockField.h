//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Reflection/BsRTTIField.h"

namespace bs
{
	/** @addtogroup Internal-Utility
	 *  @{
	 */

	/** @addtogroup RTTI-Internal
	 *  @{
	 */

	/**
	 * Base class containing common functionality for a managed data block class field.
	 *
	 * @note
	 * Managed data blocks are just blocks of memory that may, or may not be released automatically when they are no longer
	 * referenced. They are useful when wanting to return some temporary data only for serialization purposes.
	 */
	struct RTTIManagedDataBlockFieldBase : public RTTIField
	{
		/** Retrieves a managed data block from the specified instance. */
		virtual SPtr<DataStream> GetValue(RTTITypeBase* rtti, void* object, u32& size) = 0;

		/** Sets a managed data block on the specified instance. */
		virtual void SetValue(RTTITypeBase* rtti, void* object, const SPtr<DataStream>& data, u32 size) = 0;
	};

	/** Class containing a managed data block field containing a specific type. */
	template <class InterfaceType, class DataType, class ObjectType>
	struct RTTIManagedDataBlockField : public RTTIManagedDataBlockFieldBase
	{
		typedef SPtr<DataStream> (InterfaceType::*GetterType)(ObjectType*, u32&);
		typedef void (InterfaceType::*SetterType)(ObjectType*, const SPtr<DataStream>&, u32);

		/**
		 * Initializes a field that returns a block of bytes. Can be used for serializing pretty much anything.
		 *
		 * @param[in]	name			Name of the field.
		 * @param[in]	uniqueId		Unique identifier for this field. Although name is also a unique identifier we want a
		 *								small data type that can be used for efficiently serializing data to disk and similar.
		 *								It is primarily used for compatibility between different versions of serialized data.
		 * @param[in]	getter  		The getter method for the field.
		 * @param[in]	setter  		The setter method for the field.
		 * @param[in]	info			Various optional information about the field.
		 */
		void InitSingle(String name, u16 uniqueId, GetterType getter, SetterType setter, const RTTIFieldInfo& info)
		{
			this->getter = getter;
			this->setter = setter;

			Init(std::move(name), RTTIFieldSchema(uniqueId, false, true, 0, SerializableFT_DataBlock, 0, nullptr, info));
		}

		void InitSchema() override
		{
			// Add the new schema type
			RTTIFieldTypeSchema fieldTypeSchema;
			fieldTypeSchema.FieldTypeId = Schema.FieldTypeId;
			fieldTypeSchema.FieldTypeSchema = Schema.FieldTypeSchema;
			fieldTypeSchema.Type = Schema.Type;
			fieldTypeSchema.FixedSize = Schema.Size;
			fieldTypeSchema.HasDynamicSize = Schema.HasDynamicSize;

			Schema.FieldTypes.Add(fieldTypeSchema);
		}

		u32 GetArraySize(RTTITypeBase* rtti, void* object) override
		{
			B3D_EXCEPT(InternalErrorException, "Data block types don't support arrays.");

			return 0;
		}

		void SetArraySize(RTTITypeBase* rtti, void* object, u32 size) override
		{
			B3D_EXCEPT(InternalErrorException, "Data block types don't support arrays.");
		}

		SPtr<DataStream> GetValue(RTTITypeBase* rtti, void* object, u32& size) override
		{
			InterfaceType* rttiObject = static_cast<InterfaceType*>(rtti);
			ObjectType* castObj = static_cast<ObjectType*>(object);

			return (rttiObject->*getter)(castObj, size);
		}

		void SetValue(RTTITypeBase* rtti, void* object, const SPtr<DataStream>& value, u32 size) override
		{
			InterfaceType* rttiObject = static_cast<InterfaceType*>(rtti);
			ObjectType* castObj = static_cast<ObjectType*>(object);

			(rttiObject->*setter)(castObj, value, size);
		}

	private:
		GetterType getter;
		SetterType setter;
	};

	/** @} */
	/** @} */
} // namespace bs
