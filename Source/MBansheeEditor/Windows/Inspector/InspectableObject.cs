//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System.Collections.Generic;
using System;
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup Inspector
     *  @{
     */

    /// <summary>
    /// Displays GUI for a serializable property containing a generic object. Inspectable object fields are displayed
    /// in separate rows.
    /// </summary>
    public class InspectableObject : InspectableField
    {
        private const int IndentAmount = 5;

        private object propertyValue;
        private List<InspectableField> children = new List<InspectableField>();

        private GUILayoutY guiLayout;
        private GUILayoutX guiChildLayout;
        private GUILayoutX guiTitleLayout;
        private GUILayoutX guiInternalTitleLayout;
        private bool isExpanded;
        private bool forceUpdate = true;
        private State state;

        /// <summary>
        /// Creates a new inspectable array GUI for the specified property.
        /// </summary>
        /// <param name="parent">Parent Inspector this field belongs to.</param>
        /// <param name="title">Name of the property, or some other value to set as the title.</param>
        /// <param name="path">Full path to this property (includes name of this property and all parent properties).</param>
        /// <param name="depth">Determines how deep within the inspector nesting hierarchy is this field. Some fields may
        ///                     contain other fields, in which case you should increase this value by one.</param>
        /// <param name="layout">Parent layout that all the field elements will be added to.</param>
        /// <param name="property">Serializable property referencing the array whose contents to display.</param>
        public InspectableObject(Inspector parent, string title, string path, int depth, InspectableFieldLayout layout, 
            SerializableProperty property)
            : base(parent, title, path, SerializableProperty.FieldType.Object, depth, layout, property)
        {
            isExpanded = parent.Persistent.GetBool(path + "_Expanded");
        }

        /// <inheritdoc/>
        public override GUILayoutX GetTitleLayout()
        {
            return guiTitleLayout;
        }

        /// <inheritdoc/>
        public override InspectableState Refresh(int layoutIndex)
        {
            // Check if modified internally and rebuild if needed
            object newPropertyValue = property.GetValue<object>();
            if (forceUpdate)
            {
                propertyValue = newPropertyValue;
                BuildGUI(layoutIndex);
                
                forceUpdate = false;
            }
            else if (propertyValue == null && newPropertyValue != null)
            {
                propertyValue = newPropertyValue;
                BuildGUI(layoutIndex);
                
            }
            else if (newPropertyValue == null && propertyValue != null)
            {
                propertyValue = null;
                BuildGUI(layoutIndex);
            }

            InspectableState state = InspectableState.NotModified;
            int currentIndex = 0;
            for (int i = 0; i < children.Count; i++)
            {
                state |= children[i].Refresh(currentIndex);
                currentIndex += children[i].GetNumLayoutElements();
            }

            return state;
        }

        /// <summary>
        /// Rebuilds the GUI object header if needed. 
        /// </summary>
        /// <param name="layoutIndex">Index at which to insert the GUI elements.</param>
        protected void BuildGUI(int layoutIndex)
        {
            Action BuildEmptyGUI = () =>
            {
                guiInternalTitleLayout = guiTitleLayout.InsertLayoutX(0);

                guiInternalTitleLayout.AddElement(new GUILabel(title));
                guiInternalTitleLayout.AddElement(new GUILabel("Empty", GUIOption.FixedWidth(100)));

                if (!property.IsValueType)
                {
                    GUIContent createIcon = new GUIContent(EditorBuiltin.GetInspectorWindowIcon(InspectorWindowIcon.Create), 
                        new LocEdString("Create"));
                    GUIButton createBtn = new GUIButton(createIcon, GUIOption.FixedWidth(30));
                    createBtn.OnClick += OnCreateButtonClicked;
                    guiInternalTitleLayout.AddElement(createBtn);
                }
            };

           Action BuildFilledGUI = () =>
           {
               guiInternalTitleLayout = guiTitleLayout.InsertLayoutX(0);

               GUIToggle guiFoldout = new GUIToggle(title, EditorStyles.Foldout);
               guiFoldout.Value = isExpanded;
               guiFoldout.OnToggled += OnFoldoutToggled;
               guiInternalTitleLayout.AddElement(guiFoldout);

               GUIContent clearIcon = new GUIContent(EditorBuiltin.GetInspectorWindowIcon(InspectorWindowIcon.Clear), 
                   new LocEdString("Clear"));
               GUIButton clearBtn = new GUIButton(clearIcon, GUIOption.FixedWidth(20));
               clearBtn.OnClick += OnClearButtonClicked;
               guiInternalTitleLayout.AddElement(clearBtn);

               if (isExpanded)
               {
                   SerializableObject serializableObject = property.GetObject();
                   SerializableField[] fields = serializableObject.Fields;

                   if (fields.Length > 0)
                   {
                       guiChildLayout = guiLayout.AddLayoutX();
                       guiChildLayout.AddSpace(IndentAmount);

                       GUIPanel guiContentPanel = guiChildLayout.AddPanel();
                       GUILayoutX guiIndentLayoutX = guiContentPanel.AddLayoutX();
                       guiIndentLayoutX.AddSpace(IndentAmount);
                       GUILayoutY guiIndentLayoutY = guiIndentLayoutX.AddLayoutY();
                       guiIndentLayoutY.AddSpace(IndentAmount);
                       GUILayoutY guiContentLayout = guiIndentLayoutY.AddLayoutY();
                       guiIndentLayoutY.AddSpace(IndentAmount);
                       guiIndentLayoutX.AddSpace(IndentAmount);
                       guiChildLayout.AddSpace(IndentAmount);

                       short backgroundDepth = (short) (Inspector.START_BACKGROUND_DEPTH - depth - 1);
                       string bgPanelStyle = depth%2 == 0
                           ? EditorStylesInternal.InspectorContentBgAlternate
                           : EditorStylesInternal.InspectorContentBg;
                       GUIPanel backgroundPanel = guiContentPanel.AddPanel(backgroundDepth);
                       GUITexture inspectorContentBg = new GUITexture(null, bgPanelStyle);
                       backgroundPanel.AddElement(inspectorContentBg);

                       int currentIndex = 0;
                       foreach (var field in fields)
                       {
                           if (!field.Inspectable)
                               continue;

                           string childPath = path + "/" + field.Name;

                           InspectableField inspectable = CreateInspectable(parent, field.Name, childPath,
                               currentIndex, depth + 1, new InspectableFieldLayout(guiContentLayout), field.GetProperty(), InspectableFieldStyle.Create(field));

                           children.Add(inspectable);
                           currentIndex += inspectable.GetNumLayoutElements();
                       }
                   }
               }
               else
                   guiChildLayout = null;
           };

           if (state == State.None)
           {
               if (propertyValue != null)
               {
                   BuildFilledGUI();
                   state = State.Filled;
               }
               else
               {
                   BuildEmptyGUI();

                   state = State.Empty;
               }
           }
           else if (state == State.Empty)
           {
               if (propertyValue != null)
               {
                   guiInternalTitleLayout.Destroy();
                   BuildFilledGUI();
                   state = State.Filled;
               }
           }
           else if (state == State.Filled)
           {
               foreach (var child in children)
                   child.Destroy();

               children.Clear();
               guiInternalTitleLayout.Destroy();

               if (guiChildLayout != null)
               {
                   guiChildLayout.Destroy();
                   guiChildLayout = null;
               }

               if (propertyValue == null)
               {
                   BuildEmptyGUI();
                   state = State.Empty;
               }
               else
               {
                   BuildFilledGUI();
               }
           }
        }

        /// <inheritdoc/>
        protected internal override void Initialize(int index)
        {
            guiLayout = layout.AddLayoutY(index);
            guiTitleLayout = guiLayout.AddLayoutX();

            propertyValue = property.GetValue<object>();
            BuildGUI(index);
        }

        /// <summary>
        /// Triggered when the user clicks on the expand/collapse toggle in the title bar.
        /// </summary>
        /// <param name="expanded">Determines whether the contents were expanded or collapsed.</param>
        private void OnFoldoutToggled(bool expanded)
        {
            parent.Persistent.SetBool(path + "_Expanded", expanded);

            isExpanded = expanded;
            forceUpdate = true;
        }

        /// <summary>
        /// Triggered when the user clicks on the create button on the title bar. Creates a brand new object with default
        /// values in the place of the current array.
        /// </summary>
        private void OnCreateButtonClicked()
        {
            property.SetValue(property.CreateObjectInstance<object>());
        }

        /// <summary>
        /// Triggered when the user clicks on the clear button on the title bar. Deletes the current object and sets
        /// the reference to the object in the parent object to null. This is only relevant for objects of reference types.
        /// </summary>
        private void OnClearButtonClicked()
        {
            property.SetValue<object>(null);
        }

        /// <summary>
        /// Possible states object GUI can be in.
        /// </summary>
        private enum State
        {
            None,
            Empty,
            Filled
        }
    }

    /** @} */
}
