//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
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

/** @defgroup Components Components
  *	Built-in components.
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

#if (BS_PLATFORM == BS_PLATFORM_WIN32) && !defined(__MINGW32__)
#	ifdef BS_EXPORTS
#		define BS_EXPORT __declspec(dllexport)
#	else
#       if defined( __MINGW32__ )
#           define BS_EXPORT
#       else
#    		define BS_EXPORT __declspec(dllimport)
#       endif
#	endif
#elif defined ( BS_GCC_VISIBILITY )
#    define BS_EXPORT  __attribute__ ((visibility("default")))
#else
#    define BS_EXPORT
#endif

#include "BsGameObject.h"
#include "BsEnums.h"
#include "BsHEString.h"
#include "BsPaths.h"

namespace BansheeEngine
{
	static const StringID RenderAPIDX9 = "D3D9RenderAPI";
	static const StringID RenderAPIDX11 = "D3D11RenderAPI";
	static const StringID RenderAPIOpenGL = "GLRenderAPI";
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
	class RenderableElement;
	class GUISlider;
	class GUISliderVert;
	class GUISliderHorz;
	class GUIProgressBar;
	class GUICanvas;

	class RenderableHandler;
	class ProfilerOverlay;
	class ProfilerOverlayInternal;
	class DrawHelper;
	class Renderable;
	class RenderableCore;
	class PlainText;
	class ScriptCode;
	class ScriptCodeImportOptions;
	class RendererMeshData;

	// 2D
	class TextSprite;
	class ImageSprite;
	class SpriteTexture;
	class SpriteMaterial;
	struct SpriteMaterialInfo;

	// Components
	class CRenderable;
	class CLight;
	class CAnimation;
	class CBone;

	typedef GameObjectHandle<CGUIWidget> HGUIWidget;
	typedef GameObjectHandle<CRenderable> HRenderable;
	typedef GameObjectHandle<CAnimation> HAnimation;
	typedef GameObjectHandle<CBone> HBone;
	typedef GameObjectHandle<ProfilerOverlay> HProfilerOverlay;

	typedef ResourceHandle<SpriteTexture> HSpriteTexture;
	typedef ResourceHandle<PlainText> HPlainText;
	typedef ResourceHandle<ScriptCode> HScriptCode;
	typedef ResourceHandle<GUISkin> HGUISkin;

	/**	RTTI types. */
	enum TypeID_Banshee
	{
		/* TID_CCamera = 30000, */
		TID_CRenderable = 30001,
		TID_SpriteTexture = 30002,
		/* TID_Camera = 30003, */
		TID_Renderable = 30004,
		TID_PlainText = 30005,
		TID_ScriptCode = 30006,
		TID_ScriptCodeImportOptions = 30007,
		TID_GUIElementStyle = 30008,
		TID_GUISkin = 30009,
		TID_GUISkinEntry = 30010,
		TID_Light = 30011,
		TID_CLight = 30012,
		TID_GameSettings = 30013,
		TID_ResourceMapping = 30014,
		TID_StandardPostProcessSettings = 30015,
		TID_AutoExposureSettings = 30016,
		TID_TonemappingSettings = 30017,
		TID_WhiteBalanceSettings = 30018,
		TID_ColorGradingSettings = 30019
	};
}