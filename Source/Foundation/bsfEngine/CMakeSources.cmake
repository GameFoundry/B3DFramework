set(BS_ENGINE_SRC_RESOURCES
	"bsfEngine/Resources/BsScriptCodeImportOptions.cpp"
	"bsfEngine/Resources/BsScriptCodeImporter.cpp"
	"bsfEngine/Resources/BsScriptCode.cpp"
	"bsfEngine/Resources/BsPlainTextImporter.cpp"
	"bsfEngine/Resources/BsPlainText.cpp"
	"bsfEngine/Resources/BsBuiltinResources.cpp"
	"bsfEngine/Resources/BsEngineShaderIncludeHandler.cpp"
	"bsfEngine/Resources/BsGameResourceManager.cpp"
	"bsfEngine/Resources/BsBuiltinResourcesHelper.cpp"
)

set(BS_ENGINE_SRC_SCRIPT
	"bsfEngine/Script/BsScriptManager.cpp"
)

set(BS_ENGINE_INC_INPUT
	"bsfEngine/Input/BsInputConfiguration.h"
	"bsfEngine/Input/BsVirtualInput.h"
)

set(BS_ENGINE_SRC_GUI
	"bsfEngine/GUI/BsGUIElement.cpp"
	"bsfEngine/GUI/BsGUILabel.cpp"
	"bsfEngine/GUI/BsGUIManager.cpp"
	"bsfEngine/GUI/BsGUISkin.cpp"
	"bsfEngine/GUI/BsGUILayout.cpp"
	"bsfEngine/GUI/BsGUILayoutX.cpp"
	"bsfEngine/GUI/BsGUILayoutY.cpp"
	"bsfEngine/GUI/BsGUIButton.cpp"
	"bsfEngine/GUI/BsGUITexture.cpp"
	"bsfEngine/GUI/BsGUIToggle.cpp"
	"bsfEngine/GUI/BsGUIInputBox.cpp"
	"bsfEngine/GUI/BsGUIMouseEvent.cpp"
	"bsfEngine/GUI/BsGUIInputCaret.cpp"
	"bsfEngine/GUI/BsGUIInputSelection.cpp"
	"bsfEngine/GUI/BsGUIInputTool.cpp"
	"bsfEngine/GUI/BsGUIElementBase.cpp"
	"bsfEngine/GUI/BsGUIScrollBarVert.cpp"
	"bsfEngine/GUI/BsGUIScrollBarHorz.cpp"
	"bsfEngine/GUI/BsGUIScrollArea.cpp"
	"bsfEngine/GUI/BsGUIScrollBar.cpp"
	"bsfEngine/GUI/BsGUIToggleGroup.cpp"
	"bsfEngine/GUI/BsDragAndDropManager.cpp"
	"bsfEngine/GUI/BsGUIViewport.cpp"
	"bsfEngine/GUI/BsGUIContent.cpp"
	"bsfEngine/GUI/BsGUIListBox.cpp"
	"bsfEngine/GUI/BsGUIMenu.cpp"
	"bsfEngine/GUI/BsGUIHelper.cpp"
	"bsfEngine/GUI/BsGUIDropDownBoxManager.cpp"
	"bsfEngine/GUI/BsGUIContextMenu.cpp"
	"bsfEngine/GUI/BsGUIButtonBase.cpp"
	"bsfEngine/GUI/BsGUITextInputEvent.cpp"
	"bsfEngine/GUI/BsProfilerOverlay.cpp"
	"bsfEngine/GUI/BsGUIOptions.cpp"
	"bsfEngine/GUI/BsGUIRenderTexture.cpp"
	"bsfEngine/GUI/BsGUIElementContainer.cpp"
	"bsfEngine/GUI/BsGUIDropDownHitBox.cpp"
	"bsfEngine/GUI/BsGUILayoutUtility.cpp"
	"bsfEngine/GUI/BsGUISliderHandle.cpp"
	"bsfEngine/GUI/BsGUISlider.cpp"
	"bsfEngine/GUI/BsGUIProgressBar.cpp"
	"bsfEngine/GUI/BsGUISpace.cpp"
	"bsfEngine/GUI/BsGUIDropDownContent.cpp"
	"bsfEngine/GUI/BsGUIElementStyle.cpp"
	"bsfEngine/GUI/BsGUIDimensions.cpp"
	"bsfEngine/GUI/BsGUIPanel.cpp"
	"bsfEngine/GUI/BsGUIDropDownMenu.cpp"
	"bsfEngine/GUI/BsDropDownAreaPlacement.cpp"
	"bsfEngine/GUI/BsGUITooltipManager.cpp"
	"bsfEngine/GUI/BsGUITooltip.cpp"
	"bsfEngine/GUI/BsGUIWidget.cpp"
	"bsfEngine/GUI/BsShortcutKey.cpp"
	"bsfEngine/GUI/BsShortcutManager.cpp"
	"bsfEngine/GUI/BsCGUIWidget.cpp"
	"bsfEngine/GUI/BsGUICanvas.cpp"
	"bsfEngine/GUI/BsGUINavGroup.cpp"
)

