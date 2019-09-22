//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include <utility>
#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Reflection/BsRTTIPlain.h"
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
	 * Types of fields we can serialize:
	 * 			
	 * - Plain - Native data types, POD (Plain old data) structures, or in general types we don't want to (or can't) inherit from IReflectable.
	 *			 Type must be copyable by memcpy.
	 *			
	 * - DataBlock - Array of bytes of a certain size. When returning a data block you may specify if its managed or unmanaged.
	 *				 Managed data blocks have their buffers deleted after they go out of scope. This is useful if you need to return some
	 *				 temporary data. On the other hand if the data in the block belongs to your class, and isn't temporary, keep the data unmanaged.
	 *				
	 * - Reflectable - Field that is of IReflectable type. Cannot be a pointer to IReflectable and must be actual value type.
	 *				   Type and its fields are serialized recursively. Supports versioning so you may add/remove fields from the type
	 *				   without breaking previously serialized data.
	 *
	 * - ReflectablePtr - A pointer to IReflectable. Same as "Reflectable" except that data isn't serialized as a value type,
	 *					  but as a pointer, which may be referenced by multiple other instances. All references are saved upon
	 *					  serialization and restored upon deserialization.
	 */
	enum SerializableFieldType
	{
		SerializableFT_Plain,
		SerializableFT_DataBlock,
		SerializableFT_Reflectable,
		SerializableFT_ReflectablePtr
	};

	/** Contains serializable meta-data about a single RTTI field. */
	struct BS_UTILITY_EXPORT RTTIFieldSchema : IReflectable
	{
		RTTIFieldSchema() = default;
		RTTIFieldSchema(INT16 id, bool isArray, bool hasDynamicSize, BitLength size, SerializableFieldType type,
			UINT32 fieldTypeId, SPtr<RTTISchema> fieldTypeSchema, const RTTIFieldInfo& info)
			: id(id), isArray(isArray), hasDynamicSize(hasDynamicSize), size(size), type(type)
			, fieldTypeId(fieldTypeId), fieldTypeSchema(std::move(fieldTypeSchema)), info(info)
		{ }
		
		UINT16 id = 0;
		bool isArray = false;
		bool hasDynamicSize = false;
		BitLength size = 0;
		SerializableFieldType type = SerializableFT_Plain;
		UINT32 fieldTypeId = 0;
		SPtr<RTTISchema> fieldTypeSchema;
		RTTIFieldInfo info;

		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
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
	struct BS_UTILITY_EXPORT RTTIField
	{	
		String name;
		RTTIFieldSchema schema;
		
		virtual ~RTTIField() = default;

		/**
		 * Gets the size of an array contained by the field, if the field represents an array. Throws exception if field
		 * is not an array.
		 */
		virtual UINT32 getArraySize(RTTITypeBase* rtti, void* object) = 0;

		/**
		 * Changes the size of an array contained by the field, if the field represents an array. Throws exception if field
		 * is not an array.
		 */
		virtual void setArraySize(RTTITypeBase* rtti, void* object, UINT32 size) = 0;

		/** Initializes the field's RTTI schema. Should be called once after construction. */
		virtual void initSchema() {}
		
		/**
		 * Throws an exception depending if the field is or isn't an array.
		 *
		 * @param[in]	array	If true, then exception will be thrown if field is not an array.
		 * 						If false, then it will be thrown if field is an array.
		 */
		void checkIsArray(bool array) const;

	protected:
		void init(String name, const RTTIFieldSchema& schema)
		{
			this->name = std::move(name);
			this->schema = schema;
		}
	};

	/** @} */
	/** @} */
}
