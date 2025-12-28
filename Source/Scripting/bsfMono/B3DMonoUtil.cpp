//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMonoUtil.h"
#include "Debug/B3DDebug.h"
#include "String/B3DUnicode.h"
#include "B3DMonoAssembly.h"
#include "B3DMonoClass.h"
#include "B3DMonoProperty.h"

#if B3D_USE_DOTNETCORE
#include "B3DMonoLoader.h"
#else
#include <mono/jit/jit.h>
#include <mono/metadata/appdomain.h>
#include <mono/metadata/object.h>
#include <mono/metadata/class.h>
#include <mono/metadata/reflection.h>
#include <mono/metadata/mono-debug.h>
#endif

using namespace b3d;
static bool sGenericHelpersInitialized = false;
static b3d::MonoProperty* sGenericParamsProp = nullptr;

WString MonoUtil::MonoToWString(MonoString* str)
{
	if(str == nullptr)
		return StringUtil::kWblank;

	int len = mono_string_length(str);
	mono_unichar2* monoChars = mono_string_chars(str);

	WString ret(len, '0');
	for(int i = 0; i < len; i++)
		ret[i] = monoChars[i];

	return ret;
}

String MonoUtil::MonoToString(MonoString* str)
{
	WString wideString = MonoToWString(str);

	return UTF8::FromWide(wideString);
}

MonoString* MonoUtil::WstringToMono(const WString& str)
{
	return WstringToMono(str.c_str());
}

MonoString* MonoUtil::WstringToMono(const wchar_t* string)
{
	if(sizeof(wchar_t) == 2) // Assuming UTF-16
		return mono_string_from_utf16((mono_unichar2*)string);
	else // Assuming UTF-32
		return mono_string_from_utf32((mono_unichar4*)string);
}

MonoString* MonoUtil::StringToMono(const String& str)
{
	return WstringToMono(UTF8::ToWide(str));
}

MonoString* MonoUtil::StringToMono(const char* string)
{
	if(string == nullptr)
		return nullptr;

	return WstringToMono(UTF8::ToWide(string));
}

void MonoUtil::GetClassName(MonoObject* obj, String& ns, String& typeName)
{
	if(obj == nullptr)
		return;

	::MonoClass* monoClass = mono_object_get_class(obj);
	GetClassName(monoClass, ns, typeName);
}

void MonoUtil::GetClassName(::MonoClass* monoClass, String& ns, String& typeName)
{
	::MonoClass* nestingClass = mono_class_get_nesting_type(monoClass);

	if(nestingClass == nullptr)
	{
		ns = mono_class_get_namespace(monoClass);
		typeName = mono_class_get_name(monoClass);

		return;
	}
	else
	{
		const char* className = mono_class_get_name(monoClass);

		// Class name is generally never null, except for the case of specialized generic types, which we handle
		// separately
		if(className)
			typeName = String("+") + className;

		do
		{
			::MonoClass* nextNestingClass = mono_class_get_nesting_type(nestingClass);
			if(nextNestingClass != nullptr)
			{
				typeName = String("+") + mono_class_get_name(nestingClass) + typeName;
				nestingClass = nextNestingClass;
			}
			else
			{
				ns = mono_class_get_namespace(nestingClass);
				typeName = mono_class_get_name(nestingClass) + typeName;

				break;
			}
		}
		while(true);
	}
}

void MonoUtil::GetClassName(MonoReflectionType* monoReflType, String& ns, String& typeName)
{
	MonoType* monoType = mono_reflection_type_get_type(monoReflType);
	::MonoClass* monoClass = mono_class_from_mono_type(monoType);

	GetClassName(monoClass, ns, typeName);
}

::MonoClass* MonoUtil::GetClass(MonoObject* object)
{
	return mono_object_get_class(object);
}

::MonoClass* MonoUtil::GetClass(MonoReflectionType* type)
{
	MonoType* monoType = mono_reflection_type_get_type(type);
	return mono_class_from_mono_type(monoType);
}

