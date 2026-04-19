//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DMonoPrerequisites.h"
#include "Utility/B3DModule.h"

#include <mono/jit/details/jit-types.h>
#include <mono/metadata/details/assembly-types.h>
#include <mono/metadata/details/mono-config-types.h>
#include <mono/metadata/details/mono-gc-types.h>
#include <mono/metadata/details/mono-debug-types.h>
#include <mono/utils/details/mono-publib-types.h>
#include <mono/utils/details/mono-logger-types.h>
#include <mono/metadata/details/threads-types.h>
#include <mono/metadata/details/appdomain-types.h>
#include <mono/metadata/details/object-types.h>
#include <mono/metadata/details/metadata-types.h>
#include <mono/metadata/details/image-types.h>
#include <mono/metadata/details/loader-types.h>
#include <mono/utils/details/mono-error-types.h>
#include <mono/metadata/details/class-types.h>
#include <mono/metadata/details/debug-helpers-types.h>
#include <mono/metadata/details/reflection-types.h>
#include <mono/metadata/tokentype.h>
#include <mono/metadata/attrdefs.h>

// Function pointer types
#define MONO_API_FUNCTION(ret, name, args) typedef ret (*FNPTR_##name)args;
#include "B3DMonoLoaderFunctions.h"
#undef MONO_API_FUNCTION

// Function pointer variable declarations
#define MONO_API_FUNCTION(ret, name, args) extern FNPTR_##name name;
#include "B3DMonoLoaderFunctions.h"
#undef MONO_API_FUNCTION

#define mono_array_addr(array,type,index) ((type*)mono_array_addr_with_size ((array), sizeof (type), (index)))
#define mono_array_get(array,type,index) ( *(type*)mono_array_addr ((array), type, (index)) )
#define mono_array_set(array,type,index,value)	\
	do {	\
		type *__p = (type *) mono_array_addr ((array), type, (index));	\
		*__p = (value);	\
	} while (0)
#define mono_array_setref(array,index,value)	\
	do {	\
		void **__p = (void **) mono_array_addr ((array), void*, (index));	\
		mono_gc_wbarrier_set_arrayref ((array), __p, (MonoObject*)(value));	\
		/* *__p = (value);*/	\
	} while (0)
#define mono_array_memcpy_refs(dest,destidx,src,srcidx,count)	\
	do {	\
		void **__p = (void **) mono_array_addr ((dest), void*, (destidx));	\
		void **__s = mono_array_addr ((src), void*, (srcidx));	\
		mono_gc_wbarrier_arrayref_copy (__p, __s, (count));	\
	} while (0)

namespace b3d
{
	/** @addtogroup Mono
	 *  @{
	 */

	/** Dynamically loads the Mono dynamic library and sets up the function pointers defined in B3DMonoLoaderFunctions.h. */
	class B3D_MONO_EXPORT MonoLoader : public Module<MonoLoader>
	{
	public:
		void Load();
		void Unload();

	private:
		DynamicLibrary* mLibrary = nullptr;
	};

	/** @} */
} // namespace b3d
