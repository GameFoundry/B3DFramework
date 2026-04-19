//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMonoArray.h"
#include "B3DMonoManager.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"

#if B3D_USE_DOTNETCORE
#include "B3DMonoLoader.h"
#else
#include <mono/jit/jit.h>
#include <mono/metadata/object.h>
#include <mono/metadata/class.h>
#endif

namespace b3d { namespace Detail {
template <>
String ScriptArrayGet<String>(MonoArray* array, u32 idx)
{
	return MonoUtil::MonoToString(mono_array_get(array, MonoString*, idx));
}

template <>
WString ScriptArrayGet<WString>(MonoArray* array, u32 idx)
{
	return MonoUtil::MonoToWString(mono_array_get(array, MonoString*, idx));
}

template <>
Path ScriptArrayGet<Path>(MonoArray* array, u32 idx)
{
	return MonoUtil::MonoToString(mono_array_get(array, MonoString*, idx));
}

template <>
void ScriptArraySet<String>(MonoArray* array, u32 idx, const String& value)
{
	MonoString* monoString = MonoUtil::StringToMono(value);
	mono_array_setref(array, idx, monoString);
}

template <>
void ScriptArraySet<WString>(MonoArray* array, u32 idx, const WString& value)
{
	MonoString* monoString = MonoUtil::WstringToMono(value);
	mono_array_setref(array, idx, monoString);
}

template <>
void ScriptArraySet<Path>(MonoArray* array, u32 idx, const Path& value)
{
	MonoString* monoString = MonoUtil::StringToMono(value.ToString());
	mono_array_setref(array, idx, monoString);
}

template <>
void ScriptArraySet<std::nullptr_t>(MonoArray* array, u32 idx, const std::nullptr_t& value)
{
	void** item = (void**)ScriptArray::GetArrayAddrInternal(array, sizeof(void*), idx);
	*item = nullptr;
}
}} // namespace b3d::Detail

using namespace b3d;

ScriptArray::ScriptArray(MonoArray* existingArray)
	: mInternal(existingArray)
{
}

ScriptArray::ScriptArray(MonoClass& klass, u32 size)
	: mInternal(nullptr)
{
	mInternal = mono_array_new(MonoManager::Instance().GetDomain(), klass.GetInternalClass(), size);
}

ScriptArray::ScriptArray(::MonoClass* klass, u32 size)
	: mInternal(nullptr)
{
	mInternal = mono_array_new(MonoManager::Instance().GetDomain(), klass, size);
}

u32 ScriptArray::Size() const
{
	return (u32)mono_array_length(mInternal);
}

u32 ScriptArray::ElementSize() const
{
	::MonoClass* arrayClass = mono_object_get_class((MonoObject*)(mInternal));
	::MonoClass* elementClass = mono_class_get_element_class(arrayClass);

	return (u32)mono_class_array_element_size(elementClass);
}

void ScriptArray::SetRaw(u32 idx, const u8* value, u32 size, u32 count)
{
	SetArrayValInternal(mInternal, idx, value, size, count);
}

u8* ScriptArray::GetArrayAddrInternal(MonoArray* array, u32 size, u32 idx)
{
	return (u8*)mono_array_addr_with_size(array, size, idx);
}

void ScriptArray::SetArrayValInternal(MonoArray* array, u32 idx, const u8* value, u32 size, u32 count)
{
	::MonoClass* arrayClass = mono_object_get_class((MonoObject*)(array));
	::MonoClass* elementClass = mono_class_get_element_class(arrayClass);

	B3D_ASSERT((u32)mono_class_array_element_size(elementClass) == size);
	B3D_ASSERT((idx + count) <= mono_array_length(array));

	if(mono_class_is_valuetype(elementClass))
		mono_value_copy_array(array, idx, (void*)value, count);
	else
	{
		u8* dest = GetArrayAddrInternal(array, size, idx);
		mono_gc_wbarrier_arrayref_copy(dest, (void*)value, count);
	}
}

::MonoClass* ScriptArray::GetElementClass(::MonoClass* arrayClass)
{
	return mono_class_get_element_class(arrayClass);
}

u32 ScriptArray::GetRank(::MonoClass* arrayClass)
{
	return (u32)mono_class_get_rank(arrayClass);
}

::MonoClass* ScriptArray::BuildArrayClass(::MonoClass* elementClass, u32 rank)
{
	return mono_array_class_get(elementClass, rank);
}
