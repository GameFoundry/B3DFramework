//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "Reflection/B3DIReflectable.h"
#include "B3DMonoArray.h"

namespace b3d
{
	/** @addtogroup bsfScript
	 *  @{
	 */

	/**	Contains key and value data used for serializing a managed dictionary. */
	struct B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableDictionaryKeyValue : public IReflectable
	{
		ManagedSerializableDictionaryKeyValue() {}

		ManagedSerializableDictionaryKeyValue(const TShared<ManagedSerializableFieldData>& key, const TShared<ManagedSerializableFieldData>& value);

		TShared<ManagedSerializableFieldData> Key;
		TShared<ManagedSerializableFieldData> Value;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedSerializableDictionaryKeyValueRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const;
	};

	/**
	 * Allows access to an underlying managed dictionary, or a cached version of that dictionary that can be
	 * serialized/deserialized.
	 *
	 * @note
	 * This class can be in two states:
	 *	 - Linked - When the object has a link to a managed object. This is the default state when a new instance
	 *				of ManagedSerializableDictionary is created. Any operations during this state will operate directly
	 *				on the linked managed object. A GC handle will be kept to the linked managed object. The handle can
	 *				be freed by transfering to serialized mode or by destroying this object.
	 *	 - Serialized - When the object has no link to the managed object but instead just contains cached object
	 *					and field data that may be used for initializing a managed object. Any operations during
	 *					this state will operate only on the cached internal data.
	 *
	 * You can transfer an object in linked state to serialized state by calling serialize(). If an object is in serialized
	 * state you can call deserialize() to populated a managed object from the cached data.
	 */
	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableDictionary : public IReflectable
	{
	private:
		struct ConstructPrivately
		{};

	public:
		typedef UnorderedMap<TShared<ManagedSerializableFieldData>, TShared<ManagedSerializableFieldData>> CachedEntriesMap;

		/**
		 * Helper class that enumerates over all entires in a managed dictionary. Operates on managed object if the parent
		 * dictionary is in linked state, or on cached data otherwise.
		 */
		class Enumerator
		{
		public:
			/**
			 * Constructs a new enumerator for a managed dictionary.
			 *
			 * @param[in]	parent		Owning dictionary object.
			 */
			Enumerator(const ManagedSerializableDictionary* parent);
			Enumerator(const Enumerator& other);
			~Enumerator();

			Enumerator& operator=(const Enumerator& other);

			/**
			 * Returns the wrapped key data at the current enumerator position. Only valid to call this if enumerator is
			 * valid (meaning last call to moveNext() returned true).
			 */
			TShared<ManagedSerializableFieldData> GetKey() const;

			/**
			 * Returns the wrapped value data at the current enumerator position. Only valid to call this if enumerator is
			 * valid (meaning last call to moveNext() returned true).
			 */
			TShared<ManagedSerializableFieldData> GetValue() const;

			/**
			 * Moves the enumerator to the next position. Initially enumerator is at an invalid position and must be called
			 * at least once before accesing its data.
			 *
			 * @return	Returns if the enumerator is at valid position. When the enumerator returns false it means there are
			 *			no more elements to enumerate.
			 */
			bool MoveNext();

		private:
			uint32_t mKeysArrayHandle = 0;
			uint32_t mValuesArrayHandle = 0;
			u32 mNumEntries = 0;
			u32 mCurrentIdx = (u32)-1;
			::MonoClass* mKeyType = nullptr;
			::MonoClass* mValueType = nullptr;

			CachedEntriesMap::const_iterator mCachedIter;
			bool mIteratorInitialized;

			const ManagedSerializableDictionary* mParent;
		};

	public:
		ManagedSerializableDictionary(const ConstructPrivately& dummy, const TShared<ManagedTypeInfoDictionary>& typeInfo, MonoObject* managedInstance);
		ManagedSerializableDictionary(const ConstructPrivately& dummy);
		~ManagedSerializableDictionary();

		/**
		 * Returns the internal managed instance of the dictionary. This will return null if the object is in serialized
		 * mode.
		 */
		MonoObject* GetManagedInstance() const;

		/**	Returns the type information for the internal dictionary. */
		TShared<ManagedTypeInfoDictionary> GetTypeInfo() const { return mDictionaryTypeInfo; }

		/**
		 * Returns the dictionary value at the specified key. If the key doesn't exist the default value for the type is
		 * returned. Operates on managed object if in linked state, or on cached data otherwise.
		 *
		 * @param[in]	key		Wrapper around the key data at which to retrieve the value.
		 * @return				A wrapper around the value in the dictionary at the specified key.
		 */
		TShared<ManagedSerializableFieldData> GetFieldData(const TShared<ManagedSerializableFieldData>& key);

		/**
		 * Sets the dictionary value at the specified key. Operates on managed object if in linked state, or on cached data
		 * otherwise.
		 *
		 * @param[in]	key		Wrapper around the key data at which to set the value.
		 * @param[in]	val		Wrapper around the value to set at the specified key.
		 */
		void SetFieldData(const TShared<ManagedSerializableFieldData>& key, const TShared<ManagedSerializableFieldData>& val);

		/**
		 * Deletes the key/value pair at the specified key. If the key doesn't exist this operation does nothing. Operates
		 * on managed object if in linked state, or on cached data otherwise.
		 *
		 * @param[in]	key		Wrapper around the key data at which to delete the value.
		 */
		void RemoveFieldData(const TShared<ManagedSerializableFieldData>& key);

		/**
		 * Checks if the dictionary contains the specified key. Operates on managed object if in linked state, or on cached
		 * data otherwise.
		 *
		 * @param[in]	key		Wrapper around the key data which to check.
		 */
		bool Contains(const TShared<ManagedSerializableFieldData>& key) const;

		/** Returns an enumerator object that allows you to iterate over all key/value pairs in the dictionary. */
		Enumerator GetEnumerator() const;

		/**
		 * Serializes the internal managed object into a set of cached data that can be saved in memory/disk and can be
		 * deserialized later. The internal managed object will be freed (if no other references to it). Calling serialize()
		 * again will have no result.
		 */
		void Serialize();

		/**
		 * Deserializes a set of cached data into a managed object. This action may fail in case the cached data contains a
		 * type that no longer exists in which case null is returned.
		 *
		 * @return		Newly created object initialized with the cached data.
		 */
		MonoObject* Deserialize();

		/**
		 * Creates a managed serializable dictionary that references an existing managed dictionary. Created object will be
		 * in linked mode.
		 *
		 * @param[in]	managedInstance		Constructed managed instance of the dictionary to link with. Its type must
		 *									correspond with the provided type info.
		 * @param[in]	typeInfo			Type information for the dictionary and its key/value pair.
		 */
		static TShared<ManagedSerializableDictionary> CreateFromExisting(MonoObject* managedInstance, const TShared<ManagedTypeInfoDictionary>& typeInfo);

		/**
		 * Creates a managed serializable dictionary that creates and references a brand new managed dictionary instance.
		 *
		 * @param[in]	typeInfo	Type of the dictionary to create.
		 */
		static TShared<ManagedSerializableDictionary> CreateNew(const TShared<ManagedTypeInfoDictionary>& typeInfo);

		/**
		 * Creates a managed dictionary instance.
		 *
		 * @param[in]	typeInfo	Type of the dictionary to create.
		 */
		static MonoObject* CreateManagedInstance(const TShared<ManagedTypeInfoDictionary>& typeInfo);

	protected:
		/**
		 * Retrieves needed Mono types and methods. Should be called before performing any operations with the managed
		 * object.
		 */
		void InitMonoObjects(MonoClass* dictionaryClass);

		/**
		 * Sets the dictionary value at the specified key. Operates on the provided managed object.
		 *
		 * @param[in]	obj		Managed object to which to assign the data.
		 * @param[in]	key		Wrapper around the key data at which to set the value.
		 * @param[in]	val		Wrapper around the value to set at the specified key.
		 */
		void SetFieldData(MonoObject* obj, const TShared<ManagedSerializableFieldData>& key, const TShared<ManagedSerializableFieldData>& val);

		uint32_t mGCHandle = 0;

		MonoMethod* mAddMethod = nullptr;
		MonoMethod* mRemoveMethod = nullptr;
		MonoMethod* mTryGetValueMethod = nullptr;
		MonoMethod* mContainsKeyMethod = nullptr;
		MonoProperty* mCountProp = nullptr;
		MonoProperty* mKeysProp = nullptr;
		MonoMethod* mKeysCopyTo = nullptr;
		MonoProperty* mValuesProp = nullptr;
		MonoMethod* mValuesCopyTo = nullptr;

		TShared<ManagedTypeInfoDictionary> mDictionaryTypeInfo;
		CachedEntriesMap mCachedEntries;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

		/**	Creates an empty and uninitialized object used for serialization purposes. */
		static TShared<ManagedSerializableDictionary> CreateEmpty();

	public:
		friend class ManagedSerializableDictionaryRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const;
	};

	/** @} */
} // namespace b3d
