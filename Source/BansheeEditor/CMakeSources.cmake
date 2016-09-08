set(BS_BANSHEEEDITOR_INC_SETTINGS
	"Include/BsEditorSettings.h"
	"Include/BsSettings.h"
	"Include/BsProjectSettings.h"
)

set(BS_BANSHEEEDITOR_SRC_LIBRARY
	"Source/BsProjectLibrary.cpp"
	"Source/BsProjectLibraryEntries.cpp"
	"Source/BsProjectResourceMeta.cpp"
	"Source/BsEditorShaderIncludeHandler.cpp"
)

set(BS_BANSHEEEDITOR_INC_EDITORWINDOW
	"Include/BsDockManager.h"
	"Include/BsDockManagerLayout.h"
	"Include/BsDropDownWindow.h"
	"Include/BsDropDownWindowManager.h"
	"Include/BsEditorWidget.h"
	"Include/BsEditorWidgetContainer.h"
	"Include/BsEditorWidgetLayout.h"
	"Include/BsEditorWidgetManager.h"
	"Include/BsEditorWindow.h"
	"Include/BsEditorWindowBase.h"
	"Include/BsEditorWindowManager.h"
	"Include/BsMainEditorWindow.h"
	"Include/BsModalWindow.h"
)

set(BS_BANSHEEEDITOR_SRC_CODEEDITOR
	"Source/BsCodeEditor.cpp"
	"Source/Win32/BsVSCodeEditor.cpp"
)

set(BS_BANSHEEEDITOR_INC_SCENE
	"Include/BsGizmoManager.h"
	"Include/BsSceneGrid.h"
	"Include/BsScenePicking.h"
	"Include/BsSelection.h"
	"Include/BsSelectionRenderer.h"
)

set(BS_BANSHEEEDITOR_SRC_GUI
	"Source/BsGUIColor.cpp"
	"Source/BsGUIColorField.cpp"
	"Source/BsGUIDockSlider.cpp"
	"Source/BsGUIDropButton.cpp"
	"Source/BsGUIFieldBase.cpp"
	"Source/BsGUIFloatField.cpp"
	"Source/BsGUIHoverHitBox.cpp"
	"Source/BsGUIIntField.cpp"
	"Source/BsGUIListBoxField.cpp"
	"Source/BsGUIMenuBar.cpp"
	"Source/BsGUIResourceTreeView.cpp"
	"Source/BsGUISceneTreeView.cpp"
	"Source/BsGUISliderField.cpp"
	"Source/BsGUIStatusBar.cpp"
	"Source/BsGUITabbedTitleBar.cpp"
	"Source/BsGUITabButton.cpp"
	"Source/BsGUITextField.cpp"
	"Source/BsGUIToggleField.cpp"
	"Source/BsGUITreeView.cpp"
	"Source/BsGUITreeViewEditBox.cpp"
	"Source/BsGUIVector2Field.cpp"
	"Source/BsGUIVector3Field.cpp"
	"Source/BsGUIVector4Field.cpp"
	"Source/BsGUIWindowFrame.cpp"
	"Source/BsGUIWindowFrameWidget.cpp"
)

set(BS_BANSHEEEDITOR_INC_LIBRARY
	"Include/BsProjectLibrary.h"
	"Include/BsProjectLibraryEntries.h"
	"Include/BsProjectResourceMeta.h"
	"Include/BsEditorShaderIncludeHandler.h"
)

set(BS_BANSHEEEDITOR_INC_GUI
	"Include/BsGUIColor.h"
	"Include/BsGUIColorField.h"
	"Include/BsGUIDockSlider.h"
	"Include/BsGUIDropButton.h"
	"Include/BsGUIFieldBase.h"
	"Include/BsGUIFloatField.h"
	"Include/BsGUIHoverHitBox.h"
	"Include/BsGUIIntField.h"
	"Include/BsGUIListBoxField.h"
	"Include/BsGUIMenuBar.h"
	"Include/BsGUIResourceTreeView.h"
	"Include/BsGUISceneTreeView.h"
	"Include/BsGUISliderField.h"
	"Include/BsGUIStatusBar.h"
	"Include/BsGUITabbedTitleBar.h"
	"Include/BsGUITabButton.h"
	"Include/BsGUITextField.h"
	"Include/BsGUIToggleField.h"
	"Include/BsGUITreeView.h"
	"Include/BsGUITreeViewEditBox.h"
	"Include/BsGUIVector2Field.h"
	"Include/BsGUIVector3Field.h"
	"Include/BsGUIVector4Field.h"
	"Include/BsGUIWindowFrame.h"
	"Include/BsGUIWindowFrameWidget.h"
)

