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

    internal class GUIAnimFieldDisplay
    {
        private SceneObject root;
        private GUIScrollArea scrollArea;
        private List<string> paths = new List<string>();

        private GUIAnimFieldEntry[] fields;

        public GUIAnimFieldDisplay(GUILayout layout, SceneObject root)
        {
            this.root = root;

            scrollArea = new GUIScrollArea();
            layout.AddElement(scrollArea);
        }

        public Action<string[]> OnSelectionChanged;

        public void SetFields(string[] paths)
        {
            this.paths.Clear();
            this.paths.AddRange(paths);

            Rebuild();
        }

        public void AddField(string path)
        {
            paths.Add(path);

            Rebuild();
        }

        private SerializableProperty FindProperty(string path)
        {
            if (string.IsNullOrEmpty(path) || root == null)
                return null;

            string[] entries = path.Split('/');

            // Find scene object referenced by the path
            SceneObject so = null;
            int pathIdx = 0;
            for (; pathIdx < entries.Length; pathIdx++)
            {
                string entry = entries[pathIdx];

                // Not a scene object, break
                if (entry[0] != '!')
                    break;

                if (pathIdx == 0)
                    so = root;
                else
                {
                    string childName = entry.Substring(1, entry.Length - 1);
                    so = so.FindChild(childName);

                    if (so == null)
                        break;
                }
            }

            // Cannot find scene object with specified hierarchy & name
            if (so == null)
                return null;

            pathIdx++;
            if (pathIdx >= entries.Length)
                return null;

            // If path is referencing a component, find it
            Component component = null;
            {
                string entry = entries[pathIdx];
                if (entry[0] == ':')
                {
                    string componentName = entry.Substring(1, entry.Length - 1);

                    Component[] components = so.GetComponents();
                    component = Array.Find(components, x => x.GetType().Name == componentName);

                    // Cannot find component with specified type
                    if (component == null)
                        return null;
                }
            }

            // Look for a field within a component
            if (component != null)
            {
                pathIdx++;
                if (pathIdx >= entries.Length)
                    return null;

                SerializableObject componentObj = new SerializableObject(component);

                StringBuilder pathBuilder = new StringBuilder();
                for(; pathIdx < entries.Length; pathIdx++)
                    pathBuilder.Append(entries[pathIdx] + "/");

                return componentObj.FindProperty(pathBuilder.ToString());
            }
            else // Field is one of the builtin ones on the SceneObject itself
            {
                if ((pathIdx + 1) < entries.Length)
                    return null;

                string entry = entries[pathIdx];
                if (entry == "Position")
                {
                    SerializableProperty property = new SerializableProperty(
                        SerializableProperty.FieldType.Vector3,
                        typeof(Vector3),
                        () => so.LocalPosition, 
                        (x) => so.LocalPosition = (Vector3)x);

                    return property;
                }
                else if (entry == "Rotation")
                {
                    SerializableProperty property = new SerializableProperty(
                        SerializableProperty.FieldType.Vector3,
                        typeof(Vector3),
                        () => so.LocalRotation.ToEuler(), 
                        (x) => so.LocalRotation = Quaternion.FromEuler((Vector3)x));

                    return property;
                }
                else if (entry == "Scale")
                {
                    SerializableProperty property = new SerializableProperty(
                        SerializableProperty.FieldType.Vector3,
                        typeof(Vector3),
                        () => so.LocalScale, 
                        (x) => so.LocalScale = (Vector3)x);

                    return property;
                }

                return null;
            }
        }

        private void Rebuild()
        {
            scrollArea.Layout.Clear();
            fields = null;

            if (paths == null || root == null)
                return;

            fields = new GUIAnimFieldEntry[paths.Count];
            for (int i = 0; i < paths.Count; i++)
            {
                SerializableProperty property = FindProperty(paths[i]);
                if (property != null)
                {
                    switch (property.Type)
                    {
                        case SerializableProperty.FieldType.Vector2:
                            fields[i] = new GUIAnimVec2Entry(scrollArea.Layout, paths[i]);
                            break;
                        case SerializableProperty.FieldType.Vector3:
                            fields[i] = new GUIAnimVec3Entry(scrollArea.Layout, paths[i]);
                            break;
                        case SerializableProperty.FieldType.Vector4:
                            fields[i] = new GUIAnimVec4Entry(scrollArea.Layout, paths[i]);
                            break;
                        case SerializableProperty.FieldType.Color:
                            fields[i] = new GUIAnimColorEntry(scrollArea.Layout, paths[i]);
                            break;
                        case SerializableProperty.FieldType.Bool:
                        case SerializableProperty.FieldType.Int:
                        case SerializableProperty.FieldType.Float:
                            fields[i] = new GUIAnimFieldEntry(scrollArea.Layout, paths[i]);
                            break;
                    }
                }
                else
                {
                    // TODO - Add special field type for missing properties
                }
            }
        }
    }

    internal class GUIAnimFieldEntry
    {
        private const int MAX_PATH_LENGTH = 20;
        protected const int INDENT_AMOUNT = 10;

        private GUIToggle toggle;
        protected string path;

        public Action<string, bool> OnSelectionChanged;

        public GUIAnimFieldEntry(GUILayout layout, string path)
        {
            toggle = new GUIToggle(GetDisplayName(path), EditorStyles.SelectableLabel);
            layout.AddElement(toggle);

            // TODO - Show current value (if not complex)
            // TODO - Button to remove properties

            this.path = path;
        }

        public virtual void Toggle(bool on)
        { }

        private static string GetDisplayName(string path)
        {
            if (string.IsNullOrEmpty(path))
                return "";

            string soName;
            string compName;
            string propertyPath;

            GetNames(path, out soName, out compName, out propertyPath);

            if (soName == null || propertyPath == null)
                return "";

            string truncatedPropPath;
            if (propertyPath.Length > MAX_PATH_LENGTH)
                truncatedPropPath = "..." + propertyPath.Substring(propertyPath.Length - MAX_PATH_LENGTH);
            else
                truncatedPropPath = propertyPath;

            if (compName != null)
                return soName + "(" + compName + ")." + truncatedPropPath;
            else
                return soName + "." + truncatedPropPath;
        }

        private static void GetNames(string path, out string soName, out string compName, out string propertyPath)
        {
            string[] entries = path.Split('/');

            // Find name of the last scene object in the path
            int pathIdx = 0;
            for (; pathIdx < entries.Length; pathIdx++)
            {
                string entry = entries[pathIdx];

                // Not a scene object, break
                if (entry[0] != '!')
                    break;
            }

            if (pathIdx >= entries.Length)
            {
                soName = null;
                compName = null;
                propertyPath = null;

                return;
            }

            soName = entries[pathIdx].Substring(1, entries[pathIdx].Length - 1);

            pathIdx++;
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

            StringBuilder pathBuilder = new StringBuilder();
            for (; pathIdx < entries.Length; pathIdx++)
                pathBuilder.Append(entries[pathIdx] + "/");

            propertyPath = pathBuilder.ToString();
        }
    }

    internal class GUIAnimComplexEntry : GUIAnimFieldEntry
    {
        private GUIToggle[] children;
        private GUILayout childLayout;

        protected GUIAnimComplexEntry(GUILayout layout, string path, string[] childEntries)
            : base(layout, path)
        {
            childLayout = layout.AddLayoutX();
            childLayout.AddSpace(INDENT_AMOUNT);

            // TODO - Foldout to expand/collapse child layout

            GUILayout indentLayout = childLayout.AddLayoutY();

            children = new GUIToggle[childEntries.Length];
            for (int i = 0; i < childEntries.Length; i++)
            {
                int index = i;

                children[i] = new GUIToggle(new LocEdString(childEntries[i]), EditorStyles.SelectableLabel);
                children[i].OnToggled += x => { OnSelectionChanged?.Invoke(path + childEntries[index], x); };

                indentLayout.AddElement(children[i]);
            }

            Toggle(false);
        }

        public override void Toggle(bool on)
        {
            childLayout.Active = on;
        }
    }

    internal class GUIAnimVec2Entry : GUIAnimComplexEntry
    {
        public GUIAnimVec2Entry(GUILayout layout, string path)
            : base(layout, path,  new[] { ".x", ".y" })
        { }
    }

    internal class GUIAnimVec3Entry : GUIAnimComplexEntry
    {
        public GUIAnimVec3Entry(GUILayout layout, string path)
            : base(layout, path, new[] { ".x", ".y", ".z" })
        { }
    }

    internal class GUIAnimVec4Entry : GUIAnimComplexEntry
    {
        public GUIAnimVec4Entry(GUILayout layout, string path)
            : base(layout, path,  new[] { ".x", ".y", ".z", ".w" })
        { }
    }

    internal class GUIAnimColorEntry : GUIAnimComplexEntry
    {
        public GUIAnimColorEntry(GUILayout layout, string path)
            : base(layout, path, new[] { ".r", ".g", ".b", ".a" })
        { }
    }

    /** @} */
}