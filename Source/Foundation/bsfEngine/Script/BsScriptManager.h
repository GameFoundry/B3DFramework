//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "Utility/BsModule.h"

namespace bs 
{
	/** @addtogroup Script-Internal
	 *  @{
	 */

	/**	Abstraction that handles a specific set of script libraries. */
	class BS_EXPORT ScriptLibrary
	{
	public:
		virtual ~ScriptLibrary() = default;

		/**	Called when the script system is being activated. */
		virtual void initialize() = 0;

		/** Called when the script libraries should be reloaded (for example when they are recompiled). */
		virtual void reload() = 0;

		/**	Called when the script system is being destroyed. */
		virtual void destroy() = 0;
	};

	/**	Handles initialization of a scripting system. */
	class BS_EXPORT ScriptManager : public Module<ScriptManager>
	{
	public:
		ScriptManager() = default;
		~ScriptManager() = default;

		/** Initializes the currently active script library loading the scripts contained within. */
		void initialize();

		/**
		 * Reloads any scripts in the currently active library. Should be called after some change to the scripts was made
		 * (for example project was changed, or scripts were recompiled).
		 */
		void reload();

		/** Sets the active script library that controls what kind and which scripts are loaded. */
		void _setScriptLibrary(const SPtr<ScriptLibrary>& library);

	private:
		/** @copydoc ScriptManager::onShutDown */
		void onShutDown() override;

		SPtr<ScriptLibrary> mScriptLibrary;
	};

	/** @} */
}