set(BS_BANSHEEEDITOR_INC_SETTINGS
	"Settings/BsEditorSettings.h"
	"Settings/BsSettings.h"
	"Settings/BsProjectSettings.h"
)

set(BS_BANSHEEEDITOR_SRC_LIBRARY
	"Library/BsProjectLibrary.cpp"
	"Library/BsProjectLibraryEntries.cpp"
	"Library/BsProjectResourceMeta.cpp"
	"Library/BsEditorShaderIncludeHandler.cpp"
)

set(BS_BANSHEEEDITOR_INC_EDITORWINDOW
	"EditorWindow/BsDockManager.h"
	"EditorWindow/BsDockManagerLayout.h"
	"EditorWindow/BsDropDownWindow.h"
	"EditorWindow/BsDropDownWindowManager.h"
	"EditorWindow/BsEditorWidget.h"
	"EditorWindow/BsEditorWidgetContainer.h"
	"EditorWindow/BsEditorWidgetLayout.h"
	"EditorWindow/BsEditorWidgetManager.h"
	"EditorWindow/BsEditorWindow.h"
	"EditorWindow/BsEditorWindowBase.h"
	"EditorWindow/BsEditorWindowManager.h"
	"EditorWindow/BsMainEditorWindow.h"
	"EditorWindow/BsModalWindow.h"
)

set(BS_BANSHEEEDITOR_INC_SCENE
	"Scene/BsGizmoManager.h"
	"Scene/BsSceneGrid.h"
	"Scene/BsScenePicking.h"
	"Scene/BsSelection.h"
	"Scene/BsSelectionRenderer.h"
)

set(BS_BANSHEEEDITOR_SRC_GUI
	"GUI/BsGUIColor.cpp"
	"GUI/BsGUIColorField.cpp"
	"GUI/BsGUIDockSlider.cpp"
	"GUI/BsGUIDropButton.cpp"
	"GUI/BsGUIFieldBase.cpp"
	"GUI/BsGUIFloatField.cpp"
	"GUI/BsGUIHoverHitBox.cpp"
	"GUI/BsGUIIntField.cpp"
	"GUI/BsGUIListBoxField.cpp"
	"GUI/BsGUIMenuBar.cpp"
	"GUI/BsGUIResourceTreeView.cpp"
	"GUI/BsGUISceneTreeView.cpp"
	"GUI/BsGUISliderField.cpp"
	"GUI/BsGUIStatusBar.cpp"
	"GUI/BsGUITabbedTitleBar.cpp"
	"GUI/BsGUITabButton.cpp"
	"GUI/BsGUITextField.cpp"
	"GUI/BsGUIToggleField.cpp"
	"GUI/BsGUITreeView.cpp"
	"GUI/BsGUITreeViewEditBox.cpp"
	"GUI/BsGUIVector2Field.cpp"
	"GUI/BsGUIVector3Field.cpp"
	"GUI/BsGUIVector4Field.cpp"
	"GUI/BsGUIWindowFrame.cpp"
	"GUI/BsGUIWindowFrameWidget.cpp"
)

set(BS_BANSHEEEDITOR_INC_LIBRARY
	"Library/BsProjectLibrary.h"
	"Library/BsProjectLibraryEntries.h"
	"Library/BsProjectResourceMeta.h"
	"Library/BsEditorShaderIncludeHandler.h"
)

set(BS_BANSHEEEDITOR_INC_GUI
	"GUI/BsGUIColor.h"
	"GUI/BsGUIColorField.h"
	"GUI/BsGUIDockSlider.h"
	"GUI/BsGUIDropButton.h"
	"GUI/BsGUIFieldBase.h"
	"GUI/BsGUIFloatField.h"
	"GUI/BsGUIHoverHitBox.h"
	"GUI/BsGUIIntField.h"
	"GUI/BsGUIListBoxField.h"
	"GUI/BsGUIMenuBar.h"
	"GUI/BsGUIResourceTreeView.h"
	"GUI/BsGUISceneTreeView.h"
	"GUI/BsGUISliderField.h"
	"GUI/BsGUIStatusBar.h"
	"GUI/BsGUITabbedTitleBar.h"
	"GUI/BsGUITabButton.h"
	"GUI/BsGUITextField.h"
	"GUI/BsGUIToggleField.h"
	"GUI/BsGUITreeView.h"
	"GUI/BsGUITreeViewEditBox.h"
	"GUI/BsGUIVector2Field.h"
	"GUI/BsGUIVector3Field.h"
	"GUI/BsGUIVector4Field.h"
	"GUI/BsGUIWindowFrame.h"
	"GUI/BsGUIWindowFrameWidget.h"
)

