//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisitesUtil.h"
#include "BsAny.h"

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

	/** Various flags you can assign to RTTI fields. */
	enum RTTIFieldFlag
	{
		/** 
		 * This flag is only used on field types of ReflectablePtr type, and it is used
		 * to solve circular references. Circular references cause an issue when deserializing, 
		 * as the algorithm doesn't know which object to deserialize first. By making one of 
		 * the references weak, you tell the algorithm that it doesn't have to guarantee 
		 * the object will be fully deserialized before being assigned to the field.
		 *
		 * In short: If you make a reference weak, when "set" method of that field is called,
		 * it is not guaranteed the value provided is fully initialized, so you should not access any of its
		 * data until deserialization is fully complete. You only need to use this flag if the RTTI system
		 * complains that is has found a circular reference.
		 */
		RTTI_Flag_WeakRef = 0x01,
		/** 
		 * This flags signals various systems that the flagged field should not be searched when looking for 
		 * object references. This normally means the value of this field will no be retrieved during reference
		 * searches but it will likely still be retrieved during other operations (for example serialization).
		 * This is used as an optimization to avoid retrieving values of potentially very expensive fields that
		 * would not contribute to the reference search anyway. Whether or not a field contributes to the reference
		 * search depends on the search and should be handled on a case by case basis.
		 */
		RTTI_Flag_SkipInReferenceSearch = 0x02
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
		Any valueGetter;
		Any valueSetter;

		Any arraySizeGetter;
		Any arraySizeSetter;

		String mName;
		UINT16 mUniqueId;
		bool mIsVectorType;
		SerializableFieldType mType;
		UINT64 mFlags;

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

		/** Returns flags that were set in the field meta-data. */
		UINT64 getFlags() const { return mFlags; }

		/**
		 * Gets the size of an array contained by the field, if the field represents an array. Throws exception if field 
		 * is not an array.
		 */
		virtual UINT32 getArraySize(void* object) = 0;

		/**
		 * Changes the size of an array contained by the field, if the field represents an array. Throws exception if field 
		 * is not an array.
		 */
		virtual void setArraySize(void* object, UINT32 size) = 0;

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
		void initAll(Any valueGetter, Any valueSetter, Any arraySizeGetter, Any arraySizeSetter,
			String mName, UINT16 mUniqueId, bool mIsVectorType, SerializableFieldType type, UINT64 flags)
		{
			this->valueGetter = valueGetter;
			this->valueSetter = valueSetter;
			this->arraySizeGetter = arraySizeGetter;
			this->arraySizeSetter = arraySizeSetter;
			this->mName = mName;
			this->mUniqueId = mUniqueId;
			this->mIsVectorType = mIsVectorType;
			this->mType = type;
			this->mFlags = flags;
		}
	};

	/** @} */
	/** @} */
}