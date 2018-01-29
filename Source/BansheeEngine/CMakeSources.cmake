set(BS_BANSHEEENGINE_SRC_RESOURCES
	"Resources/BsScriptCodeImportOptions.cpp"
	"Resources/BsScriptCodeImporter.cpp"
	"Resources/BsScriptCode.cpp"
	"Resources/BsPlainTextImporter.cpp"
	"Resources/BsPlainText.cpp"
	"Resources/BsBuiltinResources.cpp"
	"Resources/BsEngineShaderIncludeHandler.cpp"
	"Resources/BsGameResourceManager.cpp"
	"Resources/BsBuiltinResourcesHelper.cpp"
)

set(BS_BANSHEEENGINE_SRC_SCRIPT
	"Script/BsScriptManager.cpp"
)

set(BS_BANSHEEENGINE_INC_INPUT
	"Input/BsInputConfiguration.h"
	"Input/BsVirtualInput.h"
)

set(BS_BANSHEEENGINE_SRC_GUI
	"GUI/BsGUIElement.cpp"
	"GUI/BsGUILabel.cpp"
	"GUI/BsGUIManager.cpp"
	"GUI/BsGUISkin.cpp"
	"GUI/BsGUILayout.cpp"
	"GUI/BsGUILayoutX.cpp"
	"GUI/BsGUILayoutY.cpp"
	"GUI/BsGUIButton.cpp"
	"GUI/BsGUITexture.cpp"
	"GUI/BsGUIToggle.cpp"
	"GUI/BsGUIInputBox.cpp"
	"GUI/BsGUIMouseEvent.cpp"
	"GUI/BsGUIInputCaret.cpp"
	"GUI/BsGUIInputSelection.cpp"
	"GUI/BsGUIInputTool.cpp"
	"GUI/BsGUIElementBase.cpp"
	"GUI/BsGUIScrollBarVert.cpp"
	"GUI/BsGUIScrollBarHorz.cpp"
	"GUI/BsGUIScrollArea.cpp"
	"GUI/BsGUIScrollBar.cpp"
	"GUI/BsGUIToggleGroup.cpp"
	"GUI/BsDragAndDropManager.cpp"
	"GUI/BsGUIViewport.cpp"
	"GUI/BsGUIContent.cpp"
	"GUI/BsGUIListBox.cpp"
	"GUI/BsGUIMenu.cpp"
	"GUI/BsGUIHelper.cpp"
	"GUI/BsGUIDropDownBoxManager.cpp"
	"GUI/BsGUIContextMenu.cpp"
	"GUI/BsGUIButtonBase.cpp"
	"GUI/BsGUITextInputEvent.cpp"
	"GUI/BsProfilerOverlay.cpp"
	"GUI/BsGUIOptions.cpp"
	"GUI/BsGUIRenderTexture.cpp"
	"GUI/BsGUIElementContainer.cpp"
	"GUI/BsGUIDropDownHitBox.cpp"
	"GUI/BsGUILayoutUtility.cpp"
	"GUI/BsGUISliderHandle.cpp"
	"GUI/BsGUISlider.cpp"
	"GUI/BsGUIProgressBar.cpp"
	"GUI/BsGUISpace.cpp"
	"GUI/BsGUIDropDownContent.cpp"
	"GUI/BsGUIElementStyle.cpp"
	"GUI/BsGUIDimensions.cpp"
	"GUI/BsGUIPanel.cpp"
	"GUI/BsGUIDropDownMenu.cpp"
	"GUI/BsDropDownAreaPlacement.cpp"
	"GUI/BsGUITooltipManager.cpp"
	"GUI/BsGUITooltip.cpp"
	"GUI/BsGUIWidget.cpp"
	"GUI/BsShortcutKey.cpp"
	"GUI/BsShortcutManager.cpp"
	"GUI/BsCGUIWidget.cpp"
	"GUI/BsGUICanvas.cpp"
)

