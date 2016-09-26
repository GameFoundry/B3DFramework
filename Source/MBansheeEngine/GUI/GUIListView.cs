//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System.Collections.Generic;

namespace BansheeEngine
{
    /** @addtogroup GUI-Engine
     *  @{
     */

    /// <summary>
    /// GUI element that can efficiently display a list of entries that share the same height. This element is mostly an
    /// optimization as only visible entries have actual GUI elements, as opposed to just adding GUI elements directly
    /// in a vertical GUI layout. This allows the list view to have thousands of elements with little performance impact.
    /// 
    /// Contains shared functionality used by all instances of <see cref="GUIListView{TEntry,TData}"/>.
    /// </summary>
    /// <typeparam name="TData">>Type used for storing the data for all list entries.</typeparam>
    public abstract class GUIListViewBase<TData>
        where TData : GUIListViewData
    {
        // TODO - Only fixed size is supported. It should be nice if this object could just be placed in layout like any
        // other GUI element. Would likely need some kind of a way to get notified when parent layout changes.
        // (Possibly add a callback to GUIPanel when updateLayout is called?)

        protected List<TData> entries = new List<TData>();
        protected GUIScrollArea scrollArea;
        protected GUILabel topPadding;
        protected GUILabel bottomPadding;
        protected int width;
        protected int height;
        protected int entryHeight;
        protected float scrollPct;
        protected bool scrollToLatest;
        protected int totalHeight;
        protected internal bool contentsDirty = true;

        /// <summary>
        /// Total number of entries in the list.
        /// </summary>
        public int NumEntries
        {
            get { return entries.Count; }
        }

        /// <summary>
        /// Height of a single entry in the list, in pixels.
        /// </summary>
        public int EntryHeight
        {
            get { return entryHeight; }
            set { entryHeight = value; }
        }

        /// <summary>
        /// Primary GUI scroll area that all entries are contained within.
        /// </summary>
        internal GUIScrollArea ScrollArea 
        {
            get { return scrollArea; }
        }

        /// <summary>
        /// Creates a new empty list view.
        /// </summary>
        /// <param name="width">Width of the list view, in pixels.</param>
        /// <param name="height">Height of the list view, in pixels.</param>
        /// <param name="entryHeight">Height of a single element in the list, in pixels.</param>
        /// <param name="layout">GUI layout into which the list view will be placed into.</param>
        protected GUIListViewBase(int width, int height, int entryHeight, GUILayout layout)
        {
            scrollArea = new GUIScrollArea(ScrollBarType.ShowIfDoesntFit, ScrollBarType.NeverShow, 
                GUIOption.FixedWidth(width), GUIOption.FixedHeight(height));
            layout.AddElement(scrollArea);

            topPadding = new GUILabel(new LocString());
            bottomPadding = new GUILabel(new LocString());

            scrollArea.Layout.AddElement(topPadding);
            scrollArea.Layout.AddElement(bottomPadding);

            this.width = width;
            this.height = height;
            this.entryHeight = entryHeight;
        }

        /// <summary>
        /// Adds a new entry to the end of the list.
        /// </summary>
        /// <param name="data">Data of the entry to add.</param>
        public void AddEntry(TData data)
        {
            entries.Add(data);
            contentsDirty = true;
        }

        /// <summary>
        /// Removes an entry from the specified index. If the index is out of range nothing happens.
        /// </summary>
        /// <param name="index">Sequential index of the element to remove from the list.</param>
        public void RemoveEntry(int index)
        {
            if (index >= 0 && index < entries.Count)
            {
                entries.RemoveAt(index);
                contentsDirty = true;
            }
        }

        /// <summary>
        /// Removes all entries from the list.
        /// </summary>
        public void Clear()
        {
            entries.Clear();
            contentsDirty = true;
        }

        /// <summary>
        /// Finds an index of the specified entry in the list.
        /// </summary>
        /// <param name="data">Data of the entry to search for.</param>
        /// <returns>Index of the entry if found, -1 otherwise.</returns>
        public int FindEntry(TData data)
        {
            return entries.FindIndex(x => x.Equals(data));
        }

        /// <summary>
        /// Adds a new entry at the specified index. If the index is out of range the entry is added at the end of the list.
        /// </summary>
        /// <param name="index">Sequential index at which to insert the entry. </param>
        /// <param name="data">Data of the entry to insert.</param>
        public void InsertEntry(int index, TData data)
        {
            if (index >= 0 && index <= entries.Count)
                entries.Insert(index, data);
            else
                entries.Add(data);

            contentsDirty = true;
        }

        /// <summary>
        /// Changes the size of the list view.
        /// </summary>
        /// <param name="width">Width in pixels.</param>
        /// <param name="height">Height in pixels.</param>
        public void SetSize(int width, int height)
        {
            if (width != this.width || height != this.height)
            {
                this.width = width;
                this.height = height;

                Rect2I bounds = scrollArea.Bounds;
                bounds.width = width;
                bounds.height = height;
                scrollArea.Bounds = bounds;
            }
        }

        /// <summary>
        /// Updates the visuals of the list view. Should be called once per frame.
        /// </summary>
        public abstract void Update();
    }

