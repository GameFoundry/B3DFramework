//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsMonoMethod.h"
#include "BsMonoManager.h"
#include "BsMonoUtil.h"
#include "BsMonoClass.h"
#include "Error/BsException.h"
#include <mono/jit/jit.h>
#include <mono/metadata/attrdefs.h>

namespace bs
{
	MonoMethod::MonoMethod(::MonoMethod* method)
		:mMethod(method), mCachedReturnType(nullptr), mCachedParameters(nullptr),
		mCachedNumParameters(0), mIsStatic(false), mHasCachedSignature(false)
	{

	}

	MonoMethod::~MonoMethod()
	{
		if (mCachedParameters != nullptr)
			bs_free(mCachedParameters);
	}

	MonoObject* MonoMethod::invoke(MonoObject* instance, void** params)
	{
		MonoObject* exception = nullptr;
		MonoObject* retVal = mono_runtime_invoke(mMethod, instance, params, &exception);

		MonoUtil::throwIfException(exception);
		return retVal;
	}		

	MonoObject* MonoMethod::invokeVirtual(MonoObject* instance, void** params)
	{
		::MonoMethod* virtualMethod = mono_object_get_virtual_method(instance, mMethod);

		MonoObject* exception = nullptr;
		MonoObject* retVal = mono_runtime_invoke(virtualMethod, instance, params, &exception);

		MonoUtil::throwIfException(exception);
		return retVal;
	}		

	void* MonoMethod::getThunk() const
	{
		return mono_method_get_unmanaged_thunk(mMethod);
	}

	String MonoMethod::GetName() const
	{
		return String(mono_method_get_name(mMethod));
	}

	MonoClass* MonoMethod::getReturnType() const
	{
		if (!mHasCachedSignature)
			cacheSignature();

		return mCachedReturnType;
	}

	UINT32 MonoMethod::GetNumParameters() const
	{
		if (!mHasCachedSignature)
			cacheSignature();

		return mCachedNumParameters;
	}

	MonoClass* MonoMethod::getParameterType(UINT32 paramIdx) const
	{
		if (!mHasCachedSignature)
			cacheSignature();

		if (paramIdx >= mCachedNumParameters)
			BS_EXCEPT(InvalidParametersException, "Parameter index out of range. Valid range is [0, " +
					ToString(mCachedNumParameters - 1) + "]");

		return mCachedParameters[paramIdx];
	}

	bool MonoMethod::IsStatic() const
	{
		if (!mHasCachedSignature)
			cacheSignature();

		return mIsStatic;
	}

	bool MonoMethod::HasAttribute(MonoClass* monoClass) const
	{
		// TODO - Consider caching custom attributes or just initializing them all at load

		MonoCustomAttrInfo* attrInfo = mono_custom_attrs_from_method(mMethod);
		if (attrInfo == nullptr)
			return false;

		bool hasAttr = mono_custom_attrs_has_attr(attrInfo, monoClass->_getInternalClass()) != 0;

		mono_custom_attrs_free(attrInfo);

		return hasAttr;
	}

	MonoObject* MonoMethod::getAttribute(MonoClass* monoClass) const
	{
		// TODO - Consider caching custom attributes or just initializing them all at load

		MonoCustomAttrInfo* attrInfo = mono_custom_attrs_from_method(mMethod);
		if (attrInfo == nullptr)
			return nullptr;

		MonoObject* foundAttr = nullptr;
		if(mono_custom_attrs_has_attr(attrInfo, monoClass->_getInternalClass()))
			foundAttr = mono_custom_attrs_get_attr(attrInfo, monoClass->_getInternalClass());

		mono_custom_attrs_free(attrInfo);
		return foundAttr;
	}

	MonoMemberVisibility MonoMethod::GetVisibility()
	{
		uint32_t flags = mono_method_get_flags(mMethod, nullptr) & MONO_METHOD_ATTR_ACCESS_MASK;

		if (flags == MONO_METHOD_ATTR_PRIVATE)
			return MonoMemberVisibility::Private;
		else if (flags == MONO_METHOD_ATTR_FAM_AND_ASSEM)
			return MonoMemberVisibility::ProtectedInternal;
		else if (flags == MONO_METHOD_ATTR_ASSEM)
			return MonoMemberVisibility::Internal;
		else if (flags == MONO_METHOD_ATTR_FAMILY)
			return MonoMemberVisibility::Protected;
		else if (flags == MONO_METHOD_ATTR_PUBLIC)
			return MonoMemberVisibility::Public;

		assert(false);
		return MonoMemberVisibility::Private;
	}

	void MonoMethod::CacheSignature() const
	{
		MonoMethodSignature* methodSignature = mono_method_signature(mMethod);

		MonoType* returnType = mono_signature_get_return_type(methodSignature);
		if (returnType != nullptr)
		{
			::MonoClass* returnClass = mono_class_from_mono_type(returnType);
			if (returnClass != nullptr)
				mCachedReturnType = MonoManager::instance().FindClass(returnClass);
		}

		mCachedNumParameters = (UINT32)mono_signature_get_param_count(methodSignature);
		if (mCachedParameters != nullptr)
		{
			bs_free(mCachedParameters);
			mCachedParameters = nullptr;
		}

		if (mCachedNumParameters > 0)
		{
			mCachedParameters = (MonoClass**)bs_alloc(mCachedNumParameters * sizeof(MonoClass*));

			void* iter = nullptr;
			for (UINT32 i = 0; i < mCachedNumParameters; i++)
			{
				MonoType* curParamType = mono_signature_get_params(methodSignature, &iter);
				::MonoClass* rawClass = mono_class_from_mono_type(curParamType);
				mCachedParameters[i] = MonoManager::instance().FindClass(rawClass);
			}
		}

		mIsStatic = !mono_signature_is_instance(methodSignature);
		mHasCachedSignature = true;
	}
}
