set(BS_BANSHEEENGINE_INC_COMPONENTS
	"Include/BsCRenderable.h"
	"Include/BsCLight.h"
	"Include/BsCAnimation.h"
	"Include/BsCBone.h"
)

set(BS_BANSHEEENGINE_SRC_RESOURCES
	"Source/BsScriptCodeImportOptions.cpp"
	"Source/BsScriptCodeImporter.cpp"
	"Source/BsScriptCode.cpp"
	"Source/BsPlainTextImporter.cpp"
	"Source/BsPlainText.cpp"
	"Source/BsBuiltinResources.cpp"
	"Source/BsEngineShaderIncludeHandler.cpp"
	"Source/BsGameResourceManager.cpp"
	"Source/BsBuiltinResourcesHelper.cpp"
)

set(BS_BANSHEEENGINE_SRC_SCRIPT
	"Source/BsScriptManager.cpp"
)

set(BS_BANSHEEENGINE_INC_INPUT
	"Include/BsInputConfiguration.h"
	"Include/BsVirtualInput.h"
)

set(BS_BANSHEEENGINE_SRC_GUI
	"Source/BsGUIElement.cpp"
	"Source/BsGUILabel.cpp"
	"Source/BsGUIManager.cpp"
	"Source/BsGUISkin.cpp"
	"Source/BsGUILayout.cpp"
	"Source/BsGUILayoutX.cpp"
	"Source/BsGUILayoutY.cpp"
	"Source/BsGUIButton.cpp"
	"Source/BsGUITexture.cpp"
	"Source/BsGUIToggle.cpp"
	"Source/BsGUIInputBox.cpp"
	"Source/BsGUIMouseEvent.cpp"
	"Source/BsGUIInputCaret.cpp"
	"Source/BsGUIInputSelection.cpp"
	"Source/BsGUIInputTool.cpp"
	"Source/BsGUIElementBase.cpp"
	"Source/BsGUIScrollBarVert.cpp"
	"Source/BsGUIScrollBarHorz.cpp"
	"Source/BsGUIScrollArea.cpp"
	"Source/BsGUIScrollBar.cpp"
	"Source/BsGUIToggleGroup.cpp"
	"Source/BsDragAndDropManager.cpp"
	"Source/BsGUIViewport.cpp"
	"Source/BsGUIContent.cpp"
	"Source/BsGUIListBox.cpp"
	"Source/BsGUIMenu.cpp"
	"Source/BsGUIHelper.cpp"
	"Source/BsGUIDropDownBoxManager.cpp"
	"Source/BsGUIContextMenu.cpp"
	"Source/BsGUIButtonBase.cpp"
	"Source/BsGUITextInputEvent.cpp"
	"Source/BsProfilerOverlay.cpp"
	"Source/BsGUIOptions.cpp"
	"Source/BsGUIRenderTexture.cpp"
	"Source/BsGUIElementContainer.cpp"
	"Source/BsGUIDropDownHitBox.cpp"
	"Source/BsGUILayoutUtility.cpp"
	"Source/BsGUISliderHandle.cpp"
	"Source/BsGUISlider.cpp"
	"Source/BsGUIProgressBar.cpp"
	"Source/BsGUISpace.cpp"
	"Source/BsGUIDropDownContent.cpp"
	"Source/BsGUIElementStyle.cpp"
	"Source/BsGUIDimensions.cpp"
	"Source/BsGUIPanel.cpp"
	"Source/BsGUIDropDownMenu.cpp"
	"Source/BsDropDownAreaPlacement.cpp"
	"Source/BsGUITooltipManager.cpp"
	"Source/BsGUITooltip.cpp"
	"Source/BsGUIWidget.cpp"
	"Source/BsShortcutKey.cpp"
	"Source/BsShortcutManager.cpp"
	"Source/BsCGUIWidget.cpp"
	"Source/BsGUICanvas.cpp"
)

set(BS_BANSHEEENGINE_INC_PLATFORM
	"Include/BsCursor.h"
	"Include/BsSplashScreen.h"
)

set(BS_BANSHEEENGINE_INC_RENDERER
	"Include/BsLight.h"
	"Include/BsRenderable.h"
	"Include/BsRenderableElement.h"
	"Include/BsRenderer.h"
	"Include/BsRendererMaterial.h"
	"Include/BsRendererMaterialManager.h"
	"Include/BsRenderQueue.h"
	"Include/BsSceneManager.h"
	"Include/BsRendererUtility.h"
	"Include/BsStandardPostProcessSettings.h"	
)

