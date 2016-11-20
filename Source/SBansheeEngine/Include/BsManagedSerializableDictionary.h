//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsIReflectable.h"

namespace bs
{
	/** @addtogroup SBansheeEngine
	 *  @{
	 */

	/**	Contains key and value data used for serializing a managed dictionary. */
	struct BS_SCR_BE_EXPORT ManagedSerializableDictionaryKeyValue : public IReflectable
	{
		ManagedSerializableDictionaryKeyValue() {}
		ManagedSerializableDictionaryKeyValue(const SPtr<ManagedSerializableFieldData>& key,
			const SPtr<ManagedSerializableFieldData>& value);

		SPtr<ManagedSerializableFieldData> key;
		SPtr<ManagedSerializableFieldData> value;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedSerializableDictionaryKeyValueRTTI;
		static RTTITypeBase* getRTTIStatic();
		virtual RTTITypeBase* getRTTI() const override;
	};

	/**
	 * Allows access to an underlying managed dictionary, or a cached version of that dictionary that can be
	 * serialized/deserialized.
	 *
	 * @note
	 * This class can be in two states:
	 *	 - Linked - When the object has a link to a managed object. This is the default state when a new instance
	 *				of ManagedSerializableObject is created. Any operations during this state will operate directly
	 *				on the linked managed object.
	 *	 - Serialized - When the object has no link to the managed object but instead just contains cached object
	 *					and field data that may be used for initializing a managed object. Any operations during
	 *					this state will operate only on the cached internal data.
	 * You can transfer between these states by calling serialize(linked->serialized) & deserialize (serialized->linked).
	 */
	class BS_SCR_BE_EXPORT ManagedSerializableDictionary : public IReflectable
	{
	private:
		struct ConstructPrivately {};
		
		/**	Generates a hash value for field data. */
		struct BS_SCR_BE_EXPORT Hash
		{
			inline size_t operator()(const SPtr<ManagedSerializableFieldData>& x) const;
		};

		/**	Compares two field data objects. */
		struct BS_SCR_BE_EXPORT Equals
		{
			inline bool operator()(const SPtr<ManagedSerializableFieldData>& a, const SPtr<ManagedSerializableFieldData>& b) const;
		};

	public:
		typedef UnorderedMap<SPtr<ManagedSerializableFieldData>, SPtr<ManagedSerializableFieldData>> CachedEntriesMap;

		/**
		 * Helper class that enumerates over all entires in a managed dictionary. Operates on managed object if the parent
		 * dictionary is in linked state, or on cached data otherwise.
		 */
		class Enumerator
		{
		public:
			/**
			 * Constructs a new enumerator for the provided managed object.
			 *
			 * @param[in]	instance	Managed instance of type System.Collections.Generic.Dictionary.
			 * @param[in]	parent		Serializable parent of the managed instance.
			 */
			Enumerator(MonoObject* instance, const ManagedSerializableDictionary* parent);

			/**
			 * Returns the wrapped key data at the current enumerator position. Only valid to call this if enumerator is
			 * valid (meaning last call to moveNext() returned true).
			 */
			SPtr<ManagedSerializableFieldData> getKey() const;

			/**
			 * Returns the wrapped value data at the current enumerator position. Only valid to call this if enumerator is
			 * valid (meaning last call to moveNext() returned true).
			 */
			SPtr<ManagedSerializableFieldData> getValue() const;

			/**
			 * Moves the enumerator to the next position. Initially enumerator is at an invalid position and must be called
			 * at least once before accesing its data.
			 *
			 * @return	Returns if the enumerator is at valid position. When the enumerator returns false it means there are
			 *			no more elements to enumerate.
			 */
			bool moveNext();

		private:
			MonoObject* mInstance;
			MonoObject* mCurrent;
			CachedEntriesMap::const_iterator mCachedIter;
			bool mIteratorInitialized;

			const ManagedSerializableDictionary* mParent;
		};

	public:
		ManagedSerializableDictionary(const ConstructPrivately& dummy, const SPtr<ManagedSerializableTypeInfoDictionary>& typeInfo, MonoObject* managedInstance);
		ManagedSerializableDictionary(const ConstructPrivately& dummy);

		/**
		 * Returns the internal managed instance of the dictionary. This will return null if the object is in serialized
		 * mode.
		 */
		MonoObject* getManagedInstance() const { return mManagedInstance; }

		/**	Returns the type information for the internal dictionary. */
		SPtr<ManagedSerializableTypeInfoDictionary> getTypeInfo() const { return mDictionaryTypeInfo; }

