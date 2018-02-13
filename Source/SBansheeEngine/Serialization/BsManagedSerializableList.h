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
	 * Allows access to an underlying managed list, or a cached version of that list that can be serialized/deserialized.
	 *
	 * @note
	 * This class can be in two states:
	 *	 - Linked - When the object has a link to a managed object. This is the default state when a new instance
	 *				of ManagedSerializableList is created. Any operations during this state will operate directly
	 *				on the linked managed object. A GC handle will be kept to the linked managed object. The handle can
	 *				be freed by transfering to serialized mode or by destroying this object.
	 *	 - Serialized - When the object has no link to the managed object but instead just contains cached object
	 *					and field data that may be used for initializing a managed object. Any operations during
	 *					this state will operate only on the cached internal data.
	 *					
	 * You can transfer an object in linked state to serialized state by calling serialize(). If an object is in serialized
	 * state you can call deserialize() to populated a managed object from the cached data. 	
	 */
	class BS_SCR_BE_EXPORT ManagedSerializableList : public IReflectable
	{
	private:
		struct ConstructPrivately {};

	public:
		ManagedSerializableList(const ConstructPrivately& dummy, const SPtr<ManagedSerializableTypeInfoList>& typeInfo, 
			MonoObject* managedInstance);
		ManagedSerializableList(const ConstructPrivately& dummy);
		~ManagedSerializableList();

		/** Returns the internal managed instance of the list. This will return null if the object is in serialized mode. */
		MonoObject* getManagedInstance() const;

		/**	Returns the type information for the internal list. */
		SPtr<ManagedSerializableTypeInfoList> getTypeInfo() const { return mListTypeInfo; }

		/** Changes the size of the list. Operates on managed object if in linked state, or on cached data otherwise. */
		void resize(UINT32 newSize);

		/**
		 * Sets a new element value at the specified list index. Operates on managed object if in linked state, or on cached
		 * data otherwise.
		 *
		 * @param[in]	arrayIdx	Index at which to set the value.
		 * @param[in]	val			Wrapper around the value to store in the list. Must be of the list element type.
		 */
		void setFieldData(UINT32 arrayIdx, const SPtr<ManagedSerializableFieldData>& val);

		/**
		 * Returns the element value at the specified list index. Operates on managed object if in linked state, or on
		 * cached data otherwise.
		 *
		 * @param[in]	arrayIdx	Index at which to retrieve the value.
		 * @return					A wrapper around the element value in the list.
		 */
		SPtr<ManagedSerializableFieldData> getFieldData(UINT32 arrayIdx);

		/** Returns the size of the list. Operates on managed object if in linked state, or on cached data otherwise. */
		UINT32 getLength() const { return mNumElements; }

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
		 * Creates a managed serializable list that references an existing managed list. Created object will be in linked
		 * mode.
		 *
		 * @param[in]	managedInstance	Constructed managed instance of the list to link with. Its type must correspond with
		 *								the provided type info.
		 * @param[in]	typeInfo		Type information for the list and its elements.
		 */
		static SPtr<ManagedSerializableList> createFromExisting(MonoObject* managedInstance, 
			const SPtr<ManagedSerializableTypeInfoList>& typeInfo);

		/**
		 * Creates a managed serializable list that creates and references a brand new managed list instance.
		 *
		 * @param[in]	typeInfo	Type of the list to create.
		 * @param[in]	size		Initial size of the list.
		 */
		static SPtr<ManagedSerializableList> createNew(const SPtr<ManagedSerializableTypeInfoList>& typeInfo, UINT32 size);

		/**
		 * Creates a managed list instance.
		 *
		 * @param[in]	typeInfo	Type of the list to create.
		 * @param[in]	size		Initial size of the list.
		 */
		static MonoObject* createManagedInstance(const SPtr<ManagedSerializableTypeInfoList>& typeInfo, UINT32 size);

	protected:
		/**
		 * Retrieves needed Mono types and methods. Should be called before performing any operations with the managed
		 * object.
		 */
		void initMonoObjects(MonoClass* listClass);

		/**	Returns the size of the list. Operates on the internal managed object. */
		UINT32 getLengthInternal() const;

		/**
		 * Sets a new element value at the specified array index. Operates on the provided managed instance.
		 * 
		 * @param[in]	obj			Managed instance in which to set the data in.
		 * @param[in]	arrayIdx	Index at which to set the value.
		 * @param[in]	val			Wrapper around the value to store in the array. Must be of the array element type.
		 */
		void setFieldData(MonoObject* obj, UINT32 arrayIdx, const SPtr<ManagedSerializableFieldData>& val);

		/** Appends data to the end of the list. Operates on the internal managed object. */
		void addFieldDataInternal(const SPtr<ManagedSerializableFieldData>& val);

		uint32_t mGCHandle = 0;

		MonoMethod* mAddMethod = nullptr;
		MonoMethod* mAddRangeMethod = nullptr;
		MonoMethod* mClearMethod = nullptr;
		MonoMethod* mCopyToMethod = nullptr;
		MonoProperty* mItemProp = nullptr;
		MonoProperty* mCountProp = nullptr;

		SPtr<ManagedSerializableTypeInfoList> mListTypeInfo;
		Vector<SPtr<ManagedSerializableFieldData>> mCachedEntries;
		UINT32 mNumElements = 0;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
		
		/**	Creates an empty and uninitialized object used for serialization purposes. */
		static SPtr<ManagedSerializableList> createEmpty();

	public:
		friend class ManagedSerializableListRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** @} */
}