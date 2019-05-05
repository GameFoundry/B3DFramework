//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"

#if BS_PLATFORM == BS_PLATFORM_WIN32
	struct HINSTANCE__;
	typedef struct HINSTANCE__* hInstance;
#endif

namespace bs
{
	/** @addtogroup General
	 *  @{
	 */

#if BS_PLATFORM == BS_PLATFORM_WIN32
#    define DYNLIB_HANDLE hInstance
#    define DYNLIB_LOAD( a ) LoadLibraryEx( a, NULL, LOAD_WITH_ALTERED_SEARCH_PATH )
#    define DYNLIB_GETSYM( a, b ) GetProcAddress( a, b )
#    define DYNLIB_UNLOAD( a ) !FreeLibrary( a )

#elif BS_PLATFORM == BS_PLATFORM_LINUX || BS_PLATFORM == BS_PLATFORM_OSX
#    define DYNLIB_HANDLE void*
#    define DYNLIB_LOAD( a ) dlopen( a, RTLD_LAZY | RTLD_GLOBAL)
#    define DYNLIB_GETSYM( a, b ) dlsym( a, b )
#    define DYNLIB_UNLOAD( a ) dlclose( a )

#endif

	/** Class that holds data about a dynamic library. */
	class BS_UTILITY_EXPORT DynLib final
	{
	public:
		/** Platform-specific file extension for a dynamic library (e.g. "dll"). */
#if BS_PLATFORM == BS_PLATFORM_LINUX
		static constexpr const char* EXTENSION = "so";
#elif BS_PLATFORM == BS_PLATFORM_OSX
		static constexpr const char* EXTENSION = "dylib";
#elif BS_PLATFORM == BS_PLATFORM_WIN32
		static constexpr const char* EXTENSION = "dll";
#else
	#error Unhandled platform
#endif

		/** Platform-specific name suffix for a dynamic library (e.g. "lib" on Unix) */
#if BS_PLATFORM == BS_PLATFORM_LINUX
		static constexpr const char* PREFIX = "lib";
#elif BS_PLATFORM == BS_PLATFORM_OSX
		static constexpr const char* PREFIX = "lib";
#elif BS_PLATFORM == BS_PLATFORM_WIN32
		static constexpr const char* PREFIX = nullptr;
#else
	#error Unhandled platform
#endif

		/** Constructs the dynamic library object and loads the library with the specified name. */
		DynLib(String name);
		~DynLib();

		/** Loads the library. Does nothing if library is already loaded. */
		void load();

		/** Unloads the library. Does nothing if library is not loaded. */
		void unload();

		/** Get the name of the library. */
		const String& getName() const { return mName; }

		/**
		 * Returns the address of the given symbol from the loaded library.
		 *
		 * @param[in] strName	The name of the symbol to search for.
		 * @return				If the function succeeds, the returned value is a handle to the symbol. Otherwise null.
		 */
		void* getSymbol(const String& strName) const;

	protected:
		friend class DynLibManager;

		/** Gets the last loading error. */
		String dynlibError();

	protected:
		const String mName;
		DYNLIB_HANDLE mHandle = nullptr;
	};

	/** @} */
}
