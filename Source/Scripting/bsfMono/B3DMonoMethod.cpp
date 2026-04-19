//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMonoMethod.h"
#include "B3DMonoManager.h"
#include "B3DMonoUtil.h"
#include "B3DMonoClass.h"

#if B3D_USE_DOTNETCORE
#include "B3DMonoLoader.h"
#else
#include <mono/jit/jit.h>
#include <mono/metadata/attrdefs.h>
#include <mono/metadata/object.h>
#include <mono/metadata/class.h>
#include <mono/metadata/reflection.h>
#endif

namespace b3d {
MonoMethod::MonoMethod(::MonoMethod* method)
	: mMethod(method), mCachedReturnType(nullptr), mCachedParameters(nullptr), mCachedNumParameters(0), mIsStatic(false), mHasCachedSignature(false)
{
}

MonoMethod::~MonoMethod()
{
	if(mCachedParameters != nullptr)
		B3DFree(mCachedParameters);
}

MonoObject* MonoMethod::Invoke(MonoObject* instance, void** params)
{
	MonoObject* exception = nullptr;
	MonoObject* retVal = mono_runtime_invoke(mMethod, instance, params, &exception);

	MonoUtil::ThrowIfException(exception);
	return retVal;
}

MonoObject* MonoMethod::InvokeVirtual(MonoObject* instance, void** params)
{
	::MonoMethod* virtualMethod = mono_object_get_virtual_method(instance, mMethod);

	MonoObject* exception = nullptr;
	MonoObject* retVal = mono_runtime_invoke(virtualMethod, instance, params, &exception);

	MonoUtil::ThrowIfException(exception);
	return retVal;
}

void* MonoMethod::GetThunk() const
{
	return mono_method_get_unmanaged_thunk(mMethod);
}

String MonoMethod::GetName() const
{
	return String(mono_method_get_name(mMethod));
}

MonoClass* MonoMethod::GetReturnType() const
{
	if(!mHasCachedSignature)
		CacheSignature();

	return mCachedReturnType;
}

u32 MonoMethod::GetNumParameters() const
{
	if(!mHasCachedSignature)
		CacheSignature();

	return mCachedNumParameters;
}

MonoClass* MonoMethod::GetParameterType(u32 paramIdx) const
{
	if(!mHasCachedSignature)
		CacheSignature();

	if(!B3D_ENSURE_LOG(paramIdx < mCachedNumParameters, "Parameter index out of range. Valid range is [0, {0}]", mCachedNumParameters - 1))
		return nullptr;

	return mCachedParameters[paramIdx];
}

bool MonoMethod::IsStatic() const
{
	if(!mHasCachedSignature)
		CacheSignature();

	return mIsStatic;
}

bool MonoMethod::HasAttribute(MonoClass* monoClass) const
{
	// TODO - Consider caching custom attributes or just initializing them all at load

	MonoCustomAttrInfo* attrInfo = mono_custom_attrs_from_method(mMethod);
	if(attrInfo == nullptr)
		return false;

	bool hasAttr = mono_custom_attrs_has_attr(attrInfo, monoClass->GetInternalClass()) != 0;

	mono_custom_attrs_free(attrInfo);

	return hasAttr;
}

MonoObject* MonoMethod::GetAttribute(MonoClass* monoClass) const
{
	// TODO - Consider caching custom attributes or just initializing them all at load

	MonoCustomAttrInfo* attrInfo = mono_custom_attrs_from_method(mMethod);
	if(attrInfo == nullptr)
		return nullptr;

	MonoObject* foundAttr = nullptr;
	if(mono_custom_attrs_has_attr(attrInfo, monoClass->GetInternalClass()))
		foundAttr = mono_custom_attrs_get_attr(attrInfo, monoClass->GetInternalClass());

	mono_custom_attrs_free(attrInfo);
	return foundAttr;
}

MonoMemberVisibility MonoMethod::GetVisibility()
{
	uint32_t flags = mono_method_get_flags(mMethod, nullptr) & MONO_METHOD_ATTR_ACCESS_MASK;

	if(flags == MONO_METHOD_ATTR_PRIVATE)
		return MonoMemberVisibility::Private;
	else if(flags == MONO_METHOD_ATTR_FAM_AND_ASSEM)
		return MonoMemberVisibility::ProtectedInternal;
	else if(flags == MONO_METHOD_ATTR_ASSEM)
		return MonoMemberVisibility::Internal;
	else if(flags == MONO_METHOD_ATTR_FAMILY)
		return MonoMemberVisibility::Protected;
	else if(flags == MONO_METHOD_ATTR_PUBLIC)
		return MonoMemberVisibility::Public;

	B3D_ASSERT(false);
	return MonoMemberVisibility::Private;
}

void MonoMethod::CacheSignature() const
{
	MonoMethodSignature* methodSignature = mono_method_signature(mMethod);

	MonoType* returnType = mono_signature_get_return_type(methodSignature);
	if(returnType != nullptr)
	{
		::MonoClass* returnClass = mono_class_from_mono_type(returnType);
		if(returnClass != nullptr)
			mCachedReturnType = MonoManager::Instance().FindClass(returnClass);
	}

	mCachedNumParameters = (u32)mono_signature_get_param_count(methodSignature);
	if(mCachedParameters != nullptr)
	{
		B3DFree(mCachedParameters);
		mCachedParameters = nullptr;
	}

	if(mCachedNumParameters > 0)
	{
		mCachedParameters = (MonoClass**)B3DAllocate(mCachedNumParameters * sizeof(MonoClass*));

		void* iter = nullptr;
		for(u32 i = 0; i < mCachedNumParameters; i++)
		{
			MonoType* curParamType = mono_signature_get_params(methodSignature, &iter);
			::MonoClass* rawClass = mono_class_from_mono_type(curParamType);
			mCachedParameters[i] = MonoManager::Instance().FindClass(rawClass);
		}
	}

	mIsStatic = !mono_signature_is_instance(methodSignature);
	mHasCachedSignature = true;
}
} // namespace b3d
