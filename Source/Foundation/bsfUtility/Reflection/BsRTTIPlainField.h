//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Reflection/BsRTTIField.h"
#include "Error/BsException.h"

namespace bs
{
	/** @addtogroup Internal-Utility
	 *  @{
	 */

	/** @addtogroup RTTI-Internal
	 *  @{
	 */

	/**
	 * Base class containing common functionality for a plain class field.
	 * 			
	 * @note	
	 * Plain fields are considered those that may be serialized directly by copying their memory. (All built-in types,
	 * strings, etc.)
	 */
	struct RTTIPlainFieldBase : public RTTIField
	{
		virtual ~RTTIPlainFieldBase() = default;

		/** Throws an exception if the current field type and provided template types don't match. */
		template<class DataType>
		void checkType()
		{
			// TODO: Low priority. Because I wanted to get rid of SerializableType I have no way of checking the actual
			// type of the field and the type provided to get/set methods matches

			/*if(mType.id != SerializableType<DataType>().id)
			{
				BS_EXCEPT(InternalErrorException,
					"Invalid field type.",
					"SerializableSimpleTypeFieldBase::checkType()");
			}*/
		}
		
		/** Returns the unique identifier for the type owned by the field. */
		virtual UINT32 getTypeId()
		{
			return 0;
		}

		/** Gets the dynamic size of the object. If object has no dynamic size, static size of the object is returned. */
		virtual BitLength getDynamicSize(RTTITypeBase* rtti, void* object, bool compress)
		{
			return 0;
		}

		/**
		 * Gets the dynamic size of an array element. If the element has no dynamic size, static size of the element
		 * is returned.
		 */
		virtual BitLength getArrayElemDynamicSize(RTTITypeBase* rtti, void* object, int index, bool compress)
		{
			return 0;
		}

		/**
		 * Retrieves the value from the provided field of the provided object, and copies it into the stream. It does not
		 * check if stream buffer is large enough. If @p compress is true then the value is allowed to be compressed into
		 * less bytes than its raw type, and at sub-byte increments (e.g. one bit for a boolean).
		 */
		virtual void toStream(RTTITypeBase* rtti, void* object, Bitstream& stream, bool compress = false) = 0;

		/**
		 * Retrieves the value at the specified array index on the provided field of the provided object, and copies it into
		 * the stream. It does not check if stream buffer is large enough. If @p compress is true then the value is allowed
		 * to be compressed into less bytes than its raw type, and at sub-byte increments (e.g. one bit for a boolean).
		 */
		virtual void arrayElemToStream(RTTITypeBase* rtti, void* object, int index, Bitstream& stream, bool compress = false) = 0;

		/**
		 * Sets the value on the provided field of the provided object. Value is copied from the stream. It does not check
		 * the value in the stream buffer in any way. You must make sure the stream points to the proper location and contains
		 * the proper type. If @p compress is true then the value is allowed to be compressed into less bytes than its raw type,
		 * and at sub-byte increments (e.g. one bit for a boolean).
		 */
		virtual void fromBuffer(RTTITypeBase* rtti, void* object, Bitstream& stream, bool compress = false) = 0;

		/**
		 * Sets the value at the specified array index on the provided field of the provided object. Value is copied from
		 * the stream. It does not check the value in the stream in any way. You must make sure the stream points to the
		 * proper location and contains the proper type. If @p compress is true then the value is allowed to be compressed into
		 * less bytes than its raw type, and at sub-byte increments (e.g. one bit for a boolean).
		 */
		virtual void arrayElemFromBuffer(RTTITypeBase* rtti, void* object, int index, Bitstream& stream, bool compress = false) = 0;
	};

	/** Represents a plain class field containing a specific type. */
	template <class InterfaceType, class DataType, class ObjectType>
	struct RTTIPlainField : public RTTIPlainFieldBase
	{
		typedef DataType& (InterfaceType::*GetterType)(ObjectType*);
		typedef void (InterfaceType::*SetterType)(ObjectType*, DataType&);

		typedef DataType& (InterfaceType::*ArrayGetterType)(ObjectType*, UINT32);
		typedef void (InterfaceType::*ArraySetterType)(ObjectType*, UINT32, DataType&);
		typedef UINT32(InterfaceType::*ArrayGetSizeType)(ObjectType*);
		typedef void(InterfaceType::*ArraySetSizeType)(ObjectType*, UINT32);

