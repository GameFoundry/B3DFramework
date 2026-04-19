//********************************* B3D Framework - Copyright 2026 Marko Pintera *****************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DApplication.h"

namespace b3d
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */
	/** @cond SCRIPT_EXTENSIONS */

	/** Script-exported API for Application. */
	class B3D_SCRIPT_EXPORT(ExtensionClassForType(Application), DocumentationGroup(Application), ExportName(Application), API(Framework)) ApplicationEx
	{
	public:
		B3D_SCRIPT_EXPORT()
		static void StartUp(const ApplicationCreateInformation& desc);

		B3D_SCRIPT_EXPORT()
		static void StartUp(VideoMode videoMode, const String& title, bool fullscreen);

		B3D_SCRIPT_EXPORT()
		static void RunMainLoop();

		B3D_SCRIPT_EXPORT()
		static void ShutDown();
	};

	/** @endcond */
	/** @} */
} // namespace b3d
