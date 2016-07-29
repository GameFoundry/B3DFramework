//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.Collections.Generic;
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup Windows
     *  @{
     */

    /// <summary>
    /// Displays animation curve editor window.
    /// </summary>
    [DefaultSize(900, 500)]
    internal class AnimationWindow : EditorWindow
    {
        private const int FIELD_DISPLAY_WIDTH = 200;

        private bool isInitialized;
        private GUIFloatField lengthField;
        private GUIIntField fpsField;
        private GUIFloatField yRangeField;
        private GUIButton addKeyframeBtn;

        private GUIButton addPropertyBtn;
        private GUIButton delPropertyBtn;

        private GUILayout buttonLayout;
        private int buttonLayoutHeight;

        private GUIPanel editorPanel;
        private GUIAnimFieldDisplay guiFieldDisplay;
        private GUICurveEditor guiCurveEditor;

        private Dictionary<string, EdAnimationCurve> curves = new Dictionary<string, EdAnimationCurve>();
        private List<string> selectedFields = new List<string>();

        /// <summary>
        /// Opens the animation window.
        /// </summary>
        [MenuItem("Windows/Animation", ButtonModifier.CtrlAlt, ButtonCode.A, 6000)]
        private static void OpenGameWindow()
        {
            OpenWindow<AnimationWindow>();
        }

        /// <inheritdoc/>
        protected override LocString GetDisplayName()
        {
            return new LocEdString("Animation");
        }

        private void OnInitialize()
        {
            Selection.OnSelectionChanged += OnSelectionChanged;
            EditorInput.OnPointerPressed += OnPointerPressed;
            EditorInput.OnPointerMoved += OnPointerMoved;
            EditorInput.OnPointerReleased += OnPointerReleased;
            EditorInput.OnButtonUp += OnButtonUp;

            Rebuild();
        }

        private void OnEditorUpdate()
        {

        }

        private void OnDestroy()
        {
            Selection.OnSelectionChanged -= OnSelectionChanged;
            EditorInput.OnPointerPressed -= OnPointerPressed;
            EditorInput.OnPointerMoved -= OnPointerMoved;
            EditorInput.OnPointerReleased -= OnPointerReleased;
            EditorInput.OnButtonUp -= OnButtonUp;
        }

        protected override void WindowResized(int width, int height)
        {
            if (!isInitialized)
                return;

            guiFieldDisplay.SetSize(FIELD_DISPLAY_WIDTH, height - buttonLayoutHeight*2);

            int curveEditorWidth = Math.Max(0, width - FIELD_DISPLAY_WIDTH);
            guiCurveEditor.SetSize(curveEditorWidth, height - buttonLayoutHeight);
            guiCurveEditor.Redraw();
        }

        private void Rebuild()
        {
            GUI.Clear();
            selectedFields.Clear();
            curves.Clear();
            isInitialized = false;

            SceneObject selectedSO = Selection.SceneObject;
            if (selectedSO == null)
            {
                GUILabel warningLbl = new GUILabel(new LocEdString("Select an object to animate in the Hierarchy or Scene windows."));

                GUILayoutY vertLayout = GUI.AddLayoutY();
                vertLayout.AddFlexibleSpace();
                GUILayoutX horzLayout = vertLayout.AddLayoutX();
                vertLayout.AddFlexibleSpace();

                horzLayout.AddFlexibleSpace();
                horzLayout.AddElement(warningLbl);
                horzLayout.AddFlexibleSpace();
                
                return;
            }

            // TODO - Retrieve Animation & AnimationClip from the selected object, fill curves dictionary
            //  - If not available, show a button to create new animation clip

            lengthField = new GUIFloatField(new LocEdString("Length"), 50);
            fpsField = new GUIIntField(new LocEdString("FPS"), 50);
            yRangeField = new GUIFloatField(new LocEdString("Y range"), 50);
            addKeyframeBtn = new GUIButton(new LocEdString("Add keyframe"));

            addPropertyBtn = new GUIButton(new LocEdString("Add property"));
            delPropertyBtn = new GUIButton(new LocEdString("Delete selected"));

            lengthField.Value = 60.0f;
            fpsField.Value = 1;
            yRangeField.Value = 20.0f;

            addPropertyBtn.OnClick += () =>
            {
                Vector2I windowPos = ScreenToWindowPos(Input.PointerPosition);
                FieldSelectionWindow fieldSelection = DropDownWindow.Open<FieldSelectionWindow>(this, windowPos);
                fieldSelection.OnFieldSelected += OnFieldAdded;
            };

            delPropertyBtn.OnClick += () =>
            {
                LocEdString title = new LocEdString("Warning");
                LocEdString message = new LocEdString("Are you sure you want to remove all selected fields?");

                DialogBox.Open(title, message, DialogBox.Type.YesNo, x =>
                {
                    if (x == DialogBox.ResultType.Yes)
                    {
                        RemoveSelectedFields();
                    }
                });
            };

            lengthField.OnChanged += x =>
            {
                guiCurveEditor.SetRange(lengthField.Value, yRangeField.Value);
            };
            fpsField.OnChanged += x =>
            {
                guiCurveEditor.SetFPS(x);
            };
            yRangeField.OnChanged += x =>
            {
                guiCurveEditor.SetRange(lengthField.Value, yRangeField.Value);
            };
            addKeyframeBtn.OnClick += () =>
            {
                guiCurveEditor.AddKeyFrameAtMarker();
            };

            GUILayout mainLayout = GUI.AddLayoutY();

            buttonLayout = mainLayout.AddLayoutX();
            buttonLayout.AddSpace(5);
            buttonLayout.AddElement(lengthField);
            buttonLayout.AddSpace(5);
            buttonLayout.AddElement(yRangeField);
            buttonLayout.AddSpace(5);
            buttonLayout.AddElement(fpsField);
            buttonLayout.AddSpace(5);
            buttonLayout.AddElement(addKeyframeBtn);
            buttonLayout.AddSpace(5);

            buttonLayoutHeight = lengthField.Bounds.height;

            GUILayout contentLayout = mainLayout.AddLayoutX();
            GUILayout fieldDisplayLayout = contentLayout.AddLayoutY(GUIOption.FixedWidth(FIELD_DISPLAY_WIDTH));

            guiFieldDisplay = new GUIAnimFieldDisplay(fieldDisplayLayout, FIELD_DISPLAY_WIDTH,
                Height - buttonLayoutHeight * 2, selectedSO);
            guiFieldDisplay.OnEntrySelected += OnFieldSelected;

            GUILayout bottomButtonLayout = fieldDisplayLayout.AddLayoutX();
            bottomButtonLayout.AddElement(addPropertyBtn);
            bottomButtonLayout.AddElement(delPropertyBtn);

            GUILayout curveLayout = contentLayout.AddLayoutY();

            editorPanel = curveLayout.AddPanel();

            int curveEditorWidth = Math.Max(0, Width - FIELD_DISPLAY_WIDTH);
            guiCurveEditor = new GUICurveEditor(this, editorPanel, curveEditorWidth, Height - buttonLayoutHeight);
            guiCurveEditor.Redraw();

            isInitialized = true;
        }

        private void OnPointerPressed(PointerEvent ev)
        {
            if (!isInitialized)
                return;

            guiCurveEditor.OnPointerPressed(ev);
        }

        private void OnPointerMoved(PointerEvent ev)
        {
            if (!isInitialized)
                return;

            guiCurveEditor.OnPointerMoved(ev);
        }

        private void OnPointerReleased(PointerEvent ev)
        {
            if (!isInitialized)
                return;

            guiCurveEditor.OnPointerReleased(ev);
        }

        private void OnButtonUp(ButtonEvent ev)
        {
            if (!isInitialized)
                return;

            guiCurveEditor.OnButtonUp(ev);
        }

        private void UpdateDisplayedCurves()
        {
            List<EdAnimationCurve> curvesToDisplay = new List<EdAnimationCurve>();
            for (int i = 0; i < selectedFields.Count; i++)
            {
                EdAnimationCurve curve;
                if(curves.TryGetValue(selectedFields[i], out curve))
                    curvesToDisplay.Add(curve);
            }

            guiCurveEditor.SetCurves(curvesToDisplay.ToArray());
            guiCurveEditor.Redraw();
        }

        private void OnFieldAdded(string path, SerializableProperty.FieldType type)
        {
            guiFieldDisplay.AddField(path);

            switch (type)
            {
                case SerializableProperty.FieldType.Vector4:
                {
                    string[] subPaths = { ".x", ".y", ".z", ".w" };

                    for (int i = 0; i < subPaths.Length; i++)
                    {
                        string subFieldPath = path + subPaths[i];
                        curves[subFieldPath] = new EdAnimationCurve();
                        selectedFields.Add(subFieldPath);
                    }
                }
                    break;
                case SerializableProperty.FieldType.Vector3:
                    {
                        string[] subPaths = { ".x", ".y", ".z" };

                        for (int i = 0; i < subPaths.Length; i++)
                        {
                            string subFieldPath = path + subPaths[i];
                            curves[subFieldPath] = new EdAnimationCurve();
                            selectedFields.Add(subFieldPath);
                        }
                    }
                    break;
                case SerializableProperty.FieldType.Vector2:
                    {
                        string[] subPaths = { ".x", ".y" };

                        for (int i = 0; i < subPaths.Length; i++)
                        {
                            string subFieldPath = path + subPaths[i];
                            curves[subFieldPath] = new EdAnimationCurve();
                            selectedFields.Add(subFieldPath);
                        }
                    }
                    break;
                case SerializableProperty.FieldType.Color:
                    {
                        string[] subPaths = { ".r", ".g", ".b", ".a" };

                        for (int i = 0; i < subPaths.Length; i++)
                        {
                            string subFieldPath = path + subPaths[i];
                            curves[subFieldPath] = new EdAnimationCurve();
                            selectedFields.Add(subFieldPath);
                        }
                    }
                    break;
                default: // Primitive type
                    {
                        curves[path] = new EdAnimationCurve();
                        selectedFields.Add(path);
                    }
                    break;
            }

            UpdateDisplayedCurves();
        }

        private bool IsPathParent(string child, string parent)
        {
            string[] childEntries = child.Split('/', '.');
            string[] parentEntries = parent.Split('/', '.');

            if (parentEntries.Length >= child.Length)
                return false;

            int compareLength = Math.Min(childEntries.Length, parentEntries.Length);
            for (int i = 0; i < compareLength; i++)
            {
                if (childEntries[i] != parentEntries[i])
                    return false;
            }

            return true;
        }

        private void OnFieldSelected(string path)
        {
            if (!Input.IsButtonHeld(ButtonCode.LeftShift) && !Input.IsButtonHeld(ButtonCode.RightShift))
                selectedFields.Clear();

            if (!string.IsNullOrEmpty(path))
            {
                selectedFields.RemoveAll(x => { return x == path || IsPathParent(x, path); });
                selectedFields.Add(path);
            }

            guiFieldDisplay.SetSelection(selectedFields.ToArray());

            UpdateDisplayedCurves();
        }

        private void RemoveSelectedFields()
        {
            for (int i = 0; i < selectedFields.Count; i++)
            {
                selectedFields.Remove(selectedFields[i]);
                curves.Remove(selectedFields[i]);
            }

            List<string> existingFields = new List<string>();
            foreach(var KVP in curves)
                existingFields.Add(KVP.Key);

            guiFieldDisplay.SetFields(existingFields.ToArray());

            selectedFields.Clear();
            UpdateDisplayedCurves();
        }

        private void OnSelectionChanged(SceneObject[] sceneObjects, string[] resourcePaths)
        {
            Rebuild();
        }
    }

    /** @} */
}
