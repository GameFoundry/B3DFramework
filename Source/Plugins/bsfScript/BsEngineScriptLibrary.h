//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Script/BsScriptManager.h"

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