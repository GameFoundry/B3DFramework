//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DMonoPrerequisites.h"

namespace b3d
{
	/** @addtogroup Mono
	 *  @{
	 */

	/**	Utility class containing methods for various common Mono/Script related operations. */
	class B3D_MONO_EXPORT MonoUtil
	{
	public:
		/**	Converts a Mono (managed) string to a native wide string. */
		static WString MonoToWString(MonoString* str);

		/**	Converts a Mono (managed) string to a native narrow string. */
		static String MonoToString(MonoString* str);

		/**	Converts a native wide string to a Mono (managed) string. */
		static MonoString* WstringToMono(const WString& str);

		/**	Converts a native wide string to a Mono (managed) string. */
		static MonoString* WstringToMono(const wchar_t* string);

		/**	Converts a native narrow string to a Mono (managed) string. */
		static MonoString* StringToMono(const String& str);

		/**	Converts a native narrow string to a Mono (managed) string. */
		static MonoString* StringToMono(const char* string);

		/**	Outputs name and namespace for the type of the specified object. */
		static void GetClassName(MonoObject* obj, String& ns, String& typeName);

		/**	Outputs name and namespace for the specified type. */
		static void GetClassName(::MonoClass* monoClass, String& ns, String& typeName);

		/**	Outputs name and namespace for the specified type. */
		static void GetClassName(MonoReflectionType* monoReflType, String& ns, String& typeName);

		/** Returns the class of the provided object. */
		static ::MonoClass* GetClass(MonoObject* object);

		/** Returns the class of the provided type. */
		static ::MonoClass* GetClass(MonoReflectionType* type);

		/** Returns the type of the provided object. */
		static MonoReflectionType* GetType(MonoObject* object);

		/** Returns the type of the provided class. */
		static MonoReflectionType* GetType(::MonoClass* klass);

		/**
		 * Creates a new GC handle for the provided managed object. The handle can be stored and later used for retrieving
		 * the MonoObject* related to it by calling getObjectFromGCHandle(). This is a strong handle, meaning it will
		 * prevent the garbage collector from collecting the object until it is released by calling freeGCHandle().
		 *
		 * @param[in]	object		Managed object to create the handle for.
		 * @param[in]	pinned		If true the object will be pinned in memory, meaning you will be allowed to store
		 *							a reference to the MonoObject directly. Never store MonoObject* unless they have been
		 *							previously pinned (instead use getObjectFromGCHandle*( to get the current pointer).
		 *							Note that pinning can have an impact on memory fragmentation as it prevents the GC from
		 *							moving the object, so use it sparingly.
		 */
		static u32 NewGcHandle(MonoObject* object, bool pinned = true);

		/**
		 * Creates a new GC handle for the provided managed object. The handle can be stored and later used for retrieving
		 * the MonoObject* related to it by calling getObjectFromGCHandle(). This is a weak handle, meaning it will NOT
		 * prevent the garbage collector from collecting the object. getObjectFromGCHandle() will return null if the GC
		 * collected the object and handle is no longer valid.
		 */
		static u32 NewWeakGcHandle(MonoObject* object);

		/** Frees a GC handle previously allocated with newGCHandle. */
		static void FreeGcHandle(u32 handle);

		/** Returns a MonoObject from an allocated GC handle. */
		static MonoObject* GetObjectFromGcHandle(u32 handle);

		/** Converts a managed value type into a reference type by boxing it. */
		static MonoObject* Box(::MonoClass* klass, void* value);

		/** Unboxes a managed object back to a raw value type. */
		static void* Unbox(MonoObject* object);

		/**
		 * Copies the value from @p src to @p dest. This must be a value-type of type @p klass. You need to use this
		 * form of copying if @p dest is a struct that gets passed to managed code and it contains a reference type. This
		 * way the GC is informed about the reference in the struct. You can use normal copies otherwise.
		 */
		static void ValueCopy(void* dest, void* src, ::MonoClass* klass);

