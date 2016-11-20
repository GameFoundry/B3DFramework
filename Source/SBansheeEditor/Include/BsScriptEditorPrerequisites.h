//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsEditorPrerequisites.h"

#if (BS_PLATFORM == BS_PLATFORM_WIN32) && !defined(__MINGW32__)
#	ifdef BS_SCR_BED_EXPORTS
#		define BS_SCR_BED_EXPORT __declspec(dllexport)
#	else
#       if defined( __MINGW32__ )
#           define BS_SCR_BED_EXPORT
#       else
#    		define BS_SCR_BED_EXPORT __declspec(dllimport)
#       endif
#	endif
#elif defined ( BS_GCC_VISIBILITY )
#    define BS_SCR_BED_EXPORT  __attribute__ ((visibility("default")))
#else
#    define BS_SCR_BED_EXPORT
#endif

/** @addtogroup Plugins
 *  @{
 */

/** @defgroup SBansheeEditor SBansheeEditor
 *	Contains script interop objects and other scripting functionality for the editor layer.
 *  @{
 */

/** @defgroup ScriptInteropEditor Interop
 *	Script interop objects for communicating between native code and MBansheeEditor managed assembly.
 */

/** @} */
/** @} */

namespace bs
{
	class ScriptEditorWindow;
	class GUIGameObjectField;
	class GUIResourceField;
	class GUITextureField;
	class ScriptHandleSliderBase;
	class ScriptPlatformInfo;
}