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

        private SpriteTexture ICON_SO; // TODO
        private SpriteTexture ICON_COMPONENT; // TODO

        private GUILayoutY mainLayout;

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
            public GUILayoutY childLayout;
            public GUILayoutX indentLayout;

            public Element[] children;
        }

        /// <summary>
        /// Triggers when the user selects a field. The subscriber will be receive a scene object the field is part of,
        /// component the field is part of, and a path to the field, each entry separated by "/". Component can be null
        /// in which case it is assumed the field is part of the SceneObject itself.
        /// </summary>
        public Action<SceneObject, Component, string> OnElementSelected;

        /// <summary>
        /// Creates a new GUIFieldSelector and registers its GUI elements in the provided layout.
        /// </summary>
        /// <param name="layout">Layout into which to add the selector GUI hierarchy.</param>
        /// <param name="so">Scene object to inspect the fields for.</param>
        public GUIFieldSelector(GUILayout layout, SceneObject so)
        {
            rootSO = so;
            mainLayout = layout.AddLayoutY();

            Rebuild();
        }

        /// <summary>
        /// Rebuilds all of the selection GUI.
        /// </summary>
        private void Rebuild()
        {
            mainLayout.Clear();
            rootElement = new Element();

            if (rootSO == null)
                return;

            rootElement.so = rootSO;
            AddSceneObjectRows(rootElement);
        }

        /// <summary>
        /// Registers a set of rows for all components in a <see cref="SceneObject"/>, as well as its transform and
        /// child objects.
        /// </summary>
        /// <param name="parent">Row element under which to create the new rows.</param>
        private void AddSceneObjectRows(Element parent)
        {
            Component[] components = parent.so.GetComponents();

            parent.children = new Element[components.Length + 2];

            // Add transform
            parent.children[0] = AddFoldoutRow(mainLayout, ICON_COMPONENT, "Transform", parent.so, 
                null, "", ToggleTransformFoldout);

            // Add components
            for (int i = 0; i < components.Length; i++)
            {
                Component childComponent = components[i];
                Action<Element, bool> toggleCallback = 
                    (toggleParent, expand) =>
                {
                    SerializableObject componentObject = new SerializableObject(childComponent.GetType(), null);
                    ToggleObjectFoldout(toggleParent, componentObject, expand);
                };

                string name = childComponent.GetType().Name;
                parent.children[i + 1] = AddFoldoutRow(mainLayout, ICON_COMPONENT, name, parent.so, childComponent, "", 
                    toggleCallback);
            }

            // Add children
            parent.children[parent.children.Length - 1] = AddFoldoutRow(mainLayout, ICON_SO, "Children", parent.so, null, "",
                ToggleChildFoldout);
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
                if (!field.Inspectable)
                    continue;

                string propertyPath = parent.path + "/" + field.Name;

                switch (field.Type)
                {
                    case SerializableProperty.FieldType.Bool:
                    case SerializableProperty.FieldType.Float:
                    case SerializableProperty.FieldType.Int:
                    case SerializableProperty.FieldType.Color:
                    case SerializableProperty.FieldType.Vector2:
                    case SerializableProperty.FieldType.Vector3:
                    case SerializableProperty.FieldType.Vector4:
                        elements.Add(AddFieldRow(parent.childLayout, field.Name, parent.so, parent.comp, propertyPath));
                        break;
                    case SerializableProperty.FieldType.Object:
                        Action<Element, bool> toggleCallback = 
                            (toggleParent, expand) =>
                        {
                            SerializableObject childObject = new SerializableObject(field.InternalType, null);
                            ToggleObjectFoldout(toggleParent, childObject, expand);
                        };
                        elements.Add(AddFoldoutRow(parent.childLayout, null, field.Name, parent.so, parent.comp, 
                            propertyPath, toggleCallback));
                        break;
                }
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
        /// <returns>Element object storing all information about the added field.</returns>
        private Element AddFieldRow(GUILayout layout, string name, SceneObject so, Component component, string path)
        {
            Element element = new Element(so, component, path);

            GUILayoutX elementLayout = layout.AddLayoutX();
           
            GUILabel label = new GUILabel(new LocEdString(name));
            elementLayout.AddElement(label);

            GUIButton selectBtn = new GUIButton(new LocEdString("Select"));
            selectBtn.OnClick += () => { DoOnElementSelected(element); };

            elementLayout.AddFlexibleSpace();
            elementLayout.AddElement(selectBtn);

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

            element.toggle = new GUIToggle(EditorStyles.Expand);
            element.toggle.OnToggled += x => toggleCallback(element, x);

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

                parent.children[0] = AddFieldRow(parent.childLayout, "Position", parent.so, null, parent.path + "/Position");
                parent.children[1] = AddFieldRow(parent.childLayout, "Rotation", parent.so, null, parent.path + "/Rotation");
                parent.children[2] = AddFieldRow(parent.childLayout, "Scale", parent.so, null, parent.path + "/Scale");
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

                    parent.children[i] = AddFoldoutRow(parent.childLayout, ICON_SO, child.Name, child, null, "",
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
            {
                AddSceneObjectRows(parent);
            }
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
        /// Triggered when the user selects a field.
        /// </summary>
        /// <param name="element">Row element for the field that was selected.</param>
        private void DoOnElementSelected(Element element)
        {
            if (OnElementSelected != null)
                OnElementSelected(element.so, element.comp, element.path);
        }
    }

    /** @} */
}