MonoReflectionType* MonoUtil::GetType(MonoObject* object)
{
	::MonoClass* klass = GetClass(object);
	return GetType(klass);
}

MonoReflectionType* MonoUtil::GetType(::MonoClass* klass)
{
	MonoType* monoType = mono_class_get_type(klass);
	return mono_type_get_object(MonoManager::Instance().GetDomain(), monoType);
}

u32 MonoUtil::NewGcHandle(MonoObject* object, bool pinned)
{
	return mono_gchandle_new(object, pinned);
}

u32 MonoUtil::NewWeakGcHandle(MonoObject* object)
{
	return mono_gchandle_new_weakref(object, false);
}

void MonoUtil::FreeGcHandle(u32 handle)
{
	B3D_ASSERT(handle != 0);

	mono_gchandle_free(handle);
}

MonoObject* MonoUtil::GetObjectFromGcHandle(u32 handle)
{
	return mono_gchandle_get_target(handle);
}

MonoObject* MonoUtil::Box(::MonoClass* klass, void* value)
{
	return mono_value_box(MonoManager::Instance().GetDomain(), klass, value);
}

void* MonoUtil::Unbox(MonoObject* object)
{
	return mono_object_unbox(object);
}

void MonoUtil::ValueCopy(void* dest, void* src, ::MonoClass* klass)
{
	mono_value_copy(dest, src, klass);
}

void MonoUtil::ReferenceCopy(void* dest, MonoObject* object)
{
	mono_gc_wbarrier_generic_store(dest, object);
}

bool MonoUtil::IsSubClassOf(::MonoClass* subClass, ::MonoClass* parentClass)
{
	return mono_class_is_subclass_of(subClass, parentClass, true) != 0;
}

bool MonoUtil::IsValueType(::MonoClass* klass)
{
	return mono_class_is_valuetype(klass) != 0;
}

bool MonoUtil::IsEnum(::MonoClass* object)
{
	return mono_class_is_enum(object) != 0;
}

MonoPrimitiveType MonoUtil::GetEnumPrimitiveType(::MonoClass* enumClass)
{
	MonoType* monoType = mono_class_get_type(enumClass);
	MonoType* underlyingType = mono_type_get_underlying_type(monoType);

	return GetPrimitiveType(mono_class_from_mono_type(underlyingType));
}

MonoPrimitiveType MonoUtil::GetPrimitiveType(::MonoClass* monoClass)
{
	MonoType* monoType = mono_class_get_type(monoClass);
	int monoPrimitiveType = mono_type_get_type(monoType);

	switch(monoPrimitiveType)
	{
	case MONO_TYPE_BOOLEAN:
		return MonoPrimitiveType::Boolean;
	case MONO_TYPE_CHAR:
		return MonoPrimitiveType::Char;
	case MONO_TYPE_I1:
		return MonoPrimitiveType::I8;
	case MONO_TYPE_U1:
		return MonoPrimitiveType::U8;
	case MONO_TYPE_I2:
		return MonoPrimitiveType::I16;
	case MONO_TYPE_U2:
		return MonoPrimitiveType::U16;
	case MONO_TYPE_I4:
		return MonoPrimitiveType::I32;
	case MONO_TYPE_U4:
		return MonoPrimitiveType::U32;
	case MONO_TYPE_I8:
		return MonoPrimitiveType::I64;
	case MONO_TYPE_U8:
		return MonoPrimitiveType::U64;
	case MONO_TYPE_R4:
		return MonoPrimitiveType::R32;
	case MONO_TYPE_R8:
		return MonoPrimitiveType::R64;
	case MONO_TYPE_STRING:
		return MonoPrimitiveType::String;
	case MONO_TYPE_CLASS:
		return MonoPrimitiveType::Class;
	case MONO_TYPE_VALUETYPE:
		return MonoPrimitiveType::ValueType;
	case MONO_TYPE_ARRAY:
	case MONO_TYPE_SZARRAY:
		return MonoPrimitiveType::Array;
	case MONO_TYPE_GENERICINST:
		return MonoPrimitiveType::Generic;
	default:
		break;
	}

	return MonoPrimitiveType::Unknown;
}

