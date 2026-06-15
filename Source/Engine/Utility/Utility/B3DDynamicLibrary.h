//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"

namespace b3d
{
	/** @addtogroup System
	 *  @{
	 */

	/** Class that holds data about a dynamic library. */
	class B3D_EXPORT DynamicLibrary final
	{
	public:
		/** Platform-specific file extension for a dynamic library (e.g. "dll"). */
		static constexpr const char* kExtension = B3D_DYNLIB_EXTENSION;

		/** Platform-specific name prefix for a dynamic library (e.g. "lib" on Unix), or null if none. */
		static constexpr const char* kPrefix = B3D_DYNLIB_PREFIX;

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
		void* mHandle = nullptr;
	};

	/** @} */
} // namespace b3d
