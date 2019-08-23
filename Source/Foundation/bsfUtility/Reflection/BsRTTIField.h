//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Reflection/BsRTTIPlain.h"
#include "Utility/BsAny.h"

namespace bs
{
	class RTTITypeBase;

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
		String mName;
		UINT16 mUniqueId;
		bool mIsVectorType;
		SerializableFieldType mType;
		RTTIFieldInfo mInfo;

		virtual ~RTTIField() = default;

		/** Checks is the field plain type and castable to RTTIPlainFieldBase. */
		bool isPlainType() const { return mType == SerializableFT_Plain; }

		/** Checks is the field a data block type and castable to RTTIManagedDataBlockFieldBase. */
		bool isDataBlockType() const { return mType == SerializableFT_DataBlock; }

		/** Checks is the field a reflectable type and castable to RTTIReflectableFieldBase. */
		bool isReflectableType() const { return mType == SerializableFT_Reflectable; }

		/** Checks is the field a reflectable pointer type and castable to RTTIReflectablePtrFieldBase. */
		bool isReflectablePtrType() const { return mType == SerializableFT_ReflectablePtr; }

		/** Checks is the field contains an array or a single entry. */
		bool isArray() const { return mIsVectorType; }

		/** Returns additional information about the field. */
		const RTTIFieldInfo& getInfo() const { return mInfo; }

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

		/** Returns the type id for the type used in this field. */
		virtual UINT32 getTypeSize() = 0;

		/**
		 * Query if the field has dynamic size.
		 *
		 * @note	
		 * Field should have dynamic size if:
		 *  - The field can have varying size
		 * 	- The field size is over 255
		 * @note			
		 * Types like integers, floats, bools, POD structs dont have dynamic size.
		 * Types like strings, vectors, maps do.
		 * @note		
		 * If your type has a static size but that size exceeds 255 bytes you also need to
		 * use dynamic field size. (You will be warned during compilation if you don't follow this rule)
		 */
		virtual bool hasDynamicSize() = 0;

		/**
		 * Throws an exception if this field doesn't contain a plain value.
		 *
		 * @param[in]	array	If true then the field must support plain array type.
		 */
		void checkIsPlain(bool array);

		/**
		 * Throws an exception if this field doesn't contain a complex value.
		 *
		 * @param[in]	array	If true then the field must support complex array type.
		 */
		void checkIsComplex(bool array);

		/**
		 * Throws an exception if this field doesn't contain a complex pointer value.
		 *
		 * @param[in]	array	If true then the field must support complex pointer array type.
		 */
		void checkIsComplexPtr(bool array);

		/**
		 * Throws an exception depending if the field is or isn't an array.
		 *
		 * @param[in]	array	If true, then exception will be thrown if field is not an array.
		 * 						If false, then it will be thrown if field is an array.
		 */
		void checkIsArray(bool array);

		/** Throws an exception if this field doesn't contain a data block value. */
		void checkIsDataBlock();

	protected:
		void init(String name, UINT16 uniqueId, bool isVectorType, SerializableFieldType type, const RTTIFieldInfo& info)
		{
			this->mName = std::move(name);
			this->mUniqueId = uniqueId;
			this->mIsVectorType = isVectorType;
			this->mType = type;
			this->mInfo = info;
		}
	};

	/** @} */
	/** @} */
}
