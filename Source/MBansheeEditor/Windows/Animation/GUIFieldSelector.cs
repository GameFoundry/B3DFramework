//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.Collections.Generic;
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup AnimationEditor
     *  @{
     */

    /// <summary>
    /// Renders GUI elements that display a Scene Object, its transform, components and child objects, as well as all of
    /// their fields. User can then select one of the fields and the class will output a path to the selected field, as
    /// well as its parent scene object and component.
    /// </summary>
    public class GUIFieldSelector
    {
        private const int INDENT_AMOUNT = 5;
        private const int PADDING = 5;

        private GUIScrollArea scrollArea;
        private int foldoutWidth;

        private SceneObject rootSO;
        private Element rootElement;

        /// <summary>
        /// Stores a single entry in the field hierarchy.
        /// </summary>
        private struct Element
        {
            public Element(SceneObject so, Component comp, string path)
            {
                this.so = so;
                this.comp = comp;
                this.path = path;

                toggle = null;
                childLayout = null;
                indentLayout = null;

                children = null;
            }

            public SceneObject so;
            public Component comp;
            public string path;

            public GUIToggle toggle;
            public GUILayout childLayout;
            public GUILayout indentLayout;

            public Element[] children;
        }

        /// <summary>
        /// Triggers when the user selects a field. The subscriber will be receive a scene object the field is part of,
        /// component the field is part of, and a path to the field, each entry separated by "/". Component can be null
        /// in which case it is assumed the field is part of the SceneObject itself. Scene object names are always prefixed
        /// with "!", components are always prefixed with ":", while field entries have no prefix.
        /// 
        /// For example: !My Scene Object/:Camera/path/to/field
        /// </summary>
        public Action<SceneObject, Component, string, SerializableProperty.FieldType> OnElementSelected;

        /// <summary>
        /// Creates a new GUIFieldSelector and registers its GUI elements in the provided layout.
        /// </summary>
        /// <param name="layout">Layout into which to add the selector GUI hierarchy.</param>
        /// <param name="so">Scene object to inspect the fields for.</param>
        /// <param name="width">Width of the selector area, in pixels.</param>
        /// <param name="height">Height of the selector area, in pixels.</param>
        public GUIFieldSelector(GUILayout layout, SceneObject so, int width, int height)
        {
            rootSO = so;

            scrollArea = new GUIScrollArea();
            scrollArea.SetWidth(width);
            scrollArea.SetHeight(height);

            layout.AddElement(scrollArea);

            GUISkin skin = EditorBuiltin.GUISkin;
            GUIElementStyle style = skin.GetStyle(EditorStyles.Expand);
            foldoutWidth = style.Width;

            Rebuild();
        }

        /// <summary>
        /// Rebuilds all of the selection GUI.
        /// </summary>
        private void Rebuild()
        {
            scrollArea.Layout.Clear();
            rootElement = new Element();

            if (rootSO == null)
                return;

            rootElement.so = rootSO;
            rootElement.childLayout = scrollArea.Layout;
            rootElement.indentLayout = null;

            GUILabel header = new GUILabel(new LocEdString("Select a property"), EditorStyles.Header);
            scrollArea.Layout.AddElement(header);

            scrollArea.Layout.AddSpace(5);
            AddSceneObjectRows(rootElement);
            scrollArea.Layout.AddSpace(5);
            scrollArea.Layout.AddFlexibleSpace();
        }

        /// <summary>
        /// Registers a set of rows for all components in a <see cref="SceneObject"/>, as well as its transform and
        /// child objects.
        /// </summary>
        /// <param name="parent">Row element under which to create the new rows.</param>
        private void AddSceneObjectRows(Element parent)
        {
            string soName = "!" + parent.so.Name;
            Component[] components = parent.so.GetComponents();

            parent.children = new Element[components.Length + 2];

            SpriteTexture soIcon = EditorBuiltin.GetEditorIcon(EditorIcon.SceneObject);
            SpriteTexture compIcon = EditorBuiltin.GetEditorIcon(EditorIcon.Component);

            // Add transform
            parent.children[0] = AddFoldoutRow(parent.childLayout, soIcon, "Transform", parent.so, 
                null, parent.path + "/" + soName, ToggleTransformFoldout);

            // Add components
            for (int i = 0; i < components.Length; i++)
            {
                Component childComponent = components[i];
                Action<Element, bool> toggleCallback = 
                    (toggleParent, expand) =>
                {
                    SerializableObject componentObject = new SerializableObject(childComponent.GetType(), childComponent);
                    ToggleObjectFoldout(toggleParent, componentObject, expand);
                };
                
                string name = childComponent.GetType().Name;
                string path = parent.path + "/" + soName + "/:" + name;
                parent.children[i + 1] = AddFoldoutRow(parent.childLayout, compIcon, name, parent.so, childComponent, path,
                    toggleCallback);
            }

            // Add children
            if (parent.so.GetNumChildren() > 0)
            {
                parent.children[parent.children.Length - 1] = AddFoldoutRow(parent.childLayout, soIcon, "Children",
                    parent.so, null, parent.path + "/" + soName, ToggleChildFoldout);
            }
        }

        /// <summary>
        /// Registers a new row in the layout for the provided property. The type of row is determined by the property type.
        /// </summary>
        /// <param name="parent">Parent foldout row to which to append the new elements.</param>
        /// <param name="name">Name of the field the property belongs to.</param>
        /// <param name="path">Slash separated path to the field from its parent object.</param>
        /// <param name="property">Property to create the row for.</param>
        /// <param name="element">Element containing data for the newly created row. Only valid if method returns true.
        ///                       </param>
        /// <returns>Returns true if the row was successfully added, false otherwise.</returns>
        private bool AddPropertyRow(Element parent, string name, string path, SerializableProperty property, 
            out Element element)
        {
            switch (property.Type)
            {
                case SerializableProperty.FieldType.Bool:
                case SerializableProperty.FieldType.Float:
                case SerializableProperty.FieldType.Int:
                case SerializableProperty.FieldType.Color:
                case SerializableProperty.FieldType.Vector2:
                case SerializableProperty.FieldType.Vector3:
                case SerializableProperty.FieldType.Vector4:
                    element = AddFieldRow(parent.childLayout, name, parent.so, parent.comp, path, property.Type);
                    return true;
                case SerializableProperty.FieldType.Object:
                    {
                        Action<Element, bool> toggleCallback =
                            (toggleParent, expand) =>
                            {
                                SerializableObject childObject = new SerializableObject(property.InternalType, null);
                                ToggleObjectFoldout(toggleParent, childObject, expand);
                            };

                        element = AddFoldoutRow(parent.childLayout, null, name, parent.so, parent.comp, path, toggleCallback);
                    }
                    return true;
                case SerializableProperty.FieldType.Array:
                    {
                        Action<Element, bool> toggleCallback =
                            (toggleParent, expand) =>
                            {
                                SerializableArray childObject = property.GetArray();

                                if (childObject != null)
                                    ToggleArrayFoldout(toggleParent, childObject, expand);
                            };

                        element = AddFoldoutRow(parent.childLayout, null, name, parent.so, parent.comp, path, toggleCallback);
                    }
                    return true;
                case SerializableProperty.FieldType.List:
                    {
                        Action<Element, bool> toggleCallback =
                            (toggleParent, expand) =>
                            {
                                SerializableList childObject = property.GetList();

                                if (childObject != null)
                                    ToggleListFoldout(toggleParent, childObject, expand);
                            };

                        element = AddFoldoutRow(parent.childLayout, null, name, parent.so, parent.comp, path, toggleCallback);
                    }
                    return true;
            }

            element = new Element();
            return false;
        }

        /// <summary>
        /// Registers a set of rows for all child fields of the provided object.
        /// </summary>
        /// <param name="parent">Parent foldout row to which to append the new elements.</param>
        /// <param name="serializableObject">Type of the object whose fields to display.</param>
        private void AddObjectRows(Element parent, SerializableObject serializableObject)
        {
            List<Element> elements = new List<Element>(); 
            foreach (var field in serializableObject.Fields)
            {
                if (!field.Animable)
                    continue;

                string propertyPath = parent.path + "/" + field.Name;

                Element element;
                if(AddPropertyRow(parent, field.Name, propertyPath, field.GetProperty(), out element))
                    elements.Add(element);
            }

            // Handle special fields
            if (serializableObject.Type == typeof(Animation))
            {
                Animation anim = serializableObject.Object as Animation;
                MorphShapes morphShapes = anim?.SceneObject.GetComponent<Renderable>()?.Mesh?.MorphShapes;

                if (morphShapes != null)
                {
                    string propertyPath = parent.path + "/MorphShapes";

                    Action<Element, bool> toggleCallback =
                        (toggleParent, expand) =>
                        {
                            toggleParent.childLayout.Clear();
                            toggleParent.children = null;

                            toggleParent.indentLayout.Active = expand;

                            if (expand)
                            {
                                List<Element> childElements = new List<Element>();
                                MorphChannel[] channels = morphShapes.Channels;
                                for (int i = 0; i < channels.Length; i++)
                                {
                                    string channelName = channels[i].Name;

                                    string framePropertyPath = parent.path + "/MorphShapes/Frames/" + channelName;
                                    string weightPropertyPath = parent.path + "/MorphShapes/Weight/" + channelName;

                                    childElements.Add(AddFieldRow(toggleParent.childLayout, channelName + " (Frames)", 
                                        toggleParent.so, toggleParent.comp, framePropertyPath, 
                                        SerializableProperty.FieldType.Float));

                                    childElements.Add(AddFieldRow(toggleParent.childLayout, channelName + " (Weight)",
                                        toggleParent.so, toggleParent.comp, weightPropertyPath,
                                        SerializableProperty.FieldType.Float));
                                }

                                toggleParent.children = childElements.ToArray();
                            }
                        };

                    
                    elements.Add(AddFoldoutRow(parent.childLayout, null, "MorphShapes", parent.so, parent.comp,
                        propertyPath, toggleCallback));
                }
            }

            parent.children = elements.ToArray();
        }

        /// <summary>
        /// Registers a set of rows for all entries of the provided array.
        /// </summary>
        /// <param name="parent">Parent foldout row to which to append the new elements.</param>
        /// <param name="serializableArray">Array whose fields to display.</param>
        private void AddArrayRows(Element parent, SerializableArray serializableArray)
        {
            List<Element> elements = new List<Element>();

            int length = serializableArray.GetLength();
            for (int i = 0; i < length; i++)
            {
                string name = "[" + i + "]";
                string propertyPath = parent.path + name;

                Element element;
                if (AddPropertyRow(parent, name, propertyPath, serializableArray.GetProperty(i), out element))
                    elements.Add(element);
            }

            parent.children = elements.ToArray();
        }

        /// <summary>
        /// Registers a set of rows for all entries of the provided list.
        /// </summary>
        /// <param name="parent">Parent foldout row to which to append the new elements.</param>
        /// <param name="serializableList">List whose fields to display.</param>
        private void AddListRows(Element parent, SerializableList serializableList)
        {
            List<Element> elements = new List<Element>();

            int length = serializableList.GetLength();
            for (int i = 0; i < length; i++)
            {
                string name = "[" + i + "]";
                string propertyPath = parent.path + name;

                Element element;
                if (AddPropertyRow(parent, name, propertyPath, serializableList.GetProperty(i), out element))
                    elements.Add(element);
            }

            parent.children = elements.ToArray();
        }

        /// <summary>
        /// Registers a new row in the layout. The row cannot have any further children and can be selected as the output
        /// field.
        /// </summary>
        /// <param name="layout">Layout to append the row GUI elements to.</param>
        /// <param name="name">Name of the field.</param>
        /// <param name="so">Parent scene object of the field.</param>
        /// <param name="component">Parent component of the field. Can be null if field belongs to <see cref="SceneObject"/>.
        ///                         </param>
        /// <param name="path">Slash separated path to the field from its parent object.</param>
        /// <param name="type">Data type stored in the field.</param>
        /// <returns>Element object storing all information about the added field.</returns>
        private Element AddFieldRow(GUILayout layout, string name, SceneObject so, Component component, string path, SerializableProperty.FieldType type)
        {
            Element element = new Element(so, component, path);

            GUILayoutX elementLayout = layout.AddLayoutX();

            elementLayout.AddSpace(PADDING);
            elementLayout.AddSpace(foldoutWidth);
            GUILabel label = new GUILabel(new LocEdString(name));
            elementLayout.AddElement(label);

            GUIButton selectBtn = new GUIButton(new LocEdString("Select"));
            selectBtn.OnClick += () => { DoOnElementSelected(element, type); };

            elementLayout.AddFlexibleSpace();
            elementLayout.AddElement(selectBtn);
            elementLayout.AddSpace(5);

            element.path = path;

            return element;
        }

        /// <summary>
        /// Registers a new row in the layout. The row cannot be selected as the output field, but rather can be expanded
        /// so it displays child elements.
        /// </summary>
        /// <param name="layout">Layout to append the row GUI elements to.</param>
        /// <param name="icon">Optional icon to display next to the name. Can be null.</param>
        /// <param name="name">Name of the field.</param>
        /// <param name="so">Parent scene object of the field.</param>
        /// <param name="component">Parent component of the field. Can be null if field belongs to <see cref="SceneObject"/>.
        ///                         </param>
        /// <param name="path">Slash separated path to the field from its parent object.</param>
        /// <param name="toggleCallback">Callback to trigger when the user expands or collapses the foldout.</param>
        /// <returns>Element object storing all information about the added field.</returns>
        private Element AddFoldoutRow(GUILayout layout, SpriteTexture icon, string name, SceneObject so, Component component,
            string path, Action<Element, bool> toggleCallback)
        {
            Element element = new Element(so, component, path);

            GUILayoutY elementLayout = layout.AddLayoutY();
            GUILayoutX foldoutLayout = elementLayout.AddLayoutX();

            element.toggle = new GUIToggle("", EditorStyles.Expand);
            element.toggle.OnToggled += x => toggleCallback(element, x);

            foldoutLayout.AddSpace(PADDING);
            foldoutLayout.AddElement(element.toggle);

            if (icon != null)
            {
                GUITexture guiIcon = new GUITexture(icon, GUIOption.FixedWidth(16), GUIOption.FixedWidth(16));
                foldoutLayout.AddElement(guiIcon);
            }

            GUILabel label = new GUILabel(new LocEdString(name));
            foldoutLayout.AddElement(label);
            foldoutLayout.AddFlexibleSpace();

            element.indentLayout = elementLayout.AddLayoutX();
            element.indentLayout.AddSpace(INDENT_AMOUNT);
            element.childLayout = element.indentLayout.AddLayoutY();

            element.indentLayout.Active = false;

            return element;
        }

        /// <summary>
        /// Expands or collapses the set of rows displaying a <see cref="SceneObject"/>'s transform (position, rotation, 
        /// scale).
        /// </summary>
        /// <param name="parent">Parent row element whose children to expand/collapse.</param>
        /// <param name="expand">True to expand, false to collapse.</param>
        private void ToggleTransformFoldout(Element parent, bool expand)
        {
            parent.childLayout.Clear();
            parent.children = null;

            parent.indentLayout.Active = expand;

            if (expand)
            {
                parent.children = new Element[3];

                parent.children[0] = AddFieldRow(parent.childLayout, "Position", parent.so, null, parent.path + "/Position", SerializableProperty.FieldType.Vector3);
                parent.children[1] = AddFieldRow(parent.childLayout, "Rotation", parent.so, null, parent.path + "/Rotation", SerializableProperty.FieldType.Vector3);
                parent.children[2] = AddFieldRow(parent.childLayout, "Scale", parent.so, null, parent.path + "/Scale", SerializableProperty.FieldType.Vector3);
            }
        }

        /// <summary>
        /// Expands or collapses the set of rows displaying all <see cref="SceneObject"/> children of a 
        /// <see cref="SceneObject"/>.
        /// </summary>
        /// <param name="parent">Parent row element whose children to expand/collapse.</param>
        /// <param name="expand">True to expand, false to collapse.</param>
        private void ToggleChildFoldout(Element parent, bool expand)
        {
            parent.childLayout.Clear();
            parent.children = null;

            parent.indentLayout.Active = expand;

            if (expand)
            {
                int numChildren = parent.so.GetNumChildren();

                parent.children = new Element[numChildren];

                for (int i = 0; i < numChildren; i++)
                {
                    SceneObject child = parent.so.GetChild(i);

                    SpriteTexture soIcon = EditorBuiltin.GetEditorIcon(EditorIcon.SceneObject);
                    parent.children[i] = AddFoldoutRow(parent.childLayout, soIcon, child.Name, child, null, parent.path,
                        ToggleSceneObjectFoldout);
                }
            }
        }

        /// <summary>
        /// Expands or collapses the set of rows displaying all children of a <see cref="SceneObject"/>. This includes
        /// it's components, transform and child scene objects.
        /// </summary>
        /// <param name="parent">Parent row element whose children to expand/collapse.</param>
        /// <param name="expand">True to expand, false to collapse.</param>
        private void ToggleSceneObjectFoldout(Element parent, bool expand)
        {
            parent.childLayout.Clear();
            parent.children = null;

            parent.indentLayout.Active = expand;

            if (expand)
                AddSceneObjectRows(parent);
        }

        /// <summary>
        /// Expands or collapses the set of rows displaying all fields of a serializable object.
        /// </summary>
        /// <param name="parent">Parent row element whose children to expand/collapse.</param>
        /// <param name="obj">Object describing the type of the object whose fields to display.</param>
        /// <param name="expand">True to expand, false to collapse.</param>
        private void ToggleObjectFoldout(Element parent, SerializableObject obj,
            bool expand)
        {
            parent.childLayout.Clear();
            parent.children = null;

            parent.indentLayout.Active = expand;

            if (expand)
                AddObjectRows(parent, obj);
        }

        /// <summary>
        /// Expands or collapses the set of rows displaying all entries of a serializable array.
        /// </summary>
        /// <param name="parent">Parent row element whose children to expand/collapse.</param>
        /// <param name="obj">Object describing the array whose entries to display.</param>
        /// <param name="expand">True to expand, false to collapse.</param>
        private void ToggleArrayFoldout(Element parent, SerializableArray obj,
            bool expand)
        {
            parent.childLayout.Clear();
            parent.children = null;

            parent.indentLayout.Active = expand;

            if (expand)
                AddArrayRows(parent, obj);
        }

        /// <summary>
        /// Expands or collapses the set of rows displaying all entries of a serializable list.
        /// </summary>
        /// <param name="parent">Parent row element whose children to expand/collapse.</param>
        /// <param name="obj">Object describing the list whose entries to display.</param>
        /// <param name="expand">True to expand, false to collapse.</param>
        private void ToggleListFoldout(Element parent, SerializableList obj,
            bool expand)
        {
            parent.childLayout.Clear();
            parent.children = null;

            parent.indentLayout.Active = expand;

            if (expand)
                AddListRows(parent, obj);
        }

        /// <summary>
        /// Triggered when the user selects a field.
        /// </summary>
        /// <param name="element">Row element for the field that was selected.</param>
        /// <param name="type">Data type stored in the field.</param>
        private void DoOnElementSelected(Element element, SerializableProperty.FieldType type)
        {
            if (OnElementSelected != null)
                OnElementSelected(element.so, element.comp, element.path, type);
        }
    }

    /** @} */
}