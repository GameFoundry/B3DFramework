//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsMonoField.h"
#include "BsMonoProperty.h"
#include "BsMonoAssembly.h"
#include "BsMonoManager.h"
#include "BsException.h"
#include "BsMonoUtil.h"

#include <mono/jit/jit.h>
#include <mono/metadata/debug-helpers.h>

namespace BansheeEngine
{
	size_t MonoClass::MethodId::Hash::operator()(const MonoClass::MethodId& v) const
	{
		size_t seed = 0;
		hash_combine(seed, v.name);
		hash_combine(seed, v.numParams);
		return seed;
	}

	bool MonoClass::MethodId::Equals::operator()(const MonoClass::MethodId& a, const MonoClass::MethodId& b) const
	{
		return a.name == b.name && a.numParams == b.numParams;
	}

	MonoClass::MethodId::MethodId(const String& name, UINT32 numParams)
		:name(name), numParams(numParams)
	{
		
	}

	MonoClass::MonoClass(const String& ns, const String& type, ::MonoClass* monoClass, const MonoAssembly* parentAssembly)
		: mParentAssembly(parentAssembly), mClass(monoClass), mNamespace(ns), mTypeName(type), mHasCachedFields(false)
		, mHasCachedProperties(false), mHasCachedMethods(false)
	{
		mFullName = ns + "." + type;
	}

	MonoClass::~MonoClass()
	{
		for(auto& mapEntry : mMethods)
		{
			bs_delete(mapEntry.second);
		}

		mMethods.clear();

		for(auto& mapEntry : mFields)
		{
			bs_delete(mapEntry.second);
		}

		mFields.clear();

		for(auto& mapEntry : mProperties)
		{
			bs_delete(mapEntry.second);
		}

		mProperties.clear();
	}

	MonoMethod* MonoClass::getMethod(const String& name, UINT32 numParams) const
	{
		MethodId mehodId(name, numParams);
		auto iterFind = mMethods.find(mehodId);
		if(iterFind != mMethods.end())
			return iterFind->second;

		::MonoMethod* method = mono_class_get_method_from_name(mClass, name.c_str(), (int)numParams);
		if (method == nullptr)
			return nullptr;

		MonoMethod* newMethod = new (bs_alloc<MonoMethod>()) MonoMethod(method);
		mMethods[mehodId] = newMethod;

		return newMethod;
	}

	MonoMethod* MonoClass::getMethodExact(const String& name, const String& signature) const
	{
		MethodId mehodId(name + "(" + signature + ")", 0);
		auto iterFind = mMethods.find(mehodId);
		if(iterFind != mMethods.end())
			return iterFind->second;

		::MonoMethod* method;
		void* iter = nullptr;

		const char* rawName = name.c_str();
		const char* rawSig = signature.c_str();
		while ((method = mono_class_get_methods(mClass, &iter)))
		{
			if (strcmp(rawName, mono_method_get_name(method)) == 0)
			{
				const char* curSig = mono_signature_get_desc(mono_method_signature(method), false);
				if(strcmp(rawSig, curSig) == 0)
				{
					MonoMethod* newMethod = new (bs_alloc<MonoMethod>()) MonoMethod(method);
					mMethods[mehodId] = newMethod;

					return newMethod;
				}
			}
		}

		return nullptr;
	}

	bool MonoClass::hasField(const String& name) const
	{
		MonoClassField* field = mono_class_get_field_from_name(mClass, name.c_str());

		return field != nullptr;
	}

	bool MonoClass::isSubClassOf(const MonoClass* monoClass) const
	{
		if(monoClass == nullptr)
			return false;

		return mono_class_is_subclass_of(mClass, monoClass->mClass, true) != 0;
	}

	MonoField* MonoClass::getField(const String& name) const
	{
		auto iterFind = mFields.find(name);
		if(iterFind != mFields.end())
			return iterFind->second;

		MonoClassField* field = mono_class_get_field_from_name(mClass, name.c_str());
		if(field == nullptr)
			return nullptr;

		MonoField* newField = new (bs_alloc<MonoField>()) MonoField(field);
		mFields[name] = newField;

		return newField;
	}

	MonoProperty* MonoClass::getProperty(const String& name) const
	{
		auto iterFind = mProperties.find(name);
		if(iterFind != mProperties.end())
			return iterFind->second;

		::MonoProperty* property = mono_class_get_property_from_name(mClass, name.c_str());
		if (property == nullptr)
			return nullptr;

		MonoProperty* newProperty = new (bs_alloc<MonoProperty>()) MonoProperty(property);
		mProperties[name] = newProperty;

		return newProperty;
	}

	const Vector<MonoField*>& MonoClass::getAllFields() const
	{
		if(mHasCachedFields)
			return mCachedFieldList;

		mCachedFieldList.clear();

		void* iter = nullptr;
		MonoClassField* curClassField = mono_class_get_fields(mClass, &iter);
		while(curClassField != nullptr)
		{
			const char* fieldName = mono_field_get_name(curClassField);
			MonoField* curField = getField(fieldName);

			mCachedFieldList.push_back(curField);

			curClassField = mono_class_get_fields(mClass, &iter);
		}

		mHasCachedFields = true;
		return mCachedFieldList;
	}