set(BS_BANSHEEENGINE_INC_PLATFORM
	"Platform/BsCursor.h"
	"Platform/BsSplashScreen.h"
)

set(BS_BANSHEEENGINE_INC_RENDERER
	"Renderer/BsRenderableElement.h"
	"Renderer/BsRendererMaterial.h"
	"Renderer/BsRendererMaterialManager.h"
	"Renderer/BsRenderQueue.h"
	"Renderer/BsRendererUtility.h"
)

set(BS_BANSHEEENGINE_SRC_RTTI
	"Private/RTTI/BsGUISkinRTTI.cpp"
)

set(BS_BANSHEEENGINE_SRC_2D
	"2D/BsImageSprite.cpp"
	"2D/BsSprite.cpp"
	"2D/BsSpriteTexture.cpp"
	"2D/BsTextSprite.cpp"
	"2D/BsSpriteMaterial.cpp"
	"2D/BsSpriteMaterials.cpp"
	"2D/BsSpriteManager.cpp"
)

set(BS_BANSHEEENGINE_SRC_UTILITY
	"Utility/BsDrawHelper.cpp"
	"Utility/BsGameSettings.cpp"
	"Utility/BsPaths.cpp"
	"Utility/BsShapeMeshes2D.cpp"
	"Utility/BsShapeMeshes3D.cpp"
)

set(BS_BANSHEEENGINE_SRC_DEBUG
	"Debug/BsDebugDraw.cpp"
)

set(BS_BANSHEEENGINE_SRC_LOCALIZATION
	"Localization/BsHEString.cpp"
)

set(BS_BANSHEEENGINE_INC_2D
	"2D/BsImageSprite.h"
	"2D/BsSprite.h"
	"2D/BsSpriteTexture.h"
	"2D/BsTextSprite.h"
	"2D/BsSpriteMaterial.h"
	"2D/BsSpriteMaterials.h"
	"2D/BsSpriteManager.h"
)

set(BS_BANSHEEENGINE_INC_RTTI
	"Private/RTTI/BsSpriteTextureRTTI.h"
	"Private/RTTI/BsPlainTextRTTI.h"
	"Private/RTTI/BsScriptCodeRTTI.h"
	"Private/RTTI/BsScriptCodeImportOptionsRTTI.h"
	"Private/RTTI/BsGUIElementStyleRTTI.h"
	"Private/RTTI/BsGUISkinRTTI.h"
	"Private/RTTI/BsProfilerOverlayRTTI.h"
	"Private/RTTI/BsCGUIWidgetRTTI.h"
	"Private/RTTI/BsGameSettingsRTTI.h"
	"Private/RTTI/BsResourceMappingRTTI.h"
)

set(BS_BANSHEEENGINE_INC_NOFILTER
	"BsApplication.h"
	"BsPrerequisites.h"
)

set(BS_BANSHEEENGINE_INC_RESOURCES
	"Resources/BsPlainText.h"
	"Resources/BsPlainTextImporter.h"
	"Resources/BsScriptCode.h"
	"Resources/BsScriptCodeImporter.h"
	"Resources/BsScriptCodeImportOptions.h"
	"Resources/BsBuiltinResources.h"
	"Resources/BsEngineShaderIncludeHandler.h"
	"Resources/BsGameResourceManager.h"
	"Resources/BsBuiltinResourcesHelper.h"
)

set(BS_BANSHEEENGINE_INC_SCRIPT
	"Script/BsScriptManager.h"
)

set(BS_BANSHEEENGINE_INC_UTILITY
	"Utility/BsDrawHelper.h"
	"Utility/BsEnums.h"
	"Utility/BsGameSettings.h"
	"Utility/BsPaths.h"
	"Utility/BsRectOffset.h"
	"Utility/BsShapeMeshes2D.h"
	"Utility/BsShapeMeshes3D.h"
)

set(BS_BANSHEEENGINE_INC_DEBUG
	"Debug/BsDebugDraw.h"
)

