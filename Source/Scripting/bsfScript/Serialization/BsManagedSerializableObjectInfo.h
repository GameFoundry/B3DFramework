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
		Serializable = 1 << 0,
		Inspectable = 1 << 1,
		Range = 1 << 2,
		Step = 1 << 3,
		Animable = 1 << 4,
		AsLayerMask = 1 << 5,
		PassByCopy = 1 << 6,
		NotNull = 1 << 7,
		NativeWrapper = 1 << 8,
		ApplyOnDirty = 1 << 9,
		AsQuaternion = 1 << 10,
		Category = 1 << 11,
		Order = 1 << 12,
		Inline = 1 << 13,
		LoadOnAssign = 1 << 14,
		HDR = 1 << 15,
	};

	typedef Flags<ScriptFieldFlag> ScriptFieldFlags;
	B3D_FLAGS_OPERATORS(ScriptFieldFlag);

	/** Flags that are used to further desribe a type of a managed serializable object. */
	enum class ScriptTypeFlag
	{
		Serializable = 1 << 0,
		Inspectable = 1 << 1
	};

	typedef Flags<ScriptTypeFlag> ScriptTypeFlags;
	B3D_FLAGS_OPERATORS(ScriptTypeFlag);

	/**	Contains information about a type of a managed serializable object. */
	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableTypeInfo : public IReflectable
	{
	public:
		virtual ~ManagedSerializableTypeInfo() = default;

		/**	Checks if the current type matches the provided type. */
		virtual bool Matches(const SPtr<ManagedSerializableTypeInfo>& typeInfo) const = 0;

		/**
		 * Checks does the managed type this object represents still exists.
		 *
		 * @note	For example if assemblies get refreshed user could have renamed or removed some types.
		 */
		virtual bool IsTypeLoaded() const = 0;

		/**
		 * Returns the internal managed class of the type this object represents. Returns null if the type doesn't exist.
		 */
		virtual ::MonoClass* GetMonoClass() const = 0;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedSerializableTypeInfoRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;
	};

	/**	Contains information about a type of a managed serializable primitive (for example int, float, etc.). */
	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableTypeInfoPrimitive : public ManagedSerializableTypeInfo
	{
	public:
		bool Matches(const SPtr<ManagedSerializableTypeInfo>& typeInfo) const override;
		bool IsTypeLoaded() const override;
		::MonoClass* GetMonoClass() const override;

		ScriptPrimitiveType MType;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedSerializableTypeInfoPrimitiveRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;
	};

	/**	Contains information about a type of a managed serializable enum. */
	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableTypeInfoEnum : public ManagedSerializableTypeInfo
	{
	public:
		bool Matches(const SPtr<ManagedSerializableTypeInfo>& typeInfo) const override;
		bool IsTypeLoaded() const override;
		::MonoClass* GetMonoClass() const override;

		ScriptPrimitiveType MUnderlyingType;
		String MTypeNamespace;
		String MTypeName;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedSerializableTypeInfoEnumRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;
	};

	/**	Contains information about a type of a managed serializable game object or resourcee. */
	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableTypeInfoRef : public ManagedSerializableTypeInfo
	{
	public:
		bool Matches(const SPtr<ManagedSerializableTypeInfo>& typeInfo) const override;
		bool IsTypeLoaded() const override;
		::MonoClass* GetMonoClass() const override;

		ScriptReferenceType MType;
		u32 MRtiiTypeId;
		String MTypeNamespace;
		String MTypeName;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedSerializableTypeInfoRefRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;
	};

	/**	Contains information about a type of a reference to a resource. */
	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableTypeInfoRRef : public ManagedSerializableTypeInfo
	{
	public:
		bool Matches(const SPtr<ManagedSerializableTypeInfo>& typeInfo) const override;
		bool IsTypeLoaded() const override;
		::MonoClass* GetMonoClass() const override;

		SPtr<ManagedSerializableTypeInfo> MResourceType;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedSerializableTypeInfoRRefRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;
	};

	/**	Contains information about a type of a managed serializable complex object (for example struct or class). */
	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableTypeInfoObject : public ManagedSerializableTypeInfo
	{
	public:
		bool Matches(const SPtr<ManagedSerializableTypeInfo>& typeInfo) const override;
		bool IsTypeLoaded() const override;
		::MonoClass* GetMonoClass() const override;

		String MTypeNamespace;
		String MTypeName;
		bool MValueType;
		u32 MRtiiTypeId;
		ScriptTypeFlags MFlags;
		u32 MTypeId;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedSerializableTypeInfoObjectRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;
	};

	/**	Contains information about a type of a managed serializable Array. */
	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableTypeInfoArray : public ManagedSerializableTypeInfo
	{
	public:
		bool Matches(const SPtr<ManagedSerializableTypeInfo>& typeInfo) const override;
		bool IsTypeLoaded() const override;
		::MonoClass* GetMonoClass() const override;

		SPtr<ManagedSerializableTypeInfo> MElementType;
		u32 MRank;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedSerializableTypeInfoArrayRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;
	};

	/**	Contains information about a type of a managed serializable List. */
	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableTypeInfoList : public ManagedSerializableTypeInfo
	{
	public:
		bool Matches(const SPtr<ManagedSerializableTypeInfo>& typeInfo) const override;
		bool IsTypeLoaded() const override;
		::MonoClass* GetMonoClass() const override;

		SPtr<ManagedSerializableTypeInfo> MElementType;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedSerializableTypeInfoListRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;
	};

	/**	Contains information about a type of a managed serializable Dictionary. */
	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableTypeInfoDictionary : public ManagedSerializableTypeInfo
	{
	public:
		bool Matches(const SPtr<ManagedSerializableTypeInfo>& typeInfo) const override;
		bool IsTypeLoaded() const override;
		::MonoClass* GetMonoClass() const override;

		SPtr<ManagedSerializableTypeInfo> MKeyType;
		SPtr<ManagedSerializableTypeInfo> MValueType;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedSerializableTypeInfoDictionaryRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;
	};

	/**	Contains data about a single member in a managed complex object. */
	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableMemberInfo : public IReflectable
	{
	public:
		ManagedSerializableMemberInfo() = default;
		virtual ~ManagedSerializableMemberInfo() = default;

		/**	Determines should the member be serialized when serializing the parent object. */
		bool IsSerializable() const { return MFlags.IsSet(ScriptFieldFlag::Serializable); }

		/**
		 * Returns a boxed value contained in the member in the specified object instance.
		 *
		 * @param[in]	instance	Object instance to access the member on.
		 * @return					A boxed value of the member.
		 */
		virtual MonoObject* GetValue(MonoObject* instance) const = 0;

		/**
		 * Sets a value of the member in the specified object instance.
		 *
		 * @param[in]	instance	Object instance to access the member on.
		 * @param[in]	value		Value to set on the property. For value type it should be a pointer to the value and for
		 *							reference type it should be a pointer to MonoObject.
		 */
		virtual void SetValue(MonoObject* instance, void* value) const = 0;

		/**
		 * Checks if the attribute of the provided type exists on the member and returns it, or returns null if the
		 * attribute is not present.
		 */
		virtual MonoObject* GetAttribute(MonoClass* monoClass) = 0;

		String MName;
		u32 MFieldId = 0;
		u32 MParentTypeId;

		SPtr<ManagedSerializableTypeInfo> MTypeInfo;
		ScriptFieldFlags MFlags;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedSerializableMemberInfoRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;
	};

	/**	Contains data about a single field in a managed complex object. */
	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableFieldInfo : public ManagedSerializableMemberInfo
	{
	public:
		ManagedSerializableFieldInfo() = default;

		MonoObject* GetAttribute(MonoClass* monoClass) override;
		MonoObject* GetValue(MonoObject* instance) const override;
		void SetValue(MonoObject* instance, void* value) const override;

		MonoField* MMonoField = nullptr;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedSerializableFieldInfoRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;
	};

	/**	Contains data about a single property in a managed complex object. */
	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializablePropertyInfo : public ManagedSerializableMemberInfo
	{
	public:
		ManagedSerializablePropertyInfo() = default;

		MonoObject* GetAttribute(MonoClass* monoClass) override;
		MonoObject* GetValue(MonoObject* instance) const override;
		void SetValue(MonoObject* instance, void* value) const override;

		MonoProperty* MMonoProperty = nullptr;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedSerializablePropertyInfoRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;
	};

	/** Contains data about fields of a complex object, and the object's class hierarchy if it belongs to one. */
	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableObjectInfo : public IReflectable
	{
	public:
		ManagedSerializableObjectInfo() = default;

		/** Returns the managed type name of the object's type, including the namespace in format "namespace.typename". */
		String GetFullTypeName() const { return MTypeInfo->MTypeNamespace + "." + MTypeInfo->MTypeName; }

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
		SPtr<ManagedSerializableMemberInfo> FindMatchingField(const SPtr<ManagedSerializableMemberInfo>& fieldInfo, const SPtr<ManagedSerializableTypeInfo>& fieldTypeInfo) const;

		SPtr<ManagedSerializableTypeInfoObject> MTypeInfo;
		MonoClass* MMonoClass = nullptr;

		UnorderedMap<String, u32> MFieldNameToId;
		UnorderedMap<u32, SPtr<ManagedSerializableMemberInfo>> MFields;

		SPtr<ManagedSerializableObjectInfo> MBaseClass;
		Vector<std::weak_ptr<ManagedSerializableObjectInfo>> MDerivedClasses;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedSerializableObjectInfoRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;
	};

	/**	Contains information about all managed serializable objects in a specific managed assembly. */
	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableAssemblyInfo : public IReflectable
	{
	public:
		String MName;

		UnorderedMap<String, u32> MTypeNameToId;
		UnorderedMap<u32, SPtr<ManagedSerializableObjectInfo>> MObjectInfos;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ManagedSerializableAssemblyInfoRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;
	};

	/** @} */
} // namespace bs
