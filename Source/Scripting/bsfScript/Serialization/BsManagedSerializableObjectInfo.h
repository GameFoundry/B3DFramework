//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Reflection/BsIReflectable.h"

namespace bs
{
	/** @addtogroup bsfScript
	 *  @{
	 */

	/**	Valid serializable script types. */
	enum class ScriptPrimitiveType
	{
		Bool,
		Char,
		I8,
		U8,
		I16,
		U16,
		I32,
		U32,
		I64,
		U64,
		Float,
		Double,
		String,
		Count // Keep at end
	};

	/** Valid reference script types. */
	enum class ScriptReferenceType
	{
		BuiltinResourceBase,
		BuiltinResource,
		ManagedResourceBase,
		ManagedResource,
		BuiltinComponentBase,
		BuiltinComponent,
		ManagedComponentBase,
		ManagedComponent,
		SceneObject,
		ReflectableObject,
		Count // Keep at end
	};

	/**	Flags that are used to further define a field in a managed serializable object. */
	enum class ScriptFieldFlag
	{
		Serializable        = 1 << 0,
		Inspectable         = 1 << 1,
		Range               = 1 << 2,
		Step                = 1 << 3,
		Animable            = 1 << 4,
		AsLayerMask         = 1 << 5,
		PassByCopy          = 1 << 6,
		NotNull             = 1 << 7,
		NativeWrapper       = 1 << 8,
		ApplyOnDirty        = 1 << 9,
		AsQuaternion        = 1 << 10,
		Category			= 1 << 11,
		Order				= 1 << 12,
		Inline				= 1 << 13
	};

	typedef Flags<ScriptFieldFlag> ScriptFieldFlags;
	BS_FLAGS_OPERATORS(ScriptFieldFlag);

	/** Flags that are used to further desribe a type of a managed serializable object. */
	enum class ScriptTypeFlag
	{
		Serializable = 1 << 0,
		Inspectable  = 1 << 1
	};

	typedef Flags<ScriptTypeFlag> ScriptTypeFlags;
	BS_FLAGS_OPERATORS(ScriptTypeFlag);

	/**	Contains information about a type of a managed serializable object. */
	class BS_SCR_BE_EXPORT ManagedSerializableTypeInfo : public IReflectable
	{
	public:
		virtual ~ManagedSerializableTypeInfo() = default;

		/**	Checks if the current type matches the provided type. */
		virtual bool matches(const SPtr<ManagedSerializableTypeInfo>& typeInfo) const = 0;

		/**
		 * Checks does the managed type this object represents still exists.
		 *
		 * @note	For example if assemblies get refreshed user could have renamed or removed some types.
		 */
		virtual bool isTypeLoaded() const = 0;

		/**
		 * Returns the internal managed class of the type this object represents. Returns null if the type doesn't exist.
		 */
		virtual ::MonoClass* getMonoClass() const = 0;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedSerializableTypeInfoRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/**	Contains information about a type of a managed serializable primitive (for example int, float, etc.). */
	class BS_SCR_BE_EXPORT ManagedSerializableTypeInfoPrimitive : public ManagedSerializableTypeInfo
	{
	public:
		/** @copydoc ManagedSerializableTypeInfo::matches */
		bool matches(const SPtr<ManagedSerializableTypeInfo>& typeInfo) const override;

		/** @copydoc ManagedSerializableTypeInfo::isTypeLoaded */
		bool isTypeLoaded() const override;

		/** @copydoc ManagedSerializableTypeInfo::getMonoClass */
		::MonoClass* getMonoClass() const override;

		ScriptPrimitiveType mType;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedSerializableTypeInfoPrimitiveRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/**	Contains information about a type of a managed serializable enum. */
	class BS_SCR_BE_EXPORT ManagedSerializableTypeInfoEnum : public ManagedSerializableTypeInfo
	{
	public:
		/** @copydoc ManagedSerializableTypeInfo::matches */
		bool matches(const SPtr<ManagedSerializableTypeInfo>& typeInfo) const override;

		/** @copydoc ManagedSerializableTypeInfo::isTypeLoaded */
		bool isTypeLoaded() const override;

		/** @copydoc ManagedSerializableTypeInfo::getMonoClass */
		::MonoClass* getMonoClass() const override;

		ScriptPrimitiveType mUnderlyingType;
		String mTypeNamespace;
		String mTypeName;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedSerializableTypeInfoEnumRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/**	Contains information about a type of a managed serializable game object or resourcee. */
	class BS_SCR_BE_EXPORT ManagedSerializableTypeInfoRef : public ManagedSerializableTypeInfo
	{
	public:
		/** @copydoc ManagedSerializableTypeInfo::matches */
		bool matches(const SPtr<ManagedSerializableTypeInfo>& typeInfo) const override;

