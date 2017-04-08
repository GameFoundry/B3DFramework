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
		Texture,
		SpriteTexture,
		ManagedResource,
		PlainText,
		ScriptCode,
		Shader,
		ShaderInclude,
		Material,
		Mesh,
		Prefab,
		Font,
		StringTable,
		GUISkin,
		SceneObject,
		BuiltinComponent,
		PhysicsMaterial,
		PhysicsMesh,
		AudioClip,
		AnimationClip,
		ManagedComponent,
		Resource,
		BuiltinComponentBase,
		ManagedComponentBase,
		Count // Keep at end
	};

	/**	Flags that are used to further define a field in a managed serializable object. */
	enum class ScriptFieldFlag
	{
		Serializable = 0x01,
		Inspectable = 0x02,
		Range = 0x04,
		Step = 0x08,
		Animable = 0x10
	};

	typedef Flags<ScriptFieldFlag> ScriptFieldFlags;
	BS_FLAGS_OPERATORS(ScriptFieldFlag);

	/**	Contains information about a type of a managed serializable object. */
	class BS_SCR_BE_EXPORT ManagedSerializableTypeInfo : public IReflectable
	{
	public:
		virtual ~ManagedSerializableTypeInfo() {}

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

	/**	Contains information about a type of a managed serializable game object or resource reference. */
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
		ManagedSerializableMemberInfo();
		virtual ~ManagedSerializableMemberInfo() {}

		/**	Determines should the member be serialized when serializing the parent object. */
		bool isSerializable() const { return mFlags.isSet(ScriptFieldFlag::Serializable); }

		/** Returns the minimum value associated to a Range attribute. */
		virtual float getRangeMinimum() const = 0;

		/** Returns the maximum value associated to a Range attribute. */
		virtual float getRangeMaximum() const = 0;

		/** Checks whether the field should be rendered as a slider. */
		virtual bool renderAsSlider() const = 0;

		/** Returns the step value of the member. */
		virtual float getStep() const = 0;

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

		String mName;
		UINT32 mFieldId;
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
		ManagedSerializableFieldInfo();

		/** @copydoc ManagedSerializableMemberInfo::getRangeMinimum */
		float getRangeMinimum() const override;

		/** @copydoc ManagedSerializableMemberInfo::getRangeMaximum */
		float getRangeMaximum() const override;

		/** @copydoc ManagedSerializableMemberInfo::renderAsSlider */
		bool renderAsSlider() const override;

		/** @copydoc ManagedSerializableMemberInfo::getStep */
		float getStep() const override;

		/** @copydoc ManagedSerializableMemberInfo::getValue */
		MonoObject* getValue(MonoObject* instance) const override;

		/** @copydoc ManagedSerializableMemberInfo::setValue */
		void setValue(MonoObject* instance, void* value) const override;

		MonoField* mMonoField;

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
		ManagedSerializablePropertyInfo();

		/** @copydoc ManagedSerializableMemberInfo::getRangeMinimum */
		float getRangeMinimum() const override;

		/** @copydoc ManagedSerializableMemberInfo::getRangeMaximum */
		float getRangeMaximum() const override;

		/** @copydoc ManagedSerializableMemberInfo::renderAsSlider */
		bool renderAsSlider() const override;

		/** @copydoc ManagedSerializableMemberInfo::getStep */
		float getStep() const override;

		/** @copydoc ManagedSerializableMemberInfo::getValue */
		MonoObject* getValue(MonoObject* instance) const override;

		/** @copydoc ManagedSerializableMemberInfo::setValue */
		void setValue(MonoObject* instance, void* value) const override;

		MonoProperty* mMonoProperty;

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
		ManagedSerializableObjectInfo();

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
		MonoClass* mMonoClass;

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