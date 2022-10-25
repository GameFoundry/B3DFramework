//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsApplication.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */
	/** @cond SCRIPT_EXTENSIONS */

	/** Script-exported API for Application. */
	class BS_SCRIPT_EXPORT(DocumentationGroup(Application), ExportName(Application), API(Framework)) ApplicationEx
	{
	public:
		BS_SCRIPT_EXPORT()
		static void StartUp(const START_UP_DESC& desc);

		BS_SCRIPT_EXPORT()
		static void StartUp(VideoMode videoMode, const String& title, bool fullscreen);

		BS_SCRIPT_EXPORT()
		static void RunMainLoop();

		BS_SCRIPT_EXPORT()
		static void ShutDown();
	};

	/** @endcond */
	/** @} */
} // namespace bs
