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

		/** @copydoc RTTIField::hasDynamicSize */
		bool hasDynamicSize() override
		{
			return false;
		}

		/** Gets the dynamic size of the object. If object has no dynamic size, static size of the object is returned. */
		virtual UINT32 getDynamicSize(RTTITypeBase* rtti, void* object)
		{
			return 0;
		}

		/**
		 * Gets the dynamic size of an array element. If the element has no dynamic size, static size of the element 
		 * is returned.
		 */
		virtual UINT32 getArrayElemDynamicSize(RTTITypeBase* rtti, void* object, int index)
		{
			return 0;
		}

		/**
		 * Retrieves the value from the provided field of the provided object, and copies it into the buffer. It does not 
		 * check if buffer is large enough.
		 */
		virtual void toBuffer(RTTITypeBase* rtti, void* object, void* buffer) = 0;

		/**
		 * Retrieves the value at the specified array index on the provided field of the provided object, and copies it into
		 * the buffer. It does not check if buffer is large enough.
		 */
		virtual void arrayElemToBuffer(RTTITypeBase* rtti, void* object, int index, void* buffer) = 0;

		/**
		 * Sets the value on the provided field of the provided object. Value is copied from the buffer. It does not check 
		 * the value in the buffer in any way. You must make sure buffer points to the proper location and contains the 
		 * proper type.
		 */
		virtual void fromBuffer(RTTITypeBase* rtti, void* object, void* buffer) = 0;

		/**
		 * Sets the value at the specified array index on the provided field of the provided object. Value is copied from 
		 * the buffer. It does not check the value in the buffer in any way. You must make sure buffer points to the proper 
		 * location and contains the proper type.
		 */
		virtual void arrayElemFromBuffer(RTTITypeBase* rtti, void* object, int index, void* buffer) = 0;
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
		 * @param[in]	flags		Various flags you can use to specialize how outside systems handle this field. See "RTTIFieldFlag".
		 */
		void initSingle(const String& name, UINT16 uniqueId, GetterType getter, SetterType setter, UINT64 flags)
		{
			static_assert(sizeof(RTTIPlainType<DataType>::id) > 0, "Type has no RTTI ID."); // Just making sure provided type has a type ID

			static_assert((RTTIPlainType<DataType>::hasDynamicSize != 0 || (sizeof(DataType) <= 255)), 
				"Trying to create a plain RTTI field with size larger than 255. In order to use larger sizes for plain types please specialize " \
				" RTTIPlainType, set hasDynamicSize to true.");

			this->getter = getter;
			this->setter = setter;

			init(name, uniqueId, false, SerializableFT_Plain, flags);
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
		 * @param[in]	flags		Various flags you can use to specialize how outside systems handle this field. See "RTTIFieldFlag".
		 */
		void initArray(const String& name, UINT16 uniqueId, ArrayGetterType getter,
			ArrayGetSizeType getSize, ArraySetterType setter, ArraySetSizeType setSize, UINT64 flags)
		{
			static_assert((RTTIPlainType<DataType>::id != 0) || true, ""); // Just making sure provided type has a type ID

			static_assert((RTTIPlainType<DataType>::hasDynamicSize != 0 || (sizeof(DataType) <= 255)), 
				"Trying to create a plain RTTI field with size larger than 255. In order to use larger sizes for plain types please specialize " \
				" RTTIPlainType, set hasDynamicSize to true.");

			arrayGetter = getter;
			arraySetter = setter;
			arrayGetSize = getSize;
			arraySetSize = setSize;

			init(name, uniqueId, true, SerializableFT_Plain, flags);
		}

		/** @copydoc RTTIField::getTypeSize */
		UINT32 getTypeSize() override
		{
			return sizeof(DataType);
		}

		/** @copydoc RTTIPlainFieldBase::getTypeId */
		UINT32 getTypeId() override
		{
			return RTTIPlainType<DataType>::id;
		}

		/** @copydoc RTTIPlainFieldBase::hasDynamicSize */
		bool hasDynamicSize() override
		{
			return RTTIPlainType<DataType>::hasDynamicSize != 0;
		}

		/** @copydoc RTTIPlainFieldBase::getDynamicSize */
		UINT32 getDynamicSize(RTTITypeBase* rtti, void* object) override
		{
			checkIsArray(false);
			checkType<DataType>();

			InterfaceType* rttiObject = static_cast<InterfaceType*>(rtti);
			ObjectType* castObject = static_cast<ObjectType*>(object);
			DataType value = (rttiObject->*getter)(castObject);

			return RTTIPlainType<DataType>::getDynamicSize(value);
		}

		/** @copydoc RTTIPlainFieldBase::getArrayElemDynamicSize */
		UINT32 getArrayElemDynamicSize(RTTITypeBase* rtti, void* object, int index) override
		{
			checkIsArray(true);
			checkType<DataType>();

			InterfaceType* rttiObject = static_cast<InterfaceType*>(rtti);
			ObjectType* castObject = static_cast<ObjectType*>(object);
			DataType value = (rttiObject->*arrayGetter)(castObject, index);

			return RTTIPlainType<DataType>::getDynamicSize(value);
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
				BS_EXCEPT(InternalErrorException, "Specified field (" + mName + ") has no array size setter.");
			}

			InterfaceType* rttiObject = static_cast<InterfaceType*>(rtti);
			ObjectType* castObject = static_cast<ObjectType*>(object);
			(rttiObject->*arraySetSize)(castObject, size);
		}

		/** @copydoc RTTIPlainFieldBase::toBuffer */
		void toBuffer(RTTITypeBase* rtti, void* object, void* buffer) override
		{
			checkIsArray(false);
			checkType<DataType>();

			InterfaceType* rttiObject = static_cast<InterfaceType*>(rtti);
			ObjectType* castObject = static_cast<ObjectType*>(object);
			DataType value = (rttiObject->*getter)(castObject);

			RTTIPlainType<DataType>::toMemory(value, (char*)buffer);
		}

		/** @copydoc RTTIPlainFieldBase::arrayElemToBuffer */
		void arrayElemToBuffer(RTTITypeBase* rtti, void* object, int index, void* buffer) override
		{
			checkIsArray(true);
			checkType<DataType>();

			InterfaceType* rttiObject = static_cast<InterfaceType*>(rtti);
			ObjectType* castObject = static_cast<ObjectType*>(object);
			DataType value = (rttiObject->*arrayGetter)(castObject, index);

			RTTIPlainType<DataType>::toMemory(value, (char*)buffer);
		}

		/** @copydoc RTTIPlainFieldBase::fromBuffer */
		void fromBuffer(RTTITypeBase* rtti, void* object, void* buffer) override
		{
			checkIsArray(false);
			checkType<DataType>();

			InterfaceType* rttiObject = static_cast<InterfaceType*>(rtti);
			ObjectType* castObject = static_cast<ObjectType*>(object);

			DataType value;
			RTTIPlainType<DataType>::fromMemory(value, (char*)buffer);

			if(!setter)
			{
				BS_EXCEPT(InternalErrorException,
					"Specified field (" + mName + ") has no setter.");
			}

			(rttiObject->*setter)(castObject, value);
		}

		/** @copydoc RTTIPlainFieldBase::arrayElemFromBuffer */
		void arrayElemFromBuffer(RTTITypeBase* rtti, void* object, int index, void* buffer) override
		{
			checkIsArray(true);
			checkType<DataType>();

			InterfaceType* rttiObject = static_cast<InterfaceType*>(rtti);
			ObjectType* castObject = static_cast<ObjectType*>(object);

			DataType value;
			RTTIPlainType<DataType>::fromMemory(value, (char*)buffer);

			if(!arraySetter)
			{
				BS_EXCEPT(InternalErrorException, 
					"Specified field (" + mName + ") has no setter.");
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
