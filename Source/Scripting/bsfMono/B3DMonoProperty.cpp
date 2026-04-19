//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMonoProperty.h"
#include "B3DMonoMethod.h"
#include "B3DMonoManager.h"
#include "B3DMonoClass.h"

#if B3D_USE_DOTNETCORE
#include "B3DMonoLoader.h"
#else
#include <mono/jit/jit.h>
#include <mono/metadata/class.h>
#include <mono/metadata/object.h>
#include <mono/metadata/reflection.h>
#endif

namespace b3d {
MonoProperty::MonoProperty(::MonoProperty* monoProp)
	: mProperty(monoProp), mReturnType(nullptr), mIsIndexed(false), mIsFullyInitialized(false)
{
	mGetMethod = mono_property_get_get_method(mProperty);
	mSetMethod = mono_property_get_set_method(mProperty);

	mName = mono_property_get_name(mProperty);
}

MonoObject* MonoProperty::Get(MonoObject* instance) const
{
	if(mGetMethod == nullptr)
		return nullptr;

	return mono_runtime_invoke(mGetMethod, instance, nullptr, nullptr);
}

void MonoProperty::Set(MonoObject* instance, void* value) const
{
	if(mSetMethod == nullptr)
		return;

	void* args[1];
	args[0] = value;
	mono_runtime_invoke(mSetMethod, instance, args, nullptr);
}

MonoObject* MonoProperty::GetIndexed(MonoObject* instance, u32 index) const
{
	void* args[1];
	args[0] = &index;
	return mono_runtime_invoke(mGetMethod, instance, args, nullptr);
}

void MonoProperty::SetIndexed(MonoObject* instance, u32 index, void* value) const
{
	void* args[2];
	args[0] = &index;
	args[1] = value;
	mono_runtime_invoke(mSetMethod, instance, args, nullptr);
}

bool MonoProperty::IsIndexed() const
{
	if(!mIsFullyInitialized)
		InitializeDeferred();

	return mIsIndexed;
}

MonoClass* MonoProperty::GetReturnType() const
{
	if(!mIsFullyInitialized)
		InitializeDeferred();

	return mReturnType;
}

bool MonoProperty::HasAttribute(MonoClass* monoClass)
{
	// TODO - Consider caching custom attributes or just initializing them all at load

	::MonoClass* parentClass = mono_property_get_parent(mProperty);
	MonoCustomAttrInfo* attrInfo = mono_custom_attrs_from_property(parentClass, mProperty);
	if(attrInfo == nullptr)
		return false;

	bool hasAttr = mono_custom_attrs_has_attr(attrInfo, monoClass->GetInternalClass()) != 0;

	mono_custom_attrs_free(attrInfo);

	return hasAttr;
}

MonoObject* MonoProperty::GetAttribute(MonoClass* monoClass)
{
	// TODO - Consider caching custom attributes or just initializing them all at load

	::MonoClass* parentClass = mono_property_get_parent(mProperty);
	MonoCustomAttrInfo* attrInfo = mono_custom_attrs_from_property(parentClass, mProperty);
	if(attrInfo == nullptr)
		return nullptr;

	MonoObject* foundAttr = nullptr;
	if(mono_custom_attrs_has_attr(attrInfo, monoClass->GetInternalClass()))
		foundAttr = mono_custom_attrs_get_attr(attrInfo, monoClass->GetInternalClass());

	mono_custom_attrs_free(attrInfo);
	return foundAttr;
}

MonoMemberVisibility MonoProperty::GetVisibility()
{
	MonoMemberVisibility getterVisibility = MonoMemberVisibility::Public;
	if(mGetMethod)
	{
		MonoMethod getterWrapper(mGetMethod);
		getterVisibility = getterWrapper.GetVisibility();
	}

	MonoMemberVisibility setterVisibility = MonoMemberVisibility::Public;
	if(mSetMethod)
	{
		MonoMethod setterWrapper(mSetMethod);
		setterVisibility = setterWrapper.GetVisibility();
	}

	if(getterVisibility < setterVisibility)
		return getterVisibility;

	return setterVisibility;
}

void MonoProperty::InitializeDeferred() const
{
	if(mGetMethod != nullptr)
	{
		MonoMethodSignature* signature = mono_method_signature(mGetMethod);

		MonoType* returnType = mono_signature_get_return_type(signature);
		if(returnType != nullptr)
		{
			::MonoClass* returnClass = mono_class_from_mono_type(returnType);
			if(returnClass != nullptr)
				mReturnType = MonoManager::Instance().FindClass(returnClass);
		}

		u32 numParams = mono_signature_get_param_count(signature);
		mIsIndexed = numParams == 1;
	}
	else if(mSetMethod != nullptr)
	{
		MonoMethodSignature* signature = mono_method_signature(mSetMethod);

		MonoType* returnType = mono_signature_get_return_type(signature);
		if(returnType != nullptr)
		{
			::MonoClass* returnClass = mono_class_from_mono_type(returnType);
			if(returnClass != nullptr)
				mReturnType = MonoManager::Instance().FindClass(returnClass);
		}

		u32 numParams = mono_signature_get_param_count(signature);
		mIsIndexed = numParams == 2;
	}

	mIsFullyInitialized = true;
}
} // namespace b3d