set(BS_ENGINE_INC_PLATFORM
	"bsfEngine/Platform/BsCursor.h"
)

set(BS_ENGINE_INC_RENDERER
	"bsfEngine/Renderer/BsRendererMaterial.h"
	"bsfEngine/Renderer/BsRendererMaterialManager.h"
	"bsfEngine/Renderer/BsRenderElement.h"
	"bsfEngine/Renderer/BsRenderQueue.h"
	"bsfEngine/Renderer/BsRendererUtility.h"
)

set(BS_ENGINE_SRC_RTTI
	"bsfEngine/Private/RTTI/BsGUISkinRTTI.cpp"
)

set(BS_ENGINE_SRC_2D
	"bsfEngine/2D/BsImageSprite.cpp"
	"bsfEngine/2D/BsSprite.cpp"
	"bsfEngine/2D/BsTextSprite.cpp"
	"bsfEngine/2D/BsSpriteMaterial.cpp"
	"bsfEngine/2D/BsSpriteMaterials.cpp"
	"bsfEngine/2D/BsSpriteManager.cpp"
)

set(BS_ENGINE_SRC_UTILITY
	"bsfEngine/Utility/BsDrawHelper.cpp"
	"bsfEngine/Utility/BsGameSettings.cpp"
	"bsfEngine/Utility/BsPaths.cpp"
	"bsfEngine/Utility/BsShapeMeshes2D.cpp"
	"bsfEngine/Utility/BsShapeMeshes3D.cpp"
)

set(BS_ENGINE_SRC_DEBUG
	"bsfEngine/Debug/BsDebugDraw.cpp"
)

set(BS_ENGINE_SRC_LOCALIZATION
	"bsfEngine/Localization/BsHEString.cpp"
)

set(BS_ENGINE_INC_2D
	"bsfEngine/2D/BsImageSprite.h"
	"bsfEngine/2D/BsSprite.h"
	"bsfEngine/2D/BsTextSprite.h"
	"bsfEngine/2D/BsSpriteMaterial.h"
	"bsfEngine/2D/BsSpriteMaterials.h"
	"bsfEngine/2D/BsSpriteManager.h"
)

set(BS_ENGINE_INC_RTTI
	"bsfEngine/Private/RTTI/BsPlainTextRTTI.h"
	"bsfEngine/Private/RTTI/BsScriptCodeRTTI.h"
	"bsfEngine/Private/RTTI/BsScriptCodeImportOptionsRTTI.h"
	"bsfEngine/Private/RTTI/BsGUIElementStyleRTTI.h"
	"bsfEngine/Private/RTTI/BsGUISkinRTTI.h"
	"bsfEngine/Private/RTTI/BsCGUIWidgetRTTI.h"
	"bsfEngine/Private/RTTI/BsGameSettingsRTTI.h"
	"bsfEngine/Private/RTTI/BsResourceMappingRTTI.h"
)

set(BS_ENGINE_INC_NOFILTER
	"bsfEngine/BsApplication.h"
	"bsfEngine/BsPrerequisites.h"
)

set(BS_ENGINE_INC_RESOURCES
	"bsfEngine/Resources/BsPlainText.h"
	"bsfEngine/Resources/BsPlainTextImporter.h"
	"bsfEngine/Resources/BsScriptCode.h"
	"bsfEngine/Resources/BsScriptCodeImporter.h"
	"bsfEngine/Resources/BsScriptCodeImportOptions.h"
	"bsfEngine/Resources/BsBuiltinResources.h"
	"bsfEngine/Resources/BsEngineShaderIncludeHandler.h"
	"bsfEngine/Resources/BsGameResourceManager.h"
	"bsfEngine/Resources/BsBuiltinResourcesHelper.h"
)