set(BS_BANSHEEENGINE_INC_LOCALIZATION
	"Localization/BsHEString.h"
)

set(BS_BANSHEEENGINE_SRC_RENDERER
	"Renderer/BsRendererMaterialManager.cpp"
	"Renderer/BsRenderQueue.cpp"
	"Renderer/BsRendererUtility.cpp"
)

set(BS_BANSHEEENGINE_SRC_INPUT
	"Input/BsInputConfiguration.cpp"
	"Input/BsVirtualInput.cpp"
)

set(BS_BANSHEEENGINE_INC_GUI
	"GUI/BsGUIElement.h"
	"GUI/BsGUIElementStyle.h"
	"GUI/BsGUILabel.h"
	"GUI/BsGUIManager.h"
	"GUI/BsGUISkin.h"
	"GUI/BsGUILayout.h"
	"GUI/BsGUILayoutX.h"
	"GUI/BsGUILayoutY.h"
	"GUI/BsGUISpace.h"
	"GUI/BsGUIButton.h"
	"GUI/BsGUITexture.h"
	"GUI/BsGUIToggle.h"
	"GUI/BsGUIInputBox.h"
	"GUI/BsGUICommandEvent.h"
	"GUI/BsGUIInputCaret.h"
	"GUI/BsGUIInputSelection.h"
	"GUI/BsGUIInputTool.h"
	"GUI/BsGUIElementBase.h"
	"GUI/BsGUIScrollBarVert.h"
	"GUI/BsGUIScrollBarHorz.h"
	"GUI/BsGUIScrollArea.h"
	"GUI/BsGUIScrollBar.h"
	"GUI/BsGUIToggleGroup.h"
	"GUI/BsDragAndDropManager.h"
	"GUI/BsGUIViewport.h"
	"GUI/BsGUIContent.h"
	"GUI/BsGUIListBox.h"
	"GUI/BsGUIMenu.h"
	"GUI/BsGUIContextMenu.h"
	"GUI/BsGUIHelper.h"
	"GUI/BsGUIDropDownBoxManager.h"
	"GUI/BsGUIButtonBase.h"
	"GUI/BsGUITextInputEvent.h"
	"GUI/BsGUIMouseEvent.h"
	"GUI/BsProfilerOverlay.h"
	"GUI/BsGUIOptions.h"
	"GUI/BsGUIRenderTexture.h"
	"GUI/BsGUIElementContainer.h"
	"GUI/BsGUIDropDownHitBox.h"
	"GUI/BsGUIVirtualButtonEvent.h"
	"GUI/BsGUILayoutUtility.h"
	"GUI/BsGUISliderHandle.h"
	"GUI/BsGUISlider.h"
	"GUI/BsGUIProgressBar.h"
	"GUI/BsGUIDropDownContent.h"
	"GUI/BsGUIDimensions.h"
	"GUI/BsGUIPanel.h"
	"GUI/BsGUILayoutData.h"
	"GUI/BsGUIDropDownMenu.h"
	"GUI/BsDropDownAreaPlacement.h"
	"GUI/BsGUITooltipManager.h"
	"GUI/BsGUITooltip.h"
	"GUI/BsGUIWidget.h"
	"GUI/BsCGUIWidget.h"
	"GUI/BsShortcutManager.h"
	"GUI/BsShortcutKey.h"
	"GUI/BsGUICanvas.h"
)

set(BS_BANSHEEENGINE_SRC_NOFILTER
	"BsApplication.cpp"
)

set(BS_BANSHEEENGINE_SRC_PLATFORM
	"Platform/BsCursor.cpp"
	"Platform/BsSplashScreen.cpp"
)

