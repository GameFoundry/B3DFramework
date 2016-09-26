//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.Collections.Generic;
using System.IO;
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup Library
     *  @{
     */

    /// <summary>
    /// Types of resource tile display in the library window.
    /// </summary>
    internal enum ProjectViewType
    {
        Grid64, Grid48, Grid32, List16
    }

    /// <summary>
    /// Editor window that displays all resources in the project. Resources can be displayed as a grid or list of icons,
    /// with the ability to move, cut, copy, paste resources and folders, as well as supporting drag and drop and search
    /// operations.
    /// </summary>
    internal sealed class LibraryWindow : EditorWindow, IGlobalShortcuts
    {
        /// <summary>
        /// Directions the selection cursor in library window can be moved in.
        /// </summary>
        internal enum MoveDirection
        {
            Up, Down, Left, Right
        }

        private const int DRAG_SCROLL_HEIGHT = 20;
        private const int DRAG_SCROLL_AMOUNT_PER_SECOND = 300;
        private const int FOLDER_BUTTON_WIDTH = 30;
        private const int FOLDER_SEPARATOR_WIDTH = 10;
        private const string CURRENT_LIBRARY_DIRECTORY_KEY = "__CurrentLibDir";

        private bool hasContentFocus = false;
        private bool HasContentFocus { get { return HasFocus && hasContentFocus; } }

        private string searchQuery;
        private bool IsSearchActive { get { return !string.IsNullOrEmpty(searchQuery); } }

        private ProjectViewType viewType = ProjectViewType.Grid48;

        private bool requiresRefresh;
        private List<string> selectionPaths = new List<string>();
        private int selectionAnchorStart = -1;
        private int selectionAnchorEnd = -1;
        private string pingPath = "";
        private string hoverHighlightPath = "";

        private LibraryGUIContent content;
        private GUIScrollArea contentScrollArea;
        private GUILayoutX searchBarLayout;
        private GUIButton optionsButton;
        private GUILayout folderBarLayout;
        private GUILayout folderListLayout;
        private GUITextField searchField;
        private GUITexture dragSelection;

        private ContextMenu entryContextMenu;
        private LibraryDropTarget dropTarget;

        private int autoScrollAmount;
        private bool isDraggingSelection;
        private Vector2I dragSelectionStart;
        private Vector2I dragSelectionEnd;

        private LibraryGUIEntry inProgressRenameElement;

        // Cut/Copy/Paste
        private List<string> copyPaths = new List<string>();
        private List<string> cutPaths = new List<string>();

        /// <summary>
        /// Determines how to display resource tiles in the library window.
        /// </summary>
        internal ProjectViewType ViewType
        {
            get { return viewType; }
            set { viewType = value; Refresh(); }
        }

        /// <summary>
        /// Returns a file or folder currently selected in the library window. If nothing is selected, returns the active
        /// folder. Returned path is relative to project library resources folder.
        /// </summary>
        public string SelectedEntry
        {
            get
            {
                if (selectionPaths.Count == 1)
                {
                    LibraryEntry entry = ProjectLibrary.GetEntry(selectionPaths[0]);
                    if (entry != null)
                        return entry.Path;
                }

                return CurrentFolder;
            }
        }

        /// <summary>
        /// Returns a folder currently selected in the library window. If no folder is selected, returns the active
        /// folder. Returned path is relative to project library resources folder.
        /// </summary>
        public string SelectedFolder
        {
            get
            {
                DirectoryEntry selectedDirectory = null;
                if (selectionPaths.Count == 1)
                {
                    LibraryEntry entry = ProjectLibrary.GetEntry(selectionPaths[0]);
                    if (entry != null && entry.Type == LibraryEntryType.Directory)
                        selectedDirectory = (DirectoryEntry) entry;
                }

                if (selectedDirectory != null)
                    return selectedDirectory.Path;
                
                return CurrentFolder;
            }
        }

        /// <summary>
        /// Returns the path to the folder currently displayed in the library window. Returned path is relative to project 
        /// library resources folder.
        /// </summary>
        public string CurrentFolder
        {
            get { return ProjectSettings.GetString(CURRENT_LIBRARY_DIRECTORY_KEY); }
            set { ProjectSettings.SetString(CURRENT_LIBRARY_DIRECTORY_KEY, value); }
        }

        /// <summary>
        /// Context menu that should open when user right clicks on the content area.
        /// </summary>
        internal ContextMenu ContextMenu
        {
            get { return entryContextMenu; }
        }

        /// <summary>
        /// Opens the library window if not already open.
        /// </summary>
        [MenuItem("Windows/Library", ButtonModifier.CtrlAlt, ButtonCode.L, 6000)]
        private static void OpenLibraryWindow()
        {
            OpenWindow<LibraryWindow>();
        }

        private void OnInitialize()
        {
            ProjectLibrary.OnEntryAdded += OnEntryChanged;
            ProjectLibrary.OnEntryImported += OnEntryChanged;
            ProjectLibrary.OnEntryRemoved += OnEntryChanged;

            GUILayoutY contentLayout = GUI.AddLayoutY();

            searchBarLayout = contentLayout.AddLayoutX();
            searchField = new GUITextField();
            searchField.OnChanged += OnSearchChanged;
            searchField.OnFocusGained += StopRename;

            GUIContent clearIcon = new GUIContent(EditorBuiltin.GetLibraryWindowIcon(LibraryWindowIcon.Clear), 
                new LocEdString("Clear"));
            GUIButton clearSearchBtn = new GUIButton(clearIcon);
            clearSearchBtn.OnClick += OnClearClicked;
            clearSearchBtn.SetWidth(40);

            GUIContent optionsIcon = new GUIContent(EditorBuiltin.GetLibraryWindowIcon(LibraryWindowIcon.Options), 
                new LocEdString("Options"));
            optionsButton = new GUIButton(optionsIcon);
            optionsButton.OnClick += OnOptionsClicked;
            optionsButton.SetWidth(40);
            searchBarLayout.AddElement(searchField);
            searchBarLayout.AddElement(clearSearchBtn);
            searchBarLayout.AddElement(optionsButton);

            folderBarLayout = contentLayout.AddLayoutX();

            GUIContent homeIcon = new GUIContent(EditorBuiltin.GetLibraryWindowIcon(LibraryWindowIcon.Home), 
                new LocEdString("Home"));
            GUIButton homeButton = new GUIButton(homeIcon, GUIOption.FixedWidth(FOLDER_BUTTON_WIDTH));
            homeButton.OnClick += OnHomeClicked;

            GUIContent upIcon = new GUIContent(EditorBuiltin.GetLibraryWindowIcon(LibraryWindowIcon.Up), 
                new LocEdString("Up"));
            GUIButton upButton = new GUIButton(upIcon, GUIOption.FixedWidth(FOLDER_BUTTON_WIDTH));
            upButton.OnClick += OnUpClicked;

            folderBarLayout.AddElement(homeButton);
            folderBarLayout.AddElement(upButton);
            folderBarLayout.AddSpace(10);

            contentScrollArea = new GUIScrollArea(GUIOption.FlexibleWidth(), GUIOption.FlexibleHeight());
            contentLayout.AddElement(contentScrollArea);
            contentLayout.AddFlexibleSpace();

            entryContextMenu = LibraryMenu.CreateContextMenu(this);
            content = new LibraryGUIContent(this, contentScrollArea);

            Refresh();

            dropTarget = new LibraryDropTarget(this);
            dropTarget.Bounds = GetScrollAreaBounds();
            dropTarget.OnStart += OnDragStart;
            dropTarget.OnDrag += OnDragMove;
            dropTarget.OnLeave += OnDragLeave;
            dropTarget.OnDropResource += OnResourceDragDropped;
            dropTarget.OnDropSceneObject += OnSceneObjectDragDropped;
            dropTarget.OnEnd += OnDragEnd;

            Selection.OnSelectionChanged += OnSelectionChanged;
            Selection.OnResourcePing += OnPing;
        }

        private void OnDestroy()
        {
            Selection.OnSelectionChanged -= OnSelectionChanged;
            Selection.OnResourcePing -= OnPing;

            dropTarget.Destroy();
        }

        private void OnEditorUpdate()
        {
            bool isRenameInProgress = inProgressRenameElement != null;

            if (HasContentFocus)
            {
                if (!isRenameInProgress)
                {
                    IGlobalShortcuts shortcuts = this;

                    if (VirtualInput.IsButtonDown(EditorApplication.CopyKey))
                        shortcuts.OnCopyPressed();
                    else if (VirtualInput.IsButtonDown(EditorApplication.CutKey))
                        shortcuts.OnCutPressed();
                    else if (VirtualInput.IsButtonDown(EditorApplication.PasteKey))
                        shortcuts.OnPastePressed();
                    else if (VirtualInput.IsButtonDown(EditorApplication.DuplicateKey))
                        shortcuts.OnDuplicatePressed();
                    else if (VirtualInput.IsButtonDown(EditorApplication.RenameKey))
                        shortcuts.OnRenamePressed();
                    else if (VirtualInput.IsButtonDown(EditorApplication.DeleteKey))
                        shortcuts.OnDeletePressed();

                    if (Input.IsButtonDown(ButtonCode.Return))
                    {
                        if (selectionPaths.Count == 1)
                        {
                            LibraryEntry entry = ProjectLibrary.GetEntry(selectionPaths[0]);
                            if (entry != null && entry.Type == LibraryEntryType.Directory)
                            {
                                EnterDirectory(entry.Path);
                            }
                        }
                    }
                    else if (Input.IsButtonDown(ButtonCode.Back))
                    {
                        LibraryEntry entry = ProjectLibrary.GetEntry(CurrentFolder);
                        if (entry != null && entry.Parent != null)
                        {
                            EnterDirectory(entry.Parent.Path);
                        }
                    }
                    else if (Input.IsButtonDown(ButtonCode.Up))
                    {
                        MoveSelection(MoveDirection.Up);
                    }
                    else if (Input.IsButtonDown(ButtonCode.Down))
                    {
                        MoveSelection(MoveDirection.Down);
                    }
                    else if (Input.IsButtonDown(ButtonCode.Left))
                    {
                        MoveSelection(MoveDirection.Left);
                    }
                    else if (Input.IsButtonDown(ButtonCode.Right))
                    {
                        MoveSelection(MoveDirection.Right);
                    }
                }
                else
                {
                    if (Input.IsButtonDown(ButtonCode.Return))
                    {
                        string newName = inProgressRenameElement.GetRenamedName();

                        string originalPath = inProgressRenameElement.path;
                        originalPath = originalPath.TrimEnd(Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar);

                        string newPath = Path.GetDirectoryName(originalPath);
                        string newNameWithExtension = newName + Path.GetExtension(originalPath);
                        newPath = Path.Combine(newPath, newNameWithExtension);

                        bool renameOK = true;
                        if (!PathEx.IsValidFileName(newName))
                        {
                            DialogBox.Open(new LocEdString("Error"), new LocEdString("The name you specified is not a valid file name. Try another."), DialogBox.Type.OK);
                            renameOK = false;
                        }

                        if (renameOK)
                        {
                            // Windows sees paths with dot at the end as if they didn't have it
                            // so remove the dot to ensure the project library does the same
                            string trimmedNewPath = newPath.TrimEnd('.');

                            if (originalPath != trimmedNewPath && ProjectLibrary.Exists(trimmedNewPath))
                            {
                                DialogBox.Open(new LocEdString("Error"), new LocEdString("File/folder with that name already exists in this folder."), DialogBox.Type.OK);
                                renameOK = false;
                            }
                        }

                        if (renameOK)
                        {
                            ProjectLibrary.Rename(originalPath, newNameWithExtension);
                            StopRename();
                        }
                    }
                    else if (Input.IsButtonDown(ButtonCode.Escape))
                    {
                        StopRename();
                    }
                }
            }
            else
            {
                if (isRenameInProgress && !HasFocus)
                    StopRename();
            }

            if (autoScrollAmount != 0)
            {
                Rect2I contentBounds = contentScrollArea.ContentBounds;
                float scrollPct = autoScrollAmount / (float)contentBounds.height;

                contentScrollArea.VerticalScroll += scrollPct * Time.FrameDelta;
            }

            if (requiresRefresh)
                Refresh();

            dropTarget.Update();
            content.Update();
        }

        /// <inheritdoc/>
        protected override LocString GetDisplayName()
        {
            return new LocEdString("Library");
        }

        /// <inheritdoc/>
        protected override void WindowResized(int width, int height)
        {
            base.WindowResized(width, height);

            Refresh();

            dropTarget.Bounds = GetScrollAreaBounds();
        }

        /// <summary>
        /// Attempts to find a resource tile element at the specified coordinates.
        /// </summary>
        /// <param name="windowPos">Coordinates relative to the window.</param>
        /// <returns>True if found an entry, false otherwise.</returns>
        private LibraryGUIEntry FindElementAt(Vector2I windowPos)
        {
            Vector2I scrollPos = WindowToScrollAreaCoords(windowPos);

            return content.FindElementAt(scrollPos);
        }

        /// <summary>
        /// Clears hover highlight from the currently hovered over element.
        /// </summary>
        private void ClearHoverHighlight()
        {
            content.MarkAsHovered(hoverHighlightPath, false);
            hoverHighlightPath = "";
        }

        /// <summary>
        /// Pings an element at the specified path, displaying and highlighting it in the window.
        /// </summary>
        /// <param name="path">Project library path to the element.</param>
        public void Ping(string path)
        {
            if (!string.IsNullOrEmpty(path))
            {
                string parentDir = PathEx.GetParent(path);

                if(!PathEx.Compare(parentDir, CurrentFolder))
                    EnterDirectory(parentDir);
            }

            content.MarkAsPinged(pingPath, false);
            pingPath = path;
            content.MarkAsPinged(pingPath, true);
        }

        /// <summary>
        /// Resets the library window to initial state.
        /// </summary>
        public void Reset()
        {
            CurrentFolder = ProjectLibrary.Root.Path;
            selectionAnchorStart = -1;
            selectionAnchorEnd = -1;
            selectionPaths.Clear();
            pingPath = "";
            hoverHighlightPath = "";

            Refresh();
        }

        /// <inheritdoc/>
        void IGlobalShortcuts.OnDeletePressed()
        {
            DeleteSelection();
        }

        /// <inheritdoc/>
        void IGlobalShortcuts.OnRenamePressed()
        {
            RenameSelection();
        }

        /// <inheritdoc/>
        void IGlobalShortcuts.OnDuplicatePressed()
        {
            DuplicateSelection();
        }

        /// <inheritdoc/>
        void IGlobalShortcuts.OnCopyPressed()
        {
            CopySelection();
        }

        /// <inheritdoc/>
        void IGlobalShortcuts.OnCutPressed()
        {
            CutSelection();
        }

        /// <inheritdoc/>
        void IGlobalShortcuts.OnPastePressed()
        {
            PasteToSelection();
        }

        /// <summary>
        /// Deselects all selected elements.
        /// </summary>
        /// <param name="onlyInternal">If true, do not update the global <see cref="Selection"/>, instead the operation
        ///                            will be contained to the library window internally.</param>
        internal void DeselectAll(bool onlyInternal = false)
        {
            SetSelection(new List<string>(), onlyInternal);
            selectionAnchorStart = -1;
            selectionAnchorEnd = -1;
        }

        /// <summary>
        /// Select an element at the specified path. If control or shift keys are pressed during this operations multiple 
        /// elements can be selected.
        /// </summary>
        /// <param name="path">Project library path to the element.</param>
        internal void Select(string path)
        {
            LibraryGUIEntry entry;
            if (!content.TryGetEntry(path, out entry))
                return;

            bool ctrlDown = Input.IsButtonHeld(ButtonCode.LeftControl) || Input.IsButtonHeld(ButtonCode.RightControl);
            bool shiftDown = Input.IsButtonHeld(ButtonCode.LeftShift) || Input.IsButtonHeld(ButtonCode.RightShift);

            if (shiftDown)
            {
                if (selectionAnchorStart != -1 && selectionAnchorStart < content.Entries.Count)
                {
                    int start = Math.Min(entry.index, selectionAnchorStart);
                    int end = Math.Max(entry.index, selectionAnchorStart);

                    List<string> newSelection = new List<string>();
                    for(int i = start; i <= end; i++)
                        newSelection.Add(content.Entries[i].path);

                    SetSelection(newSelection);
                    selectionAnchorEnd = entry.index;
                }
                else
                {
                    SetSelection(new List<string>() {path});
                    selectionAnchorStart = entry.index;
                    selectionAnchorEnd = entry.index;
                }
            }
            else if (ctrlDown)
            {
                List<string> newSelection = new List<string>(selectionPaths);

                if (selectionPaths.Contains(path))
                {
                    newSelection.Remove(path);
                    if (newSelection.Count == 0)
                        DeselectAll();
                    else
                    {
                        if (selectionAnchorStart == entry.index)
                        {
                            LibraryGUIEntry newAnchorEntry;
                            if (!content.TryGetEntry(newSelection[0], out newAnchorEntry))
                                selectionAnchorStart = -1;
                            else
                                selectionAnchorStart = newAnchorEntry.index;
                        }

                        if (selectionAnchorEnd == entry.index)
                        {
                            LibraryGUIEntry newAnchorEntry;
                            if (!content.TryGetEntry(newSelection[newSelection.Count - 1], out newAnchorEntry))
                                selectionAnchorEnd = -1;
                            else
                                selectionAnchorEnd = newAnchorEntry.index;
                        }

                        SetSelection(newSelection);
                    }
                }
                else
                {
                    newSelection.Add(path);
                    SetSelection(newSelection);
                    selectionAnchorEnd = entry.index;
                }
            }
            else
            {
                SetSelection(new List<string>() {path});

                selectionAnchorStart = entry.index;
                selectionAnchorEnd = entry.index;
            }
        }

        /// <summary>
        /// Selects a new element in the specified direction from the currently selected element. If shift or control are
        /// held during this operation, the selected object will be added to existing selection. If no element is selected
        /// the first or last element will be selected depending on direction.
        /// </summary>
        /// <param name="dir">Direction to move from the currently selected element.</param>
        internal void MoveSelection(MoveDirection dir)
        {
            string newPath = "";

            if (selectionPaths.Count == 0 || selectionAnchorEnd == -1)
            {
                // Nothing is selected so we arbitrarily select first or last element
                if (content.Entries.Count > 0)
                {
                    switch (dir)
                    {
                        case MoveDirection.Left:
                        case MoveDirection.Up:
                            newPath = content.Entries[content.Entries.Count - 1].path;
                            break;
                        case MoveDirection.Right:
                        case MoveDirection.Down:
                            newPath = content.Entries[0].path;
                            break;
                    }
                }
            }
            else
            {
                switch (dir)
                {
                    case MoveDirection.Left:
                        if (selectionAnchorEnd - 1 >= 0)
                            newPath = content.Entries[selectionAnchorEnd - 1].path;
                        break;
                    case MoveDirection.Up:
                        if (selectionAnchorEnd - content.ElementsPerRow >= 0)
                            newPath = content.Entries[selectionAnchorEnd - content.ElementsPerRow].path;
                        break;
                    case MoveDirection.Right:
                        if (selectionAnchorEnd + 1 < content.Entries.Count)
                            newPath = content.Entries[selectionAnchorEnd + 1].path;
                        break;
                    case MoveDirection.Down:
                        if (selectionAnchorEnd + content.ElementsPerRow < content.Entries.Count)
                            newPath = content.Entries[selectionAnchorEnd + content.ElementsPerRow].path;
                        break;
                }
            }

            if (!string.IsNullOrEmpty(newPath))
            {
                Select(newPath);
                ScrollToEntry(newPath);
            }
        }

        /// <summary>
        /// Selects a set of elements based on the provided paths.
        /// </summary>
        /// <param name="paths">Project library paths of the elements to select.</param>
        /// <param name="onlyInternal">If true, do not update the global <see cref="Selection"/>, instead the operation
        ///                            will be contained to the library window internally.</param>
        internal void SetSelection(List<string> paths, bool onlyInternal = false)
        {
            if (selectionPaths != null)
            {
                foreach (var path in selectionPaths)
                    content.MarkAsSelected(path, false);
            }

            selectionPaths = paths;
            Ping("");

            if (selectionPaths != null)
            {
                foreach (var path in selectionPaths)
                    content.MarkAsSelected(path, true);
            }

            StopRename();

            if (!onlyInternal)
            {
                if (selectionPaths != null)
                    Selection.ResourcePaths = selectionPaths.ToArray();
                else
                    Selection.ResourcePaths = new string[0];
            }
        }

        /// <summary>
        /// Changes the active directory to the provided directory. Current contents of the window will be cleared and
        /// instead contents of the new directory will be displayed.
        /// </summary>
        /// <param name="directory">Project library path to the directory.</param>
        internal void EnterDirectory(string directory)
        {
            CurrentFolder = directory;
            DeselectAll(true);

            Refresh();
        }

        /// <summary>
        /// Marks the provided set of elements for a cut operation. Cut elements can be moved to a new location by calling
        /// <see cref="Paste"/>.
        /// </summary>
        /// <param name="sourcePaths">Project library paths of the elements to cut.</param>
        internal void Cut(IEnumerable<string> sourcePaths)
        {
            foreach (var path in cutPaths)
                content.MarkAsCut(path, false);

            string[] filePaths = GetFiles(sourcePaths);

            cutPaths.Clear();
            cutPaths.AddRange(filePaths);

            foreach (var path in cutPaths)
                content.MarkAsCut(path, true);

            copyPaths.Clear();
        }

        /// <summary>
        /// Marks the provided set of elements for a copy operation. You can copy the elements by calling <see cref="Paste"/>.
        /// </summary>
        /// <param name="sourcePaths">Project library paths of the elements to copy.</param>
        internal void Copy(IEnumerable<string> sourcePaths)
        {
            copyPaths.Clear();

            string[] filePaths = GetFiles(sourcePaths);
            copyPaths.AddRange(filePaths);

            foreach (var path in cutPaths)
                content.MarkAsCut(path, false);

            cutPaths.Clear();
        }
       
        /// <summary>
        /// Duplicates the provided set of elements.
        /// </summary>
        /// <param name="sourcePaths">Project library paths of the elements to duplicate.</param>
        internal void Duplicate(IEnumerable<string> sourcePaths)
        {
            string[] filePaths = GetFiles(sourcePaths);

            foreach (var source in filePaths)
            {
                ProjectLibrary.Copy(source, LibraryUtility.GetUniquePath(source));
                ProjectLibrary.Refresh();
            }
        }

        /// <summary>
        /// Performs a cut or copy operations on the elements previously marked by calling <see cref="Cut"/> or
        /// <see cref="Copy"/>.
        /// </summary>
        /// <param name="destinationFolder">Project library folder into which to move/copy the elements.</param>
        internal void Paste(string destinationFolder)
        {
            string rootedDestinationFolder = destinationFolder;
            if (!Path.IsPathRooted(rootedDestinationFolder))
                rootedDestinationFolder = Path.Combine(ProjectLibrary.ResourceFolder, rootedDestinationFolder);

            if (copyPaths.Count > 0)
            {
                for (int i = 0; i < copyPaths.Count; i++)
                {
                    string destination = Path.Combine(rootedDestinationFolder, PathEx.GetTail(copyPaths[i]));
                    ProjectLibrary.Copy(copyPaths[i], LibraryUtility.GetUniquePath(destination));
                }

                ProjectLibrary.Refresh();
            }
            else if (cutPaths.Count > 0)
            {
                for (int i = 0; i < cutPaths.Count; i++)
                {
                    string destination = Path.Combine(rootedDestinationFolder, PathEx.GetTail(cutPaths[i]));
                    ProjectLibrary.Move(cutPaths[i], LibraryUtility.GetUniquePath(destination));
                }

                cutPaths.Clear();
                ProjectLibrary.Refresh();
            }
        }

        /// <summary>
        /// Scrolls the contents GUI area so that the element at the specified path becomes visible.
        /// </summary>
        /// <param name="path">Project library path to the element.</param>
        private void ScrollToEntry(string path)
        {
            LibraryGUIEntry entryGUI;
            if (!content.TryGetEntry(path, out entryGUI))
                return;

            Rect2I entryBounds = entryGUI.Bounds;

            Rect2I contentBounds = contentScrollArea.Layout.Bounds;
            Rect2I windowEntryBounds = entryBounds;
            windowEntryBounds.x += contentBounds.x;
            windowEntryBounds.y += contentBounds.y;

            Rect2I scrollAreaBounds = contentScrollArea.Bounds;
            bool requiresScroll = windowEntryBounds.y < scrollAreaBounds.y ||
                                  (windowEntryBounds.y + windowEntryBounds.height) > (scrollAreaBounds.y + scrollAreaBounds.height);

            if (!requiresScroll)
                return;

            int scrollableSize = contentBounds.height - scrollAreaBounds.height;
            float percent = (((entryBounds.y + entryBounds.height * 0.5f) - scrollAreaBounds.height * 0.5f) / (float)scrollableSize);

            percent = MathEx.Clamp01(percent);
            contentScrollArea.VerticalScroll = percent;
        }

        /// <summary>
        /// Rebuilds the library window GUI. Should be called any time the active folder or contents change.
        /// </summary>
        private void Refresh()
        {
            requiresRefresh = false;

            LibraryEntry[] entriesToDisplay = new LibraryEntry[0];
            if (IsSearchActive)
            {
                entriesToDisplay = ProjectLibrary.Search("*" + searchQuery + "*");
            }
            else
            {
                DirectoryEntry entry = ProjectLibrary.GetEntry(CurrentFolder) as DirectoryEntry;
                if (entry == null)
                {
                    CurrentFolder = ProjectLibrary.Root.Path;
                    entry = ProjectLibrary.GetEntry(CurrentFolder) as DirectoryEntry;
                }

                if(entry != null)
                    entriesToDisplay = entry.Children;
            }

            inProgressRenameElement = null;

            RefreshDirectoryBar();

            SortEntries(entriesToDisplay);

            Rect2I visibleContentBounds = GetScrollAreaBounds();
            content.Refresh(viewType, entriesToDisplay, visibleContentBounds);

            foreach (var path in cutPaths)
                content.MarkAsCut(path, true);

            foreach (var path in selectionPaths)
                content.MarkAsSelected(path, true);

            content.MarkAsPinged(pingPath, true);

            Rect2I contentBounds = content.Bounds;
            
            contentBounds.height = Math.Max(contentBounds.height, visibleContentBounds.height);

            GUIButton catchAll = new GUIButton("", EditorStyles.Blank);
            catchAll.Bounds = contentBounds;
            catchAll.OnClick += OnCatchAllClicked;
            catchAll.SetContextMenu(entryContextMenu);

            content.Underlay.AddElement(catchAll);

            Rect2I focusBounds = contentBounds; // Contents + Folder bar
            Rect2I scrollBounds = contentScrollArea.Bounds;
            focusBounds.x += scrollBounds.x;
            focusBounds.y += scrollBounds.y;

            Rect2I folderBarBounds = folderListLayout.Bounds;
            focusBounds.y -= folderBarBounds.height;
            focusBounds.height += folderBarBounds.height;

            GUIButton focusCatcher = new GUIButton("", EditorStyles.Blank);
            focusCatcher.Blocking = false;
            focusCatcher.OnFocusGained += () => hasContentFocus = true;
            focusCatcher.OnFocusLost += () => hasContentFocus = false;
            focusCatcher.Bounds = focusBounds;

            GUIPanel focusPanel = GUI.AddPanel(-3);
            focusPanel.AddElement(focusCatcher);

            UpdateDragSelection(dragSelectionEnd);
        }

        /// <summary>
        /// Converts coordinates relative to the window into coordinates relative to the contents scroll area.
        /// </summary>
        /// <param name="windowPos">Coordinates relative to the window.</param>
        /// <returns>Coordinates relative to the contents scroll area.</returns>
        private Vector2I WindowToScrollAreaCoords(Vector2I windowPos)
        {
            Rect2I scrollBounds = contentScrollArea.Layout.Bounds;
            Vector2I scrollPos = windowPos;
            scrollPos.x -= scrollBounds.x;
            scrollPos.y -= scrollBounds.y;

            return scrollPos;
        }

        /// <summary>
        /// Starts a drag operation that displays a selection outline allowing the user to select multiple entries at once.
        /// </summary>
        /// <param name="windowPos">Coordinates relative to the window where the drag originated.</param>
        private void StartDragSelection(Vector2I windowPos)
        {
            isDraggingSelection = true;
            dragSelectionStart = WindowToScrollAreaCoords(windowPos);
            dragSelectionEnd = dragSelectionStart;
        }

        /// <summary>
        /// Updates a selection outline drag operation by expanding the outline to the new location. Elements in the outline
        /// are selected.
        /// </summary>
        /// <param name="windowPos">Coordinates of the pointer relative to the window.</param>
        /// <returns>True if the selection outline drag is valid and was updated, false otherwise.</returns>
        private bool UpdateDragSelection(Vector2I windowPos)
        {
            if (!isDraggingSelection)
                return false;

            if (dragSelection == null)
            {
                dragSelection = new GUITexture(null, true, EditorStylesInternal.SelectionArea);
                content.Overlay.AddElement(dragSelection);
            }

            dragSelectionEnd = WindowToScrollAreaCoords(windowPos);

            Rect2I selectionArea = CalculateSelectionArea();
            SelectInArea(selectionArea);
            dragSelection.Bounds = selectionArea;

            return true;
        }

        /// <summary>
        /// Ends the selection outline drag operation. Elements in the outline are selected.
        /// </summary>
        /// <returns>True if the selection outline drag is valid and was ended, false otherwise.</returns>
        private bool EndDragSelection()
        {
            if (!isDraggingSelection)
                return false;

            if (dragSelection != null)
            {
                dragSelection.Destroy();
                dragSelection = null;
            }

            Rect2I selectionArea = CalculateSelectionArea();
            SelectInArea(selectionArea);

            isDraggingSelection = false;
            return false;
        }

        /// <summary>
        /// Calculates bounds of the selection area used for selection overlay drag operation, depending on drag starting
        /// point coordinates and current drag coordinates.
        /// </summary>
        /// <returns>Bounds of the selection area, relative to the content scroll area.</returns>
        private Rect2I CalculateSelectionArea()
        {
            Rect2I selectionArea = new Rect2I();
            if (dragSelectionStart.x < dragSelectionEnd.x)
            {
                selectionArea.x = dragSelectionStart.x;
                selectionArea.width = dragSelectionEnd.x - dragSelectionStart.x;
            }
            else
            {
                selectionArea.x = dragSelectionEnd.x;
                selectionArea.width = dragSelectionStart.x - dragSelectionEnd.x;
            }

            if (dragSelectionStart.y < dragSelectionEnd.y)
            {
                selectionArea.y = dragSelectionStart.y;
                selectionArea.height = dragSelectionEnd.y - dragSelectionStart.y;
            }
            else
            {
                selectionArea.y = dragSelectionEnd.y;
                selectionArea.height = dragSelectionStart.y - dragSelectionEnd.y;
            }

            Rect2I maxBounds = contentScrollArea.Layout.Bounds;
            maxBounds.x = 0;
            maxBounds.y = 0;

            selectionArea.Clip(maxBounds);

            return selectionArea;
        }

        /// <summary>
        /// Selects all elements overlapping the specified bounds.
        /// </summary>
        /// <param name="scrollBounds">Bounds relative to the content scroll area.</param>
        private void SelectInArea(Rect2I scrollBounds)
        {
            LibraryGUIEntry[] foundElements = content.FindElementsOverlapping(scrollBounds);

            if (foundElements.Length > 0)
            {
                selectionAnchorStart = foundElements[0].index;
                selectionAnchorEnd = foundElements[foundElements.Length - 1].index;
            }
            else
            {
                selectionAnchorStart = -1;
                selectionAnchorEnd = -1;
            }

            List<string> elementPaths = new List<string>();
            foreach (var elem in foundElements)
                elementPaths.Add(elem.path);

            SetSelection(elementPaths);
        }

        /// <summary>
        /// Updates GUI for the directory bar. Should be called whenever the active folder changes.
        /// </summary>
        private void RefreshDirectoryBar()
        {
            if (folderListLayout != null)
            {
                folderListLayout.Destroy();
                folderListLayout = null;
            }

            folderListLayout = folderBarLayout.AddLayoutX();

            string[] folders = null;
            string[] fullPaths = null;

            if (IsSearchActive)
            {
                folders = new[] {searchQuery};
                fullPaths = new[] { searchQuery };
            }
            else
            {
                string currentDir = Path.Combine("Resources", CurrentFolder);

                folders = currentDir.Split(new[] { Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar },
                    StringSplitOptions.RemoveEmptyEntries);

                fullPaths = new string[folders.Length];
                for (int i = 0; i < folders.Length; i++)
                {
                    if (i == 0)
                        fullPaths[i] = "";
                    else
                        fullPaths[i] = Path.Combine(fullPaths[i - 1], folders[i]);
                }
            }

            int availableWidth = folderBarLayout.Bounds.width - FOLDER_BUTTON_WIDTH * 2;
            int numFolders = 0;
            for (int i = folders.Length - 1; i >= 0; i--)
            {
                GUIButton folderButton = new GUIButton(folders[i]);

                if (!IsSearchActive)
                {
                    string fullPath = fullPaths[i];
                    folderButton.OnClick += () => OnFolderButtonClicked(fullPath);
                }

                GUIButton separator = new GUIButton("/", GUIOption.FixedWidth(FOLDER_SEPARATOR_WIDTH));

                folderListLayout.InsertElement(0, separator);
                folderListLayout.InsertElement(0, folderButton);
                numFolders++;

                Rect2I folderListBounds = folderListLayout.Bounds;
                if (folderListBounds.width > availableWidth)
                {
                    if (numFolders > 2)
                    {
                        separator.Destroy();
                        folderButton.Destroy();
                        break;
                    }
                }
            }
        }

        /// <summary>
        /// Performs <see cref="Cut"/> operation on the currently selected elements.
        /// </summary>
        internal void CutSelection()
        {
            if (selectionPaths.Count > 0)
                Cut(selectionPaths);
        }

        /// <summary>
        /// Performs <see cref="Copy"/> operation on the currently selected elements.
        /// </summary>
        internal void CopySelection()
        {
            if (selectionPaths.Count > 0)
                Copy(selectionPaths);
        }

        /// <summary>
        /// Performs <see cref="Duplicate"/> operation on the currently selected elements.
        /// </summary>
        internal void DuplicateSelection()
        {
            if (selectionPaths.Count > 0)
                Duplicate(selectionPaths);
        }

        /// <summary>
        /// Performs <see cref="Paste"/> operation. Elements will be pasted in the currently selected directory (if any), or
        /// the active directory otherwise.
        /// </summary>
        internal void PasteToSelection()
        {
            Paste(SelectedFolder);
        }

        /// <summary>
        /// Starts a rename operation on the currently selected elements. If more than one elements are selected only the
        /// first one will be affected.
        /// </summary>
        internal void RenameSelection()
        {
            string[] filePaths = GetFiles(selectionPaths);

            if (filePaths.Length == 0)
                return;

            if (filePaths.Length > 1)
            {
                DeselectAll();
                Select(filePaths[0]);
            }

            LibraryGUIEntry entry;
            if (content.TryGetEntry(filePaths[0], out entry))
            {
                entry.StartRename();
                inProgressRenameElement = entry;
            }
        }

        /// <summary>
        /// Deletes currently selected elements. User will be asked to confirm deletion via a dialog box.
        /// </summary>
        internal void DeleteSelection()
        {
            string[] filePaths = GetFiles(selectionPaths);

            if (filePaths.Length == 0)
                return;

            DialogBox.Open(new LocEdString("Confirm deletion"), new LocEdString("Are you sure you want to delete the selected object(s)?"),
                DialogBox.Type.YesNo,
                type =>
                {
                    if (type == DialogBox.ResultType.Yes)
                    {
                        foreach (var path in filePaths)
                            ProjectLibrary.Delete(path);

                        DeselectAll();
                        Refresh();
                    }
                });
        }

        /// <summary>
        /// Stops the rename operation, if one is in progress on any element.
        /// </summary>
        internal void StopRename()
        {
            if (inProgressRenameElement != null)
            {
                inProgressRenameElement.StopRename();
                inProgressRenameElement = null;
            }
        }

        /// <summary>
        /// Clears the search bar and refreshes the content area to display contents of the current directory.
        /// </summary>
        private void OnClearClicked()
        {
            StopRename();

            searchField.Value = "";
            searchQuery = "";
            Refresh();
        }

        /// <summary>
        /// Takes a list of resource paths and returns only those referencing files or folder and not sub-resources.
        /// </summary>
        /// <param name="resourcePaths">List of resource paths to find files for.</param>
        /// <returns>File paths for all the provided resources.</returns>
        private string[] GetFiles(IEnumerable<string> resourcePaths)
        {
            HashSet<string> filePaths = new HashSet<string>();

            foreach (var resPath in resourcePaths)
            {
                if (resPath == null)
                    continue;

                LibraryEntry entry = ProjectLibrary.GetEntry(resPath);
                if (entry == null)
                    continue;

                if (ProjectLibrary.IsSubresource(resPath))
                    continue;

                if (!filePaths.Contains(entry.Path))
                    filePaths.Add(entry.Path);
            }

            string[] output = new string[filePaths.Count];

            int i = 0;
            foreach(var path in filePaths)
                output[i++] = path;

            return output;
        }

        /// <summary>
        /// Opens the drop down options window that allows you to customize library window look and feel.
        /// </summary>
        private void OnOptionsClicked()
        {
            StopRename();

            Vector2I openPosition;
            Rect2I buttonBounds = GUIUtility.CalculateBounds(optionsButton, GUI);

            openPosition.x = buttonBounds.x + buttonBounds.width / 2;
            openPosition.y = buttonBounds.y + buttonBounds.height / 2;

            LibraryDropDown dropDown = DropDownWindow.Open<LibraryDropDown>(this, openPosition);
            dropDown.Initialize(this);
        }

        /// <summary>
        /// Returns the content scroll area bounds.
        /// </summary>
        /// <returns>Bounds of the content scroll area, relative to the window.</returns>
        private Rect2I GetScrollAreaBounds()
        {
            Rect2I bounds = GUI.Bounds;
            Rect2I folderListBounds = folderListLayout.Bounds;
            Rect2I searchBarBounds = searchBarLayout.Bounds;

            bounds.y = folderListBounds.height + searchBarBounds.height;
            bounds.height -= folderListBounds.height + searchBarBounds.height;

            return bounds;
        }

        /// <summary>
        /// Triggered when a project library entry was changed (added, modified, deleted).
        /// </summary>
        /// <param name="entry">Project library path of the changed entry.</param>
        private void OnEntryChanged(string entry)
        {
            requiresRefresh = true;
        }

        /// <summary>
        /// Triggered when the drag and drop operation is starting while over the content area. If drag operation is over
        /// an element, element will be dragged.
        /// </summary>
        /// <param name="windowPos">Coordinates where the drag operation started, relative to the window.</param>
        private void OnDragStart(Vector2I windowPos)
        {
            bool isRenameInProgress = inProgressRenameElement != null;
            if (isRenameInProgress)
                return;

            LibraryGUIEntry underCursorElem = FindElementAt(windowPos);
            if (underCursorElem == null)
            {
                StartDragSelection(windowPos);
                return;
            }

            string resourceDir = ProjectLibrary.ResourceFolder;

            string[] dragPaths = null;
            if (selectionPaths.Count > 0)
            {
                foreach (var path in selectionPaths)
                {
                    if (path == underCursorElem.path)
                    {
                        dragPaths = new string[selectionPaths.Count];
                        for (int i = 0; i < selectionPaths.Count; i++)
                        {
                            dragPaths[i] = Path.Combine(resourceDir, selectionPaths[i]);
                        }

                        break;
                    }
                }
            }

            if (dragPaths == null)
                dragPaths = new[] { Path.Combine(resourceDir, underCursorElem.path) };

            ResourceDragDropData dragDropData = new ResourceDragDropData(dragPaths);
            DragDrop.StartDrag(dragDropData);
        }

        /// <summary>
        /// Triggered when a pointer is moved while a drag operation is in progress.
        /// </summary>
        ///  <param name="windowPos">Coordinates of the pointer relative to the window.</param>
        private void OnDragMove(Vector2I windowPos)
        {
            // Auto-scroll
            Rect2I scrollAreaBounds = contentScrollArea.Bounds;
            int scrollAreaTop = scrollAreaBounds.y;
            int scrollAreaBottom = scrollAreaBounds.y + scrollAreaBounds.height;

            if (windowPos.y > scrollAreaTop && windowPos.y <= (scrollAreaTop + DRAG_SCROLL_HEIGHT))
                autoScrollAmount = -DRAG_SCROLL_AMOUNT_PER_SECOND;
            else if (windowPos.y >= (scrollAreaBottom - DRAG_SCROLL_HEIGHT) && windowPos.y < scrollAreaBottom)
                autoScrollAmount = DRAG_SCROLL_AMOUNT_PER_SECOND;
            else
                autoScrollAmount = 0;

            // Selection box
            if (UpdateDragSelection(windowPos))
                return;

            // Drag and drop (hover element under cursor)
            LibraryGUIEntry underCursorElem = FindElementAt(windowPos);

            if (underCursorElem == null)
            {
                ClearHoverHighlight();
            }
            else
            {
                if (underCursorElem.path != hoverHighlightPath)
                {
                    ClearHoverHighlight();

                    hoverHighlightPath = underCursorElem.path;
                    underCursorElem.MarkAsHovered(true);
                }
            }
        }

        /// <summary>
        /// Triggered when a pointer leaves the drop targer while a drag operation is in progress.
        /// </summary>
        private void OnDragLeave()
        {
            ClearHoverHighlight();
            autoScrollAmount = 0;
        }

        /// <summary>
        /// Triggered when a resource drop operation finishes over the content area.
        /// </summary>
        /// <param name="windowPos">Coordinates of the pointer relative to the window where the drop operation finished
        ///                         .</param>
        /// <param name="paths">Paths of the dropped resources.</param>
        private void OnResourceDragDropped(Vector2I windowPos, string[] paths)
        {
            ClearHoverHighlight();
            autoScrollAmount = 0;

            if (EndDragSelection())
                return;

            string resourceDir = ProjectLibrary.ResourceFolder;
            string destinationFolder = Path.Combine(resourceDir, CurrentFolder);

            LibraryGUIEntry underCursorElement = FindElementAt(windowPos);
            if (underCursorElement != null)
            {
                LibraryEntry entry = ProjectLibrary.GetEntry(underCursorElement.path);
                if (entry != null && entry.Type == LibraryEntryType.Directory)
                    destinationFolder = Path.Combine(resourceDir, entry.Path);
            }

            if (paths != null)
            {
                List<string> addedResources = new List<string>();
                foreach (var path in paths)
                {
                    string absolutePath = path;
                    if (!Path.IsPathRooted(absolutePath))
                        absolutePath = Path.Combine(resourceDir, path);

                    if (string.IsNullOrEmpty(absolutePath))
                        continue;

                    if (PathEx.IsPartOf(destinationFolder, absolutePath) || PathEx.Compare(absolutePath, destinationFolder))
                        continue;

                    string pathTail = PathEx.GetTail(absolutePath);
                    string destination = Path.Combine(destinationFolder, pathTail);

                    if (PathEx.Compare(absolutePath, destination))
                        continue;

                    bool newFile = !ProjectLibrary.Exists(absolutePath);
                    if (!newFile)
                    {
                        if (ProjectLibrary.IsSubresource(absolutePath))
                            continue;
                    }

                    string uniqueDestination = LibraryUtility.GetUniquePath(destination);
                    if (Directory.Exists(path))
                    {
                        if (newFile)
                            DirectoryEx.Copy(absolutePath, uniqueDestination);
                        else
                            ProjectLibrary.Move(absolutePath, uniqueDestination);
                    }
                    else if (File.Exists(path))
                    {
                        if (newFile)
                            FileEx.Copy(absolutePath, uniqueDestination);
                        else
                            ProjectLibrary.Move(absolutePath, uniqueDestination);
                    }

                    string relativeDestination = uniqueDestination.Substring(resourceDir.Length, uniqueDestination.Length - resourceDir.Length);
                    addedResources.Add(relativeDestination);
                    ProjectLibrary.Refresh();
                }

                SetSelection(addedResources);
            }
        }

        /// <summary>
        /// Triggered when a scene object drop operation finishes over the content area.
        /// </summary>
        /// <param name="windowPos">Coordinates of the pointer relative to the window where the drop operation finished
        ///                         .</param>
        /// <param name="objects">Dropped scene objects.</param>
        private void OnSceneObjectDragDropped(Vector2I windowPos, SceneObject[] objects)
        {
            ClearHoverHighlight();
            autoScrollAmount = 0;

            if (EndDragSelection())
                return;

            string destinationFolder = CurrentFolder;

            LibraryGUIEntry underCursorElement = FindElementAt(windowPos);
            if (underCursorElement != null)
            {
                LibraryEntry entry = ProjectLibrary.GetEntry(underCursorElement.path);
                if (entry != null && entry.Type == LibraryEntryType.Directory)
                    destinationFolder = entry.Path;
            }

            if (objects != null)
            {
                List<string> addedResources = new List<string>();
                foreach (var so in objects)
                {
                    if (so == null)
                        continue;

                    Prefab newPrefab = new Prefab(so, false);

                    string destination = LibraryUtility.GetUniquePath(Path.Combine(destinationFolder, so.Name + ".prefab"));
                    addedResources.Add(destination);

                    ProjectLibrary.Create(newPrefab, destination);
                    ProjectLibrary.Refresh();
                }

                SetSelection(addedResources);
            }
        }

        /// <summary>
        /// Triggered when a drag operation that originated from this window ends.
        /// </summary>
        /// <param name="windowPos">Coordinates of the pointer where the drag ended relative to the window </param>
        private void OnDragEnd(Vector2I windowPos)
        {
            EndDragSelection();
            autoScrollAmount = 0;
        }

        /// <summary>
        /// Triggered when the global selection changes.
        /// </summary>
        /// <param name="sceneObjects">A set of newly selected scene objects.</param>
        /// <param name="resourcePaths">A set of paths for newly selected resources.</param>
        private void OnSelectionChanged(SceneObject[] sceneObjects, string[] resourcePaths)
        {
            if (sceneObjects.Length > 0)
                DeselectAll(true);
            else
                SetSelection(new List<string>(resourcePaths), true);
        }

        /// <summary>
        /// Triggered when a ping operation was triggered externally.
        /// </summary>
        /// <param name="path">Path to the resource to highlight.</param>
        private void OnPing(string path)
        {
            Ping(path);
        }

        /// <summary>
        /// Triggered when a folder on the directory bar was selected.
        /// </summary>
        /// <param name="path">Project library path to the folder to enter.</param>
        private void OnFolderButtonClicked(string path)
        {
            StopRename();
            EnterDirectory(path);
        }

        /// <summary>
        /// Triggered when the user clicks on empty space between elements.
        /// </summary>
        private void OnCatchAllClicked()
        {
            DeselectAll();
        }

        /// <summary>
        /// Triggered when the user clicks on the home button on the directory bar, changing the active directory to
        /// project library root.
        /// </summary>
        private void OnHomeClicked()
        {
            StopRename();

            CurrentFolder = ProjectLibrary.Root.Path;
            Refresh();
        }

        /// <summary>
        /// Triggered when the user clicks on the up button on the directory bar, changing the active directory to the
        /// parent directory, unless already at project library root.
        /// </summary>
        private void OnUpClicked()
        {
            StopRename();

            string currentDir = CurrentFolder;
            currentDir = currentDir.Trim(Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar);

            if (!string.IsNullOrEmpty(currentDir))
            {
                string parent = Path.GetDirectoryName(currentDir);

                CurrentFolder = parent;
                Refresh();
            }
        }

        /// <summary>
        /// Triggered when the user inputs new values into the search input box. Refreshes the contents so they display
        /// elements matching the search text.
        /// </summary>
        /// <param name="newValue">Search box text.</param>
        private void OnSearchChanged(string newValue)
        {
            searchQuery = newValue;
            Refresh();
        }

        /// <summary>
        /// Sorts the specified set of project library entries by type (folder or resource), followed by name.
        /// </summary>
        /// <param name="input">Set of project library entries to sort.</param>
        private static void SortEntries(LibraryEntry[] input)
        {
            Array.Sort(input, (x, y) =>
            {
                if (x.Type == y.Type)
                    return x.Name.CompareTo(y.Name);
                else
                    return x.Type == LibraryEntryType.File ? 1 : -1;
            });
        }
    }

    /** @} */
}