set(BS_BANSHEEENGINE_SRC_RTTI
	"Source/BsGUISkinRTTI.cpp"
)

set(BS_BANSHEEENGINE_SRC_2D
	"Source/BsImageSprite.cpp"
	"Source/BsSprite.cpp"
	"Source/BsSpriteTexture.cpp"
	"Source/BsTextSprite.cpp"
	"Source/BsSpriteMaterial.cpp"
	"Source/BsSpriteMaterials.cpp"
	"Source/BsSpriteManager.cpp"
)

set(BS_BANSHEEENGINE_SRC_UTILITY
	"Source/BsDrawHelper.cpp"
	"Source/BsGameSettings.cpp"
	"Source/BsHEString.cpp"
	"Source/BsPaths.cpp"
	"Source/BsShapeMeshes2D.cpp"
	"Source/BsShapeMeshes3D.cpp"
)

set(BS_BANSHEEENGINE_INC_2D
	"Include/BsImageSprite.h"
	"Include/BsSprite.h"
	"Include/BsSpriteTexture.h"
	"Include/BsTextSprite.h"
	"Include/BsSpriteMaterial.h"
	"Include/BsSpriteMaterials.h"
	"Include/BsSpriteManager.h"
)

set(BS_BANSHEEENGINE_INC_RTTI
	"Include/BsSpriteTextureRTTI.h"
	"Include/BsPlainTextRTTI.h"
	"Include/BsScriptCodeRTTI.h"
	"Include/BsScriptCodeImportOptionsRTTI.h"
	"Include/BsGUIElementStyleRTTI.h"
	"Include/BsGUISkinRTTI.h"
	"Include/BsProfilerOverlayRTTI.h"
	"Include/BsCRenderableRTTI.h"
	"Include/BsCLightRTTI.h"
	"Include/BsRenderableRTTI.h"
	"Include/BsLightRTTI.h"
	"Include/BsCGUIWidgetRTTI.h"
	"Include/BsGameSettingsRTTI.h"
	"Include/BsResourceMappingRTTI.h"
	"Include/BsCAnimationRTTI.h"
	"Include/BsCBoneRTTI.h"
	"Include/BsStandardPostProcessSettingsRTTI.h"
)

set(BS_BANSHEEENGINE_INC_NOFILTER
	"Include/BsApplication.h"
	"Include/BsPrerequisites.h"
)

set(BS_BANSHEEENGINE_INC_RESOURCES
	"Include/BsPlainText.h"
	"Include/BsPlainTextImporter.h"
	"Include/BsScriptCode.h"
	"Include/BsScriptCodeImporter.h"
	"Include/BsScriptCodeImportOptions.h"
	"Include/BsBuiltinResources.h"
	"Include/BsEngineShaderIncludeHandler.h"
	"Include/BsGameResourceManager.h"
	"Include/BsBuiltinResourcesHelper.h"
)

set(BS_BANSHEEENGINE_SRC_COMPONENTS
	"Source/BsCRenderable.cpp"
	"Source/BsCLight.cpp"
	"Source/BsCAnimation.cpp"
	"Source/BsCBone.cpp"
)

set(BS_BANSHEEENGINE_INC_SCRIPT
	"Include/BsScriptManager.h"
)

set(BS_BANSHEEENGINE_INC_UTILITY
	"Include/BsDrawHelper.h"
	"Include/BsEnums.h"
	"Include/BsGameSettings.h"
	"Include/BsHEString.h"
	"Include/BsPaths.h"
	"Include/BsRectOffset.h"
	"Include/BsShapeMeshes2D.h"
	"Include/BsShapeMeshes3D.h"
)

set(BS_BANSHEEENGINE_SRC_RENDERER
	"Source/BsLight.cpp"
	"Source/BsRenderable.cpp"
	"Source/BsRenderer.cpp"
	"Source/BsRendererMaterial.cpp"
	"Source/BsRendererMaterialManager.cpp"
	"Source/BsRenderQueue.cpp"
	"Source/BsSceneManager.cpp"
	"Source/BsRendererUtility.cpp"
	"Source/BsStandardPostProcessSettings.cpp"	
)

set(BS_BANSHEEENGINE_SRC_INPUT
	"Source/BsInputConfiguration.cpp"
	"Source/BsVirtualInput.cpp"
)

