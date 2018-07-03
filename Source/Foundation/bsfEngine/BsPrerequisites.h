//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"

/** @addtogroup Layers
 *  @{
 */

/** @defgroup Engine Engine
 *	Layer that builds upon Core, providing specific implementations of its interfaces as well as other high level systems.
 *  @{
 */

/** @defgroup 2D 2D
  *	Two dimensional geometry (sprites).
  */

/** @defgroup Application-Engine Application
 *  Entry point into the application.
 */

/** @defgroup GUI GUI
  *	Graphical user interface, including elements, styles, events and GUI manager.
  */

/** @defgroup Input-Engine Input
 *	User input (mouse, keyboard, gamepad, etc.).
 */

/** @defgroup Platform-Engine Platform
 *  %Platform specific functionality.
 */

/** @defgroup Resources-Engine Resources
  *	Builtin engine resource types and a manager for such resources.
  */

/** @cond RTTI */
/** @defgroup RTTI-Impl-Engine RTTI types
 *  Types containing RTTI for specific classes.
 */
/** @endcond */

/** @defgroup Utility-Engine Utility
 *  Various utility methods and types used by the engine layer.
 */

/** @} */
/** @} */

/** @addtogroup Internals
 *  @{
 */

/** @defgroup Internal-Engine Engine
 *	Layer that builds upon Core, providing specific implementations of its interfaces as well as other high level systems.
 *  @{
 */

/** @defgroup 2D-Internal 2D
  *	Two dimensional geometry (sprites).
  */

/** @defgroup GUI-Internal GUI
  *	Graphical user interface, including elements, styles, events and GUI manager.
  */

/** @defgroup Renderer-Engine-Internal Renderer
  *	Abstract interface and helper functionality for rendering scene objects and other geometry.
  */

/** @defgroup Resources-Engine-Internal Resources
  *	Builtin engine resource types and a manager for such resources.
  */

/** @defgroup Script-Internal Script
 *  Interaction with scripting languages and libraries.
 */

/** @defgroup Utility-Engine-Internal Utility
 *  Various utility methods and types used by the engine layer.
 */

/** @} */
/** @} */

// DLL export
#if BS_PLATFORM == BS_PLATFORM_WIN32 // Windows
#  if BS_COMPILER == BS_COMPILER_MSVC
#    if defined(BS_STATIC_LIB)
#      define BS_EXPORT
#    else
#      if defined(BS_EXPORTS)
#        define BS_EXPORT __declspec(dllexport)
#      else
#        define BS_EXPORT __declspec(dllimport)
#      endif
#	 endif
#  else
#    if defined(BS_STATIC_LIB)
#      define BS_EXPORT
#    else
#      if defined(BS_EXPORTS)
#        define BS_EXPORT __attribute__ ((dllexport))
#      else
#        define BS_EXPORT __attribute__ ((dllimport))
#      endif
#	 endif
#  endif
#  define BS_HIDDEN
#else // Linux/Mac settings
#  define BS_EXPORT __attribute__ ((visibility ("default")))
#  define BS_HIDDEN __attribute__ ((visibility ("hidden")))
#endif

#include "Scene/BsGameObject.h"
#include "Utility/BsEnums.h"
#include "Utility/BsPaths.h"
#include "Localization/BsHEString.h"

namespace bs
{
	static const StringID RendererDefault = "RenderBeast";

	class VirtualButton;
	class VirtualInput;
	class InputConfiguration;
	struct DragCallbackInfo;
	struct ShortcutKey;

	// GUI
	class CGUIWidget;
	class GUIManager;
	class GUIWidget;
	class GUIElementBase;
	class GUIElement;
	class GUILabel;
	class GUIButtonBase;
	class GUIButton;
	class GUITexture;
	class GUIToggle;
	class GUIInputBox;
	class GUISliderHandle;
	class GUIScrollBarVert;
	class GUIScrollBarHorz;
	class GUIScrollArea;
	class GUISkin;
	class GUIRenderTexture;
	struct GUIElementStyle;
	class GUIMouseEvent;
	class GUITextInputEvent;
	class GUICommandEvent;
	class GUIVirtualButtonEvent;
	class GUILayout;
	class GUILayoutX;
	class GUILayoutY;
	class GUIPanel;
	class GUIFixedSpace;
	class GUIFlexibleSpace;
	class GUIInputCaret;
	class GUIInputSelection;
	struct GUIDimensions;
	class GUIOptions;
	class GUIToggleGroup;
	class GUIListBox;
	class GUIDropDownDataEntry;
	class GUIDropDownMenu;
	class DragAndDropManager;
	class GUIMenu;
	class GUIMenuItem;
	class GUIContent;
	class GUIContextMenu;
	class GUIDropDownHitBox;
	class GUIDropDownContent;
	class RenderElement;
	class GUISlider;
	class GUISliderVert;
	class GUISliderHorz;
	class GUIProgressBar;
	class GUICanvas;

	class RenderableHandler;
	class CProfilerOverlay;
	class ProfilerOverlay;
	class DrawHelper;
	class PlainText;
	class ScriptCode;
	class ScriptCodeImportOptions;
	class RendererMeshData;

	// 2D
	class TextSprite;
	class ImageSprite;
	class SpriteMaterial;
	struct SpriteMaterialInfo;

	typedef GameObjectHandle<CGUIWidget> HGUIWidget;
	typedef GameObjectHandle<CProfilerOverlay> HProfilerOverlay;

	typedef ResourceHandle<PlainText> HPlainText;
	typedef ResourceHandle<ScriptCode> HScriptCode;
	typedef ResourceHandle<GUISkin> HGUISkin;

	/**	RTTI types. */
	enum TypeID_Banshee
	{
		/* TID_CCamera = 30000, */
		/* TID_CRenderable = 30001, */
		/* TID_SpriteTexture = 30002, */
		/* TID_Camera = 30003, */
		/* TID_Renderable = 30004, */
		TID_PlainText = 30005,
		TID_ScriptCode = 30006,
		TID_ScriptCodeImportOptions = 30007,
		TID_GUIElementStyle = 30008,
		TID_GUISkin = 30009,
		TID_GUISkinEntry = 30010,
		/* TID_Light = 30011, */
		/* TID_CLight = 30012, */
		TID_GameSettings = 30013,
		TID_ResourceMapping = 30014,
		//TID_AutoExposureSettings = 30016,
		//TID_TonemappingSettings = 30017,
		//TID_WhiteBalanceSettings = 30018,
		//TID_ColorGradingSettings = 30019,
		//TID_DepthOfFieldSettings = 30020,
		//TID_AmbientOcclusionSettings = 30021,
		//TID_ScreenSpaceReflectionsSettings = 30022
	};
}