    /// <summary>
    /// GUI element that can efficiently display a list of entries that share the same height. This element is mostly an 
    /// optimization as only visible entries have actual GUI elements, as opposed to just adding GUI elements directly
    /// in a vertical GUI layout. This allows the list view to have thousands of elements with little performance impact.
    /// </summary>
    /// <typeparam name="TEntry">Type used for creating and updating the GUI elements of the visible entries.</typeparam>
    /// <typeparam name="TData">Type used for storing the data for all list entries.</typeparam>
    public class GUIListView<TEntry, TData> : GUIListViewBase<TData>
        where TEntry : GUIListViewEntry<TData>, new()
        where TData : GUIListViewData
    {
        private List<TEntry> visibleEntries = new List<TEntry>();

        /// <summary>
        /// Creates a new empty list view.
        /// </summary>
        /// <param name="width">Width of the list view, in pixels.</param>
        /// <param name="height">Height of the list view, in pixels.</param>
        /// <param name="entryHeight">Height of a single element in the list, in pixels.</param>
        /// <param name="layout">GUI layout into which the list view will be placed into.</param>
        public GUIListView(int width, int height, int entryHeight, GUILayout layout)
            :base(width, height, entryHeight, layout)
        { }

        /// <inheritdoc/>
        public override void Update()
        {
            int numVisibleEntries = MathEx.CeilToInt(height / (float)entryHeight) + 1;
            numVisibleEntries = MathEx.Min(numVisibleEntries, entries.Count);

            while (visibleEntries.Count < numVisibleEntries)
            {
                TEntry newEntry = new TEntry();
                newEntry.Initialize(this);
                newEntry.panel.SetHeight(entryHeight);

                visibleEntries.Add(newEntry);
                contentsDirty = true;
            }

            while (numVisibleEntries < visibleEntries.Count)
            {
                int lastIdx = visibleEntries.Count - 1;

                visibleEntries[lastIdx].Destroy();
                visibleEntries.RemoveAt(lastIdx);

                contentsDirty = true;
            }

            if (scrollPct != scrollArea.VerticalScroll)
            {
                scrollPct = scrollArea.VerticalScroll;
                contentsDirty = true;
            }

            if (contentsDirty)
            {
                int newHeight = entries.Count * entryHeight;
                if (scrollToLatest)
                {
                    if (totalHeight > height && scrollPct < 1.0f)
                        scrollToLatest = false;
                }
                else
                {
                    if (totalHeight <= height || scrollPct >= 1.0f)
                        scrollToLatest = true;
                }

                totalHeight = newHeight;
                int maxScrollOffset = MathEx.Max(0, totalHeight - height - 1);

                float newScrollPct;
                if (!scrollToLatest)
                {
                    // Calculate the new scroll pct (which will be active after we change the top/bottom padding element
                    // sizes). If we use the existing scroll pct instead then the elements will lag one frame behind, which
                    // can be very noticeable on quickly updating lists.
                    newScrollPct = (scrollPct*scrollArea.Layout.Bounds.height)/totalHeight;
                }
                else
                    newScrollPct = 1.0f;

                int startPos = MathEx.FloorToInt(newScrollPct * maxScrollOffset);
                int startIndex = MathEx.FloorToInt(startPos / (float)entryHeight);

                // Check if we're at the list bottom and the extra element is out of bounds
                if ((startIndex + visibleEntries.Count) > entries.Count)
                    startIndex--; // Keep the extra element at the top always

                topPadding.SetHeight(startIndex * entryHeight);

                for (int i = 0; i < visibleEntries.Count; i++)
                    visibleEntries[i].UpdateContents(startIndex + i, entries[startIndex + i]);

                int bottomPosition = MathEx.Min(totalHeight, (startIndex + visibleEntries.Count) * entryHeight);
                bottomPadding.SetHeight(totalHeight - bottomPosition);

                if (scrollToLatest)
                {
                    if (newHeight <= height)
                        scrollArea.VerticalScroll = 0.0f;
                    else
                        scrollArea.VerticalScroll = 1.0f;
                }

                contentsDirty = false;
            }
        }
    }

    /// <summary>
    /// Base class that contains data for individual entries used in <see cref="GUIListView{TEntry,TData}"/>.
    /// </summary>
    public class GUIListViewData
    {
        
    }

    /// <summary>
    /// Base class that displays GUI elements for visible entries used in <see cref="GUIListView{TEntry,TData}"/>.
    /// </summary>
    /// <typeparam name="TData">Type of object that contains data used for initializing the GUI elements.</typeparam>
    public abstract class GUIListViewEntry<TData>
        where TData : GUIListViewData
    {
        private GUIListViewBase<TData> parent;
        internal GUIPanel panel;
        internal GUILayoutY layout;

        protected GUILayout Layout { get { return layout; } }

        /// <summary>
        /// Initializes the GUI elements for the entry.
        /// </summary>
        /// <param name="parent">Scroll area into whose layout to insert the GUI elements.</param>
        internal void Initialize(GUIListViewBase<TData> parent)
        {
            this.parent = parent;

            GUIScrollArea scrollArea = parent.ScrollArea;
            int numElements = scrollArea.Layout.ChildCount;

            // Last panel is always the padding panel, so keep it there
            panel = scrollArea.Layout.InsertPanel(numElements - 1);
            layout = panel.AddLayoutY();

            BuildGUI();
        }

        /// <summary>
        /// Destoys the GUI elements for the entry.
        /// </summary>
        internal void Destroy()
        {
            panel.Destroy();
        }

        /// <summary>
        /// Causes all visible entries in the parent list to be updated.
        /// </summary>
        protected void RefreshEntries()
        {
            parent.contentsDirty = true;
        }

        /// <summary>
        /// Allows child classes to create GUI elements required by their entry specialization.
        /// </summary>
        public abstract void BuildGUI();

        /// <summary>
        /// Allows child classes to update GUI element(s) with new contents.
        /// </summary>
        /// <param name="index">Sequential index of the entry in the list.</param>
        /// <param name="data">Data of the entry to display.</param>
        public abstract void UpdateContents(int index, TData data);
    }

    /** @} */
}
