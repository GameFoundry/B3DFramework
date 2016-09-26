//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsMonoProperty.h"
#include "BsMonoMethod.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include <mono/jit/jit.h>
#include <mono/metadata/class.h>

namespace BansheeEngine
{
	MonoProperty::MonoProperty(::MonoProperty* monoProp)
		:mProperty(monoProp), mReturnType(nullptr), mIsIndexed(false), mIsFullyInitialized(false)
	{
		mGetMethod = mono_property_get_get_method(mProperty);
		mSetMethod = mono_property_get_set_method(mProperty);

		mName = mono_property_get_name(mProperty);
	}

	MonoObject* MonoProperty::get(MonoObject* instance) const
	{
		if (mGetMethod == nullptr)
			return nullptr;

		return mono_runtime_invoke(mGetMethod, instance, nullptr, nullptr);
	}

	void MonoProperty::set(MonoObject* instance, void* value) const
	{
		if (mSetMethod == nullptr)
			return;

		void* args[1];
		args[0] = value;
		mono_runtime_invoke(mSetMethod, instance, args, nullptr);
	}	

	MonoObject* MonoProperty::getIndexed(MonoObject* instance, UINT32 index) const
	{
		void* args[1];
		args[0] = &index;
		return mono_runtime_invoke(mGetMethod, instance, args, nullptr);
	}

	void MonoProperty::setIndexed(MonoObject* instance, UINT32 index, void* value) const
	{
		void* args[2];
		args[0] = &index;
		args[1] = value;
		mono_runtime_invoke(mSetMethod, instance, args, nullptr);
	}

	bool MonoProperty::isIndexed() const
	{
		if (!mIsFullyInitialized)
			initializeDeferred();

		return mIsIndexed;
	}

	MonoClass* MonoProperty::getReturnType() const
	{
		if (!mIsFullyInitialized)
			initializeDeferred();

		return mReturnType;
	}

	bool MonoProperty::hasAttribute(MonoClass* monoClass)
	{
		// TODO - Consider caching custom attributes or just initializing them all at load

		::MonoClass* parentClass = mono_property_get_parent(mProperty);
		MonoCustomAttrInfo* attrInfo = mono_custom_attrs_from_property(parentClass, mProperty);
		if (attrInfo == nullptr)
			return false;

		bool hasAttr = mono_custom_attrs_has_attr(attrInfo, monoClass->_getInternalClass()) != 0;

		mono_custom_attrs_free(attrInfo);

		return hasAttr;
	}

	MonoObject* MonoProperty::getAttribute(MonoClass* monoClass)
	{
		// TODO - Consider caching custom attributes or just initializing them all at load

		::MonoClass* parentClass = mono_property_get_parent(mProperty);
		MonoCustomAttrInfo* attrInfo = mono_custom_attrs_from_property(parentClass, mProperty);
		if (attrInfo == nullptr)
			return nullptr;

		MonoObject* foundAttr = mono_custom_attrs_get_attr(attrInfo, monoClass->_getInternalClass());

		mono_custom_attrs_free(attrInfo);

		return foundAttr;
	}

	MonoMemberVisibility MonoProperty::getVisibility()
	{
		MonoMemberVisibility getterVisibility = MonoMemberVisibility::Public;
		if(mGetMethod)
		{
			MonoMethod getterWrapper(mGetMethod);
			getterVisibility = getterWrapper.getVisibility();
		}

		MonoMemberVisibility setterVisibility = MonoMemberVisibility::Public;
		if (mSetMethod)
		{
			MonoMethod setterWrapper(mSetMethod);
			setterVisibility = setterWrapper.getVisibility();
		}

		if (getterVisibility < setterVisibility)
			return getterVisibility;

		return setterVisibility;
	}

	void MonoProperty::initializeDeferred() const
	{
		if (mGetMethod != nullptr)
		{
			MonoMethodSignature* signature = mono_method_signature(mGetMethod);

			MonoType* returnType = mono_signature_get_return_type(signature);
			if (returnType != nullptr)
			{
				::MonoClass* returnClass = mono_class_from_mono_type(returnType);
				if (returnClass != nullptr)
					mReturnType = MonoManager::instance().findClass(returnClass);
			}

			UINT32 numParams = mono_signature_get_param_count(signature);
			mIsIndexed = numParams == 1;
		}
		else if(mSetMethod != nullptr)
		{
			MonoMethodSignature* signature = mono_method_signature(mSetMethod);

			MonoType* returnType = mono_signature_get_return_type(signature);
			if (returnType != nullptr)
			{
				::MonoClass* returnClass = mono_class_from_mono_type(returnType);
				if (returnClass != nullptr)
					mReturnType = MonoManager::instance().findClass(returnClass);
			}

			UINT32 numParams = mono_signature_get_param_count(signature);
			mIsIndexed = numParams == 2;
		}

		mIsFullyInitialized = true;
	}
}