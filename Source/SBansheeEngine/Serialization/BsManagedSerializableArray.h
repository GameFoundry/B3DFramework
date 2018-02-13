//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Reflection/BsIReflectable.h"

namespace bs
{
	/** @addtogroup SBansheeEngine
	 *  @{
	 */

	/**
	 * Allows access to an underlying managed array, or a cached version of that array that can be serialized/deserialized.
	 *
	 * @note	
	 * This class can be in two states:
	 *	 - Linked - When the object has a link to a managed object. This is the default state when a new instance
	 *				of ManagedSerializableArray is created. Any operations during this state will operate directly
	 *				on the linked managed object. A GC handle will be kept to the linked managed object. The handle can
	 *				be freed by transfering to serialized mode or by destroying this object.
	 *	 - Serialized - When the object has no link to the managed object but instead just contains cached object
	 *					and field data that may be used for initializing a managed object. Any operations during
	 *					this state will operate only on the cached internal data.
	 *					
	 * You can transfer an object in linked state to serialized state by calling serialize(). If an object is in serialized
	 * state you can call deserialize() to populated a managed object from the cached data. 	
	 *	
	 */
	class BS_SCR_BE_EXPORT ManagedSerializableArray : public IReflectable
	{
	private:
		struct ConstructPrivately {};

	public:
		ManagedSerializableArray(const ConstructPrivately& dummy, const SPtr<ManagedSerializableTypeInfoArray>& typeInfo, MonoObject* managedInstance);
		ManagedSerializableArray(const ConstructPrivately& dummy);
		~ManagedSerializableArray();

		/**
		 * Returns the internal managed instance of the array. This will return null if the object is in serialized mode.
		 */
		MonoObject* getManagedInstance() const;

		/**	Returns the type information for the internal array. */
		SPtr<ManagedSerializableTypeInfoArray> getTypeInfo() const { return mArrayTypeInfo; }

		/**
		 * Changes the size of the array. Operates on managed object if in linked state, or on cached data otherwise. If
		 * in linked state the new instance of the managed object will be created and must be retrieved by
		 * getManagedInstance().
		 *
		 * @param[in]	newSizes	Array of sizes, one per array dimension. Number of sizes must match number of array
		 *							dimensions as specified by its type.
		 */
		void resize(const Vector<UINT32>& newSizes);

		/**
		 * Returns the size of a specific dimension of the array. Operates on managed object if in linked state, or on
		 * cached data otherwise.
		 */
		UINT32 getLength(UINT32 dimension) const { return mNumElements[dimension]; }

		/**
		 * Returns the sizes of a all dimensions of the array. Operates on managed object if in linked state, or on cached
		 * data otherwise.
		 */
		Vector<UINT32> getLengths() const { return mNumElements; }

		/**
		 * Returns the total of all sizes of all dimensions of the array. Operates on managed object if in linked state, or
		 * on cached data otherwise.
		 */
		UINT32 getTotalLength() const;

		/**
		 * Sets a new element value at the specified array index. Operates on managed object if in linked state, or on
		 * cached data otherwise.
		 *
		 * @param[in]	arrayIdx	Index at which to set the value.
		 * @param[in]	val			Wrapper around the value to store in the array. Must be of the array element type.
		 */
		void setFieldData(UINT32 arrayIdx, const SPtr<ManagedSerializableFieldData>& val);

		/**
		 * Returns the element value at the specified array index. Operates on managed object if in linked state, or on
		 * cached data otherwise.
		 *
		 * @param[in]	arrayIdx	Index at which to retrieve the value.
		 * @return					A wrapper around the element value in the array.
		 */
		SPtr<ManagedSerializableFieldData> getFieldData(UINT32 arrayIdx);

		/**
		 * Serializes the internal managed object into a set of cached data that can be saved in memory/disk and can be
		 * deserialized later. The internal managed object will be freed (if no other references to it). Calling serialize()
		 * again will have no result.
		 */
		void serialize();

		/**
		 * Deserializes a set of cached data into a managed object. This action may fail in case the cached data contains a
		 * type that no longer exists in which case null is returned.
		 *
		 * @return		Newly created object initialized with the cached data.
		 */
		MonoObject* deserialize();

		/**
		 * Creates a managed serializable array that references an existing managed array. Created object will be in linked
		 * mode.
		 *
		 * @param[in]	managedInstance		Constructed managed instance of the array to link with. Its type must correspond
		 *									with the provided type info.
		 * @param[in]	typeInfo			Type information for the array and its elements.
		 */
		static SPtr<ManagedSerializableArray> createFromExisting(MonoObject* managedInstance, const SPtr<ManagedSerializableTypeInfoArray>& typeInfo);

		/**
		 * Creates a managed serializable array that creates and references a brand new managed array instance.
		 *
		 * @param[in]	typeInfo	Type of the array to create.
		 * @param[in]	sizes		Array of sizes, one per array dimension. Number of sizes must match number of array
		 *							dimensions as specified by its type.
		 */
		static SPtr<ManagedSerializableArray> createNew(const SPtr<ManagedSerializableTypeInfoArray>& typeInfo, const Vector<UINT32>& sizes);

		/**
		 * Creates a managed array instance.
		 *
		 * @param[in]	typeInfo	Type of the array to create.
		 * @param[in]	sizes		Array of sizes, one per array dimension. Number of sizes must match number of array
		 *							dimensions as specified by its type.
		 */
		static MonoObject* createManagedInstance(const SPtr<ManagedSerializableTypeInfoArray>& typeInfo, const Vector<UINT32>& sizes);

	protected:
		/**
		 * Retrieves needed Mono types and methods. Should be called before performing any operations with the managed
		 * object.
		 */
		void initMonoObjects();

		/** Returns the size of the specified dimension of the array. Operates on the internal managed object. */
		UINT32 getLengthInternal(UINT32 dimension) const;

		/**
		 * Sets a new element value at the specified array index. Operates on the provided managed instance.
		 * 
		 * @param[in]	obj			Managed instance in which to set the data in.
		 * @param[in]	arrayIdx	Index at which to set the value.
		 * @param[in]	val			Wrapper around the value to store in the array. Must be of the array element type.
		 */
		void setFieldData(MonoArray* obj, UINT32 arrayIdx, const SPtr<ManagedSerializableFieldData>& val);

		/**	Sets a value at the specified index in the array. Operates on the provided managed object. */
		void setValueInternal(MonoArray* obj, UINT32 arrayIdx, void* val);

		/** Converts a multi-dimensional array index into a sequential one-dimensional index. */
		UINT32 toSequentialIdx(const Vector<UINT32>& idx) const;

		uint32_t mGCHandle = 0;
		::MonoClass* mElementMonoClass = nullptr;
		MonoMethod* mCopyMethod = nullptr;

		SPtr<ManagedSerializableTypeInfoArray> mArrayTypeInfo;
		Vector<SPtr<ManagedSerializableFieldData>> mCachedEntries;
		Vector<UINT32> mNumElements;
		UINT32 mElemSize = 0;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
		
		/**	Creates an empty and uninitialized object used for serialization purposes. */
		static SPtr<ManagedSerializableArray> createNew();

	public:
		friend class ManagedSerializableArrayRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** @} */
}