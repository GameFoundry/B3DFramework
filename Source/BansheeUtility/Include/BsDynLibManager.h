//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisitesUtil.h"
#include "BsModule.h"

namespace bs 
{
	/** @addtogroup General
	 *  @{
	 */

    /**
	 * This manager keeps a track of all the open dynamic-loading libraries, opens them and returns references to 
	 * already-open libraries.
	 *			
	 * @note	Not thread safe.
     */
    class BS_UTILITY_EXPORT DynLibManager : public Module<DynLibManager>
    {
    public:
        DynLibManager();
        virtual ~DynLibManager();

        /**
         * Loads the given file as a dynamic library.
         *
         * @param[in]	name	The name of the library. The extension can be omitted.
         */
        DynLib* load(const String& name);

		/** Unloads the given library. */
		void unload(DynLib* lib);

	protected:
		Map<String, DynLib*> mLoadedLibraries;
    };

	/** Easy way of accessing DynLibManager. */
	BS_UTILITY_EXPORT DynLibManager& gDynLibManager();

	/** @} */
}