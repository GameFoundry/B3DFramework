//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsEditorPrerequisites.h"

// DLL export
#if BS_PLATFORM == BS_PLATFORM_WIN32 // Windows
#  if BS_COMPILER == BS_COMPILER_MSVC
#    if defined(BS_STATIC_LIB)
#      define BS_SCR_BED_EXPORT
#    else
#      if defined(BS_SCR_BED_EXPORTS)
#        define BS_SCR_BED_EXPORT __declspec(dllexport)
#      else
#        define BS_SCR_BED_EXPORT __declspec(dllimport)
#      endif
#	 endif
#  else
#    if defined(BS_STATIC_LIB)
#      define BS_SCR_BED_EXPORT
#    else
#      if defined(BS_SCR_BED_EXPORTS)
#        define BS_SCR_BED_EXPORT __attribute__ ((dllexport))
#      else
#        define BS_SCR_BED_EXPORT __attribute__ ((dllimport))
#      endif
#	 endif
#  endif
#  define BS_SCR_BED_HIDDEN
#else // Linux/Mac settings
#  define BS_SCR_BED_EXPORT __attribute__ ((visibility ("default")))
#  define BS_SCR_BED_HIDDEN __attribute__ ((visibility ("hidden")))
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