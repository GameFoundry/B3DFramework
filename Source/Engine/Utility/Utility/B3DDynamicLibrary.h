//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"

#if B3D_PLATFORM_WIN32
struct HINSTANCE__;
typedef struct HINSTANCE__* hInstance;
#endif

namespace b3d
{
	/** @addtogroup General
	 *  @{
	 */

#if B3D_PLATFORM_WIN32
#	define DYNLIB_HANDLE hInstance
#	define DYNLIB_LOAD(a) LoadLibraryEx(a, NULL, LOAD_WITH_ALTERED_SEARCH_PATH)
#	define DYNLIB_GETSYM(a, b) GetProcAddress(a, b)
#	define DYNLIB_UNLOAD(a) !FreeLibrary(a)

#elif B3D_PLATFORM_LINUX || B3D_PLATFORM_MACOS
#	define DYNLIB_HANDLE void*
#	define DYNLIB_LOAD(a) dlopen(a, RTLD_LAZY | RTLD_GLOBAL)
#	define DYNLIB_GETSYM(a, b) dlsym(a, b)
#	define DYNLIB_UNLOAD(a) dlclose(a)

#endif

	/** Class that holds data about a dynamic library. */
	class B3D_EXPORT DynamicLibrary final
	{
	public:
		/** Platform-specific file extension for a dynamic library (e.g. "dll"). */
#if B3D_PLATFORM_LINUX
		static constexpr const char* EXTENSION = "so";
#elif B3D_PLATFORM_MACOS
		static constexpr const char* EXTENSION = "dylib";
#elif B3D_PLATFORM_WIN32
		static constexpr const char* kExtension = "dll";
#else
#	error Unhandled platform
#endif

		/** Platform-specific name suffix for a dynamic library (e.g. "lib" on Unix) */
#if B3D_PLATFORM_LINUX
		static constexpr const char* PREFIX = "lib";
#elif B3D_PLATFORM_MACOS
		static constexpr const char* PREFIX = "lib";
#elif B3D_PLATFORM_WIN32
		static constexpr const char* kPrefix = nullptr;
#else
#	error Unhandled platform
#endif

		/** Constructs the dynamic library object and loads the library with the specified name. */
		DynamicLibrary(String name);
		~DynamicLibrary();

		/** Loads the library. Does nothing if library is already loaded. */
		void Load();

		/** Unloads the library. Does nothing if library is not loaded. */
		void Unload();

		/** Get the name of the library. */
		const String& GetName() const { return mName; }

		/**
		 * Returns the address of the given symbol from the loaded library.
		 *
		 * @param name		The name of the symbol to search for.
		 * @return			If the function succeeds, the returned value is a handle to the symbol. Otherwise null.
		 */
		void* GetSymbol(const char* name) const;

	protected:
		friend class DynamicLibraryManager;

		/** Gets the last loading error. */
		String DynlibError();

	protected:
		const String mName;
		DYNLIB_HANDLE mHandle = nullptr;
	};

	/** @} */
} // namespace b3d