set(BS_BANSHEEEDITOR_INC_UNDOREDO
	"UndoRedo/BsEditorCommand.h"
	"UndoRedo/BsCmdReparentSO.h"
	"UndoRedo/BsCmdInputFieldValueChange.h"
	"UndoRedo/BsCmdRecordSO.h"
	"UndoRedo/BsCmdDeleteSO.h"
	"UndoRedo/BsCmdCreateSO.h"
	"UndoRedo/BsCmdCloneSO.h"
	"UndoRedo/BsCmdInstantiateSO.h"
	"UndoRedo/BsCmdBreakPrefab.h"
	"UndoRedo/BsUndoRedo.h"
)

set(BS_BANSHEEEDITOR_INC_RTTI
	"RTTI/BsPlatformInfoRTTI.h"
	"RTTI/BsBuildDataRTTI.h"
	"RTTI/BsDockManagerLayoutRTTI.h"
	"RTTI/BsEditorWidgetLayoutRTTI.h"
	"RTTI/BsProjectLibraryEntriesRTTI.h"
	"RTTI/BsProjectResourceMetaRTTI.h"
	"RTTI/BsEditorSettingsRTTI.h"
	"RTTI/BsSettingsRTTI.h"
	"RTTI/BsProjectSettingsRTTI.h"
	"RTTI/BsGUIWindowFrameWidgetRTTI.h"
)

set(BS_BANSHEEEDITOR_SRC_NOFILTER
	"BsEditorApplication.cpp"
)

set(BS_BANSHEEEDITOR_SRC_SCENEVIEW
	"SceneView/BsGizmoManager.cpp"
	"SceneView/BsSelectionRenderer.cpp"
	"SceneView/BsSelection.cpp"
	"SceneView/BsScenePicking.cpp"
	"SceneView/BsSceneGrid.cpp"
)

set(BS_BANSHEEEDITOR_INC_NOFILTER
	"BsEditorApplication.h"
	"BsEditorPrerequisites.h"
)

set(BS_BANSHEEEDITOR_INC_HANDLES
	"Handles/BsHandleDrawManager.h"
	"Handles/BsHandleManager.h"
	"Handles/BsHandleSlider.h"
	"Handles/BsHandleSliderDisc.h"
	"Handles/BsHandleSliderLine.h"
	"Handles/BsHandleSliderManager.h"
	"Handles/BsHandleSliderPlane.h"
)

set(BS_BANSHEEEDITOR_SRC_UTILITY
	"Utility/BsEditorUtility.cpp"
)

set(BS_BANSHEEEDITOR_SRC_EDITORWINDOW
	"EditorWindow/BsDockManager.cpp"
	"EditorWindow/BsDockManagerLayout.cpp"
	"EditorWindow/BsDropDownWindow.cpp"
	"EditorWindow/BsDropDownWindowManager.cpp"
	"EditorWindow/BsModalWindow.cpp"
	"EditorWindow/BsMainEditorWindow.cpp"
	"EditorWindow/BsEditorWindowManager.cpp"
	"EditorWindow/BsEditorWindowBase.cpp"
	"EditorWindow/BsEditorWindow.cpp"
	"EditorWindow/BsEditorWidgetManager.cpp"
	"EditorWindow/BsEditorWidgetLayout.cpp"
	"EditorWindow/BsEditorWidgetContainer.cpp"
	"EditorWindow/BsEditorWidget.cpp"
)