::MonoClass* MonoUtil::GetPrimitiveTypeClass(MonoPrimitiveType primitiveType)
{
	switch(primitiveType)
	{
	case MonoPrimitiveType::Boolean: return GetBoolClass();
	case MonoPrimitiveType::Char: return GetCharClass();
	case MonoPrimitiveType::I8: return GetSByteClass();
	case MonoPrimitiveType::U8: return GetByteClass();
	case MonoPrimitiveType::I16: return GetInt16Class();
	case MonoPrimitiveType::U16: return GetUint16Class();
	case MonoPrimitiveType::I32: return GetInt32Class();
	case MonoPrimitiveType::U32: return GetUint32Class();
	case MonoPrimitiveType::I64: return GetInt64Class();
	case MonoPrimitiveType::U64: return GetUint64Class();
	case MonoPrimitiveType::R32: return GetFloatClass();
	case MonoPrimitiveType::R64: return GetDoubleClass();
	case MonoPrimitiveType::String: return GetStringClass();
	}

	return nullptr;
}

::MonoClass* MonoUtil::GetPrimitiveTypeClass(const String& typeName)
{
	static const UnorderedMap<String, MonoPrimitiveType> kLookup = []()
	{
		UnorderedMap<String, MonoPrimitiveType> lookup;
		lookup["byte"] = MonoPrimitiveType::I8;
		lookup["Byte"] = MonoPrimitiveType::I8;
		lookup["sbyte"] = MonoPrimitiveType::U8;
		lookup["SByte"] = MonoPrimitiveType::U8;
		lookup["short"] = MonoPrimitiveType::I16;
		lookup["Int16"] = MonoPrimitiveType::I16;
		lookup["ushort"] = MonoPrimitiveType::U16;
		lookup["UInt16"] = MonoPrimitiveType::U16;
		lookup["int"] = MonoPrimitiveType::I32;
		lookup["Int32"] = MonoPrimitiveType::I32;
		lookup["uint"] = MonoPrimitiveType::U32;
		lookup["UInt32"] = MonoPrimitiveType::U32;
		lookup["long"] = MonoPrimitiveType::I32;
		lookup["Int64"] = MonoPrimitiveType::I32;
		lookup["ulong"] = MonoPrimitiveType::U64;
		lookup["UInt64"] = MonoPrimitiveType::U64;
		lookup["float"] = MonoPrimitiveType::R32;
		lookup["Single"] = MonoPrimitiveType::R32;
		lookup["double"] = MonoPrimitiveType::R64;
		lookup["Double"] = MonoPrimitiveType::R64;
		lookup["char"] = MonoPrimitiveType::Char;
		lookup["Char"] = MonoPrimitiveType::Char;
		lookup["bool"] = MonoPrimitiveType::Boolean;
		lookup["Boolean"] = MonoPrimitiveType::Boolean;

		return lookup;
	}();

	auto found = kLookup.find(typeName);
	if(found != kLookup.end())
		return GetPrimitiveTypeClass(found->second);

	return nullptr;
}

::MonoClass* MonoUtil::BindGenericParameters(::MonoClass* klass, ::MonoClass** params, u32 numParams)
{
	auto types = StackMemory<MonoType*[]>(numParams);

	for(u32 i = 0; i < numParams; i++)
		types[i] = mono_class_get_type(params[i]);

	return mono_class_bind_generic_parameters(klass, numParams, types, false);
}

void MonoUtil::GetGenericParameters(::MonoClass* klass, ::MonoClass** params, u32& numParams)
{
	MonoType* monoType = mono_class_get_type(klass);
	GetGenericParameters(mono_type_get_object(MonoManager::Instance().GetDomain(), monoType), params, numParams);
}

