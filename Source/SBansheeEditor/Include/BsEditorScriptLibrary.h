//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEditorPrerequisites.h"
#include "BsEngineScriptLibrary.h"

namespace bs
{
	/** @addtogroup SBansheeEditor
	 *  @{
	 */

	/**
	 * Handles initialization/shutdown of the script systems and loading/refresh of engine and editor-specific assemblies.
	 */
	class BS_SCR_BED_EXPORT EditorScriptLibrary : public EngineScriptLibrary
	{
	public:
		EditorScriptLibrary();

		/** @copydoc ScriptLibrary::initialize */
		void initialize() override;

		/** @copydoc ScriptLibrary::reload */
		void reload() override;

		/** @copydoc ScriptLibrary::destroy */
		void destroy() override;

	private:
		bool mScriptAssembliesLoaded;
	};

	/** @} */
}