set(BS_BANSHEEEDITOR_INC_UNDOREDO
	"Include/BsEditorCommand.h"
	"Include/BsCmdReparentSO.h"
	"Include/BsCmdInputFieldValueChange.h"
	"Include/BsCmdRecordSO.h"
	"Include/BsCmdDeleteSO.h"
	"Include/BsCmdCreateSO.h"
	"Include/BsCmdCloneSO.h"
	"Include/BsCmdInstantiateSO.h"
	"Include/BsCmdBreakPrefab.h"
	"Include/BsUndoRedo.h"
)

set(BS_BANSHEEEDITOR_INC_RTTI
	"Include/BsPlatformInfoRTTI.h"
	"Include/BsBuildDataRTTI.h"
	"Include/BsDockManagerLayoutRTTI.h"
	"Include/BsEditorWidgetLayoutRTTI.h"
	"Include/BsProjectLibraryEntriesRTTI.h"
	"Include/BsProjectResourceMetaRTTI.h"
	"Include/BsEditorSettingsRTTI.h"
	"Include/BsSettingsRTTI.h"
	"Include/BsProjectSettingsRTTI.h"
	"Include/BsGUIWindowFrameWidgetRTTI.h"
)

set(BS_BANSHEEEDITOR_SRC_NOFILTER
	"Source/BsEditorApplication.cpp"
)

set(BS_BANSHEEEDITOR_SRC_SCENEVIEW
	"Source/BsGizmoManager.cpp"
	"Source/BsSelectionRenderer.cpp"
	"Source/BsSelection.cpp"
	"Source/BsScenePicking.cpp"
	"Source/BsSceneGrid.cpp"
)

set(BS_BANSHEEEDITOR_INC_NOFILTER
	"Include/BsEditorApplication.h"
	"Include/BsEditorPrerequisites.h"
)

set(BS_BANSHEEEDITOR_INC_HANDLES
	"Include/BsHandleDrawManager.h"
	"Include/BsHandleManager.h"
	"Include/BsHandleSlider.h"
	"Include/BsHandleSliderDisc.h"
	"Include/BsHandleSliderLine.h"
	"Include/BsHandleSliderManager.h"
	"Include/BsHandleSliderPlane.h"
)

set(BS_BANSHEEEDITOR_SRC_UTILITY
	"Source/BsEditorUtility.cpp"
)

set(BS_BANSHEEEDITOR_SRC_EDITORWINDOW
	"Source/BsDockManager.cpp"
	"Source/BsDockManagerLayout.cpp"
	"Source/BsDropDownWindow.cpp"
	"Source/BsDropDownWindowManager.cpp"
	"Source/BsModalWindow.cpp"
	"Source/BsMainEditorWindow.cpp"
	"Source/BsEditorWindowManager.cpp"
	"Source/BsEditorWindowBase.cpp"
	"Source/BsEditorWindow.cpp"
	"Source/BsEditorWidgetManager.cpp"
	"Source/BsEditorWidgetLayout.cpp"
	"Source/BsEditorWidgetContainer.cpp"
	"Source/BsEditorWidget.cpp"
)

set(BS_BANSHEEEDITOR_INC_UTILITY
	"Include/BsEditorUtility.h"
	"Include/BsBuiltinEditorResources.h"
)

set(BS_BANSHEEEDITOR_SRC_TESTING
	"Source/BsEditorTestSuite.cpp"
)

set(BS_BANSHEEEDITOR_SRC_SETTINGS
	"Source/BsEditorSettings.cpp"
	"Source/BsProjectSettings.cpp"
	"Source/BsSettings.cpp"
)

set(BS_BANSHEEEDITOR_SRC_UNDOREDO
	"Source/BsEditorCommand.cpp"
	"Source/BsCmdReparentSO.cpp"
	"Source/BsCmdRecordSO.cpp"
	"Source/BsCmdDeleteSO.cpp"
	"Source/BsCmdCreateSO.cpp"
	"Source/BsCmdCloneSO.cpp"
	"Source/BsCmdInstantiateSO.cpp"
	"Source/BsCmdBreakPrefab.cpp"
	"Source/BsUndoRedo.cpp"
)

set(BS_BANSHEEEDITOR_INC_CODEEDITOR
	"Include/BsCodeEditor.h"
	"Include/Win32/BsVSCodeEditor.h"
)

set(BS_BANSHEEEDITOR_INC_BUILD
	"Include/BsBuildManager.h"
	"Include/BsPlatformInfo.h"
)

set(BS_BANSHEEEDITOR_SRC_BUILD
	"Source/BsBuildManager.cpp"
	"Source/BsBuiltinEditorResources.cpp"
	"Source/BsPlatformInfo.cpp"
)

set(BS_BANSHEEEDITOR_SRC_HANDLES
	"Source/BsHandleDrawManager.cpp"
	"Source/BsHandleManager.cpp"
	"Source/BsHandleSlider.cpp"
	"Source/BsHandleSliderDisc.cpp"
	"Source/BsHandleSliderLine.cpp"
	"Source/BsHandleSliderManager.cpp"
	"Source/BsHandleSliderPlane.cpp"
)

