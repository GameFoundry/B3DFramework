//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Reflection/B3DRTTIField.h"

namespace b3d
{
	/** @addtogroup Internal-Utility
	 *  @{
	 */

	/** @addtogroup RTTI-Internal
	 *  @{
	 */

	/**
	 * Base class containing common functionality for a data block class field.
	 *
	 * Data block fields contain blocks of memory that may, or may not be released automatically when they are no longer
	 * referenced. When such fields are being deserialized, the stream will point directly to the location we're deserializing
	 * from (e.g. a file). This allows the caller either to immediately read the data, or store the stream pointer for a streaming
	 * read later.
	 */
	struct RTTIDataBlockFieldBase : public RTTIField
	{
		/** Retrieves a data block from the specified instance. */
		virtual TShared<DataStream> GetValue(RTTIType* rtti, void* object, u32& size) = 0;

		/** Sets a data block on the specified instance. */
		virtual void SetValue(RTTIType* rtti, void* object, const TShared<DataStream>& data, u32 size) = 0;
	};

	/** Class containing a data block field containing a specific type. */
	template <class InterfaceType, class DataType, class ObjectType>
	struct RTTIDataBlockField : public RTTIDataBlockFieldBase
	{
		typedef TShared<DataStream> (InterfaceType::*GetterType)(ObjectType*, u32&);
		typedef void (InterfaceType::*SetterType)(ObjectType*, const TShared<DataStream>&, u32);

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

			Init(std::move(name), RTTIFieldSchema(uniqueId, false, RTTIFieldType::DataBlock,  info));
		}

		void InitSchema() override
		{
			// Add the new schema type
			RTTIFieldDataTypeSchema fieldTypeSchema;
			fieldTypeSchema.FieldTypeId = 0;
			fieldTypeSchema.FieldTypeSchema = nullptr;
			fieldTypeSchema.Type = RTTIFieldDataType::DataBlock;
			fieldTypeSchema.FixedSize = 0;
			fieldTypeSchema.HasDynamicSize = false;

			Schema.FieldDataTypes.Add(fieldTypeSchema);
		}

		TShared<DataStream> GetValue(RTTIType* rtti, void* object, u32& size) override
		{
			InterfaceType* rttiObject = static_cast<InterfaceType*>(rtti);
			ObjectType* castObj = static_cast<ObjectType*>(object);

			return (rttiObject->*getter)(castObj, size);
		}

		void SetValue(RTTIType* rtti, void* object, const TShared<DataStream>& value, u32 size) override
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
} // namespace b3d