set(BS_ENGINE_INC_SCRIPT
	"bsfEngine/Script/BsScriptManager.h"
)

set(BS_ENGINE_INC_UTILITY
	"bsfEngine/Utility/BsDrawHelper.h"
	"bsfEngine/Utility/BsEnums.h"
	"bsfEngine/Utility/BsGameSettings.h"
	"bsfEngine/Utility/BsPaths.h"
	"bsfEngine/Utility/BsRectOffset.h"
	"bsfEngine/Utility/BsShapeMeshes2D.h"
	"bsfEngine/Utility/BsShapeMeshes3D.h"
)

set(BS_ENGINE_INC_DEBUG
	"bsfEngine/Debug/BsDebugDraw.h"
)

set(BS_ENGINE_INC_LOCALIZATION
	"bsfEngine/Localization/BsHEString.h"
)

set(BS_ENGINE_SRC_RENDERER
	"bsfEngine/Renderer/BsRendererMaterialManager.cpp"
	"bsfEngine/Renderer/BsRenderQueue.cpp"
	"bsfEngine/Renderer/BsRendererUtility.cpp"
)

set(BS_ENGINE_SRC_INPUT
	"bsfEngine/Input/BsInputConfiguration.cpp"
	"bsfEngine/Input/BsVirtualInput.cpp"
)

set(BS_ENGINE_INC_GUI
	"bsfEngine/GUI/BsGUIElement.h"
	"bsfEngine/GUI/BsGUIElementStyle.h"
	"bsfEngine/GUI/BsGUILabel.h"
	"bsfEngine/GUI/BsGUIManager.h"
	"bsfEngine/GUI/BsGUISkin.h"
	"bsfEngine/GUI/BsGUILayout.h"
	"bsfEngine/GUI/BsGUILayoutX.h"
	"bsfEngine/GUI/BsGUILayoutY.h"
	"bsfEngine/GUI/BsGUISpace.h"
	"bsfEngine/GUI/BsGUIButton.h"
	"bsfEngine/GUI/BsGUITexture.h"
	"bsfEngine/GUI/BsGUIToggle.h"
	"bsfEngine/GUI/BsGUIInputBox.h"
	"bsfEngine/GUI/BsGUICommandEvent.h"
	"bsfEngine/GUI/BsGUIInputCaret.h"
	"bsfEngine/GUI/BsGUIInputSelection.h"
	"bsfEngine/GUI/BsGUIInputTool.h"
	"bsfEngine/GUI/BsGUIElementBase.h"
	"bsfEngine/GUI/BsGUIScrollBarVert.h"
	"bsfEngine/GUI/BsGUIScrollBarHorz.h"
	"bsfEngine/GUI/BsGUIScrollArea.h"
	"bsfEngine/GUI/BsGUIScrollBar.h"
	"bsfEngine/GUI/BsGUIToggleGroup.h"
	"bsfEngine/GUI/BsDragAndDropManager.h"
	"bsfEngine/GUI/BsGUIViewport.h"
	"bsfEngine/GUI/BsGUIContent.h"
	"bsfEngine/GUI/BsGUIListBox.h"
	"bsfEngine/GUI/BsGUIMenu.h"
	"bsfEngine/GUI/BsGUIContextMenu.h"
	"bsfEngine/GUI/BsGUIHelper.h"
	"bsfEngine/GUI/BsGUIDropDownBoxManager.h"
	"bsfEngine/GUI/BsGUIButtonBase.h"
	"bsfEngine/GUI/BsGUITextInputEvent.h"
	"bsfEngine/GUI/BsGUIMouseEvent.h"
	"bsfEngine/GUI/BsProfilerOverlay.h"
	"bsfEngine/GUI/BsGUIOptions.h"
	"bsfEngine/GUI/BsGUIRenderTexture.h"
	"bsfEngine/GUI/BsGUIElementContainer.h"
	"bsfEngine/GUI/BsGUIDropDownHitBox.h"
	"bsfEngine/GUI/BsGUIVirtualButtonEvent.h"
	"bsfEngine/GUI/BsGUILayoutUtility.h"
	"bsfEngine/GUI/BsGUISliderHandle.h"
	"bsfEngine/GUI/BsGUISlider.h"
	"bsfEngine/GUI/BsGUIProgressBar.h"
	"bsfEngine/GUI/BsGUIDropDownContent.h"
	"bsfEngine/GUI/BsGUIDimensions.h"
	"bsfEngine/GUI/BsGUIPanel.h"
	"bsfEngine/GUI/BsGUILayoutData.h"
	"bsfEngine/GUI/BsGUIDropDownMenu.h"
	"bsfEngine/GUI/BsDropDownAreaPlacement.h"
	"bsfEngine/GUI/BsGUITooltipManager.h"
	"bsfEngine/GUI/BsGUITooltip.h"
	"bsfEngine/GUI/BsGUIWidget.h"
	"bsfEngine/GUI/BsCGUIWidget.h"
	"bsfEngine/GUI/BsShortcutManager.h"
	"bsfEngine/GUI/BsShortcutKey.h"
	"bsfEngine/GUI/BsGUICanvas.h"
	"bsfEngine/GUI/BsGUINavGroup.h"
)