		/**
		 * Returns the dictionary value at the specified key. If the key doesn't exist the default value for the type is
		 * returned. Operates on managed object if in linked state, or on cached data otherwise.
		 *
		 * @param[in]	key		Wrapper around the key data at which to retrieve the value.
		 * @return				A wrapper around the value in the dictionary at the specified key.
		 */
		SPtr<ManagedSerializableFieldData> getFieldData(const SPtr<ManagedSerializableFieldData>& key);

		/**
		 * Sets the dictionary value at the specified key. Operates on managed object if in linked state, or on cached data
		 * otherwise.
		 *
		 * @param[in]	key		Wrapper around the key data at which to set the value.
		 * @param[in]	val		Wrapper around the value to set at the specified key.
		 */
		void setFieldData(const SPtr<ManagedSerializableFieldData>& key, const SPtr<ManagedSerializableFieldData>& val);

		/**
		 * Deletes the key/value pair at the specified key. If the key doesn't exist this operation does nothing. Operates
		 * on managed object if in linked state, or on cached data otherwise.
		 *
		 * @param[in]	key		Wrapper around the key data at which to delete the value.
		 */
		void removeFieldData(const SPtr<ManagedSerializableFieldData>& key);

		/**
		 * Checks if the dictionary contains the specified key. Operates on managed object if in linked state, or on cached
		 * data otherwise.
		 *
		 * @param[in]	key		Wrapper around the key data which to check.
		 */
		bool contains(const SPtr<ManagedSerializableFieldData>& key) const;

		/** Returns an enumerator object that allows you to iterate over all key/value pairs in the dictionary. */
		Enumerator getEnumerator() const;

		/**
		 * Serializes the internal managed object into a set of cached data that can be saved in memory/disk and can be
		 * deserialized later. Does nothing if object is already is serialized mode. When in serialized mode the reference
		 * to the managed instance will be lost.
		 */
		void serialize();

		/**
		 * Deserializes a set of cached data into a managed object. This action may fail in case the cached	data contains a
		 * type that no longer exists. You may check if it completely successfully if getManagedInstance() returns non-null
		 * after.
		 *
		 * This action transfers the object into linked mode. All further operations will operate directly on the managed
		 * instance and the cached data will be cleared. If you call this method on an already linked object the old object
		 * will be replaced and initialized with empty data (since cached data does not exist).
		 */
		void deserialize();

		/**
		 * Creates a managed serializable dictionary that references an existing managed dictionary. Created object will be
		 * in linked mode.
		 *
		 * @param[in]	managedInstance		Constructed managed instance of the dictionary to link with. Its type must
		 *									correspond with the provided type info.
		 * @param[in]	typeInfo			Type information for the dictionary and its key/value pair.
		 */
		static SPtr<ManagedSerializableDictionary> createFromExisting(MonoObject* managedInstance, const SPtr<ManagedSerializableTypeInfoDictionary>& typeInfo);

		/**
		 * Creates a managed serializable dictionary that creates and references a brand new managed dictionary instance.
		 *
		 * @param[in]	typeInfo	Type of the dictionary to create.
		 */
		static SPtr<ManagedSerializableDictionary> createNew(const SPtr<ManagedSerializableTypeInfoDictionary>& typeInfo);

		/**
		 * Creates a managed dictionary instance.
		 *
		 * @param[in]	typeInfo	Type of the dictionary to create.
		 */
		static MonoObject* createManagedInstance(const SPtr<ManagedSerializableTypeInfoDictionary>& typeInfo);

	protected:
		/**
		 * Retrieves needed Mono types and methods. Should be called before performing any operations with the managed
		 * object.
		 */
		void initMonoObjects(MonoClass* dictionaryClass);

		MonoObject* mManagedInstance;

		MonoMethod* mAddMethod;
		MonoMethod* mRemoveMethod;
		MonoMethod* mTryGetValueMethod;
		MonoMethod* mContainsKeyMethod;
		MonoMethod* mGetEnumerator;
		MonoMethod* mEnumMoveNext;
		MonoProperty* mEnumCurrentProp;
		MonoProperty* mKeyProp;
		MonoProperty* mValueProp;

		SPtr<ManagedSerializableTypeInfoDictionary> mDictionaryTypeInfo;
		CachedEntriesMap mCachedEntries;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
		
		/**	Creates an empty and uninitialized object used for serialization purposes. */
		static SPtr<ManagedSerializableDictionary> createEmpty();

	public:
		friend class ManagedSerializableDictionaryRTTI;
		static RTTITypeBase* getRTTIStatic();
		virtual RTTITypeBase* getRTTI() const override;
	};

	/** @} */
}