source_group("Source Files\\Resources" FILES ${BS_BANSHEEENGINE_SRC_RESOURCES})
source_group("Source Files\\Script" FILES ${BS_BANSHEEENGINE_SRC_SCRIPT})
source_group("Header Files\\Input" FILES ${BS_BANSHEEENGINE_INC_INPUT})
source_group("Source Files\\GUI" FILES ${BS_BANSHEEENGINE_SRC_GUI})
source_group("Header Files\\Platform" FILES ${BS_BANSHEEENGINE_INC_PLATFORM})
source_group("Header Files\\Renderer" FILES ${BS_BANSHEEENGINE_INC_RENDERER})
source_group("Source Files\\RTTI" FILES ${BS_BANSHEEENGINE_SRC_RTTI})
source_group("Source Files\\2D" FILES ${BS_BANSHEEENGINE_SRC_2D})
source_group("Source Files\\Utility" FILES ${BS_BANSHEEENGINE_SRC_UTILITY})
source_group("Header Files\\2D" FILES ${BS_BANSHEEENGINE_INC_2D})
source_group("Header Files\\RTTI" FILES ${BS_BANSHEEENGINE_INC_RTTI})
source_group("Header Files" FILES ${BS_BANSHEEENGINE_INC_NOFILTER})
source_group("Header Files\\Resources" FILES ${BS_BANSHEEENGINE_INC_RESOURCES})
source_group("Header Files\\Script" FILES ${BS_BANSHEEENGINE_INC_SCRIPT})
source_group("Header Files\\Utility" FILES ${BS_BANSHEEENGINE_INC_UTILITY})
source_group("Source Files\\Renderer" FILES ${BS_BANSHEEENGINE_SRC_RENDERER})
source_group("Source Files\\Input" FILES ${BS_BANSHEEENGINE_SRC_INPUT})
source_group("Header Files\\GUI" FILES ${BS_BANSHEEENGINE_INC_GUI})
source_group("Source Files" FILES ${BS_BANSHEEENGINE_SRC_NOFILTER})
source_group("Source Files\\Platform" FILES ${BS_BANSHEEENGINE_SRC_PLATFORM})
source_group("Source Files\\Debug" FILES ${BS_BANSHEEENGINE_SRC_DEBUG})
source_group("Header Files\\Debug" FILES ${BS_BANSHEEENGINE_INC_DEBUG})
source_group("Source Files\\Localization" FILES ${BS_BANSHEEENGINE_SRC_LOCALIZATION})
source_group("Header Files\\Localization" FILES ${BS_BANSHEEENGINE_INC_LOCALIZATION})

set(BS_BANSHEEENGINE_SRC
	${BS_BANSHEEENGINE_SRC_RESOURCES}
	${BS_BANSHEEENGINE_SRC_SCRIPT}
	${BS_BANSHEEENGINE_INC_INPUT}
	${BS_BANSHEEENGINE_SRC_GUI}
	${BS_BANSHEEENGINE_INC_PLATFORM}
	${BS_BANSHEEENGINE_INC_RENDERER}
	${BS_BANSHEEENGINE_SRC_RTTI}
	${BS_BANSHEEENGINE_SRC_2D}
	${BS_BANSHEEENGINE_SRC_UTILITY}
	${BS_BANSHEEENGINE_INC_2D}
	${BS_BANSHEEENGINE_INC_RTTI}
	${BS_BANSHEEENGINE_INC_NOFILTER}
	${BS_BANSHEEENGINE_INC_RESOURCES}
	${BS_BANSHEEENGINE_INC_SCRIPT}
	${BS_BANSHEEENGINE_INC_UTILITY}
	${BS_BANSHEEENGINE_SRC_RENDERER}
	${BS_BANSHEEENGINE_SRC_INPUT}
	${BS_BANSHEEENGINE_INC_GUI}
	${BS_BANSHEEENGINE_SRC_NOFILTER}
	${BS_BANSHEEENGINE_SRC_PLATFORM}
	${BS_BANSHEEENGINE_INC_DEBUG}
	${BS_BANSHEEENGINE_SRC_DEBUG}
	${BS_BANSHEEENGINE_INC_LOCALIZATION}
	${BS_BANSHEEENGINE_SRC_LOCALIZATION}
)