//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMonoClass.h"
#include "B3DMonoMethod.h"
#include "B3DMonoField.h"
#include "B3DMonoProperty.h"
#include "B3DMonoAssembly.h"
#include "B3DMonoManager.h"
#include "B3DMonoUtil.h"

#if B3D_USE_DOTNETCORE
#include "B3DMonoLoader.h"
#else
#include <mono/jit/jit.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/object.h>
#include <mono/metadata/class.h>
#include <mono/metadata/reflection.h>
#endif

namespace b3d {
size_t MonoClass::MethodId::Hash::operator()(const MonoClass::MethodId& v) const
{
	size_t seed = 0;
	B3DCombineHash(seed, v.Name);
	B3DCombineHash(seed, v.NumParams);
	return seed;
}

bool MonoClass::MethodId::Equals::operator()(const MonoClass::MethodId& a, const MonoClass::MethodId& b) const
{
	return a.Name == b.Name && a.NumParams == b.NumParams;
}

MonoClass::MethodId::MethodId(const String& name, u32 numParams)
	: Name(name), NumParams(numParams)
{
}

MonoClass::MonoClass(const String& ns, const String& type, ::MonoClass* monoClass, const MonoAssembly* parentAssembly)
	: mParentAssembly(parentAssembly), mClass(monoClass), mNamespace(ns), mTypeName(type), mHasCachedFields(false), mHasCachedProperties(false), mHasCachedMethods(false)
{
	mFullName = ns + "." + type;
}

MonoClass::~MonoClass()
{
	for(auto& mapEntry : mMethods)
	{
		B3DDelete(mapEntry.second);
	}

	mMethods.clear();

	for(auto& mapEntry : mFields)
	{
		B3DDelete(mapEntry.second);
	}

	mFields.clear();

	for(auto& mapEntry : mProperties)
	{
		B3DDelete(mapEntry.second);
	}

	mProperties.clear();
}

MonoMethod* MonoClass::GetMethod(const String& name, u32 numParams) const
{
	MethodId mehodId(name, numParams);
	auto iterFind = mMethods.find(mehodId);
	if(iterFind != mMethods.end())
		return iterFind->second;

	::MonoMethod* method = mono_class_get_method_from_name(mClass, name.c_str(), (int)numParams);
	if(method == nullptr)
		return nullptr;

	MonoMethod* newMethod = new(B3DAllocate<MonoMethod>()) MonoMethod(method);
	mMethods[mehodId] = newMethod;

	return newMethod;
}

MonoMethod* MonoClass::GetMethodExact(const String& name, const String& signature) const
{
	MethodId mehodId(name + "(" + signature + ")", 0);
	auto iterFind = mMethods.find(mehodId);
	if(iterFind != mMethods.end())
		return iterFind->second;

	::MonoMethod* method;
	void* iter = nullptr;

	const char* rawName = name.c_str();
	const char* rawSig = signature.c_str();
	while((method = mono_class_get_methods(mClass, &iter)))
	{
		if(strcmp(rawName, mono_method_get_name(method)) == 0)
		{
			const char* curSig = mono_signature_get_desc(mono_method_signature(method), false);
			if(strcmp(rawSig, curSig) == 0)
			{
				MonoMethod* newMethod = new(B3DAllocate<MonoMethod>()) MonoMethod(method);
				mMethods[mehodId] = newMethod;

				return newMethod;
			}
		}
	}

	return nullptr;
}

bool MonoClass::HasField(const String& name) const
{
	MonoClassField* field = mono_class_get_field_from_name(mClass, name.c_str());

	return field != nullptr;
}

bool MonoClass::IsSubClassOf(const MonoClass* monoClass) const
{
	if(monoClass == nullptr)
		return false;

	return mono_class_is_subclass_of(mClass, monoClass->mClass, true) != 0;
}

MonoField* MonoClass::GetField(const String& name) const
{
	auto iterFind = mFields.find(name);
	if(iterFind != mFields.end())
		return iterFind->second;

	MonoClassField* field = mono_class_get_field_from_name(mClass, name.c_str());
	if(field == nullptr)
		return nullptr;

	MonoField* newField = new(B3DAllocate<MonoField>()) MonoField(field);
	mFields[name] = newField;

	return newField;
}

MonoProperty* MonoClass::GetProperty(const String& name) const
{
	auto iterFind = mProperties.find(name);
	if(iterFind != mProperties.end())
		return iterFind->second;

	::MonoProperty* property = mono_class_get_property_from_name(mClass, name.c_str());
	if(property == nullptr)
		return nullptr;

	MonoProperty* newProperty = new(B3DAllocate<MonoProperty>()) MonoProperty(property);
	mProperties[name] = newProperty;

	return newProperty;
}

const Vector<MonoField*>& MonoClass::GetAllFields() const
{
	if(mHasCachedFields)
		return mCachedFieldList;

	mCachedFieldList.clear();

	void* iter = nullptr;
	MonoClassField* curClassField = mono_class_get_fields(mClass, &iter);
	while(curClassField != nullptr)
	{
		const char* fieldName = mono_field_get_name(curClassField);
		MonoField* curField = GetField(fieldName);

		mCachedFieldList.push_back(curField);

		curClassField = mono_class_get_fields(mClass, &iter);
	}

	mHasCachedFields = true;
	return mCachedFieldList;
}

const Vector<MonoProperty*>& MonoClass::GetAllProperties() const
{
	if(mHasCachedProperties)
		return mCachedPropertyList;

	mCachedPropertyList.clear();

	void* iter = nullptr;
	::MonoProperty* curClassProperty = mono_class_get_properties(mClass, &iter);
	while(curClassProperty != nullptr)
	{
		const char* propertyName = mono_property_get_name(curClassProperty);
		MonoProperty* curProperty = GetProperty(propertyName);

		mCachedPropertyList.push_back(curProperty);

		curClassProperty = mono_class_get_properties(mClass, &iter);
	}

	mHasCachedProperties = true;
	return mCachedPropertyList;
}

const Vector<MonoMethod*>& MonoClass::GetAllMethods() const
{
	if(mHasCachedMethods)
		return mCachedMethodList;

	mCachedMethodList.clear();

	void* iter = nullptr;
	::MonoMethod* curClassMethod = mono_class_get_methods(mClass, &iter);
	while(curClassMethod != nullptr)
	{
		MonoMethodSignature* sig = mono_method_signature(curClassMethod);

		const char* sigDesc = mono_signature_get_desc(sig, false);
		const char* methodName = mono_method_get_name(curClassMethod);

		MonoMethod* curMethod = GetMethodExact(methodName, sigDesc);
		mCachedMethodList.push_back(curMethod);

		curClassMethod = mono_class_get_methods(mClass, &iter);
	}

	mHasCachedMethods = true;
	return mCachedMethodList;
}

Vector<MonoClass*> MonoClass::GetAllAttributes() const
{
	// TODO - Consider caching custom attributes or just initializing them all at load
	Vector<MonoClass*> attributes;

	MonoCustomAttrInfo* attrInfo = mono_custom_attrs_from_class(mClass);
	if(attrInfo == nullptr)
		return attributes;

	for(i32 i = 0; i < attrInfo->num_attrs; i++)
	{
		::MonoClass* attribClass = mono_method_get_class(attrInfo->attrs[i].ctor);
		MonoClass* klass = MonoManager::Instance().FindClass(attribClass);

		if(klass != nullptr)
			attributes.push_back(klass);
	}

	mono_custom_attrs_free(attrInfo);

	return attributes;
}

MonoObject* MonoClass::InvokeMethod(const String& name, MonoObject* instance, void** params, u32 numParams)
{
	return GetMethod(name, numParams)->Invoke(instance, params);
}

void MonoClass::AddInternalCall(const String& name, const void* method)
{
	String fullMethodName = mFullName + "::" + name;
	mono_add_internal_call(fullMethodName.c_str(), method);
}

MonoObject* MonoClass::CreateInstance(bool construct) const
{
	MonoObject* obj = mono_object_new(MonoManager::Instance().GetDomain(), mClass);

	if(construct)
		mono_runtime_object_init(obj);

	return obj;
}

MonoObject* MonoClass::CreateInstance(void** params, u32 numParams)
{
	MonoObject* obj = mono_object_new(MonoManager::Instance().GetDomain(), mClass);
	GetMethod(".ctor", numParams)->Invoke(obj, params);

	return obj;
}

MonoObject* MonoClass::CreateInstance(const String& ctorSignature, void** params)
{
	MonoObject* obj = mono_object_new(MonoManager::Instance().GetDomain(), mClass);
	GetMethodExact(".ctor", ctorSignature)->Invoke(obj, params);

	return obj;
}

void MonoClass::Construct(MonoObject* object)
{
	mono_runtime_object_init(object);
}

bool MonoClass::HasAttribute(MonoClass* monoClass) const
{
	// TODO - Consider caching custom attributes or just initializing them all at load

	MonoCustomAttrInfo* attrInfo = mono_custom_attrs_from_class(mClass);
	if(attrInfo == nullptr)
		return false;

	bool hasAttr = mono_custom_attrs_has_attr(attrInfo, monoClass->GetInternalClass()) != 0;

	mono_custom_attrs_free(attrInfo);

	return hasAttr;
}

MonoObject* MonoClass::GetAttribute(MonoClass* monoClass) const
{
	// TODO - Consider caching custom attributes or just initializing them all at load

	MonoCustomAttrInfo* attrInfo = mono_custom_attrs_from_class(mClass);
	if(attrInfo == nullptr)
		return nullptr;

	MonoObject* foundAttr = nullptr;
	if(mono_custom_attrs_has_attr(attrInfo, monoClass->GetInternalClass()))
		foundAttr = mono_custom_attrs_get_attr(attrInfo, monoClass->GetInternalClass());

	mono_custom_attrs_free(attrInfo);
	return foundAttr;
}

MonoClass* MonoClass::GetBaseClass() const
{
	::MonoClass* monoBase = mono_class_get_parent(mClass);
	if(monoBase == nullptr)
		return nullptr;

	String ns;
	String type;
	MonoUtil::GetClassName(monoBase, ns, type);

	return MonoManager::Instance().FindClass(ns, type);
}

bool MonoClass::IsInstanceOfType(MonoObject* object) const
{
	if(object == nullptr)
		return false;

	::MonoClass* monoClass = mono_object_get_class(object);
	return mono_class_is_subclass_of(monoClass, mClass, false) != 0;
}

u32 MonoClass::GetInstanceSize() const
{
	u32 dummy = 0;

	if(mono_class_is_valuetype(mClass))
		return mono_class_value_size(mClass, &dummy);

	return mono_class_instance_size(mClass);
}
} // namespace b3d
