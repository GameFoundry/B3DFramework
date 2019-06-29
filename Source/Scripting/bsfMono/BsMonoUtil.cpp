//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsMonoUtil.h"
#include "Debug/BsDebug.h"
#include <mono/jit/jit.h>
#include "String/BsUnicode.h"
#include <mono/metadata/mono-debug.h>
#include "BsMonoAssembly.h"
#include "BsMonoClass.h"
#include "BsMonoProperty.h"

namespace bs
{
	static bool sGenericHelpersInitialized = false;
	static MonoProperty* sGenericParamsProp = nullptr;

	WString MonoUtil::monoToWString(MonoString* str)
	{
		if (str == nullptr)
			return StringUtil::WBLANK;

		int len = mono_string_length(str);
		mono_unichar2* monoChars = mono_string_chars(str);

		WString ret(len, '0');
		for (int i = 0; i < len; i++)
			ret[i] = monoChars[i];

		return ret;
	}

	String MonoUtil::monoToString(MonoString* str)
	{
		WString wideString = monoToWString(str);

		return UTF8::fromWide(wideString);
	}

	MonoString* MonoUtil::wstringToMono(const WString& str)
	{
		if (sizeof(wchar_t) == 2) // Assuming UTF-16
			return mono_string_from_utf16((mono_unichar2*)str.c_str());
		else // Assuming UTF-32
			return mono_string_from_utf32((mono_unichar4*)str.c_str());
	}

	MonoString* MonoUtil::stringToMono(const String& str)
	{
		return wstringToMono(UTF8::toWide(str));
	}

	void MonoUtil::getClassName(MonoObject* obj, String& ns, String& typeName)
	{
		if (obj == nullptr)
			return;

		::MonoClass* monoClass = mono_object_get_class(obj);
		getClassName(monoClass, ns, typeName);
	}