set(BS_BANSHEEEDITOR_INC_TESTING
	"Include/BsEditorTestSuite.h"
)

source_group("Header Files\\Settings" FILES ${BS_BANSHEEEDITOR_INC_SETTINGS})
source_group("Source Files\\Library" FILES ${BS_BANSHEEEDITOR_SRC_LIBRARY})
source_group("Header Files\\EditorWindow" FILES ${BS_BANSHEEEDITOR_INC_EDITORWINDOW})
source_group("Source Files\\CodeEditor" FILES ${BS_BANSHEEEDITOR_SRC_CODEEDITOR})
source_group("Header Files\\Scene" FILES ${BS_BANSHEEEDITOR_INC_SCENE})
source_group("Source Files\\GUI" FILES ${BS_BANSHEEEDITOR_SRC_GUI})
source_group("Header Files\\Library" FILES ${BS_BANSHEEEDITOR_INC_LIBRARY})
source_group("Header Files\\GUI" FILES ${BS_BANSHEEEDITOR_INC_GUI})
source_group("Header Files\\UndoRedo" FILES ${BS_BANSHEEEDITOR_INC_UNDOREDO})
source_group("Header Files\\RTTI" FILES ${BS_BANSHEEEDITOR_INC_RTTI})
source_group("Source Files" FILES ${BS_BANSHEEEDITOR_SRC_NOFILTER})
source_group("Source Files\\SceneView" FILES ${BS_BANSHEEEDITOR_SRC_SCENEVIEW})
source_group("Header Files" FILES ${BS_BANSHEEEDITOR_INC_NOFILTER})
source_group("Header Files\\Handles" FILES ${BS_BANSHEEEDITOR_INC_HANDLES})
source_group("Source Files\\Utility" FILES ${BS_BANSHEEEDITOR_SRC_UTILITY})
source_group("Source Files\\EditorWindow" FILES ${BS_BANSHEEEDITOR_SRC_EDITORWINDOW})
source_group("Header Files\\Utility" FILES ${BS_BANSHEEEDITOR_INC_UTILITY})
source_group("Source Files\\Testing" FILES ${BS_BANSHEEEDITOR_SRC_TESTING})
source_group("Source Files\\Settings" FILES ${BS_BANSHEEEDITOR_SRC_SETTINGS})
source_group("Source Files\\UndoRedo" FILES ${BS_BANSHEEEDITOR_SRC_UNDOREDO})
source_group("Header Files\\CodeEditor" FILES ${BS_BANSHEEEDITOR_INC_CODEEDITOR})
source_group("Header Files\\Build" FILES ${BS_BANSHEEEDITOR_INC_BUILD})
source_group("Source Files\\Build" FILES ${BS_BANSHEEEDITOR_SRC_BUILD})
source_group("Source Files\\Handles" FILES ${BS_BANSHEEEDITOR_SRC_HANDLES})
source_group("Header Files\\Testing" FILES ${BS_BANSHEEEDITOR_INC_TESTING})

set(BS_BANSHEEEDITOR_SRC
	${BS_BANSHEEEDITOR_INC_SETTINGS}
	${BS_BANSHEEEDITOR_SRC_LIBRARY}
	${BS_BANSHEEEDITOR_INC_EDITORWINDOW}
	${BS_BANSHEEEDITOR_SRC_CODEEDITOR}
	${BS_BANSHEEEDITOR_INC_SCENE}
	${BS_BANSHEEEDITOR_SRC_GUI}
	${BS_BANSHEEEDITOR_INC_LIBRARY}
	${BS_BANSHEEEDITOR_INC_GUI}
	${BS_BANSHEEEDITOR_INC_UNDOREDO}
	${BS_BANSHEEEDITOR_INC_RTTI}
	${BS_BANSHEEEDITOR_SRC_NOFILTER}
	${BS_BANSHEEEDITOR_SRC_SCENEVIEW}
	${BS_BANSHEEEDITOR_INC_NOFILTER}
	${BS_BANSHEEEDITOR_INC_HANDLES}
	${BS_BANSHEEEDITOR_SRC_UTILITY}
	${BS_BANSHEEEDITOR_SRC_EDITORWINDOW}
	${BS_BANSHEEEDITOR_INC_UTILITY}
	${BS_BANSHEEEDITOR_SRC_TESTING}
	${BS_BANSHEEEDITOR_SRC_SETTINGS}
	${BS_BANSHEEEDITOR_SRC_UNDOREDO}
	${BS_BANSHEEEDITOR_INC_CODEEDITOR}
	${BS_BANSHEEEDITOR_INC_BUILD}
	${BS_BANSHEEEDITOR_SRC_BUILD}
	${BS_BANSHEEEDITOR_SRC_HANDLES}
	${BS_BANSHEEEDITOR_INC_TESTING}
)