//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.Collections.Generic;
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup Inspectors
     *  @{
     */

    /// <summary>
    /// Renders an inspector for the <see cref="GUISkin"/> resource.
    /// </summary>
    [CustomInspector(typeof(GUISkin))]
    internal class GUISkinInspector : Inspector
    {
        private GUIDictionaryField<string, GUIElementStyle, GUIElementStyleEntry> valuesField;

        private Dictionary<string, GUIElementStyle> styles = new Dictionary<string, GUIElementStyle>();

        /// <inheritdoc/>
        protected internal override void Initialize()
        {
            BuildGUI();
        }

        /// <inheritdoc/>
        protected internal override InspectableState Refresh()
        {
            valuesField.Refresh();

            return InspectableState.NotModified;
        }

        /// <summary>
        /// Recreates all the GUI elements used by this inspector.
        /// </summary>
        private void BuildGUI()
        {
            Layout.Clear();
            styles.Clear();

            GUISkin guiSkin = InspectedObject as GUISkin;
            if (guiSkin == null)
                return;

            string[] styleNames = guiSkin.StyleNames;
            foreach (var styleName in styleNames)
                styles[styleName] = guiSkin.GetStyle(styleName);

            valuesField = GUIDictionaryField<string, GUIElementStyle, GUIElementStyleEntry>.Create
                (new LocEdString("Styles"), styles, Layout);
            valuesField.IsExpanded = Persistent.GetBool("valuesField_Expanded");
            valuesField.OnExpand += x => Persistent.SetBool("valuesField_Expanded", x);

            valuesField.OnChanged += x =>
            {
                if (x != null)
                {
                    foreach (var KVP in x)
                    {
                        if (guiSkin.HasStyle(KVP.Key))
                        {
                            GUIElementStyle oldValue = guiSkin.GetStyle(KVP.Key);
                            if (oldValue != KVP.Value)
                                guiSkin.SetStyle(KVP.Key, KVP.Value);
                        }
                        else
                            guiSkin.SetStyle(KVP.Key, KVP.Value);
                    }

                    string[] oldStyleNames = guiSkin.StyleNames;
                    foreach (var styleName in oldStyleNames)
                    {
                        if (!x.ContainsKey(styleName))
                            guiSkin.RemoveStyle(styleName);
                    }

                    styles = x;
                }
                else
                {
                    foreach (var KVP in styles)
                        guiSkin.RemoveStyle(KVP.Key);

                    styles.Clear();
                }

                EditorApplication.SetDirty(guiSkin);
            };

            valuesField.OnValueChanged += x =>
            {
                guiSkin.SetStyle(x, styles[x]);
                EditorApplication.SetDirty(guiSkin);
            };

            valuesField.OnValueRemoved += x =>
            {
                guiSkin.RemoveStyle(x);
                EditorApplication.SetDirty(guiSkin);
            };

            Layout.AddSpace(10);
        }

        /// <summary>
        /// Row element used for displaying GUI for GUI element style dictionary elements.
        /// </summary>
        private class GUIElementStyleEntry : GUIDictionaryFieldRow
        {
            private GUITextField keyField;
            private GUIElementStyleGUI valueField;

            /// <inheritdoc/>
            protected override GUILayoutX CreateKeyGUI(GUILayoutY layout)
            {
                GUILayoutX titleLayout = layout.AddLayoutX();
                keyField = new GUITextField(new LocEdString("Name"));
                titleLayout.AddElement(keyField);

                keyField.OnChanged += SetKey;

                return titleLayout;
            }

            /// <inheritdoc/>
            protected override void CreateValueGUI(GUILayoutY layout)
            {
                if(valueField == null)
                    valueField = new GUIElementStyleGUI();

                valueField.BuildGUI(layout, Depth);
            }

            /// <inheritdoc/>
            protected override void OnEditModeChanged(bool editMode)
            {
                keyField.Disabled = !editMode;
            }

            /// <inheritdoc/>
            internal protected override InspectableState Refresh()
            {
                keyField.Value = GetKey<string>();
                return valueField.Refresh(GetValue<GUIElementStyle>());
            }
        }

        /// <summary>
        /// Creates GUI elements for editing/displaying <see cref="GUIElementStyle"/>
        /// </summary>
        private class GUIElementStyleGUI
        {
            private const int IndentAmount = 5;

            private GUIResourceField fontField;
            private GUIIntField fontSizeField;
            private GUIEnumField horzAlignField;
            private GUIEnumField vertAlignField;
            private GUIEnumField imagePositionField;
            private GUIToggleField wordWrapField;

            private GUIElementStateStyleGUI normalGUI;
            private GUIElementStateStyleGUI hoverGUI;
            private GUIElementStateStyleGUI activeGUI;
            private GUIElementStateStyleGUI focusedGUI;
            private GUIElementStateStyleGUI normalOnGUI;
            private GUIElementStateStyleGUI hoverOnGUI;
            private GUIElementStateStyleGUI activeOnGUI;
            private GUIElementStateStyleGUI focusedOnGUI;

            private RectOffsetGUI borderGUI;
            private RectOffsetGUI marginsGUI;
            private RectOffsetGUI contentOffsetGUI;

            private GUIToggleField fixedWidthField;
            private GUIIntField widthField;
            private GUIIntField minWidthField;
            private GUIIntField maxWidthField;

            private GUIToggleField fixedHeightField;
            private GUIIntField heightField;
            private GUIIntField minHeightField;
            private GUIIntField maxHeightField;

            private GUIElementStyle style;
            private bool isExpanded;
            private InspectableState modifiedState;

            /// <summary>
            /// Creates a new GUI element style GUI.
            /// </summary>
            public GUIElementStyleGUI()
            {
                normalGUI = new GUIElementStateStyleGUI();
                hoverGUI = new GUIElementStateStyleGUI();
                activeGUI = new GUIElementStateStyleGUI();
                focusedGUI = new GUIElementStateStyleGUI();
                normalOnGUI = new GUIElementStateStyleGUI();
                hoverOnGUI = new GUIElementStateStyleGUI();
                activeOnGUI = new GUIElementStateStyleGUI();
                focusedOnGUI = new GUIElementStateStyleGUI();
            }

            /// <summary>
            /// Builds GUI for the specified GUI element style.
            /// </summary>
            /// <param name="layout">Layout to append the GUI elements to.</param>
            /// <param name="depth">Determines the depth at which the element is rendered.</param>
            public void BuildGUI(GUILayout layout, int depth)
            {
                short backgroundDepth = (short)(Inspector.START_BACKGROUND_DEPTH - depth - 1);
                string bgPanelStyle = depth % 2 == 0
                    ? EditorStylesInternal.InspectorContentBgAlternate
                    : EditorStylesInternal.InspectorContentBg;

                GUIToggle foldout = new GUIToggle(new LocEdString("Style"), EditorStyles.Foldout);
                GUITexture inspectorContentBg = new GUITexture(null, bgPanelStyle);

                layout.AddElement(foldout);
                GUIPanel panel = layout.AddPanel();
                GUIPanel backgroundPanel = panel.AddPanel(backgroundDepth);
                
                backgroundPanel.AddElement(inspectorContentBg);

                GUILayoutX guiIndentLayoutX = panel.AddLayoutX();
                guiIndentLayoutX.AddSpace(IndentAmount);
                GUILayoutY guiIndentLayoutY = guiIndentLayoutX.AddLayoutY();
                guiIndentLayoutY.AddSpace(IndentAmount);
                GUILayoutY contentLayout = guiIndentLayoutY.AddLayoutY();
                guiIndentLayoutY.AddSpace(IndentAmount);
                guiIndentLayoutX.AddSpace(IndentAmount);

                fontField = new GUIResourceField(typeof (Font), new LocEdString("Font"));
                fontSizeField = new GUIIntField(new LocEdString("Font size"));
                horzAlignField = new GUIEnumField(typeof (TextHorzAlign), new LocEdString("Horizontal alignment"));
                vertAlignField = new GUIEnumField(typeof(TextVertAlign), new LocEdString("Vertical alignment"));
                imagePositionField = new GUIEnumField(typeof(GUIImagePosition), new LocEdString("Image position"));
                wordWrapField = new GUIToggleField(new LocEdString("Word wrap"));

                contentLayout.AddElement(fontField);
                contentLayout.AddElement(fontSizeField);
                contentLayout.AddElement(horzAlignField);
                contentLayout.AddElement(vertAlignField);
                contentLayout.AddElement(imagePositionField);
                contentLayout.AddElement(wordWrapField);

                normalGUI.BuildGUI(new LocEdString("Normal"), contentLayout);
                hoverGUI.BuildGUI(new LocEdString("Hover"), contentLayout);
                activeGUI.BuildGUI(new LocEdString("Active"), contentLayout);
                focusedGUI.BuildGUI(new LocEdString("Focused"), contentLayout);
                normalOnGUI.BuildGUI(new LocEdString("NormalOn"), contentLayout);
                hoverOnGUI.BuildGUI(new LocEdString("HoverOn"), contentLayout);
                activeOnGUI.BuildGUI(new LocEdString("ActiveOn"), contentLayout);
                focusedOnGUI.BuildGUI(new LocEdString("FocusedOn"), contentLayout);

                borderGUI = new RectOffsetGUI(new LocEdString("Border"), contentLayout);
                marginsGUI = new RectOffsetGUI(new LocEdString("Margins"), contentLayout);
                contentOffsetGUI = new RectOffsetGUI(new LocEdString("Content offset"), contentLayout);

                fixedWidthField = new GUIToggleField(new LocEdString("Fixed width"));
                widthField = new GUIIntField(new LocEdString("Width"));
                minWidthField = new GUIIntField(new LocEdString("Min. width"));
                maxWidthField = new GUIIntField(new LocEdString("Max. width"));

                fixedHeightField = new GUIToggleField(new LocEdString("Fixed height"));
                heightField = new GUIIntField(new LocEdString("Height"));
                minHeightField = new GUIIntField(new LocEdString("Min. height"));
                maxHeightField = new GUIIntField(new LocEdString("Max. height"));

                contentLayout.AddElement(fixedWidthField);
                contentLayout.AddElement(widthField);
                contentLayout.AddElement(minWidthField);
                contentLayout.AddElement(maxWidthField);

                contentLayout.AddElement(fixedHeightField);
                contentLayout.AddElement(heightField);
                contentLayout.AddElement(minHeightField);
                contentLayout.AddElement(maxHeightField);

                foldout.OnToggled += x =>
                {
                    panel.Active = x;
                    isExpanded = x;
                };

                fontField.OnChanged += x => 
                {
                    Font font = Resources.Load<Font>(x);

                    GetStyle().Font = font; 
                    MarkAsModified(); 
                    ConfirmModify(); 
                };
                fontSizeField.OnChanged += x => { GetStyle().FontSize = x; MarkAsModified(); };
                fontSizeField.OnFocusLost += ConfirmModify;
                fontSizeField.OnConfirmed += ConfirmModify;
                horzAlignField.OnSelectionChanged += x =>
                {
                    GetStyle().TextHorzAlign = (TextHorzAlign)x; 
                    MarkAsModified(); 
                    ConfirmModify();
                };
                vertAlignField.OnSelectionChanged += x =>
                {
                    GetStyle().TextVertAlign = (TextVertAlign)x; 
                    MarkAsModified(); 
                    ConfirmModify();
                };
                imagePositionField.OnSelectionChanged += x =>
                {
                    GetStyle().ImagePosition = (GUIImagePosition)x; 
                    MarkAsModified(); 
                    ConfirmModify();
                };
                wordWrapField.OnChanged += x => { GetStyle().WordWrap = x; MarkAsModified(); ConfirmModify(); };

                normalGUI.OnChanged += x => { GetStyle().Normal = x; MarkAsModified(); ConfirmModify(); };
                hoverGUI.OnChanged += x => { GetStyle().Hover = x; MarkAsModified(); ConfirmModify(); };
                activeGUI.OnChanged += x => { GetStyle().Active = x; MarkAsModified(); ConfirmModify(); };
                focusedGUI.OnChanged += x => { GetStyle().Focused = x; MarkAsModified(); ConfirmModify(); };
                normalOnGUI.OnChanged += x => { GetStyle().NormalOn = x; MarkAsModified(); ConfirmModify(); };
                hoverOnGUI.OnChanged += x => { GetStyle().HoverOn = x; MarkAsModified(); ConfirmModify(); };
                activeOnGUI.OnChanged += x => { GetStyle().ActiveOn = x; MarkAsModified(); ConfirmModify(); };
                focusedOnGUI.OnChanged += x => { GetStyle().FocusedOn = x; MarkAsModified(); ConfirmModify(); };

                borderGUI.OnChanged += x => { GetStyle().Border = x; MarkAsModified(); };
                marginsGUI.OnChanged += x => { GetStyle().Margins = x; MarkAsModified(); };
                contentOffsetGUI.OnChanged += x => { GetStyle().ContentOffset = x; MarkAsModified(); };

                borderGUI.OnConfirmed += ConfirmModify;
                marginsGUI.OnConfirmed += ConfirmModify;
                contentOffsetGUI.OnConfirmed += ConfirmModify;

                fixedWidthField.OnChanged += x => { GetStyle().FixedWidth = x; MarkAsModified(); ConfirmModify(); };
                widthField.OnChanged += x => GetStyle().Width = x;
                widthField.OnFocusLost += ConfirmModify;
                widthField.OnConfirmed += ConfirmModify;
                minWidthField.OnChanged += x => GetStyle().MinWidth = x;
                minWidthField.OnFocusLost += ConfirmModify;
                minWidthField.OnConfirmed += ConfirmModify;
                maxWidthField.OnChanged += x => GetStyle().MaxWidth = x;
                maxWidthField.OnFocusLost += ConfirmModify;
                maxWidthField.OnConfirmed += ConfirmModify;

                fixedHeightField.OnChanged += x => { GetStyle().FixedHeight = x; MarkAsModified(); ConfirmModify(); };
                heightField.OnChanged += x => GetStyle().Height = x;
                heightField.OnFocusLost += ConfirmModify;
                heightField.OnConfirmed += ConfirmModify;
                minHeightField.OnChanged += x => GetStyle().MinHeight = x;
                minHeightField.OnFocusLost += ConfirmModify;
                minHeightField.OnConfirmed += ConfirmModify;
                maxHeightField.OnChanged += x => GetStyle().MaxHeight = x;
                maxHeightField.OnFocusLost += ConfirmModify;
                maxHeightField.OnConfirmed += ConfirmModify;

                foldout.Value = isExpanded;
                panel.Active = isExpanded;
            }

            /// <summary>
            /// Updates all GUI elements from the style if style changes.
            /// </summary>
            /// <param name="style">Style to display in the GUI.</param>
            /// <returns>State representing was anything modified between two last calls to <see cref="Refresh"/>.</returns>
            public InspectableState Refresh(GUIElementStyle style)
            {
                this.style = style;

                InspectableState oldModifiedState = modifiedState;
                if (modifiedState.HasFlag(InspectableState.Modified))
                    modifiedState = InspectableState.NotModified;

                if (style == null)
                    return oldModifiedState;

                fontField.Value = style.Font;
                fontSizeField.Value = style.FontSize;
                horzAlignField.Value = (ulong)style.TextHorzAlign;
                vertAlignField.Value = (ulong)style.TextVertAlign;
                imagePositionField.Value = (ulong)style.ImagePosition;
                wordWrapField.Value = style.WordWrap;

                normalGUI.Refresh(style.Normal);
                hoverGUI.Refresh(style.Hover);
                activeGUI.Refresh(style.Active);
                focusedGUI.Refresh(style.Focused);
                normalOnGUI.Refresh(style.NormalOn);
                hoverOnGUI.Refresh(style.HoverOn);
                activeOnGUI.Refresh(style.ActiveOn);
                focusedOnGUI.Refresh(style.FocusedOn);

                borderGUI.Refresh(style.Border);
                marginsGUI.Refresh(style.Margins);
                contentOffsetGUI.Refresh(style.ContentOffset);

                fixedWidthField.Value = style.FixedWidth;
                widthField.Value = style.Width;
                minWidthField.Value = style.MinWidth;
                maxWidthField.Value = style.MaxWidth;
                fixedHeightField.Value = style.FixedHeight;
                heightField.Value = style.Height;
                minHeightField.Value = style.MinHeight;
                maxHeightField.Value = style.MaxHeight;

                widthField.Active = style.FixedWidth;
                minWidthField.Active = !style.FixedWidth;
                maxWidthField.Active = !style.FixedWidth;

                heightField.Active = style.FixedHeight;
                minHeightField.Active = !style.FixedHeight;
                maxHeightField.Active = !style.FixedHeight;

                return oldModifiedState;
            }

            /// <summary>
            /// Returns the style displayed in the GUI.
            /// </summary>
            /// <returns>Style displayed in the GUI.</returns>
            private GUIElementStyle GetStyle()
            {
                return style;
            }

            /// <summary>
            /// Marks the contents of the style as modified.
            /// </summary>
            private void MarkAsModified()
            {
                modifiedState |= InspectableState.ModifyInProgress;
            }

            /// <summary>
            /// Confirms any queued modifications, signaling parent element.
            /// </summary>
            private void ConfirmModify()
            {
                if (modifiedState.HasFlag(InspectableState.ModifyInProgress))
                    modifiedState |= InspectableState.Modified;
            }

            /// <summary>
            /// Creates GUI elements for editing/displaying <see cref="GUIElementStateStyle"/>
            /// </summary>
            private class GUIElementStateStyleGUI
            {
                private GUIElementStateStyle state;
                private GUIToggle foldout;
                private GUIResourceField textureField;
                private GUIColorField textColorField;
                private bool isExpanded;

                /// <summary>
                /// Triggered when some value in the style state changes.
                /// </summary>
                public Action<GUIElementStateStyle> OnChanged;

                /// <summary>
                /// Creates a new GUI element state style GUI.
                /// </summary>
                public GUIElementStateStyleGUI()
                { }

                /// <summary>
                /// Builds the GUI for the specified state style.
                /// </summary>
                /// <param name="title">Text to display on the title bar.</param>
                /// <param name="layout">Layout to append the GUI elements to.</param>
                public void BuildGUI(LocString title, GUILayout layout)
                {
                    foldout = new GUIToggle(title, EditorStyles.Foldout);
                    textureField = new GUIResourceField(typeof(SpriteTexture), new LocEdString("Texture"));
                    textColorField = new GUIColorField(new LocEdString("Text color"));

                    foldout.OnToggled += x =>
                    {
                        textureField.Active = x;
                        textColorField.Active = x;
                        isExpanded = x;
                    };

                    textureField.OnChanged += x =>
                    {
                        SpriteTexture texture = Resources.Load<SpriteTexture>(x);
                        state.Texture = texture;

                        if (OnChanged != null)
                            OnChanged(state);
                    };

                    textColorField.OnChanged += x =>
                    {
                        state.TextColor = x;

                        if (OnChanged != null)
                            OnChanged(state);
                    };

                    layout.AddElement(foldout);
                    layout.AddElement(textureField);
                    layout.AddElement(textColorField);

                    foldout.Value = isExpanded;
                    textureField.Active = isExpanded;
                    textColorField.Active = isExpanded;
                }

                /// <summary>
                /// Updates all GUI elements from the current state values.
                /// </summary>
                /// <param name="state">State object to display in the GUI.</param>
                public void Refresh(GUIElementStateStyle state)
                {
                    this.state = state;

                    textureField.Value = state.Texture;
                    textColorField.Value = state.TextColor;
                }
            }

            /// <summary>
            /// Creates GUI elements for editing/displaying <see cref="RectOffset"/>
            /// </summary>
            private class RectOffsetGUI
            {
                private RectOffset offset;
                private GUIIntField offsetLeftField;
                private GUIIntField offsetRightField;
                private GUIIntField offsetTopField;
                private GUIIntField offsetBottomField;

                /// <summary>
                /// Triggered when some value in the offset rectangle changes.
                /// </summary>
                public Action<RectOffset> OnChanged;

                /// <summary>
                /// Triggered when the user confirms input.
                /// </summary>
                public Action OnConfirmed;

                /// <summary>
                /// Creates a new rectangle offset GUI.
                /// </summary>
                /// <param name="title">Text to display on the title bar.</param>
                /// <param name="layout">Layout to append the GUI elements to.</param>
                public RectOffsetGUI(LocString title, GUILayout layout)
                {
                    GUILayoutX rectLayout = layout.AddLayoutX();
                    rectLayout.AddElement(new GUILabel(title, GUIOption.FixedWidth(100)));
                    GUILayoutY rectContentLayout = rectLayout.AddLayoutY();

                    GUILayoutX rectTopRow = rectContentLayout.AddLayoutX();
                    GUILayoutX rectBotRow = rectContentLayout.AddLayoutX();

                    offsetLeftField = new GUIIntField(new LocEdString("Left"), 40);
                    offsetRightField = new GUIIntField(new LocEdString("Right"), 40);
                    offsetTopField = new GUIIntField(new LocEdString("Top"), 40);
                    offsetBottomField = new GUIIntField(new LocEdString("Bottom"), 40);

                    rectTopRow.AddElement(offsetLeftField);
                    rectTopRow.AddElement(offsetRightField);
                    rectBotRow.AddElement(offsetTopField);
                    rectBotRow.AddElement(offsetBottomField);

                    offsetLeftField.OnChanged += x =>
                    {
                        offset.left = x;

                        if(OnChanged != null)
                            OnChanged(offset);
                    };

                    offsetRightField.OnChanged += x =>
                    {
                        offset.right = x;

                        if (OnChanged != null)
                            OnChanged(offset);
                    };

                    offsetTopField.OnChanged += x =>
                    {
                        offset.top = x;

                        if (OnChanged != null)
                            OnChanged(offset);
                    };

                    offsetBottomField.OnChanged += x =>
                    {
                        offset.bottom = x;

                        if (OnChanged != null)
                            OnChanged(offset);
                    };

                    Action DoOnConfirmed = () =>
                    {
                        if (OnConfirmed != null)
                            OnConfirmed();
                    };

                    offsetLeftField.OnConfirmed += DoOnConfirmed;
                    offsetLeftField.OnFocusLost += DoOnConfirmed;
                    offsetRightField.OnConfirmed += DoOnConfirmed;
                    offsetRightField.OnFocusLost += DoOnConfirmed;
                    offsetTopField.OnConfirmed += DoOnConfirmed;
                    offsetTopField.OnFocusLost += DoOnConfirmed;
                    offsetBottomField.OnConfirmed += DoOnConfirmed;
                    offsetBottomField.OnFocusLost += DoOnConfirmed;
                }

                /// <summary>
                /// Updates all GUI elements from the offset.
                /// </summary>
                /// <param name="offset">Offset to update the GUI to.</param>
                public void Refresh(RectOffset offset)
                {
                    this.offset = offset;

                    offsetLeftField.Value = offset.left;
                    offsetRightField.Value = offset.right;
                    offsetTopField.Value = offset.top;
                    offsetBottomField.Value = offset.bottom;
                }
            }
        }
    }

    /** @} */
}
