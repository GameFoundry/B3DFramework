//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsMonoPrerequisites.h"

namespace bs
{
	/** @addtogroup Mono
	 *  @{
	 */

	/**	Utility class containing methods for various common Mono/Script related operations. */
	class BS_MONO_EXPORT MonoUtil
	{
	public:
		/**	Converts a Mono (managed) string to a native wide string. */
		static WString monoToWString(MonoString* str);

		/**	Converts a Mono (managed) string to a native narrow string. */
		static String monoToString(MonoString* str);

		/**	Converts a native wide string to a Mono (managed) string. */
		static MonoString* wstringToMono(const WString& str);

		/**	Converts a native narrow string to a Mono (managed) string. */
		static MonoString* stringToMono(const String& str);

		/**	Outputs name and namespace for the type of the specified object. */
		static void getClassName(MonoObject* obj, String& ns, String& typeName);

		/**	Outputs name and namespace for the specified type. */
		static void getClassName(::MonoClass* monoClass, String& ns, String& typeName);

		/**	Outputs name and namespace for the specified type. */
		static void getClassName(MonoReflectionType* monoReflType, String& ns, String& typeName);

		/** Returns the class of the provided object. */
		static ::MonoClass* getClass(MonoObject* object);

		/** Returns the class of the provided type. */
		static ::MonoClass* getClass(MonoReflectionType* type);

		/** Returns the type of the provided object. */
		static MonoReflectionType* getType(MonoObject* object);

		/** Returns the type of the provided class. */
		static MonoReflectionType* getType(::MonoClass* klass);

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
		static UINT32 newGCHandle(MonoObject* object, bool pinned = true);

		/**
		 * Creates a new GC handle for the provided managed object. The handle can be stored and later used for retrieving
		 * the MonoObject* related to it by calling getObjectFromGCHandle(). This is a weak handle, meaning it will NOT
		 * prevent the garbage collector from collecting the object. getObjectFromGCHandle() will return null if the GC
		 * collected the object and handle is no longer valid.
		 */
		static UINT32 newWeakGCHandle(MonoObject* object);

		/** Frees a GC handle previously allocated with newGCHandle. */
		static void freeGCHandle(UINT32 handle);

		/** Returns a MonoObject from an allocated GC handle. */
		static MonoObject* getObjectFromGCHandle(UINT32 handle);

		/** Converts a managed value type into a reference type by boxing it. */
		static MonoObject* box(::MonoClass* klass, void* value);

		/** Unboxes a managed object back to a raw value type. */
		static void* unbox(MonoObject* object);

		/** 
		 * Copies the value from @p src to @p dest. This must be a value-type of type @p klass. You need to use this
		 * form of copying if @p dest is a struct that gets passed to managed code and it contains a reference type. This
		 * way the GC is informed about the reference in the struct. You can use normal copies otherwise.
		 */
		static void valueCopy(void* dest, void* src, ::MonoClass* klass);

		/**
		 * Copies the pointer to a reference type @p object to @p dest, ensuring @p dest also points to the object. This
		 * needs to be used if @p dest is being passed to managed code (e.g. an output parameter in a method). Otherwise
		 * normal copies can be used. @p dest must be large enough to hold sizeof(MonoObject*).
		 */
		static void referenceCopy(void* dest, MonoObject* object);

		/**	Checks if this class is a sub class of the specified class. */
		static bool isSubClassOf(::MonoClass* subClass, ::MonoClass* parentClass);

		/** Checks is the specified class a value type. */
		static bool isValueType(::MonoClass* object);

		/** Checks is the specified class an enum. */
		static bool isEnum(::MonoClass* object);

		/** Returns the underlying primitive type for an enum. */
		static MonoPrimitiveType getEnumPrimitiveType(::MonoClass* enumClass);

		/** Returns the primitive type of the provided class. */
		static MonoPrimitiveType getPrimitiveType(::MonoClass* monoClass);

		/** Binds parameters to a generic class, and returns a new instantiable class with the bound parameters. */
		static ::MonoClass* bindGenericParameters(::MonoClass* klass, ::MonoClass** params, UINT32 numParams);

		/** 
		 * Returns the generic parameters of the provided type. @p params must be a pre-allocated buffer able to hold the
		 * class types for each parameter. If @p params is null, then @p numParams will be populated with the number of
		 * available parameters.
		 */
		static void getGenericParameters(::MonoClass* klass, ::MonoClass** params, UINT32& numParams);

		/** 
		 * Returns the generic parameters of the provided type. @p params must be a pre-allocated buffer able to hold the
		 * class types for each parameter. If @p params is null, then @p numParams will be populated with the number of
		 * available parameters.
		 */
		static void getGenericParameters(::MonoReflectionType* type, ::MonoClass** params, UINT32& numParams);

		/** Returns Mono class for a 16-bit unsigned integer. */
		static ::MonoClass* getUINT16Class();

		/** Returns Mono class for a 16-bit signed integer. */
		static ::MonoClass* getINT16Class();

		/** Returns Mono class for a 32-bit unsigned integer. */
		static ::MonoClass* getUINT32Class();

		/** Returns Mono class for a 32-bit signed integer. */
		static ::MonoClass* getINT32Class();

		/** Returns Mono class for a 64-bit unsigned integer. */
		static ::MonoClass* getUINT64Class();

		/** Returns Mono class for a 32-bit signed integer. */
		static ::MonoClass* getINT64Class();

		/** Returns Mono class for a string. */
		static ::MonoClass* getStringClass();

		/** Returns Mono class for a floating point number. */
		static ::MonoClass* getFloatClass();

		/** Returns Mono class for a double floating point number. */
		static ::MonoClass* getDoubleClass();

		/** Returns Mono class for a boolean. */
		static ::MonoClass* getBoolClass();

		/** Returns Mono class for an unsigned byte. */
		static ::MonoClass* getByteClass();

		/** Returns Mono class for a byte. */
		static ::MonoClass* getSByteClass();

		/** Returns Mono class for a char. */
		static ::MonoClass* getCharClass();

		/** Returns Mono class for a generic object. */
		static ::MonoClass* getObjectClass();

		/** @copydoc throwIfException(MonoObject*) */
		static void throwIfException(MonoException* exception);

		/**	Throws a native exception if the provided object is a valid managed exception. */
		static void throwIfException(MonoObject* exception);

		/** Invokes a thunk retrieved from MonoMethod::getThunk const and automatically handles exceptions. */
		template<class T, class... Args>
		static void invokeThunk(T* thunk, Args... args)
		{
			MonoException* exception = nullptr;
			thunk(std::forward<Args>(args)..., &exception);

			throwIfException(exception);
		}
	};

	/** @} */
}

#include "BsMonoArray.h"