		/**
		 * Initializes a plain field containing a single value.
		 *
		 * @param[in]	name		Name of the field.
		 * @param[in]	uniqueId	Unique identifier for this field. Although name is also a unique identifier we want a
		 *							small data type that can be used for efficiently serializing data to disk and similar.
		 *							It is primarily used for compatibility between different versions of serialized data.
		 * @param[in]	getter  	The getter method for the field.
		 * @param[in]	setter  	The setter method for the field.
		 * @param[in]	info		Various optional information about the field.
		 */
		void initSingle(String name, UINT16 uniqueId, GetterType getter, SetterType setter, const RTTIFieldInfo& info)
		{
			static_assert(sizeof(RTTIPlainType<DataType>::id) > 0, "Type has no RTTI ID."); // Just making sure provided type has a type ID

			BitLength size = RTTIPlainType<DataType>::getSize(DataType(), info, false);
			if (RTTIPlainType<DataType>::hasDynamicSize == 0 && size.bytes > 255)
			{
				assert(false);
				BS_LOG(Error, RTTI, "Trying to create a plain RTTI field with size larger than 255. In order to use larger sizes for plain " \
					"types please specialize RTTIPlainType, set hasDynamicSize to true.");
			}

			this->getter = getter;
			this->setter = setter;

			init(std::move(name), RTTIFieldSchema(uniqueId, false, RTTIPlainType<DataType>::hasDynamicSize, size,
				SerializableFT_Plain, RTTIPlainType<DataType>::id, nullptr, info));
		}

		/**
		 * Initializes a plain field containing multiple values in an array.
		 *
		 * @param[in]	name		Name of the field.
		 * @param[in]	uniqueId	Unique identifier for this field. Although name is also a unique identifier we want a
		 *							small data type that can be used for efficiently serializing data to disk and similar.
		 *							It is primarily used for compatibility between different versions of serialized data.
		 * @param[in]	getter  	The getter method for the field.
		 * @param[in]	getSize 	Getter method that returns the size of an array.
		 * @param[in]	setter  	The setter method for the field.
		 * @param[in]	setSize 	Setter method that allows you to resize an array. Can be null.
		 * @param[in]	info		Various optional information about the field.
		 */
		void initArray(String name, UINT16 uniqueId, ArrayGetterType getter,
			ArrayGetSizeType getSize, ArraySetterType setter, ArraySetSizeType setSize, const RTTIFieldInfo& info)
		{
			static_assert((RTTIPlainType<DataType>::id != 0) || true, ""); // Just making sure provided type has a type ID

			BitLength size = RTTIPlainType<DataType>::getSize(DataType(), info, false);
			if (RTTIPlainType<DataType>::hasDynamicSize == 0 && size.bytes > 255)
			{
				assert(false);
				BS_LOG(Error, RTTI, "Trying to create a plain RTTI field with size larger than 255. In order to use larger sizes for plain " \
					"types please specialize RTTIPlainType, set hasDynamicSize to true.");
			}

			arrayGetter = getter;
			arraySetter = setter;
			arrayGetSize = getSize;
			arraySetSize = setSize;

			init(std::move(name), RTTIFieldSchema(uniqueId, true, RTTIPlainType<DataType>::hasDynamicSize, size,
				SerializableFT_Plain, RTTIPlainType<DataType>::id, nullptr, info));
		}

		/** @copydoc RTTIPlainFieldBase::getTypeId */
		UINT32 getTypeId() override
		{
			return RTTIPlainType<DataType>::id;
		}

		/** @copydoc RTTIPlainFieldBase::getDynamicSize */
		BitLength getDynamicSize(RTTITypeBase* rtti, void* object, bool compress) override
		{
			checkIsArray(false);
			checkType<DataType>();

			InterfaceType* rttiObject = static_cast<InterfaceType*>(rtti);
			ObjectType* castObject = static_cast<ObjectType*>(object);
			DataType value = (rttiObject->*getter)(castObject);

			return RTTIPlainType<DataType>::getSize(value, schema.info, compress);
		}