	void MonoUtil::getClassName(::MonoClass* monoClass, String& ns, String& typeName)
	{
		::MonoClass* nestingClass = mono_class_get_nesting_type(monoClass);

		if (nestingClass == nullptr)
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
				if (nextNestingClass != nullptr)
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
			} while (true);
		}
	}

	void MonoUtil::getClassName(MonoReflectionType* monoReflType, String& ns, String& typeName)
	{
		MonoType* monoType = mono_reflection_type_get_type(monoReflType);
		::MonoClass* monoClass = mono_class_from_mono_type(monoType);

		getClassName(monoClass, ns, typeName);
	}

	::MonoClass* MonoUtil::getClass(MonoObject* object)
	{
		return mono_object_get_class(object);
	}

	::MonoClass* MonoUtil::getClass(MonoReflectionType* type)
	{
		MonoType* monoType = mono_reflection_type_get_type(type);
		return mono_class_from_mono_type(monoType);
	}

	MonoReflectionType* MonoUtil::getType(MonoObject* object)
	{
		::MonoClass* klass = getClass(object);
		return getType(klass);
	}

	MonoReflectionType* MonoUtil::getType(::MonoClass* klass)
	{
		MonoType* monoType = mono_class_get_type(klass);
		return mono_type_get_object(MonoManager::instance().getDomain(), monoType);
	}

	UINT32 MonoUtil::newGCHandle(MonoObject* object, bool pinned)
	{
		return mono_gchandle_new(object, pinned);
	}

	UINT32 MonoUtil::newWeakGCHandle(MonoObject* object)
	{
		return mono_gchandle_new_weakref(object, false);
	}

	void MonoUtil::freeGCHandle(UINT32 handle)
	{
		assert(handle != 0);

		mono_gchandle_free(handle);
	}

	MonoObject* MonoUtil::getObjectFromGCHandle(UINT32 handle)
	{
		return mono_gchandle_get_target(handle);
	}

	MonoObject* MonoUtil::box(::MonoClass* klass, void* value)
	{
		return mono_value_box(MonoManager::instance().getDomain(), klass, value);
	}

	void* MonoUtil::unbox(MonoObject* object)
	{
		return mono_object_unbox(object);
	}

	void MonoUtil::valueCopy(void* dest, void* src, ::MonoClass* klass)
	{
		mono_value_copy(dest, src, klass);
	}

	void MonoUtil::referenceCopy(void* dest, MonoObject* object)
	{
		mono_gc_wbarrier_generic_store(dest, object);
	}

	bool MonoUtil::isSubClassOf(::MonoClass* subClass, ::MonoClass* parentClass)
	{
		return mono_class_is_subclass_of(subClass, parentClass, true) != 0;
	}

	bool MonoUtil::isValueType(::MonoClass* klass)
	{
		return mono_class_is_valuetype(klass) != 0;
	}

	bool MonoUtil::isEnum(::MonoClass* object)
	{
		return mono_class_is_enum(object) != 0;
	}

	MonoPrimitiveType MonoUtil::getEnumPrimitiveType(::MonoClass* enumClass)
	{
		MonoType* monoType = mono_class_get_type(enumClass);
		MonoType* underlyingType = mono_type_get_underlying_type(monoType);

		return getPrimitiveType(mono_class_from_mono_type(underlyingType));
	}

	MonoPrimitiveType MonoUtil::getPrimitiveType(::MonoClass* monoClass)
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

	::MonoClass* MonoUtil::bindGenericParameters(::MonoClass* klass, ::MonoClass** params, UINT32 numParams)
	{
		auto buffer = bs_managed_stack_alloc<MonoType*>(numParams);

		MonoType** types = buffer;
		for (UINT32 i = 0; i < numParams; i++)
			types[i] = mono_class_get_type(params[i]);

		return mono_class_bind_generic_parameters(klass, numParams, types, false);
	}

	void MonoUtil::getGenericParameters(::MonoClass* klass, ::MonoClass** params, UINT32& numParams)
	{
		MonoType* monoType = mono_class_get_type(klass);
		getGenericParameters(mono_type_get_object(MonoManager::instance().getDomain(), monoType), params, numParams);
	}

	void MonoUtil::getGenericParameters(::MonoReflectionType* type, ::MonoClass** params, UINT32& numParams)
	{
		if(!sGenericHelpersInitialized)
		{
			MonoAssembly* corlib = MonoManager::instance().getAssembly("corlib");
			MonoClass* type = corlib->getClass("System", "Type");
			sGenericParamsProp = type->getProperty("GenericTypeArguments");

			sGenericHelpersInitialized = true;
		}

		MonoArray* array = (MonoArray*)sGenericParamsProp->get((MonoObject*)type);
		if(array)
		{
			ScriptArray scriptArray(array);

			numParams = scriptArray.size();

			if(params)
			{
				for(UINT32 i = 0; i < numParams; i++)
				{
					MonoReflectionType* paramType = scriptArray.get<MonoReflectionType*>(i);
					if(paramType)
						params[i] = getClass(paramType);
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

	::MonoClass* MonoUtil::getUINT16Class()
	{
		return mono_get_uint16_class();
	}

	::MonoClass* MonoUtil::getINT16Class()
	{
		return mono_get_int16_class();
	}

	::MonoClass* MonoUtil::getUINT32Class()
	{
		return mono_get_uint32_class();
	}

	::MonoClass* MonoUtil::getINT32Class()
	{
		return mono_get_int32_class();
	}

	::MonoClass* MonoUtil::getUINT64Class()
	{
		return mono_get_uint64_class();
	}

	::MonoClass* MonoUtil::getINT64Class()
	{
		return mono_get_int64_class();
	}

	::MonoClass* MonoUtil::getStringClass()
	{
		return mono_get_string_class();
	}

	::MonoClass* MonoUtil::getFloatClass()
	{
		return mono_get_single_class();
	}

	::MonoClass* MonoUtil::getDoubleClass()
	{
		return mono_get_double_class();
	}

	::MonoClass* MonoUtil::getBoolClass()
	{
		return mono_get_boolean_class();
	}

	::MonoClass* MonoUtil::getByteClass()
	{
		return mono_get_byte_class();
	}

	::MonoClass* MonoUtil::getSByteClass()
	{
		return mono_get_sbyte_class();
	}

	::MonoClass* MonoUtil::getCharClass()
	{
		return mono_get_char_class();
	}

	::MonoClass* MonoUtil::getObjectClass()
	{
		return mono_get_object_class();
	}

	void MonoUtil::throwIfException(MonoException* exception)
	{
		throwIfException(reinterpret_cast<MonoObject*>(exception));
	}

	void MonoUtil::throwIfException(MonoObject* exception)
	{
		if (exception != nullptr)
		{
			::MonoClass* exceptionClass = mono_object_get_class(exception);
			::MonoProperty* exceptionMsgProp = mono_class_get_property_from_name(exceptionClass, "Message");
			::MonoMethod* exceptionMsgGetter = mono_property_get_get_method(exceptionMsgProp);
			MonoString* exceptionMsg = (MonoString*)mono_runtime_invoke(exceptionMsgGetter, exception, nullptr, nullptr);

			::MonoProperty* exceptionStackProp = mono_class_get_property_from_name(exceptionClass, "StackTrace");
			::MonoMethod* exceptionStackGetter = mono_property_get_get_method(exceptionStackProp);
			MonoString* exceptionStackTrace = (MonoString*)mono_runtime_invoke(exceptionStackGetter, exception, nullptr, nullptr);

			// Note: If you modify this format make sure to also modify Debug.ParseExceptionMessage in managed code.
			String msg = "Managed exception: " + monoToString(exceptionMsg) + "\n" + monoToString(exceptionStackTrace);

			BS_LOG(Error, Script, msg);
		}
	}
}
