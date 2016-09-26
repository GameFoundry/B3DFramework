//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.Collections;
using System.Collections.Generic;
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup GUI-Editor 
     *  @{
     */

    /// <summary>
    /// Base class for objects that display GUI for a modifyable list of elements. Elements can be added, removed and moved.
    /// </summary>
    public abstract class GUIListFieldBase
    {
        private const int IndentAmount = 5;

        protected List<GUIListFieldRow> rows = new List<GUIListFieldRow>();
        protected GUILayoutY guiLayout;
        protected GUIIntField guiSizeField;
        protected GUILayoutX guiChildLayout;
        protected GUILayoutX guiTitleLayout;
        protected GUILayoutX guiInternalTitleLayout;
        protected GUILayoutY guiContentLayout;
        protected GUIToggle guiFoldout;

        protected bool isExpanded;
        protected int depth;
        protected LocString title;

        private State state;
        private bool isModified;

        /// <summary>
        /// Expands or collapses the entries of the dictionary.
        /// </summary>
        public bool IsExpanded
        {
            get { return isExpanded; }
            set
            {
                if (isExpanded != value)
                    ToggleFoldout(value, true);
            }
        }

        /// <summary>
        /// Event that triggers when the list foldout is expanded or collapsed (rows are shown or hidden).
        /// </summary>
        public Action<bool> OnExpand;

        /// <summary>
        /// Constructs a new GUI list.
        /// </summary>
        /// <param name="title">Label to display on the list GUI title.</param>
        /// <param name="layout">Layout to which to append the array GUI elements to.</param>
        /// <param name="depth">Determines at which depth to render the background. Useful when you have multiple
        ///                     nested containers whose backgrounds are overlaping. Also determines background style,
        ///                     depths divisible by two will use an alternate style.</param>
        protected GUIListFieldBase(LocString title, GUILayout layout, int depth)
        {
            this.title = title;
            this.depth = depth;
            guiLayout = layout.AddLayoutY();
            guiTitleLayout = guiLayout.AddLayoutX();
        }

        /// <summary>
        /// (Re)builds the list GUI elements. Must be called at least once in order for the contents to be populated.
        /// </summary>
        public void BuildGUI()
        {
            UpdateHeaderGUI();

            if (!IsNull())
            {
                // Hidden dependency: Initialize must be called after all elements are 
                // in the dictionary so we do it in two steps
                int numRows = GetNumRows();
                int oldNumRows = rows.Count;

                for (int i = oldNumRows; i < numRows; i++)
                {
                    GUIListFieldRow newRow = CreateRow();
                    rows.Add(newRow);
                }

                for (int i = oldNumRows - 1; i >= numRows; i--)
                {
                    rows[i].Destroy();
                    rows.RemoveAt(i);
                }

                for (int i = oldNumRows; i < numRows; i++)
                    rows[i].Initialize(this, guiContentLayout, i, depth + 1);

                for (int i = 0; i < rows.Count; i++)
                    rows[i].SetIndex(i);
            }
            else
            {
                foreach (var row in rows)
                    row.Destroy();

                rows.Clear();
            }
        }

        /// <summary>
        /// Rebuilds the GUI list header if needed.
        /// </summary>
        protected void UpdateHeaderGUI()
        {
            Action BuildEmptyGUI = () =>
            {
                guiInternalTitleLayout = guiTitleLayout.InsertLayoutX(0);

                guiInternalTitleLayout.AddElement(new GUILabel(title));
                guiInternalTitleLayout.AddElement(new GUILabel("Empty", GUIOption.FixedWidth(100)));

                GUIContent createIcon = new GUIContent(EditorBuiltin.GetInspectorWindowIcon(InspectorWindowIcon.Create), 
                    new LocEdString("Create"));
                GUIButton createBtn = new GUIButton(createIcon, GUIOption.FixedWidth(30));
                createBtn.OnClick += OnCreateButtonClicked;
                guiInternalTitleLayout.AddElement(createBtn);
            };

            Action BuildFilledGUI = () =>
            {
                guiInternalTitleLayout = guiTitleLayout.InsertLayoutX(0);

                guiFoldout = new GUIToggle(title, EditorStyles.Foldout);
                guiFoldout.Value = isExpanded;
                guiFoldout.OnToggled += x => ToggleFoldout(x, false);
                guiSizeField = new GUIIntField("", GUIOption.FixedWidth(50));
                guiSizeField.SetRange(0, int.MaxValue);

                GUIContent resizeIcon = new GUIContent(EditorBuiltin.GetInspectorWindowIcon(InspectorWindowIcon.Resize), 
                    new LocEdString("Resize"));
                GUIButton guiResizeBtn = new GUIButton(resizeIcon, GUIOption.FixedWidth(30));
                guiResizeBtn.OnClick += OnResizeButtonClicked;

                GUIContent clearIcon = new GUIContent(EditorBuiltin.GetInspectorWindowIcon(InspectorWindowIcon.Clear), 
                    new LocEdString("Clear"));
                GUIButton guiClearBtn = new GUIButton(clearIcon, GUIOption.FixedWidth(30));
                guiClearBtn.OnClick += OnClearButtonClicked;

                guiInternalTitleLayout.AddElement(guiFoldout);
                guiInternalTitleLayout.AddElement(guiSizeField);
                guiInternalTitleLayout.AddElement(guiResizeBtn);
                guiInternalTitleLayout.AddElement(guiClearBtn);

                guiSizeField.Value = GetNumRows();

                guiChildLayout = guiLayout.AddLayoutX();
                guiChildLayout.AddSpace(IndentAmount);
                guiChildLayout.Active = isExpanded;

                GUIPanel guiContentPanel = guiChildLayout.AddPanel();
                GUILayoutX guiIndentLayoutX = guiContentPanel.AddLayoutX();
                guiIndentLayoutX.AddSpace(IndentAmount);
                GUILayoutY guiIndentLayoutY = guiIndentLayoutX.AddLayoutY();
                guiIndentLayoutY.AddSpace(IndentAmount);
                guiContentLayout = guiIndentLayoutY.AddLayoutY();
                guiIndentLayoutY.AddSpace(IndentAmount);
                guiIndentLayoutX.AddSpace(IndentAmount);
                guiChildLayout.AddSpace(IndentAmount);

                short backgroundDepth = (short)(Inspector.START_BACKGROUND_DEPTH - depth - 1);
                string bgPanelStyle = depth % 2 == 0
                    ? EditorStylesInternal.InspectorContentBgAlternate
                    : EditorStylesInternal.InspectorContentBg;

                GUIPanel backgroundPanel = guiContentPanel.AddPanel(backgroundDepth);
                GUITexture inspectorContentBg = new GUITexture(null, bgPanelStyle);
                backgroundPanel.AddElement(inspectorContentBg);
            };

            if (state == State.None)
            {
                if (!IsNull())
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
                if (!IsNull())
                {
                    guiInternalTitleLayout.Destroy();
                    BuildFilledGUI();
                    state = State.Filled;
                }
            }
            else if (state == State.Filled)
            {
                if (IsNull())
                {
                    guiInternalTitleLayout.Destroy();
                    guiChildLayout.Destroy();
                    BuildEmptyGUI();

                    state = State.Empty;
                }
            }
        }

        /// <summary>
        /// Returns the layout that is used for positioning the elements in the title bar.
        /// </summary>
        /// <returns>Horizontal layout for positioning the title bar elements.</returns>
        public GUILayoutX GetTitleLayout()
        {
            return guiTitleLayout;
        }

        /// <summary>
        /// Refreshes contents of all list rows and checks if anything was modified.
        /// </summary>
        /// <returns>State representing was anything modified between two last calls to <see cref="Refresh"/>.</returns>
        public virtual InspectableState Refresh()
        {
            InspectableState state = InspectableState.NotModified;

            for (int i = 0; i < rows.Count; i++)
                state |= rows[i].Refresh();

            if (isModified)
            {
                state |= InspectableState.Modified;
                isModified = false;
            }

            return state;
        }

        /// <summary>
        /// Destroys the GUI elements.
        /// </summary>
        public void Destroy()
        {
            if (guiTitleLayout != null)
            {
                guiTitleLayout.Destroy();
                guiTitleLayout = null;
            }

            if (guiChildLayout != null)
            {
                guiChildLayout.Destroy();
                guiChildLayout = null;
            }

            for (int i = 0; i < rows.Count; i++)
                rows[i].Destroy();

            rows.Clear();
        }

        /// <summary>
        /// Creates a new list row GUI.
        /// </summary>
        /// <returns>Object containing the list row GUI.</returns>
        protected abstract GUIListFieldRow CreateRow();

        /// <summary>
        /// Checks is the list instance not assigned.
        /// </summary>
        /// <returns>True if there is not a list instance.</returns>
        protected abstract bool IsNull();

        /// <summary>
        /// Returns the number of rows in the list.
        /// </summary>
        /// <returns>Number of rows in the list.</returns>
        protected abstract int GetNumRows();

        /// <summary>
        /// Gets a value of an element at the specified index in the list.
        /// </summary>
        /// <param name="seqIndex">Sequential index of the element whose value to retrieve.</param>
        /// <returns>Value of the list element at the specified index.</returns>
        protected internal abstract object GetValue(int seqIndex);

        /// <summary>
        /// Sets a value of an element at the specified index in the list.
        /// </summary>
        /// <param name="seqIndex">Sequential index of the element whose value to set.</param>
        /// <param name="value">Value to assign to the element. Caller must ensure it is of valid type.</param>
        protected internal abstract void SetValue(int seqIndex, object value);

        /// <summary>
        /// Triggered when the user clicks on the expand/collapse toggle in the title bar.
        /// </summary>
        /// <param name="expanded">Determines whether the contents were expanded or collapsed.</param>
        /// <param name="external">True if the foldout was expanded/collapsed from outside code.</param>
        private void ToggleFoldout(bool expanded, bool external)
        {
            isExpanded = expanded;

            if (guiChildLayout != null)
                guiChildLayout.Active = isExpanded;

            if (external)
            {
                if (guiFoldout != null)
                    guiFoldout.Value = isExpanded;
            }

            if (OnExpand != null)
                OnExpand(expanded);
        }

        /// <summary>
        /// Triggered when the user clicks on the create button on the title bar. Creates a brand new list with zero
        /// elements in the place of the current list.
        /// </summary>
        protected void OnCreateButtonClicked()
        {
            CreateList();

            BuildGUI();
            isModified = true;
        }

        /// <summary>
        /// Triggered when the user clicks on the resize button on the title bar. Changes the size of the list while
        /// preserving existing contents.
        /// </summary>
        protected void OnResizeButtonClicked()
        {
            ResizeList();

            BuildGUI();
            isModified = true;
        }

        /// <summary>
        /// Triggered when the user clicks on the clear button on the title bar. Deletes the current list object.
        /// </summary>
        protected void OnClearButtonClicked()
        {
            ClearList();

            BuildGUI();
            isModified = true;
        }

        /// <summary>
        /// Triggered when the user clicks on the delete button next to a list entry. Deletes an element in the list.
        /// </summary>
        /// <param name="index">Sequential index of the element in the list to remove.</param>
        protected internal void OnDeleteButtonClicked(int index)
        {
            DeleteElement(index);

            guiSizeField.Value = GetNumRows();
            BuildGUI();
            isModified = true;
        }

        /// <summary>
        /// Triggered when the user clicks on the clone button next to a list entry. Clones the element and adds the clone 
        /// to the back of the list. 
        /// </summary>
        /// <param name="index">Sequential index of the element in the list to clone.</param>
        protected internal void OnCloneButtonClicked(int index)
        {
            CloneElement(index);

            guiSizeField.Value = GetNumRows();
            BuildGUI();
            isModified = true;
        }

        /// <summary>
        /// Triggered when the user clicks on the move up button next to a list entry. Moves an element from the current
        /// list index to the one right before it, if not at zero.
        /// </summary>
        /// <param name="index">Sequential index of the element in the list to move.</param>
        protected internal void OnMoveUpButtonClicked(int index)
        {
            MoveUpElement(index);

            BuildGUI();
            isModified = true;
        }

        /// <summary>
        /// Triggered when the user clicks on the move down button next to a list entry. Moves an element from the current
        /// list index to the one right after it, if the element isn't already the last element.
        /// </summary>
        /// <param name="index">Sequential index of the element in the list to move.</param>
        protected internal void OnMoveDownButtonClicked(int index)
        {
            MoveDownElement(index);

            BuildGUI();
            isModified = true;
        }

        /// <summary>
        /// Creates a brand new list with zero elements in the place of the current list.
        /// </summary>
        protected abstract void CreateList();

        /// <summary>
        /// Changes the size of the list while preserving existing contents.
        /// </summary>
        protected abstract void ResizeList();

        /// <summary>
        /// Deletes the current list object.
        /// </summary>
        protected abstract void ClearList();

        /// <summary>
        /// Deletes an element in the list.
        /// </summary>
        /// <param name="index">Sequential index of the element in the list to remove.</param>
        protected internal abstract void DeleteElement(int index);

        /// <summary>
        /// Clones the element and adds the clone to the back of the list. 
        /// </summary>
        /// <param name="index">Sequential index of the element in the list to clone.</param>
        protected internal abstract void CloneElement(int index);

        /// <summary>
        /// Moves an element from the current list index to the one right before it, if not at zero.
        /// </summary>
        /// <param name="index">Sequential index of the element in the list to move.</param>
        protected internal abstract void MoveUpElement(int index);

        /// <summary>
        /// Moves an element from the current list index to the one right after it, if the element isn't already the last 
        /// element.
        /// </summary>
        /// <param name="index">Sequential index of the element in the list to move.</param>
        protected internal abstract void MoveDownElement(int index);

        /// <summary>
        /// Possible states list GUI can be in.
        /// </summary>
        private enum State
        {
            None,
            Empty,
            Filled
        }
    }

    /// <summary>
    /// Creates GUI elements that allow viewing and manipulation of a <see cref="System.Array"/>. When constructing the
    /// object user can provide a custom type that manages GUI for individual array elements.
    /// </summary>
    /// <typeparam name="ElementType">Type of elements stored in the array.</typeparam>
    /// <typeparam name="RowType">Type of rows that are used to handle GUI for individual array elements.</typeparam>
    public class GUIArrayField<ElementType, RowType> : GUIListFieldBase where RowType : GUIListFieldRow, new() 
    {
        /// <summary>
        /// Triggered when the reference array has been changed. This does not include changes that only happen to its 
        /// internal elements.
        /// </summary>
        public Action<ElementType[]> OnChanged;

        /// <summary>
        /// Triggered when an element in the array has been changed.
        /// </summary>
        public Action OnValueChanged;

        /// <summary>
        /// Array object whose contents are displayed.
        /// </summary>
        public ElementType[] Array { get { return array; } }
        protected ElementType[] array;

        /// <summary>
        /// Constructs a new GUI array field.
        /// </summary>
        /// <param name="title">Label to display on the array GUI title.</param>
        /// <param name="array">Object containing the array data. Can be null.</param>
        /// <param name="layout">Layout to which to append the array GUI elements to.</param>
        /// <param name="depth">Determines at which depth to render the background. Useful when you have multiple
        ///                     nested containers whose backgrounds are overlaping. Also determines background style,
        ///                     depths divisible by two will use an alternate style.</param>
        protected GUIArrayField(LocString title, ElementType[] array, GUILayout layout, int depth = 0)
            :base(title, layout, depth)
        {
            this.array = array;
        }

        /// <summary>
        /// Creates a array GUI field containing GUI elements for displaying an array.
        /// </summary>
        /// <param name="title">Label to display on the array GUI title.</param>
        /// <param name="array">Object containing the array data. Can be null.</param>
        /// <param name="layout">Layout to which to append the array GUI elements to.</param>
        /// <param name="depth">Determines at which depth to render the background. Useful when you have multiple
        ///                     nested containers whose backgrounds are overlaping. Also determines background style,
        ///                     depths divisible by two will use an alternate style.</param>
        /// <returns>New instance of an array GUI field.</returns>
        public static GUIArrayField<ElementType, RowType> Create(LocString title, ElementType[] array, GUILayout layout, 
            int depth = 0)
        {
            GUIArrayField<ElementType, RowType> guiArray = new GUIArrayField<ElementType, RowType>(title, array, layout,
                depth);

            guiArray.BuildGUI();
            return guiArray;
        }

        /// <inheritdoc/>
        protected override GUIListFieldRow CreateRow()
        {
            return new RowType();
        }

        /// <inheritdoc/>
        protected override bool IsNull()
        {
            return array == null;
        }

        /// <inheritdoc/>
        protected override int GetNumRows()
        {
            if (array != null)
                return array.GetLength(0);

            return 0;
        }

        /// <inheritdoc/>
        protected internal override object GetValue(int seqIndex)
        {
            return array.GetValue(seqIndex);
        }

        /// <inheritdoc/>
        protected internal override void SetValue(int seqIndex, object value)
        {
            array.SetValue(value, seqIndex);

            if (OnValueChanged != null)
                OnValueChanged();
        }

        /// <inheritdoc/>
        protected override void CreateList()
        {
            array = new ElementType[0];

            if (OnChanged != null)
                OnChanged(array);
        }

        /// <inheritdoc/>
        protected override void ResizeList()
        {
            int size = guiSizeField.Value;

            ElementType[] newArray = new ElementType[size];

            int oldSize = array.GetLength(0);
            int maxSize = MathEx.Min(size, oldSize);
            for (int i = 0; i < maxSize; i++)
                newArray.SetValue(array.GetValue(i), i);

            array = newArray;

            if (OnChanged != null)
                OnChanged(array);
        }

        /// <inheritdoc/>
        protected override void ClearList()
        {
            array = null;

            if (OnChanged != null)
                OnChanged(array);
        }

        /// <inheritdoc/>
        protected internal override void DeleteElement(int index)
        {
            int size = MathEx.Max(0, array.GetLength(0) - 1);
            ElementType[] newArray = new ElementType[size];

            int destIdx = 0;
            for (int i = 0; i < array.GetLength(0); i++)
            {
                if (i == index)
                    continue;

                newArray.SetValue(array.GetValue(i), destIdx);
                destIdx++;
            }

            array = newArray;

            if (OnChanged != null)
                OnChanged(array);
        }

        /// <inheritdoc/>
        protected internal override void CloneElement(int index)
        {
            int size = array.GetLength(0) + 1;
            ElementType[] newArray = new ElementType[size];

            object clonedEntry = null;
            for (int i = 0; i < array.GetLength(0); i++)
            {
                object value = array.GetValue(i);
                newArray.SetValue(value, i);

                if (i == index)
                {
                    if (value == null)
                        clonedEntry = null;
                    else
                        clonedEntry = SerializableUtility.Clone(value);
                }
            }

            newArray.SetValue(clonedEntry, size - 1);

            array = newArray;

            if (OnChanged != null)
                OnChanged(array);
        }

        /// <inheritdoc/>
        protected internal override void MoveUpElement(int index)
        {
            if ((index - 1) >= 0)
            {
                object previousEntry = array.GetValue(index - 1);

                array.SetValue(array.GetValue(index), index - 1);
                array.SetValue(previousEntry, index);

                if (OnValueChanged != null)
                    OnValueChanged();
            }
        }

        /// <inheritdoc/>
        protected internal override void MoveDownElement(int index)
        {
            if ((index + 1) < array.GetLength(0))
            {
                object nextEntry = array.GetValue(index + 1);

                array.SetValue(array.GetValue(index), index + 1);
                array.SetValue(nextEntry, index);

                if (OnValueChanged != null)
                    OnValueChanged();
            }
        }
    }

    /// <summary>
    /// Creates GUI elements that allow viewing and manipulation of a <see cref="List{T}"/>. When constructing the
    /// object user can provide a custom type that manages GUI for individual list elements.
    /// </summary>
    /// <typeparam name="ElementType">Type of elements stored in the list.</typeparam>
    /// <typeparam name="RowType">Type of rows that are used to handle GUI for individual list elements.</typeparam>
    public class GUIListField<ElementType, RowType> : GUIListFieldBase where RowType : GUIListFieldRow, new()
    {
        /// <summary>
        /// Triggered when the reference list has been changed. This does not include changes that only happen to its 
        /// internal elements.
        /// </summary>
        public Action<List<ElementType>> OnChanged;

        /// <summary>
        /// Triggered when an element in the list has been changed.
        /// </summary>
        public Action OnValueChanged;

        /// <summary>
        /// List object whose contents are displayed.
        /// </summary>
        public List<ElementType> List { get { return list; } }
        protected List<ElementType> list;

        /// <summary>
        /// Constructs a new GUI list field.
        /// </summary>
        /// <param name="title">Label to display on the list GUI title.</param>
        /// <param name="list">Object containing the list data. Can be null.</param>
        /// <param name="layout">Layout to which to append the list GUI elements to.</param>
        /// <param name="depth">Determines at which depth to render the background. Useful when you have multiple
        ///                     nested containers whose backgrounds are overlaping. Also determines background style,
        ///                     depths divisible by two will use an alternate style.</param>
        protected GUIListField(LocString title, List<ElementType> list, GUILayout layout, int depth = 0)
            : base(title, layout, depth)
        {
            this.list = list;
        }

        /// <summary>
        /// Creates a list GUI field containing GUI elements for displaying a list.
        /// </summary>
        /// <param name="title">Label to display on the list GUI title.</param>
        /// <param name="list">Object containing the list data. Can be null.</param>
        /// <param name="layout">Layout to which to append the list GUI elements to.</param>
        /// <param name="depth">Determines at which depth to render the background. Useful when you have multiple
        ///                     nested containers whose backgrounds are overlaping. Also determines background style,
        ///                     depths divisible by two will use an alternate style.</param>
        /// <returns>New instance of a list GUI field.</returns>
        public static GUIListField<ElementType, RowType> Create(LocString title, List<ElementType> list, GUILayout layout, 
            int depth = 0)
        {
            GUIListField<ElementType, RowType> guiList = new GUIListField<ElementType, RowType>(title, list, layout, depth);
            guiList.BuildGUI();

            return guiList;
        }

        /// <inheritdoc/>
        protected override GUIListFieldRow CreateRow()
        {
            return new RowType();
        }

        /// <inheritdoc/>
        protected override bool IsNull()
        {
            return list == null;
        }

        /// <inheritdoc/>
        protected override int GetNumRows()
        {
            if (list != null)
                return list.Count;

            return 0;
        }

        /// <inheritdoc/>
        protected internal override object GetValue(int seqIndex)
        {
            return list[seqIndex];
        }

        /// <inheritdoc/>
        protected internal override void SetValue(int seqIndex, object value)
        {
            list[seqIndex] = (ElementType)value;

            if (OnValueChanged != null)
                OnValueChanged();
        }

        /// <inheritdoc/>
        protected override void CreateList()
        {
            list = new List<ElementType>();

            if (OnChanged != null)
                OnChanged(list);
        }

        /// <inheritdoc/>
        protected override void ResizeList()
        {
            int size = guiSizeField.Value;
            if(size == list.Count)
                return;

            if (size < list.Count)
                list.RemoveRange(size, list.Count - size);
            else
            {
                ElementType[] extraElements = new ElementType[size - list.Count];
                list.AddRange(extraElements);
            }

            if (OnValueChanged != null)
                OnValueChanged();
        }

        /// <inheritdoc/>
        protected override void ClearList()
        {
            list = null;

            if (OnChanged != null)
                OnChanged(list);
        }

        /// <inheritdoc/>
        protected internal override void DeleteElement(int index)
        {
            list.RemoveAt(index);

            if (OnValueChanged != null)
                OnValueChanged();
        }

        /// <inheritdoc/>
        protected internal override void CloneElement(int index)
        {
            object clonedEntry = null;
            if (list[index] != null)
                clonedEntry = SerializableUtility.Clone(list[index]);

            list.Add((ElementType)clonedEntry);

            if (OnValueChanged != null)
                OnValueChanged();
        }

        /// <inheritdoc/>
        protected internal override void MoveUpElement(int index)
        {
            if ((index - 1) >= 0)
            {
                ElementType previousEntry = list[index - 1];

                list[index - 1] = list[index];
                list[index] = previousEntry;

                if (OnValueChanged != null)
                    OnValueChanged();
            }
        }

        /// <inheritdoc/>
        protected internal override void MoveDownElement(int index)
        {
            if ((index + 1) < list.Count)
            {
                ElementType nextEntry = list[index + 1];

                list[index + 1] = list[index];
                list[index] = nextEntry;

                if (OnValueChanged != null)
                    OnValueChanged();
            }
        }
    }

    /// <summary>
    /// Contains GUI elements for a single entry in a list.
    /// </summary>
    public abstract class GUIListFieldRow
    {
        private GUILayoutX rowLayout;
        private GUILayoutY contentLayout;
        private GUILayoutX titleLayout;
        private bool localTitleLayout;
        private int seqIndex;
        private int depth;
        private InspectableState modifiedState;
        protected GUIListFieldBase parent;

        /// <summary>
        /// Returns the sequential index of the list entry that this row displays.
        /// </summary>
        protected int SeqIndex { get { return seqIndex; } }

        /// <summary>
        /// Returns the depth at which the row is rendered.
        /// </summary>
        protected int Depth { get { return depth; } }

        /// <summary>
        /// Constructs a new list row object.
        /// </summary>
        protected GUIListFieldRow()
        {

        }

        /// <summary>
        /// Initializes the row and creates row GUI elements.
        /// </summary>
        /// <param name="parent">Parent array GUI object that the entry is contained in.</param>
        /// <param name="parentLayout">Parent layout that row GUI elements will be added to.</param>
        /// <param name="seqIndex">Sequential index of the list entry.</param>
        /// <param name="depth">Determines the depth at which the element is rendered.</param>
        internal void Initialize(GUIListFieldBase parent, GUILayout parentLayout, int seqIndex, int depth)
        {
            this.parent = parent;
            this.seqIndex = seqIndex;
            this.depth = depth;

            rowLayout = parentLayout.AddLayoutX();
            contentLayout = rowLayout.AddLayoutY();

            BuildGUI();
        }

        /// <summary>
        /// Changes the index of the list element this row represents.
        /// </summary>
        /// <param name="seqIndex">Sequential index of the list entry.</param>
        internal void SetIndex(int seqIndex)
        {
            this.seqIndex = seqIndex;
        }

        /// <summary>
        /// (Re)creates all row GUI elements.
        /// </summary>
        internal protected void BuildGUI()
        {
            contentLayout.Clear();

            GUILayoutX externalTitleLayout = CreateGUI(contentLayout);
            if (localTitleLayout || (titleLayout != null && titleLayout == externalTitleLayout))
                return;

            if (externalTitleLayout != null)
            {
                localTitleLayout = false;
                titleLayout = externalTitleLayout;
            }
            else
            {
                GUILayoutY buttonCenter = rowLayout.AddLayoutY();
                buttonCenter.AddFlexibleSpace();
                titleLayout = buttonCenter.AddLayoutX();
                buttonCenter.AddFlexibleSpace();

                localTitleLayout = true;
            }
            
            GUIContent cloneIcon = new GUIContent(EditorBuiltin.GetInspectorWindowIcon(InspectorWindowIcon.Clone), 
                new LocEdString("Clone"));
            GUIContent deleteIcon = new GUIContent(EditorBuiltin.GetInspectorWindowIcon(InspectorWindowIcon.Delete), 
                new LocEdString("Delete"));
            GUIContent moveUp = new GUIContent(EditorBuiltin.GetInspectorWindowIcon(InspectorWindowIcon.MoveUp), 
                new LocEdString("Move up"));
            GUIContent moveDown = new GUIContent(EditorBuiltin.GetInspectorWindowIcon(InspectorWindowIcon.MoveDown), 
                new LocEdString("Move down"));

            GUIButton cloneBtn = new GUIButton(cloneIcon, GUIOption.FixedWidth(30));
            GUIButton deleteBtn = new GUIButton(deleteIcon, GUIOption.FixedWidth(30));
            GUIButton moveUpBtn = new GUIButton(moveUp, GUIOption.FixedWidth(30));
            GUIButton moveDownBtn = new GUIButton(moveDown, GUIOption.FixedWidth(30));

            cloneBtn.OnClick += () => parent.OnCloneButtonClicked(seqIndex);
            deleteBtn.OnClick += () => parent.OnDeleteButtonClicked(seqIndex);
            moveUpBtn.OnClick += () => parent.OnMoveUpButtonClicked(seqIndex);
            moveDownBtn.OnClick += () => parent.OnMoveDownButtonClicked(seqIndex);

            titleLayout.AddElement(cloneBtn);
            titleLayout.AddElement(deleteBtn);
            titleLayout.AddElement(moveUpBtn);
            titleLayout.AddElement(moveDownBtn);
        }

        /// <summary>
        /// Creates GUI elements specific to type in the array row.
        /// </summary>
        /// <param name="layout">Layout to insert the row GUI elements to.</param>
        /// <returns>An optional title bar layout that the standard array buttons will be appended to.</returns>
        protected abstract GUILayoutX CreateGUI(GUILayoutY layout);
        
        /// <summary>
        /// Refreshes the GUI for the list row and checks if anything was modified.
        /// </summary>
        /// <returns>State representing was anything modified between two last calls to <see cref="Refresh"/>.</returns>
        protected internal virtual InspectableState Refresh()
        {
            InspectableState oldState = modifiedState;
            if (modifiedState.HasFlag(InspectableState.Modified))
                modifiedState = InspectableState.NotModified;

            return oldState;
        }

        /// <summary>
        /// Marks the contents of the row as modified.
        /// </summary>
        protected void MarkAsModified()
        {
            modifiedState |= InspectableState.ModifyInProgress;
        }

        /// <summary>
        /// Confirms any queued modifications, signaling parent elements.
        /// </summary>
        protected void ConfirmModify()
        {
            if (modifiedState.HasFlag(InspectableState.ModifyInProgress))
                modifiedState |= InspectableState.Modified;
        }

        /// <summary>
        /// Gets the value contained in this list row.
        /// </summary>
        /// <typeparam name="T">Type of the value. Must match the list's element type.</typeparam>
        /// <returns>Value in this list row.</returns>
        protected T GetValue<T>()
        {
            return (T)parent.GetValue(seqIndex);
        }

        /// <summary>
        /// Sets the value contained in this list row.
        /// </summary>
        /// <typeparam name="T">Type of the value. Must match the list's element type.</typeparam>
        /// <param name="value">Value to set.</param>
        protected void SetValue<T>(T value)
        {
            parent.SetValue(seqIndex, value);
        }

        /// <summary>
        /// Destroys all row GUI elements.
        /// </summary>
        public void Destroy()
        {
            if (rowLayout != null)
            {
                rowLayout.Destroy();
                rowLayout = null;
            }

            contentLayout = null;
            titleLayout = null;
            localTitleLayout = false;
        }
    }

    /** @} */
}