		/** @copydoc RTTIPlainFieldBase::getArrayElemDynamicSize */
		BitLength getArrayElemDynamicSize(RTTITypeBase* rtti, void* object, int index, bool compress) override
		{
			checkIsArray(true);
			checkType<DataType>();

			InterfaceType* rttiObject = static_cast<InterfaceType*>(rtti);
			ObjectType* castObject = static_cast<ObjectType*>(object);
			DataType value = (rttiObject->*arrayGetter)(castObject, index);

			return RTTIPlainType<DataType>::getSize(value, schema.info, compress);
		}

		/** Returns the size of the array managed by the field. */
		UINT32 getArraySize(RTTITypeBase* rtti, void* object) override
		{
			checkIsArray(true);

			InterfaceType* rttiObject = static_cast<InterfaceType*>(rtti);
			ObjectType* castObject = static_cast<ObjectType*>(object);
			return (rttiObject->*arrayGetSize)(castObject);
		}

		/** Changes the size of the array managed by the field. Array must be re-populated after. */
		void setArraySize(RTTITypeBase* rtti, void* object, UINT32 size) override
		{
			checkIsArray(true);

			if(!arraySetSize)
			{
				BS_EXCEPT(InternalErrorException, "Specified field (" + name + ") has no array size setter.");
			}

			InterfaceType* rttiObject = static_cast<InterfaceType*>(rtti);
			ObjectType* castObject = static_cast<ObjectType*>(object);
			(rttiObject->*arraySetSize)(castObject, size);
		}

		/** @copydoc RTTIPlainFieldBase::toBuffer */
		void toStream(RTTITypeBase* rtti, void* object, Bitstream& stream, bool compress) override
		{
			checkIsArray(false);
			checkType<DataType>();

			InterfaceType* rttiObject = static_cast<InterfaceType*>(rtti);
			ObjectType* castObject = static_cast<ObjectType*>(object);
			DataType value = (rttiObject->*getter)(castObject);

			RTTIPlainType<DataType>::toMemory(value, stream, schema.info, compress);
		}

		/** @copydoc RTTIPlainFieldBase::arrayElemToBuffer */
		void arrayElemToStream(RTTITypeBase* rtti, void* object, int index, Bitstream& stream, bool compress) override
		{
			checkIsArray(true);
			checkType<DataType>();

			InterfaceType* rttiObject = static_cast<InterfaceType*>(rtti);
			ObjectType* castObject = static_cast<ObjectType*>(object);
			DataType value = (rttiObject->*arrayGetter)(castObject, index);

			RTTIPlainType<DataType>::toMemory(value, stream, schema.info, compress);
		}

		/** @copydoc RTTIPlainFieldBase::fromBuffer */
		void fromBuffer(RTTITypeBase* rtti, void* object, Bitstream& stream, bool compress) override
		{
			checkIsArray(false);
			checkType<DataType>();

			InterfaceType* rttiObject = static_cast<InterfaceType*>(rtti);
			ObjectType* castObject = static_cast<ObjectType*>(object);

			DataType value;
			RTTIPlainType<DataType>::fromMemory(value, stream, schema.info, compress);

			if(!setter)
			{
				BS_EXCEPT(InternalErrorException,
					"Specified field (" + name + ") has no setter.");
			}

			(rttiObject->*setter)(castObject, value);
		}

		/** @copydoc RTTIPlainFieldBase::arrayElemFromBuffer */
		void arrayElemFromBuffer(RTTITypeBase* rtti, void* object, int index, Bitstream& stream, bool compress) override
		{
			checkIsArray(true);
			checkType<DataType>();

			InterfaceType* rttiObject = static_cast<InterfaceType*>(rtti);
			ObjectType* castObject = static_cast<ObjectType*>(object);

			DataType value;
			RTTIPlainType<DataType>::fromMemory(value, stream, schema.info, compress);

			if(!arraySetter)
			{
				BS_EXCEPT(InternalErrorException,
					"Specified field (" + name + ") has no setter.");
			}

			(rttiObject->*arraySetter)(castObject, index, value);
		}

	private:
		union
		{
			struct
			{
				GetterType getter;
				SetterType setter;
			};

			struct
			{
				ArrayGetterType arrayGetter;
				ArraySetterType arraySetter;

				ArrayGetSizeType arrayGetSize;
				ArraySetSizeType arraySetSize;
			};
		};
	};

	/** @} */
	/** @} */
}