		/**
		 * Copies the pointer to a reference type @p object to @p dest, ensuring @p dest also points to the object. This
		 * needs to be used if @p dest is being passed to managed code (e.g. an output parameter in a method). Otherwise
		 * normal copies can be used. @p dest must be large enough to hold sizeof(MonoObject*).
		 */
		static void ReferenceCopy(void* dest, MonoObject* object);

		/**	Checks if this class is a sub class of the specified class. */
		static bool IsSubClassOf(::MonoClass* subClass, ::MonoClass* parentClass);

		/** Checks is the specified class a value type. */
		static bool IsValueType(::MonoClass* object);

		/** Checks is the specified class an enum. */
		static bool IsEnum(::MonoClass* object);

		/** Returns the underlying primitive type for an enum. */
		static MonoPrimitiveType GetEnumPrimitiveType(::MonoClass* enumClass);

		/** Returns the primitive type of the provided class. */
		static MonoPrimitiveType GetPrimitiveType(::MonoClass* monoClass);

		/** Returns a corresponding primitive class type based on the provided enum. */
		static ::MonoClass* GetPrimitiveTypeClass(MonoPrimitiveType primitiveType);

		/** Returns a corresponding primitive class type based on the provided name (e.g. float, int, bool, etc.) */
		static ::MonoClass* GetPrimitiveTypeClass(const String& typeName);

		/** Binds parameters to a generic class, and returns a new instantiable class with the bound parameters. */
		static ::MonoClass* BindGenericParameters(::MonoClass* klass, ::MonoClass** params, u32 numParams);

		/**
		 * Returns the generic parameters of the provided type. @p params must be a pre-allocated buffer able to hold the
		 * class types for each parameter. If @p params is null, then @p numParams will be populated with the number of
		 * available parameters.
		 */
		static void GetGenericParameters(::MonoClass* klass, ::MonoClass** params, u32& numParams);

		/**
		 * Returns the generic parameters of the provided type. @p params must be a pre-allocated buffer able to hold the
		 * class types for each parameter. If @p params is null, then @p numParams will be populated with the number of
		 * available parameters.
		 */
		static void GetGenericParameters(::MonoReflectionType* type, ::MonoClass** params, u32& numParams);

		/** Returns Mono class for a 16-bit unsigned integer. */
		static ::MonoClass* GetUint16Class();

		/** Returns Mono class for a 16-bit signed integer. */
		static ::MonoClass* GetInt16Class();

		/** Returns Mono class for a 32-bit unsigned integer. */
		static ::MonoClass* GetUint32Class();

		/** Returns Mono class for a 32-bit signed integer. */
		static ::MonoClass* GetInt32Class();

		/** Returns Mono class for a 64-bit unsigned integer. */
		static ::MonoClass* GetUint64Class();

		/** Returns Mono class for a 32-bit signed integer. */
		static ::MonoClass* GetInt64Class();

		/** Returns Mono class for a string. */
		static ::MonoClass* GetStringClass();

		/** Returns Mono class for a floating point number. */
		static ::MonoClass* GetFloatClass();

		/** Returns Mono class for a double floating point number. */
		static ::MonoClass* GetDoubleClass();

		/** Returns Mono class for a boolean. */
		static ::MonoClass* GetBoolClass();

		/** Returns Mono class for an unsigned byte. */
		static ::MonoClass* GetByteClass();

		/** Returns Mono class for a byte. */
		static ::MonoClass* GetSByteClass();

		/** Returns Mono class for a char. */
		static ::MonoClass* GetCharClass();

		/** Returns Mono class for a generic object. */
		static ::MonoClass* GetObjectClass();

		/** @copydoc ThrowIfException(MonoObject*) */
		static void ThrowIfException(MonoException* exception);

		/**	Throws a native exception if the provided object is a valid managed exception. */
		static void ThrowIfException(MonoObject* exception);

		/** Invokes a thunk retrieved from MonoMethod::getThunk const and automatically handles exceptions. */
		template <class T, class... Args>
		static void InvokeThunk(T* thunk, Args... args)
		{
			MonoException* exception = nullptr;
			thunk(std::forward<Args>(args)..., &exception);

			ThrowIfException(exception);
		}
	};

	/** @} */
} // namespace b3d

#include "B3DMonoArray.h"