set(BS_BANSHEEENGINE_INC_GUI
	"Include/BsGUIElement.h"
	"Include/BsGUIElementStyle.h"
	"Include/BsGUILabel.h"
	"Include/BsGUIManager.h"
	"Include/BsGUISkin.h"
	"Include/BsGUILayout.h"
	"Include/BsGUILayoutX.h"
	"Include/BsGUILayoutY.h"
	"Include/BsGUISpace.h"
	"Include/BsGUIButton.h"
	"Include/BsGUITexture.h"
	"Include/BsGUIToggle.h"
	"Include/BsGUIInputBox.h"
	"Include/BsGUICommandEvent.h"
	"Include/BsGUIInputCaret.h"
	"Include/BsGUIInputSelection.h"
	"Include/BsGUIInputTool.h"
	"Include/BsGUIElementBase.h"
	"Include/BsGUIScrollBarVert.h"
	"Include/BsGUIScrollBarHorz.h"
	"Include/BsGUIScrollArea.h"
	"Include/BsGUIScrollBar.h"
	"Include/BsGUIToggleGroup.h"
	"Include/BsDragAndDropManager.h"
	"Include/BsGUIViewport.h"
	"Include/BsGUIContent.h"
	"Include/BsGUIListBox.h"
	"Include/BsGUIMenu.h"
	"Include/BsGUIContextMenu.h"
	"Include/BsGUIHelper.h"
	"Include/BsGUIDropDownBoxManager.h"
	"Include/BsGUIButtonBase.h"
	"Include/BsGUITextInputEvent.h"
	"Include/BsGUIMouseEvent.h"
	"Include/BsProfilerOverlay.h"
	"Include/BsGUIOptions.h"
	"Include/BsGUIRenderTexture.h"
	"Include/BsGUIElementContainer.h"
	"Include/BsGUIDropDownHitBox.h"
	"Include/BsGUIVirtualButtonEvent.h"
	"Include/BsGUILayoutUtility.h"
	"Include/BsGUISliderHandle.h"
	"Include/BsGUISlider.h"
	"Include/BsGUIProgressBar.h"
	"Include/BsGUIDropDownContent.h"
	"Include/BsGUIDimensions.h"
	"Include/BsGUIPanel.h"
	"Include/BsGUILayoutData.h"
	"Include/BsGUIDropDownMenu.h"
	"Include/BsDropDownAreaPlacement.h"
	"Include/BsGUITooltipManager.h"
	"Include/BsGUITooltip.h"
	"Include/BsGUIWidget.h"
	"Include/BsCGUIWidget.h"
	"Include/BsShortcutManager.h"
	"Include/BsShortcutKey.h"
	"Include/BsGUICanvas.h"
)

set(BS_BANSHEEENGINE_SRC_NOFILTER
	"Source/BsApplication.cpp"
)

set(BS_BANSHEEENGINE_SRC_PLATFORM
	"Source/BsCursor.cpp"
	"Source/BsSplashScreen.cpp"
)

source_group("Header Files\\Components" FILES ${BS_BANSHEEENGINE_INC_COMPONENTS})
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
source_group("Source Files\\Components" FILES ${BS_BANSHEEENGINE_SRC_COMPONENTS})
source_group("Header Files\\Script" FILES ${BS_BANSHEEENGINE_INC_SCRIPT})
source_group("Header Files\\Utility" FILES ${BS_BANSHEEENGINE_INC_UTILITY})
source_group("Source Files\\Renderer" FILES ${BS_BANSHEEENGINE_SRC_RENDERER})
source_group("Source Files\\Input" FILES ${BS_BANSHEEENGINE_SRC_INPUT})
source_group("Header Files\\GUI" FILES ${BS_BANSHEEENGINE_INC_GUI})
source_group("Source Files" FILES ${BS_BANSHEEENGINE_SRC_NOFILTER})
source_group("Source Files\\Platform" FILES ${BS_BANSHEEENGINE_SRC_PLATFORM})

set(BS_BANSHEEENGINE_SRC
	${BS_BANSHEEENGINE_INC_COMPONENTS}
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
	${BS_BANSHEEENGINE_SRC_COMPONENTS}
	${BS_BANSHEEENGINE_INC_SCRIPT}
	${BS_BANSHEEENGINE_INC_UTILITY}
	${BS_BANSHEEENGINE_SRC_RENDERER}
	${BS_BANSHEEENGINE_SRC_INPUT}
	${BS_BANSHEEENGINE_INC_GUI}
	${BS_BANSHEEENGINE_SRC_NOFILTER}
	${BS_BANSHEEENGINE_SRC_PLATFORM}
)