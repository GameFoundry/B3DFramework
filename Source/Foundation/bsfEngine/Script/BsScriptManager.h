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

		/** Called once per frame. */
		virtual void update() { }

		/** Called when the script libraries should be reloaded (for example when they are recompiled). */
		virtual void reload() = 0;

		/**	Called when the script system is being destroyed. */
		virtual void destroy() = 0;
	};

	/**	Handles initialization of a scripting system. */
	class BS_EXPORT ScriptManager : public Module<ScriptManager>
	{
	public:
		ScriptManager();
		~ScriptManager();

		/** Called once per frame. */
		void update();

		/**
		 * Reloads any scripts in the currently active library. Should be called after some change to the scripts was made
		 * (for example project was changed, or scripts were recompiled).
		 */
		void reload();

		/** 
		 * Sets the active script library that controls what kind and which scripts are loaded. Must be called before
		 * the module is started up.
		 */
		static void _setScriptLibrary(const SPtr<ScriptLibrary>& library) { sScriptLibrary = library; }

		/** Returns the currently assigned script library. */
		static const SPtr<ScriptLibrary>& _getScriptLibrary() { return sScriptLibrary; }

	private:
		static SPtr<ScriptLibrary> sScriptLibrary;
	};

	/** @} */
}