		/** @copydoc ManagedSerializableTypeInfo::isTypeLoaded */
		bool isTypeLoaded() const override;

		/** @copydoc ManagedSerializableTypeInfo::getMonoClass */
		::MonoClass* getMonoClass() const override;

		ScriptReferenceType mType;
		UINT32 mRTIITypeId;
		String mTypeNamespace;
		String mTypeName;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedSerializableTypeInfoRefRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/**	Contains information about a type of a reference to a resource. */
	class BS_SCR_BE_EXPORT ManagedSerializableTypeInfoRRef : public ManagedSerializableTypeInfo
	{
	public:
		/** @copydoc ManagedSerializableTypeInfo::matches */
		bool matches(const SPtr<ManagedSerializableTypeInfo>& typeInfo) const override;

		/** @copydoc ManagedSerializableTypeInfo::isTypeLoaded */
		bool isTypeLoaded() const override;

		/** @copydoc ManagedSerializableTypeInfo::getMonoClass */
		::MonoClass* getMonoClass() const override;

		SPtr<ManagedSerializableTypeInfo> mResourceType;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedSerializableTypeInfoRRefRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/**	Contains information about a type of a managed serializable complex object (for example struct or class). */
	class BS_SCR_BE_EXPORT ManagedSerializableTypeInfoObject : public ManagedSerializableTypeInfo
	{
	public:
		/** @copydoc ManagedSerializableTypeInfo::matches */
		bool matches(const SPtr<ManagedSerializableTypeInfo>& typeInfo) const override;

		/** @copydoc ManagedSerializableTypeInfo::isTypeLoaded */
		bool isTypeLoaded() const override;

		/** @copydoc ManagedSerializableTypeInfo::getMonoClass */
		::MonoClass* getMonoClass() const override;

		String mTypeNamespace;
		String mTypeName;
		bool mValueType;
		ScriptTypeFlags mFlags;
		UINT32 mTypeId;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedSerializableTypeInfoObjectRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/**	Contains information about a type of a managed serializable Array. */
	class BS_SCR_BE_EXPORT ManagedSerializableTypeInfoArray : public ManagedSerializableTypeInfo
	{
	public:
		/** @copydoc ManagedSerializableTypeInfo::matches */
		bool matches(const SPtr<ManagedSerializableTypeInfo>& typeInfo) const override;

		/** @copydoc ManagedSerializableTypeInfo::isTypeLoaded */
		bool isTypeLoaded() const override;

		/** @copydoc ManagedSerializableTypeInfo::getMonoClass */
		::MonoClass* getMonoClass() const override;

		SPtr<ManagedSerializableTypeInfo> mElementType;
		UINT32 mRank;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedSerializableTypeInfoArrayRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/**	Contains information about a type of a managed serializable List. */
	class BS_SCR_BE_EXPORT ManagedSerializableTypeInfoList : public ManagedSerializableTypeInfo
	{
	public:
		/** @copydoc ManagedSerializableTypeInfo::matches */
		bool matches(const SPtr<ManagedSerializableTypeInfo>& typeInfo) const override;

		/** @copydoc ManagedSerializableTypeInfo::isTypeLoaded */
		bool isTypeLoaded() const override;

		/** @copydoc ManagedSerializableTypeInfo::getMonoClass */
		::MonoClass* getMonoClass() const override;

		SPtr<ManagedSerializableTypeInfo> mElementType;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedSerializableTypeInfoListRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/**	Contains information about a type of a managed serializable Dictionary. */
	class BS_SCR_BE_EXPORT ManagedSerializableTypeInfoDictionary : public ManagedSerializableTypeInfo
	{
	public:
		/** @copydoc ManagedSerializableTypeInfo::matches */
		bool matches(const SPtr<ManagedSerializableTypeInfo>& typeInfo) const override;

		/** @copydoc ManagedSerializableTypeInfo::isTypeLoaded */
		bool isTypeLoaded() const override;

		/** @copydoc ManagedSerializableTypeInfo::getMonoClass */
		::MonoClass* getMonoClass() const override;

		SPtr<ManagedSerializableTypeInfo> mKeyType;
		SPtr<ManagedSerializableTypeInfo> mValueType;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedSerializableTypeInfoDictionaryRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/**	Contains data about a single member in a managed complex object. */
	class BS_SCR_BE_EXPORT ManagedSerializableMemberInfo : public IReflectable
	{
	public:
		ManagedSerializableMemberInfo() = default;
		virtual ~ManagedSerializableMemberInfo() = default;

		/**	Determines should the member be serialized when serializing the parent object. */
		bool isSerializable() const { return mFlags.isSet(ScriptFieldFlag::Serializable); }

