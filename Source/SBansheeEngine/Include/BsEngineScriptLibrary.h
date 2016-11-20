//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptManager.h"

namespace bs
{
	/** @addtogroup SBansheeEngine
	 *  @{
	 */

	/** Handles initialization/shutdown of the script systems and loading/refresh of engine-specific assemblies. */
	class BS_SCR_BE_EXPORT EngineScriptLibrary : public ScriptLibrary
	{
	public:
		EngineScriptLibrary();

		/** @copydoc ScriptLibrary::initialize */
		void initialize() override;

		/** @copydoc ScriptLibrary::reload */
		void reload() override;

		/** @copydoc ScriptLibrary::destroy */
		void destroy() override;

	protected:
		/** Unloads all manages assemblies and the mono domain. */
		void unloadAssemblies();

		/** Shuts down all script engine modules. */
		void shutdownModules();

	private:
		bool mScriptAssembliesLoaded;
	};

	/** @} */
}