void MonoUtil::GetGenericParameters(::MonoReflectionType* type, ::MonoClass** params, u32& numParams)
{
	if(!sGenericHelpersInitialized)
	{
		MonoAssembly* corlib = MonoManager::Instance().GetAssembly("corlib");
		MonoClass* type = corlib->GetClass("System", "Type");
		sGenericParamsProp = type->GetProperty("GenericTypeArguments");

		sGenericHelpersInitialized = true;
	}

	MonoArray* array = (MonoArray*)sGenericParamsProp->Get((MonoObject*)type);
	if(array)
	{
		ScriptArray scriptArray(array);

		numParams = scriptArray.Size();

		if(params)
		{
			for(u32 i = 0; i < numParams; i++)
			{
				MonoReflectionType* paramType = scriptArray.Get<MonoReflectionType*>(i);
				if(paramType)
					params[i] = GetClass(paramType);
			}
		}
	}
	else
	{
		numParams = 0;

		if(params)
			*params = nullptr;
	}
}

::MonoClass* MonoUtil::GetUint16Class()
{
	return mono_get_uint16_class();
}

::MonoClass* MonoUtil::GetInt16Class()
{
	return mono_get_int16_class();
}

::MonoClass* MonoUtil::GetUint32Class()
{
	return mono_get_uint32_class();
}

::MonoClass* MonoUtil::GetInt32Class()
{
	return mono_get_int32_class();
}

::MonoClass* MonoUtil::GetUint64Class()
{
	return mono_get_uint64_class();
}

::MonoClass* MonoUtil::GetInt64Class()
{
	return mono_get_int64_class();
}

::MonoClass* MonoUtil::GetStringClass()
{
	return mono_get_string_class();
}

::MonoClass* MonoUtil::GetFloatClass()
{
	return mono_get_single_class();
}

::MonoClass* MonoUtil::GetDoubleClass()
{
	return mono_get_double_class();
}

::MonoClass* MonoUtil::GetBoolClass()
{
	return mono_get_boolean_class();
}

::MonoClass* MonoUtil::GetByteClass()
{
	return mono_get_byte_class();
}

::MonoClass* MonoUtil::GetSByteClass()
{
	return mono_get_sbyte_class();
}

::MonoClass* MonoUtil::GetCharClass()
{
	return mono_get_char_class();
}

::MonoClass* MonoUtil::GetObjectClass()
{
	return mono_get_object_class();
}

void MonoUtil::ThrowIfException(MonoException* exception)
{
	ThrowIfException(reinterpret_cast<MonoObject*>(exception));
}

void MonoUtil::ThrowIfException(MonoObject* exception)
{
	if(exception != nullptr)
	{
		::MonoClass* exceptionClass = mono_object_get_class(exception);
		::MonoProperty* exceptionMsgProp = mono_class_get_property_from_name(exceptionClass, "Message");
		::MonoMethod* exceptionMsgGetter = mono_property_get_get_method(exceptionMsgProp);
		MonoString* exceptionMsg = (MonoString*)mono_runtime_invoke(exceptionMsgGetter, exception, nullptr, nullptr);

		::MonoProperty* exceptionStackProp = mono_class_get_property_from_name(exceptionClass, "StackTrace");
		::MonoMethod* exceptionStackGetter = mono_property_get_get_method(exceptionStackProp);
		MonoString* exceptionStackTrace = (MonoString*)mono_runtime_invoke(exceptionStackGetter, exception, nullptr, nullptr);

		// Note: If you modify this format make sure to also modify Debug.ParseExceptionMessage in managed code.
		String msg = "Managed exception: " + MonoToString(exceptionMsg) + "\n" + MonoToString(exceptionStackTrace);

		B3D_LOG(Error, LogScript, "{0}", msg);
	}
}
