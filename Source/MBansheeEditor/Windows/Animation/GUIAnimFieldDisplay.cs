//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.Collections.Generic;
using System.Text;
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup AnimationEditor
      *  @{
      */

    /// <summary>
    /// Renders a hierarchical list of animation curve fields and displays information about them. Also allows field 
    /// selection.
    /// </summary>
    internal class GUIAnimFieldDisplay
    {
        private SceneObject root;
        private int width;
        private int height;

        private GUIScrollArea scrollArea;
        private List<AnimFieldInfo> fieldInfos = new List<AnimFieldInfo>();

        private GUIAnimFieldEntry[] fields;
        private GUIAnimFieldLayouts layouts;

        /// <summary>
        /// Triggered when the user clicks on an new entry in the field display. Curve field path of the selected entry
        /// is provided.
        /// </summary>
        public Action<string> OnEntrySelected;

        /// <summary>
        /// Creates a new animation field display GUI element and adds it to the provided layout.
        /// </summary>
        /// <param name="layout">Layout to add the GUI element to.</param>
        /// <param name="width">Width of the GUI element, in pixels.</param>
        /// <param name="height">Height of the GUI element, in pixels.</param>
        /// <param name="root">Scene object that the root curve field paths reference.</param>
        public GUIAnimFieldDisplay(GUILayout layout, int width, int height, SceneObject root)
        {
            this.root = root;

            scrollArea = new GUIScrollArea(ScrollBarType.ShowIfDoesntFit, ScrollBarType.NeverShow);
            layout.AddElement(scrollArea);

            SetSize(width, height);
        }

        /// <summary>
        /// Changes the size of the GUI element.
        /// </summary>
        /// <param name="width">Width of the GUI element, in pixels.</param>
        /// <param name="height">Height of the GUI element, in pixels.</param>
        public void SetSize(int width, int height)
        {
            this.width = width;
            this.height = height;

            scrollArea.SetWidth(width);
            scrollArea.SetHeight(height);

            Rebuild();
        }

        /// <summary>
        /// Sets which fields to display.
        /// </summary>
        /// <param name="fields">A list of fields to display.</param>
        public void SetFields(AnimFieldInfo[] fields)
        {
            this.fieldInfos.Clear();
            this.fieldInfos.AddRange(fields);

            Rebuild();
        }

        /// <summary>
        /// Adds a new field to the existing field list, and displays it.
        /// </summary>
        /// <param name="field">Field to append to the field list, and display.</param>
        public void AddField(AnimFieldInfo field)
        {
            bool exists = fieldInfos.Exists(x =>
            {
                return x.path == field.path;
            });

            if (!exists)
            {
                fieldInfos.Add(field);
                Rebuild();
            }
        }

        /// <summary>
        /// Changes the displayed values for each field.
        /// </summary>
        /// <param name="values">Values to assign to the fields. Must match the number of displayed fields. </param>
        public void SetDisplayValues(GUIAnimFieldPathValue[] values)
        {
            for (int i = 0; i < fields.Length; i++)
            {
                string path = fields[i].Path;

                for (int j = 0; j < values.Length; j++)
                {
                    if(path == values[j].path)
                        fields[i].SetValue(values[j].value);
                }
            }
        }

        /// <summary>
        /// Sets which fields should be displayed as selected.
        /// </summary>
        /// <param name="paths">Curve field paths of fields to display as selected.</param>
        public void SetSelection(string[] paths)
        {
            Action<GUIAnimFieldEntry> updateSelection = field =>
            {
                bool foundSelected = false;
                for (int j = 0; j < paths.Length; j++)
                {
                    if (field.Path == paths[j])
                    {
                        field.SetSelection(true);
                        foundSelected = true;
                        break;
                    }
                }

                if (!foundSelected)
                    field.SetSelection(false);
            };

            for (int i = 0; i < fields.Length; i++)
            {
                updateSelection(fields[i]);

                // Check children (only one level allowed)
                GUIAnimFieldEntry[] children = fields[i].GetChildren();
                if (children == null)
                    continue;

                for (int j = 0; j < children.Length; j++)
                    updateSelection(children[j]);
            }
        }

        /// <summary>
        /// Rebuilds the entire GUI based on the current field list and their properties.
        /// </summary>
        private void Rebuild()
        {
            scrollArea.Layout.Clear();
            fields = null;

            if (fieldInfos == null || root == null)
                return;

            GUILabel header = new GUILabel(new LocEdString("Properties"), EditorStyles.Header);
            scrollArea.Layout.AddElement(header);

            layouts = new GUIAnimFieldLayouts();
            GUIPanel rootPanel = scrollArea.Layout.AddPanel();
            GUIPanel mainPanel = rootPanel.AddPanel();
            GUIPanel underlayPanel = rootPanel.AddPanel(1);
            GUIPanel overlayPanel = rootPanel.AddPanel(-1);
            GUIPanel backgroundPanel = rootPanel.AddPanel(2);

            layouts.main = mainPanel.AddLayoutY();
            layouts.underlay = underlayPanel.AddLayoutY();
            layouts.overlay = overlayPanel.AddLayoutY();
            layouts.background = backgroundPanel.AddLayoutY();

            GUIButton catchAll = new GUIButton("", EditorStyles.Blank);
            catchAll.Bounds = new Rect2I(0, 0, width, height - header.Bounds.height);
            catchAll.OnClick += () => OnEntrySelected(null);

            underlayPanel.AddElement(catchAll);

            layouts.main.AddSpace(5);
            layouts.underlay.AddSpace(5);
            layouts.overlay.AddSpace(5);
            layouts.background.AddSpace(3); // Minor hack: Background starts heigher to get it to center better

            fields = new GUIAnimFieldEntry[fieldInfos.Count];
            for (int i = 0; i < fieldInfos.Count; i++)
            {
                if (string.IsNullOrEmpty(fieldInfos[i].path))
                    continue;

                bool entryIsMissing;
                if (fieldInfos[i].isUserCurve)
                {
                    string pathSuffix;
                    SerializableProperty property = Animation.FindProperty(root, fieldInfos[i].path, out pathSuffix);
                    entryIsMissing = property == null;
                }
                else
                    entryIsMissing = false;

                if (!entryIsMissing)
                {
                    Color[] colors = new Color[fieldInfos[i].curveGroup.curveInfos.Length];
                    for (int j = 0; j < fieldInfos[i].curveGroup.curveInfos.Length; j++)
                        colors[j] = fieldInfos[i].curveGroup.curveInfos[j].color;

                    switch (fieldInfos[i].curveGroup.type)
                    {
                        case SerializableProperty.FieldType.Vector2:
                            fields[i] = new GUIAnimVec2Entry(layouts, fieldInfos[i].path, colors);
                            break;
                        case SerializableProperty.FieldType.Vector3:
                            fields[i] = new GUIAnimVec3Entry(layouts, fieldInfos[i].path, colors);
                            break;
                        case SerializableProperty.FieldType.Vector4:
                            fields[i] = new GUIAnimVec4Entry(layouts, fieldInfos[i].path, colors);
                            break;
                        case SerializableProperty.FieldType.Color:
                            fields[i] = new GUIAnimColorEntry(layouts, fieldInfos[i].path, colors);
                            break;
                        case SerializableProperty.FieldType.Bool:
                        case SerializableProperty.FieldType.Int:
                        case SerializableProperty.FieldType.Float:
                        {
                            Color color;
                            if (colors.Length > 0)
                                color = colors[0];
                            else
                                color = Color.White;

                            fields[i] = new GUIAnimSimpleEntry(layouts, fieldInfos[i].path, color);
                            break;
                        }
                    }
                }
                else
                {
                    fields[i] = new GUIAnimMissingEntry(layouts, fieldInfos[i].path);
                }

                if (fields[i] != null)
                    fields[i].OnEntrySelected += OnEntrySelected;
            }

            if (fieldInfos.Count == 0)
            {
                GUILabel warningLbl = new GUILabel(new LocEdString("No properties. Add a new property to begin animating."));

                GUILayoutY vertLayout = layouts.main.AddLayoutY();
                vertLayout.AddFlexibleSpace();
                GUILayoutX horzLayout = vertLayout.AddLayoutX();
                vertLayout.AddFlexibleSpace();

                horzLayout.AddFlexibleSpace();
                horzLayout.AddElement(warningLbl);
                horzLayout.AddFlexibleSpace();
            }

            layouts.main.AddSpace(5);
            layouts.underlay.AddSpace(5);
            layouts.overlay.AddSpace(5);
            layouts.background.AddSpace(5);

            layouts.main.AddFlexibleSpace();
            layouts.underlay.AddFlexibleSpace();
            layouts.overlay.AddFlexibleSpace();
            layouts.background.AddFlexibleSpace();
        }
    }

    /// <summary>
    /// All layouts used for placing field display GUI elements.
    /// </summary>
    internal class GUIAnimFieldLayouts
    {
        public GUILayout main;
        public GUILayout underlay;
        public GUILayout overlay;
        public GUILayout background;
    }

    /// <summary>
    /// Path/value combination used for representing the value of the currently selected frame for a specific curve path.
    /// </summary>
    internal struct GUIAnimFieldPathValue
    {
        public string path;
        public object value;
    }

    /// <summary>
    /// Base class for individual entries in a GUI animation curve field display.
    /// </summary>
    internal abstract class GUIAnimFieldEntry
    {
        private const int MAX_PATH_LENGTH = 30;
        protected const int INDENT_AMOUNT = 10;

        protected string path;
        private GUIButton selectionBtn;
        private GUITexture backgroundTexture;

        private int entryHeight;

        /// <summary>
        /// Triggered when the user selects this entry. Curve field path of the selected entry is provided.
        /// </summary>
        public Action<string> OnEntrySelected;

        /// <summary>
        /// Path of the curve field path this entry represents.
        /// </summary>
        public string Path { get { return path; } }

        /// <summary>
        /// Constructs a new animation field entry and appends the necessary GUI elements to the provided layouts.
        /// </summary>
        /// <param name="layouts">Layouts to append the GUI elements to.</param>
        /// <param name="path">Path of the curve field.</param>
        /// <param name="child">Determines if the element is a root path, or a child (sub) element.</param>
        /// <param name="indentAmount">Determines how much to horizontally indent the element, in pixels.</param>
        public GUIAnimFieldEntry(GUIAnimFieldLayouts layouts, string path, bool child, int indentAmount)
        {
            this.path = path;

            GUILayoutX toggleLayout = layouts.main.AddLayoutX();
            toggleLayout.AddSpace(indentAmount);

            selectionBtn = new GUIButton(GetDisplayName(path, child), EditorStyles.Label, GUIOption.FlexibleWidth());
            selectionBtn.OnClick += () =>
            {
                OnEntrySelected?.Invoke(path);
            };

            toggleLayout.AddElement(selectionBtn);

            entryHeight = selectionBtn.Bounds.height;

            backgroundTexture = new GUITexture(Builtin.WhiteTexture, GUITextureScaleMode.StretchToFit, 
                GUIOption.FlexibleWidth());
            backgroundTexture.SetTint(Color.Transparent);
            backgroundTexture.SetHeight(entryHeight);

            layouts.background.AddElement(backgroundTexture);
        }

        /// <summary>
        /// Hides or shows the element.
        /// </summary>
        /// <param name="on">True to show the element, false otherwise.</param>
        public virtual void Toggle(bool on)
        {
            selectionBtn.Active = on;
            backgroundTexture.Active = on;
        }

        /// <summary>
        /// Toggles whether the entry is displayed as selected, or not selected.
        /// </summary>
        /// <param name="selected">True to display as selected, false otherwise.</param>
        public void SetSelection(bool selected)
        {
            if(selected)
                backgroundTexture.SetTint(Color.DarkCyan);
            else
                backgroundTexture.SetTint(Color.Transparent);
        }

        /// <summary>
        /// Displays a floating point value in the value display.
        /// </summary>
        /// <param name="value">Value to display</param>
        public virtual void SetValue(float value) { }

        /// <summary>
        /// Changes the displayed value next to the element's name.
        /// </summary>
        /// <param name="value"></param>
        public virtual void SetValue(object value) { }

        /// <summary>
        /// Returns all child elements, if this element is complex and has children (e.g. vector).
        /// </summary>
        /// <returns>List of child elements, or null if none.</returns>
        public virtual GUIAnimFieldEntry[] GetChildren()
        {
            return null;
        }

        /// <summary>
        /// Returns the height of this element.
        /// </summary>
        /// <returns>Height of this element, in pixels.</returns>
        protected int GetEntryHeight()
        {
            return entryHeight;
        }

        /// <summary>
        /// Generates a name to display on the element's GUI based on its path.
        /// </summary>
        /// <param name="path">Path of the curve field.</param>
        /// <param name="shortName">True to generate a short path without scene object or component info.</param>
        /// <returns>Text to display on the element's GUI.</returns>
        protected static string GetDisplayName(string path, bool shortName)
        {
            if (string.IsNullOrEmpty(path))
                return "";

            string soName;
            string compName;
            string propertyPath;

            string trimmedPath = path.Trim('/');
            GetNames(trimmedPath, shortName, out soName, out compName, out propertyPath);

            if (propertyPath == null)
                return "";

            if (shortName)
                return propertyPath;
            else
            {
                string truncatedPropPath;
                if (propertyPath.Length > MAX_PATH_LENGTH)
                    truncatedPropPath = "..." + propertyPath.Substring(propertyPath.Length - MAX_PATH_LENGTH);
                else
                    truncatedPropPath = propertyPath;

                if (soName != null)
                {
                    if (compName != null)
                        return soName + "(" + compName + ") - " + truncatedPropPath;
                    else
                        return soName + " - " + truncatedPropPath;
                }
                else
                {
                    if (compName != null)
                        return "(" + compName + ") - " + truncatedPropPath;
                    else
                        return truncatedPropPath;
                }
            }
        }

        /// <summary>
        /// Parses a curve field path and breaks it down relevant components.
        /// </summary>
        /// <param name="path">Curve field path to parse.</param>
        /// <param name="shortName">If true, only the last entry of the property portion of the path will be output in 
        ///                         <paramref name="propertyPath"/>. Otherwise all properties will be output.</param>
        /// <param name="soName">Name of the scene object the path field belongs to.</param>
        /// <param name="compName">Name of the component the path field belongs to, if any.</param>
        /// <param name="propertyPath">A list of properties relative to parent component or scene object, that determine
        ///                            which field does the path reference. If <paramref name="shortName"/> is true, only
        ///                            the last property will be output (if there are multiple).</param>
        protected static void GetNames(string path, bool shortName, out string soName, out string compName, out string propertyPath)
        {
            string[] entries = path.Split('/');

            // Find name of the last scene object in the path
            int pathIdx = 0;
            soName = null;
            for (; pathIdx < entries.Length; pathIdx++)
            {
                string entry = entries[pathIdx];

                if (string.IsNullOrEmpty(entry))
                    continue;

                // Not a scene object, break
                if (entry[0] != '!')
                {
                    if (pathIdx > 0)
                    {
                        string prevEntry = entries[pathIdx - 1];
                        soName = prevEntry.Substring(1, prevEntry.Length - 1);
                    }

                    break;
                }
            }

            if (pathIdx >= entries.Length)
            {
                compName = null;
                propertyPath = null;
                return;
            }

            // If path is referencing a component, find it
            {
                string entry = entries[pathIdx];
                if (entry[0] == ':')
                    compName = entry.Substring(1, entry.Length - 1);
                else
                    compName = null;
            }

            // Look for a field name
            if (compName != null)
            {
                pathIdx++;
                if (pathIdx >= entries.Length)
                {
                    propertyPath = null;
                    return;
                }
            }

            if (shortName)
            {
                if (pathIdx < entries.Length)
                    propertyPath = entries[entries.Length - 1];
                else
                    propertyPath = null;
            }
            else
            {
                StringBuilder pathBuilder = new StringBuilder();
                for (; pathIdx < entries.Length - 1; pathIdx++)
                    pathBuilder.Append(entries[pathIdx] + "/");

                if (pathIdx < entries.Length)
                    pathBuilder.Append(entries[pathIdx]);

                propertyPath = pathBuilder.ToString();
            }
        }
    }

    /// <summary>
    /// Creates GUI for an element in animation field display, that contains no child elements.
    /// </summary>
    internal class GUIAnimSimpleEntry : GUIAnimFieldEntry
    {
        private GUILabel valueDisplay;
        private GUILayoutX underlayLayout;
        private GUILabel overlaySpacing;

        /// <summary>
        /// Constructs a new animation field entry and appends the necessary GUI elements to the provided layouts.
        /// </summary>
        /// <param name="layouts">Layouts to append the GUI elements to.</param>
        /// <param name="path">Path of the curve field.</param>
        /// <param name="color">Color of the path field curve, to display next to the element name.</param>
        /// <param name="child">Determines if the element is a root path, or a child (sub) element.</param>
        public GUIAnimSimpleEntry(GUIAnimFieldLayouts layouts, string path, Color color, bool child = false)
            : base(layouts, path, child, child ? 45 : 30)
        {
            valueDisplay = new GUILabel("", GUIOption.FixedHeight(GetEntryHeight()));
            underlayLayout = layouts.underlay.AddLayoutX();
            underlayLayout.AddSpace(child ? 30 : 15);

            GUITexture colorSquare = new GUITexture(Builtin.WhiteTexture, 
                GUIOption.FixedWidth(10), GUIOption.FixedHeight(10));
            colorSquare.SetTint(color);

            underlayLayout.AddElement(colorSquare);
            underlayLayout.AddFlexibleSpace();
            underlayLayout.AddElement(valueDisplay);
            underlayLayout.AddSpace(50);

            overlaySpacing = new GUILabel("", GUIOption.FixedHeight(GetEntryHeight()));
            layouts.overlay.AddElement(overlaySpacing);
        }

        /// <inheritdoc/>
        public override void Toggle(bool on)
        {
            underlayLayout.Active = on;
            overlaySpacing.Active = on;

            base.Toggle(on);
        }

        /// <inheritdoc/>
        public override void SetValue(float value)
        {
            string strValue = value.ToString("n2");
            valueDisplay.SetContent(strValue);
        }
    }

    /// <summary>
    /// Base class for elements in animation field display, that contain other child elements.
    /// </summary>
    internal class GUIAnimComplexEntry : GUIAnimFieldEntry
    {
        private GUILayout foldoutLayout;
        private GUIToggle foldout;
        private GUILabel underlaySpacing;

        protected GUIAnimSimpleEntry[] children;

        /// <summary>
        /// Constructs a new animation field entry and appends the necessary GUI elements to the provided layouts.
        /// </summary>
        /// <param name="layouts">Layouts to append the GUI elements to.</param>
        /// <param name="path">Path of the curve field.</param>
        /// <param name="childEntries">Sub-path names of the child entries to display.</param>
        /// <param name="colors">Colors of the curves to display, for each child entry.</param>
        public GUIAnimComplexEntry(GUIAnimFieldLayouts layouts, string path, string[] childEntries, Color[] colors)
            : base(layouts, path, false, 20)
        {
            foldout = new GUIToggle("", EditorStyles.Expand);
            foldout.OnToggled += Toggle;

            GUILabel spacer = new GUILabel("", GUIOption.FixedHeight(GetEntryHeight()));

            foldoutLayout = layouts.overlay.AddLayoutX();

            foldoutLayout.AddSpace(5);
            foldoutLayout.AddElement(foldout);
            foldoutLayout.AddElement(spacer);
            foldoutLayout.AddFlexibleSpace();

            underlaySpacing = new GUILabel("", GUIOption.FixedHeight(GetEntryHeight()));
            layouts.underlay.AddElement(underlaySpacing);

            children = new GUIAnimSimpleEntry[childEntries.Length];
            for (int i = 0; i < childEntries.Length; i++)
            {
                Color color;
                if (i < colors.Length)
                    color = colors[i];
                else
                    color = Color.White;

                children[i] = new GUIAnimSimpleEntry(layouts, path + childEntries[i], color, true);
                children[i].OnEntrySelected += x => { OnEntrySelected?.Invoke(x); };
            }

            Toggle(false);
        }

        /// <inheritdoc/>
        public override void Toggle(bool on)
        {
            foreach(var child in children)
                child.Toggle(on);
        }

        /// <inheritdoc/>
        public override GUIAnimFieldEntry[] GetChildren()
        {
            return children;
        }
    }

    /// <summary>
    /// Creates a GUI for displaying a Vector2 curve field in the animation field display GUI element.
    /// </summary>
    internal class GUIAnimVec2Entry : GUIAnimComplexEntry
    {
        /// <summary>
        /// Constructs a new animation field entry and appends the necessary GUI elements to the provided layouts.
        /// </summary>
        /// <param name="layouts">Layouts to append the GUI elements to.</param>
        /// <param name="path">Path of the curve field.</param>
        /// <param name="colors">Colors of the curves to display, for each child entry.</param>
        public GUIAnimVec2Entry(GUIAnimFieldLayouts layouts, string path, Color[] colors)
            : base(layouts, path,  new[] { ".x", ".y" }, colors)
        { }

        /// <inheritdoc/>
        public override void SetValue(object value)
        {
            if (value == null)
                return;

            Vector2 vector = (Vector2)value;
            children[0].SetValue(vector.x);
            children[1].SetValue(vector.y);
        }
    }

    /// <summary>
    /// Creates a GUI for displaying a Vector3 curve field in the animation field display GUI element.
    /// </summary>
    internal class GUIAnimVec3Entry : GUIAnimComplexEntry
    {
        /// <summary>
        /// Constructs a new animation field entry and appends the necessary GUI elements to the provided layouts.
        /// </summary>
        /// <param name="layouts">Layouts to append the GUI elements to.</param>
        /// <param name="path">Path of the curve field.</param>
        /// <param name="colors">Colors of the curves to display, for each child entry.</param>
        public GUIAnimVec3Entry(GUIAnimFieldLayouts layouts, string path, Color[] colors)
            : base(layouts, path, new[] { ".x", ".y", ".z" }, colors)
        { }

        /// <inheritdoc/>
        public override void SetValue(object value)
        {
            if (value == null)
                return;

            Vector3 vector = (Vector3)value;
            children[0].SetValue(vector.x);
            children[1].SetValue(vector.y);
            children[2].SetValue(vector.z);
        }
    }

    /// <summary>
    /// Creates a GUI for displaying a Vector4 curve field in the animation field display GUI element.
    /// </summary>
    internal class GUIAnimVec4Entry : GUIAnimComplexEntry
    {
        /// <summary>
        /// Constructs a new animation field entry and appends the necessary GUI elements to the provided layouts.
        /// </summary>
        /// <param name="layouts">Layouts to append the GUI elements to.</param>
        /// <param name="path">Path of the curve field.</param>
        /// <param name="colors">Colors of the curves to display, for each child entry.</param>
        public GUIAnimVec4Entry(GUIAnimFieldLayouts layouts, string path, Color[] colors)
            : base(layouts, path,  new[] { ".x", ".y", ".z", ".w" }, colors)
        { }

        /// <inheritdoc/>
        public override void SetValue(object value)
        {
            if (value == null)
                return;

            Vector4 vector = (Vector4)value;
            children[0].SetValue(vector.x);
            children[1].SetValue(vector.y);
            children[2].SetValue(vector.z);
            children[3].SetValue(vector.w);
        }
    }

    /// <summary>
    /// Creates a GUI for displaying a Color curve field in the animation field display GUI element.
    /// </summary>
    internal class GUIAnimColorEntry : GUIAnimComplexEntry
    {
        /// <summary>
        /// Constructs a new animation field entry and appends the necessary GUI elements to the provided layouts.
        /// </summary>
        /// <param name="layouts">Layouts to append the GUI elements to.</param>
        /// <param name="path">Path of the curve field.</param>
        /// <param name="colors">Colors of the curves to display, for each child entry.</param>
        public GUIAnimColorEntry(GUIAnimFieldLayouts layouts, string path, Color[] colors)
            : base(layouts, path, new[] { ".r", ".g", ".b", ".a" }, colors)
        { }

        /// <inheritdoc/>
        public override void SetValue(object value)
        {
            if (value == null)
                return;

            Color color = (Color)value;
            children[0].SetValue(color.r);
            children[1].SetValue(color.g);
            children[2].SetValue(color.b);
            children[3].SetValue(color.a);
        }
    }

    /// <summary>
    /// Creates a GUI for displaying an entry in the animation field display GUI element that notifies the user that
    /// a referenced curve field path cannot be found.
    /// </summary>
    internal class GUIAnimMissingEntry : GUIAnimFieldEntry
    {
        private GUILabel missingLabel;
        private GUILayoutX underlayLayout;
        private GUILabel overlaySpacing;

        public GUIAnimMissingEntry(GUIAnimFieldLayouts layouts, string path)
            : base(layouts, path, false, 15)
        {
            missingLabel = new GUILabel("Missing property!", GUIOption.FixedHeight(GetEntryHeight()));
            underlayLayout = layouts.underlay.AddLayoutX();
            underlayLayout.AddFlexibleSpace();
            underlayLayout.AddElement(missingLabel);
            underlayLayout.AddSpace(50);

            overlaySpacing = new GUILabel("", GUIOption.FixedHeight(GetEntryHeight()));
            layouts.overlay.AddElement(overlaySpacing);
        }

        /// <inheritdoc/>
        public override void Toggle(bool on)
        {
            underlayLayout.Active = on;
            overlaySpacing.Active = on;

            base.Toggle(on);
        }
    }

    /// <summary>
    /// Contains information required to display a single curve field entry in the animation field display GUI.
    /// </summary>
    internal struct AnimFieldInfo
    {
        public AnimFieldInfo(string path, FieldAnimCurves curveGroup, bool isUserCurve)
        {
            this.path = path;
            this.curveGroup = curveGroup;
            this.isUserCurve = isUserCurve;
        }

        public string path;
        public FieldAnimCurves curveGroup;
        public bool isUserCurve;
    }

    /** @} */
}