		/**
		 * Returns a boxed value contained in the member in the specified object instance.
		 *
		 * @param[in]	instance	Object instance to access the member on.
		 * @return					A boxed value of the member.
		 */
		virtual MonoObject* getValue(MonoObject* instance) const = 0;

		/**
		 * Sets a value of the member in the specified object instance. 
		 *
		 * @param[in]	instance	Object instance to access the member on.
		 * @param[in]	value		Value to set on the property. For value type it should be a pointer to the value and for
		 *							reference type it should be a pointer to MonoObject.
		 */
		virtual void setValue(MonoObject* instance, void* value) const = 0;

		/** 
		 * Checks if the attribute of the provided type exists on the member and returns it, or returns null if the
		 * attribute is not present.
		 */
		virtual MonoObject* getAttribute(MonoClass* monoClass) = 0;

		String mName;
		UINT32 mFieldId = 0;
		UINT32 mParentTypeId;

		SPtr<ManagedSerializableTypeInfo> mTypeInfo;
		ScriptFieldFlags mFlags;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedSerializableMemberInfoRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/**	Contains data about a single field in a managed complex object. */
	class BS_SCR_BE_EXPORT ManagedSerializableFieldInfo : public ManagedSerializableMemberInfo
	{
	public:
		ManagedSerializableFieldInfo() = default;

		/** @copydoc ManagedSerializableMemberInfo::getAttribute */
		MonoObject* getAttribute(MonoClass* monoClass) override;

		/** @copydoc ManagedSerializableMemberInfo::getValue */
		MonoObject* getValue(MonoObject* instance) const override;

		/** @copydoc ManagedSerializableMemberInfo::setValue */
		void setValue(MonoObject* instance, void* value) const override;

		MonoField* mMonoField = nullptr;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedSerializableFieldInfoRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/**	Contains data about a single property in a managed complex object. */
	class BS_SCR_BE_EXPORT ManagedSerializablePropertyInfo : public ManagedSerializableMemberInfo
	{
	public:
		ManagedSerializablePropertyInfo() = default;

		/** @copydoc ManagedSerializableMemberInfo::getAttribute */
		MonoObject* getAttribute(MonoClass* monoClass) override;

		/** @copydoc ManagedSerializableMemberInfo::getValue */
		MonoObject* getValue(MonoObject* instance) const override;

		/** @copydoc ManagedSerializableMemberInfo::setValue */
		void setValue(MonoObject* instance, void* value) const override;

		MonoProperty* mMonoProperty = nullptr;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedSerializablePropertyInfoRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** Contains data about fields of a complex object, and the object's class hierarchy if it belongs to one. */
	class BS_SCR_BE_EXPORT ManagedSerializableObjectInfo : public IReflectable
	{
	public:
		ManagedSerializableObjectInfo() = default;

		/** Returns the managed type name of the object's type, including the namespace in format "namespace.typename". */
		String getFullTypeName() const { return mTypeInfo->mTypeNamespace + "." + mTypeInfo->mTypeName; }

		/**
		 * Attempts to find a field part of this object that matches the provided parameters.
		 *
		 * @param[in]	fieldInfo		Object describing the managed field. Normally this will be a field that was
		 *								deserialized and you need to ensure it still exists in its parent type, while
		 *								retrieving the new field info.
		 * @param[in]	fieldTypeInfo	Type information about the type containing the object. Used for debug purposes to
		 *								ensure the current object's type matches.
		 * @return						Found field info within this object, or null if not found.
		 */
		SPtr<ManagedSerializableMemberInfo> findMatchingField(const SPtr<ManagedSerializableMemberInfo>& fieldInfo,
			const SPtr<ManagedSerializableTypeInfo>& fieldTypeInfo) const;

		SPtr<ManagedSerializableTypeInfoObject> mTypeInfo;
		MonoClass* mMonoClass = nullptr;

		UnorderedMap<String, UINT32> mFieldNameToId;
		UnorderedMap<UINT32, SPtr<ManagedSerializableMemberInfo>> mFields;

		SPtr<ManagedSerializableObjectInfo> mBaseClass;
		Vector<std::weak_ptr<ManagedSerializableObjectInfo>> mDerivedClasses;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedSerializableObjectInfoRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/**	Contains information about all managed serializable objects in a specific managed assembly. */
	class BS_SCR_BE_EXPORT ManagedSerializableAssemblyInfo : public IReflectable
	{
	public:
		String mName;

		UnorderedMap<String, UINT32> mTypeNameToId;
		UnorderedMap<UINT32, SPtr<ManagedSerializableObjectInfo>> mObjectInfos;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedSerializableAssemblyInfoRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** @} */
}