	const Vector<MonoProperty*>& MonoClass::getAllProperties() const
	{
		if (mHasCachedProperties)
			return mCachedPropertyList;

		mCachedPropertyList.clear();

		void* iter = nullptr;
		::MonoProperty* curClassProperty = mono_class_get_properties(mClass, &iter);
		while (curClassProperty != nullptr)
		{
			const char* propertyName = mono_property_get_name(curClassProperty);
			MonoProperty* curProperty = getProperty(propertyName);

			mCachedPropertyList.push_back(curProperty);

			curClassProperty = mono_class_get_properties(mClass, &iter);
		}

		mHasCachedProperties = true;
		return mCachedPropertyList;
	}

	const Vector<MonoMethod*>& MonoClass::getAllMethods() const
	{
		if (mHasCachedMethods)
			return mCachedMethodList;

		mCachedMethodList.clear();

		void* iter = nullptr;
		::MonoMethod* curClassMethod = mono_class_get_methods(mClass, &iter);
		while (curClassMethod != nullptr)
		{
			MonoMethodSignature* sig = mono_method_signature(curClassMethod);

			const char* sigDesc = mono_signature_get_desc(sig, false);
			const char* methodName = mono_method_get_name(curClassMethod);

			MonoMethod* curMethod = getMethodExact(methodName, sigDesc);
			mCachedMethodList.push_back(curMethod);

			curClassMethod = mono_class_get_methods(mClass, &iter);
		}

		mHasCachedMethods = true;
		return mCachedMethodList;
	}

	Vector<MonoClass*> MonoClass::getAllAttributes() const
	{
		// TODO - Consider caching custom attributes or just initializing them all at load
		Vector<MonoClass*> attributes;

		MonoCustomAttrInfo* attrInfo = mono_custom_attrs_from_class(mClass);
		if (attrInfo == nullptr)
			return attributes;

		for (INT32 i = 0; i < attrInfo->num_attrs; i++)
		{
			::MonoClass* attribClass = mono_method_get_class(attrInfo->attrs[i].ctor);
			MonoClass* klass = MonoManager::instance().findClass(attribClass);

			if (klass != nullptr)
				attributes.push_back(klass);
		}

		mono_custom_attrs_free(attrInfo);

		return attributes;
	}

	MonoObject* MonoClass::invokeMethod(const String& name, MonoObject* instance, void** params, UINT32 numParams)
	{
		return getMethod(name, numParams)->invoke(instance, params);
	}

	void MonoClass::addInternalCall(const String& name, const void* method)
	{
		String fullMethodName = mFullName + "::" + name;
		mono_add_internal_call(fullMethodName.c_str(), method);
	}

	MonoObject* MonoClass::createInstance(bool construct) const
	{
		MonoObject* obj = mono_object_new(MonoManager::instance().getDomain(), mClass);

		if(construct)
			mono_runtime_object_init(obj);

		return obj;
	}

	MonoObject* MonoClass::createInstance(void** params, UINT32 numParams)
	{
		MonoObject* obj = mono_object_new(MonoManager::instance().getDomain(), mClass);
		getMethod(".ctor", numParams)->invoke(obj, params);

		return obj;
	}

	MonoObject* MonoClass::createInstance(const String& ctorSignature, void** params)
	{
		MonoObject* obj = mono_object_new(MonoManager::instance().getDomain(), mClass);
		getMethodExact(".ctor", ctorSignature)->invoke(obj, params);

		return obj;
	}

	void MonoClass::construct(MonoObject* object)
	{
		mono_runtime_object_init(object);
	}

	bool MonoClass::hasAttribute(MonoClass* monoClass) const
	{
		// TODO - Consider caching custom attributes or just initializing them all at load

		MonoCustomAttrInfo* attrInfo = mono_custom_attrs_from_class(mClass);
		if(attrInfo == nullptr)
			return false;
	
		bool hasAttr = mono_custom_attrs_has_attr(attrInfo, monoClass->_getInternalClass()) != 0;

		mono_custom_attrs_free(attrInfo);

		return hasAttr;
	}

	MonoObject* MonoClass::getAttribute(MonoClass* monoClass) const
	{
		// TODO - Consider caching custom attributes or just initializing them all at load

		MonoCustomAttrInfo* attrInfo = mono_custom_attrs_from_class(mClass);
		if(attrInfo == nullptr)
			return nullptr;

		MonoObject* foundAttr = mono_custom_attrs_get_attr(attrInfo, monoClass->_getInternalClass());

		mono_custom_attrs_free(attrInfo);

		return foundAttr;
	}

	MonoClass* MonoClass::getBaseClass() const
	{
		::MonoClass* monoBase = mono_class_get_parent(mClass);
		if(monoBase == nullptr)
			return nullptr;

		String ns;
		String type;
		MonoUtil::getClassName(monoBase, ns, type);

		return MonoManager::instance().findClass(ns, type);
	}

	bool MonoClass::isInstanceOfType(MonoObject* object) const
	{
		if(object == nullptr)
			return false;

		::MonoClass* monoClass = mono_object_get_class(object);
		return mono_class_is_subclass_of(monoClass, mClass, false) != 0;
	}

	UINT32 MonoClass::getInstanceSize() const
	{
		UINT32 dummy = 0;

		if (mono_class_is_valuetype(mClass))
			return mono_class_value_size(mClass, &dummy);
		
		return mono_class_instance_size(mClass);
	}
}