set(BS_BANSHEEEDITOR_INC_UTILITY
	"Utility/BsEditorUtility.h"
	"Utility/BsBuiltinEditorResources.h"
)

set(BS_BANSHEEEDITOR_SRC_TESTING
	"Testing/BsEditorTestSuite.cpp"
)

set(BS_BANSHEEEDITOR_SRC_SETTINGS
	"Settings/BsEditorSettings.cpp"
	"Settings/BsProjectSettings.cpp"
	"Settings/BsSettings.cpp"
)

set(BS_BANSHEEEDITOR_SRC_UNDOREDO
	"UndoRedo/BsEditorCommand.cpp"
	"UndoRedo/BsCmdReparentSO.cpp"
	"UndoRedo/BsCmdRecordSO.cpp"
	"UndoRedo/BsCmdDeleteSO.cpp"
	"UndoRedo/BsCmdCreateSO.cpp"
	"UndoRedo/BsCmdCloneSO.cpp"
	"UndoRedo/BsCmdInstantiateSO.cpp"
	"UndoRedo/BsCmdBreakPrefab.cpp"
	"UndoRedo/BsUndoRedo.cpp"
)

set(BS_BANSHEEEDITOR_INC_BUILD
	"Build/BsBuildManager.h"
	"Build/BsPlatformInfo.h"
)

set(BS_BANSHEEEDITOR_SRC_BUILD
	"Build/BsBuildManager.cpp"
	"Build/BsBuiltinEditorResources.cpp"
	"Build/BsPlatformInfo.cpp"
)

set(BS_BANSHEEEDITOR_SRC_HANDLES
	"Handles/BsHandleDrawManager.cpp"
	"Handles/BsHandleManager.cpp"
	"Handles/BsHandleSlider.cpp"
	"Handles/BsHandleSliderDisc.cpp"
	"Handles/BsHandleSliderLine.cpp"
	"Handles/BsHandleSliderManager.cpp"
	"Handles/BsHandleSliderPlane.cpp"
)

set(BS_BANSHEEEDITOR_INC_TESTING
	"Testing/BsEditorTestSuite.h"
)

set(BS_BANSHEEEDITOR_INC_CODEEDITOR
	"CodeEditor/BsCodeEditor.h"
	"CodeEditor/BsMDCodeEditor.h"
)

set(BS_BANSHEEEDITOR_SRC_CODEEDITOR
	"CodeEditor/BsCodeEditor.cpp"
	"CodeEditor/BsMDCodeEditor.cpp"
)

set(BS_BANSHEEEDITOR_INC_WIN32
	"Win32/BsVSCodeEditor.h"
)

set(BS_BANSHEEEDITOR_SRC_WIN32
	"Win32/BsVSCodeEditor.cpp"
	"Win32/BsWin32BrowseDialogs.cpp"
)

set(BS_BANSHEEEDITOR_INC_LINUX
)

set(BS_BANSHEEEDITOR_SRC_LINUX
	"Linux/BsLinuxBrowseDialogs.cpp"
)

if(WIN32)
	list(APPEND BS_BANSHEEEDITOR_INC_PLATFORM ${BS_BANSHEEEDITOR_INC_WIN32})
	list(APPEND BS_BANSHEEEDITOR_SRC_PLATFORM ${BS_BANSHEEEDITOR_SRC_WIN32})
elseif(LINUX)
	list(APPEND BS_BANSHEEEDITOR_INC_PLATFORM ${BS_BANSHEEEDITOR_INC_LINUX})
	list(APPEND BS_BANSHEEEDITOR_SRC_PLATFORM ${BS_BANSHEEEDITOR_SRC_LINUX})
endif()

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
source_group("Header Files\\Win32" FILES ${BS_BANSHEEEDITOR_INC_WIN32})
source_group("Source Files\\Win32" FILES ${BS_BANSHEEEDITOR_SRC_WIN32})

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
	${BS_BANSHEEEDITOR_INC_PLATFORM}
	${BS_BANSHEEEDITOR_SRC_PLATFORM}
)