set(BS_ENGINE_SRC_NOFILTER
	"bsfEngine/BsApplication.cpp"
)

set(BS_ENGINE_SRC_PLATFORM
	"bsfEngine/Platform/BsCursor.cpp"
)

source_group("Resources" FILES ${BS_ENGINE_INC_RESOURCES} ${BS_ENGINE_SRC_RESOURCES})
source_group("Script" FILES ${BS_ENGINE_INC_SCRIPT} ${BS_ENGINE_SRC_SCRIPT})
source_group("Input" FILES ${BS_ENGINE_INC_INPUT} ${BS_ENGINE_SRC_INPUT})
source_group("Platform" FILES ${BS_ENGINE_INC_PLATFORM} ${BS_ENGINE_SRC_PLATFORM})
source_group("Renderer" FILES ${BS_ENGINE_INC_RENDERER} ${BS_ENGINE_SRC_RENDERER})
source_group("2D" FILES ${BS_ENGINE_INC_2D} ${BS_ENGINE_SRC_2D})
source_group("RTTI" FILES ${BS_ENGINE_INC_RTTI} ${BS_ENGINE_SRC_RTTI})
source_group("" FILES ${BS_ENGINE_INC_NOFILTER} ${BS_ENGINE_SRC_NOFILTER})
source_group("Utility" FILES ${BS_ENGINE_INC_UTILITY} ${BS_ENGINE_SRC_UTILITY})
source_group("GUI" FILES ${BS_ENGINE_INC_GUI} ${BS_ENGINE_SRC_GUI})
source_group("Debug" FILES ${BS_ENGINE_INC_DEBUG} ${BS_ENGINE_SRC_DEBUG})
source_group("Localization" FILES ${BS_ENGINE_INC_LOCALIZATION} ${BS_ENGINE_SRC_LOCALIZATION})

set(BS_ENGINE_SRC
	${BS_ENGINE_SRC_RESOURCES}
	${BS_ENGINE_SRC_SCRIPT}
	${BS_ENGINE_INC_INPUT}
	${BS_ENGINE_SRC_GUI}
	${BS_ENGINE_INC_PLATFORM}
	${BS_ENGINE_INC_RENDERER}
	${BS_ENGINE_SRC_RTTI}
	${BS_ENGINE_SRC_2D}
	${BS_ENGINE_SRC_UTILITY}
	${BS_ENGINE_INC_2D}
	${BS_ENGINE_INC_RTTI}
	${BS_ENGINE_INC_NOFILTER}
	${BS_ENGINE_INC_RESOURCES}
	${BS_ENGINE_INC_SCRIPT}
	${BS_ENGINE_INC_UTILITY}
	${BS_ENGINE_SRC_RENDERER}
	${BS_ENGINE_SRC_INPUT}
	${BS_ENGINE_INC_GUI}
	${BS_ENGINE_SRC_NOFILTER}
	${BS_ENGINE_SRC_PLATFORM}
	${BS_ENGINE_INC_DEBUG}
	${BS_ENGINE_SRC_DEBUG}
	${BS_ENGINE_INC_LOCALIZATION}
	${BS_ENGINE_SRC_LOCALIZATION}
)