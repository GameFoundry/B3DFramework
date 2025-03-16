//************************************ bs::framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsMonoPrerequisites.h"
#include "Utility/BsModule.h"

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
#include "BsMonoLoaderFunctions.h"
#undef MONO_API_FUNCTION

// Function pointer variable declarations
#define MONO_API_FUNCTION(ret, name, args) extern FNPTR_##name name;
#include "BsMonoLoaderFunctions.h"
#undef MONO_API_FUNCTION

namespace bs
{
	/** @addtogroup Mono
	 *  @{
	 */

	// TODO - Doc
	class B3D_MONO_EXPORT MonoLoader : Module<MonoLoader>
	{
	public:
		void Load();
		void Unload();

	private:
		DynamicLibrary* mLibrary = nullptr;
	};

	/** @} */
} // namespace bs
