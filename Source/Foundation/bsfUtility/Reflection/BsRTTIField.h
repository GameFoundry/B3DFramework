//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include <utility>
#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Reflection/BsRTTIPlain.h"
#include "Reflection/BsRTTIIterator.h"
#include "Reflection/BsIReflectable.h"
#include "Utility/BsAny.h"

namespace bs
{
	class RTTITypeBase;
	struct RTTISchema;

	/** @addtogroup Internal-Utility
	 *  @{
	 */

	/** @addtogroup RTTI-Internal
	 *  @{
	 */

	/**
	 * Data types we can serialize:
	 *
	 * - Reflectable - Reference to an object implementing the IReflectable interface. Each field in its RTTI information will be iterated over
	 *				   and visited recursively. Fields can contain other IReflectable objects, which will be iterated recursively.
	 *				   Supports versioning via field IDs, so you are freed to add/remove fields as long as they have unique IDs.
	 * 
	 * - Reflectable pointer - A SPtr<IReflectable>. Same as Reflectable type, except the object is not referenced by value, and is instead referenced by pointer.
	 *						   If multiple fields point to the same object the serialization system will ensure a single object instance is serialized and deserialized.
	 *
	 * - Plain - Native data types, POD (Plain old data) structures, or in general types we don't want to (or can't) inherit from IReflectable.
	 *			 Type must specialize RTTIPlainType<T> template.
	 *
	 * - DataBlock - Array of bytes of a certain size. When returning a data block you may specify if its managed or unmanaged.
	 *				 Managed data blocks have their buffers deleted after they go out of scope. This is useful if you need to return some
	 *				 temporary data. On the other hand if the data in the block belongs to your class, and isn't temporary, keep the data unmanaged.
	 */
	enum class RTTIFieldDataType
	{
		Plain,
		DataBlock,
		Reflectable,
		ReflectablePointer
	};

	/** Information about a type stored in a RTTIField. A single field can hold one or multiple types (e.g. in case of a map entry it will store a key/value pair). */
	struct B3D_UTILITY_EXPORT RTTIFieldTypeSchema : IReflectable
	{
		RTTIFieldTypeSchema() = default;
		RTTIFieldTypeSchema(bool hasDynamicSize, BitLength fixedSize, RTTIFieldDataType type, u32 fieldTypeId, SPtr<RTTISchema> fieldTypeSchema)
			: HasDynamicSize(hasDynamicSize), FixedSize(fixedSize), Type(type), FieldTypeId(fieldTypeId), FieldTypeSchema(std::move(fieldTypeSchema))
		{}

		bool HasDynamicSize = false;
		BitLength FixedSize = 0;
		RTTIFieldDataType Type = RTTIFieldDataType::Plain;
		u32 FieldTypeId = 0;
		SPtr<RTTISchema> FieldTypeSchema;

		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;
	};

	/** Contains serializable meta-data about a single RTTI field. */
	struct B3D_UTILITY_EXPORT RTTIFieldSchema : IReflectable
	{
		RTTIFieldSchema() = default;
		RTTIFieldSchema(i16 id, bool isArray, bool isIterator, const RTTIFieldInfo& info)
			: Id(id), IsContainer(isArray), IsIterator(isIterator), Info(info)
		{}
		RTTIFieldSchema(i16 id, bool isArray, bool hasDynamicSize, BitLength size, RTTIFieldDataType type, u32 fieldTypeId, SPtr<RTTISchema> fieldTypeSchema, const RTTIFieldInfo& info)
			: Id(id), IsContainer(isArray), HasDynamicSize(hasDynamicSize), Size(size), Type(type), FieldTypeId(fieldTypeId), FieldTypeSchema(std::move(fieldTypeSchema)), Info(info)
		{}

		u16 Id = 0;
		bool IsContainer = false;
		bool IsIterator = false; // DEPRECATED - Replace to GetFieldType() method that either returns Iterator or DataBlock
		bool HasDynamicSize = false; // DEPRECATED - Stored in FieldTypes now
		BitLength Size = 0; // DEPRECATED - Stored in FieldTypes now
		RTTIFieldDataType Type = RTTIFieldDataType::Plain; // DEPRECATED - Stored in FieldTypes now
		u32 FieldTypeId = 0; // DEPRECATED - Stored in FieldTypes now
		SPtr<RTTISchema> FieldTypeSchema; // DEPRECATED - Stored in FieldTypes now
		RTTIFieldInfo Info;
		TInlineArray<RTTIFieldTypeSchema, 2> FieldTypes; /**< Types references by the field. In 99% of the cases this is a single type, but in case of e.g. a map it will be two types (key/value pair). */

		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;
	};

	/**
	 * Structure that keeps meta-data concerning a single class field. You can use this data for setting and getting values
	 * for that field on a specific class instance.
	 *
	 * Class also contains an unique field name, and an unique field ID. Fields may contain single types or an array of types.
	 * See SerializableFieldType for information about specific field types.
	 *
	 * @note
	 * Most of the methods for retrieving and setting data accept "void *" for both the data and the owning class instance.
	 * It is up to the caller to ensure that pointer is of proper type.
	 */
	struct B3D_UTILITY_EXPORT RTTIField
	{
		String Name;
		RTTIFieldSchema Schema;

		virtual ~RTTIField() = default;

		/** Initializes the field's RTTI schema. Should be called once after construction. */
		virtual void InitSchema() {}

		/**
		 * Throws an exception depending if the field is or isn't an array.
		 *
		 * @param[in]	array	If true, then exception will be thrown if field is not an array.
		 * 						If false, then it will be thrown if field is an array.
		 */
		void CheckIsArray(bool array) const;

	protected:
		void Init(String name, const RTTIFieldSchema& schema)
		{
			this->Name = std::move(name);
			this->Schema = schema;
		}
	};

	/** @} */
	/** @} */
} // namespace bs
