//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsMonoArray.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include <mono/jit/jit.h>

namespace bs
{
	namespace Detail
	{
		template<>
		String ScriptArray_get<String>(MonoArray* array, UINT32 idx)
		{
			return MonoUtil::monoToString(mono_array_get(array, MonoString*, idx));
		}

		template<>
		WString ScriptArray_get<WString>(MonoArray* array, UINT32 idx)
		{
			return MonoUtil::monoToWString(mono_array_get(array, MonoString*, idx));
		}

		template<>
		void ScriptArray_set<String>(MonoArray* array, UINT32 idx, const String& value)
		{
			MonoString* monoString = MonoUtil::stringToMono(value);
			mono_array_setref(array, idx, monoString);
		}

		template<>
		void ScriptArray_set<WString>(MonoArray* array, UINT32 idx, const WString& value)
		{
			MonoString* monoString = MonoUtil::wstringToMono(value);
			mono_array_setref(array, idx, monoString);
		}

		template<>
		void ScriptArray_set<std::nullptr_t>(MonoArray* array, UINT32 idx, const std::nullptr_t& value)
		{
			void** item = (void**)ScriptArray::_getArrayAddr(array, sizeof(void*), idx);
			*item = nullptr;
		}

		template String ScriptArray_get(MonoArray* array, UINT32 idx);
		template WString ScriptArray_get(MonoArray* array, UINT32 idx);

		template void ScriptArray_set(MonoArray* array, UINT32 idx, const String& value);
		template void ScriptArray_set(MonoArray* array, UINT32 idx, const WString& value);
	}

	ScriptArray::ScriptArray(MonoArray* existingArray)
		:mInternal(existingArray)
	{
	}

	ScriptArray::ScriptArray(MonoClass& klass, UINT32 size)
		: mInternal(nullptr)
	{
		mInternal = mono_array_new(MonoManager::instance().getDomain(), klass._getInternalClass(), size);
	}

	ScriptArray::ScriptArray(::MonoClass* klass, UINT32 size)
		: mInternal(nullptr)
	{
		mInternal = mono_array_new(MonoManager::instance().getDomain(), klass, size);
	}

	UINT32 ScriptArray::size() const
	{
		return (UINT32)mono_array_length(mInternal);
	}

	UINT32 ScriptArray::elementSize() const
	{
		::MonoClass* arrayClass = mono_object_get_class((MonoObject*)(mInternal));
		::MonoClass* elementClass = mono_class_get_element_class(arrayClass);

		return (UINT32)mono_class_array_element_size(elementClass);
	}

	void ScriptArray::setRaw(UINT32 idx, const UINT8* value, UINT32 size, UINT32 count)
	{
		_setArrayVal(mInternal, idx, value, size, count);
	}

	UINT8* ScriptArray::_getArrayAddr(MonoArray* array, UINT32 size, UINT32 idx)
	{
		return (UINT8*)mono_array_addr_with_size(array, size, idx);
	}

	void ScriptArray::_setArrayVal(MonoArray* array, UINT32 idx, const UINT8* value, UINT32 size, UINT32 count)
	{
		::MonoClass* arrayClass = mono_object_get_class((MonoObject*)(array));
		::MonoClass* elementClass = mono_class_get_element_class(arrayClass);

		BS_ASSERT((UINT32)mono_class_array_element_size(elementClass) == size);
		BS_ASSERT((idx + count) <= mono_array_length(array));

		if(mono_class_is_valuetype(elementClass))
			mono_value_copy_array(array, idx, (void*)value, count);
		else
		{
			UINT8* dest = _getArrayAddr(array, size, idx);
			mono_gc_wbarrier_arrayref_copy(dest, (void*)value, count);
		}
	}

	::MonoClass* ScriptArray::getElementClass(::MonoClass* arrayClass)
	{
		return mono_class_get_element_class(arrayClass);
	}

	UINT32 ScriptArray::getRank(::MonoClass* arrayClass)
	{
		return (UINT32)mono_class_get_rank(arrayClass);
	}

	::MonoClass* ScriptArray::buildArrayClass(::MonoClass* elementClass, UINT32 rank)
	{
		return mono_array_class_get(elementClass, rank);
	}
}