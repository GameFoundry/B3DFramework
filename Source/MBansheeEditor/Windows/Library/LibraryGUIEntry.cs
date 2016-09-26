//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System.IO;
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup Library
     *  @{
     */

    /// <summary>
    /// Represents GUI for a single resource tile used in <see cref="LibraryGUIContent"/>.
    /// </summary>
    internal class LibraryGUIEntry
    {
        private static readonly Color PING_COLOR = Color.BansheeOrange;
        private static readonly Color SELECTION_COLOR = Color.DarkCyan;
        private static readonly Color HOVER_COLOR = new Color(Color.DarkCyan.r, Color.DarkCyan.g, Color.DarkCyan.b, 0.5f);
        private static readonly Color CUT_COLOR = new Color(1.0f, 1.0f, 1.0f, 0.5f);
        private const int VERT_PADDING = 3;
        private const int BG_HORZ_PADDING = 2;
        private const int BG_VERT_PADDING = 2;

        private const string LibraryEntryFirstBg = "LibraryEntryFirstBg";
        private const string LibraryEntryBg = "LibraryEntryBg";
        private const string LibraryEntryLastBg = "LibraryEntryLastBg";
        private const string LibraryEntryVertFirstBg = "LibraryEntryVertFirstBg";
        private const string LibraryEntryVertBg = "LibraryEntryVertBg";
        private const string LibraryEntryVertLastBg = "LibraryEntryVertLastBg";

        /// <summary>
        /// Possible visual states for the resource tile.
        /// </summary>
        enum UnderlayState // Note: Order of these is relevant
        {
            None, Hovered, Selected, Pinged
        }

        public int index;
        public string path;
        public GUITexture icon;
        public GUILabel label;
        public Rect2I bounds;

        private GUITexture underlay;
        private GUITexture groupUnderlay;
        private LibraryGUIContent owner;
        private UnderlayState underlayState;
        private GUITextBox renameTextBox;

        private int width, height;
        private LibraryGUIEntryType type;
        private bool delayedSelect;
        private float delayedSelectTime;
        private ulong delayedOpenCodeEditorFrame = ulong.MaxValue;

        /// <summary>
        /// Bounds of the entry relative to part content area.
        /// </summary>
        public Rect2I Bounds
        {
            get { return bounds; }
        }

        /// <summary>
        /// Constructs a new resource tile entry.
        /// </summary>
        /// <param name="owner">Content area this entry is part of.</param>
        /// <param name="parent">Parent layout to add this entry's GUI elements to.</param>
        /// <param name="path">Path to the project library entry to display data for.</param>
        /// <param name="index">Sequential index of the entry in the conent area.</param>
        /// <param name="width">Width of the GUI entry.</param>
        /// <param name="height">Maximum allowed height for the label.</param>"
        /// <param name="type">Type of the entry, which controls its style and/or behaviour.</param>
        public LibraryGUIEntry(LibraryGUIContent owner, GUILayout parent, string path, int index, int width, int height,
            LibraryGUIEntryType type)
        {
            GUILayout entryLayout;

            if (owner.GridLayout)
                entryLayout = parent.AddLayoutY();
            else
                entryLayout = parent.AddLayoutX();

            SpriteTexture iconTexture = GetIcon(path, owner.TileSize);

            icon = new GUITexture(iconTexture, GUITextureScaleMode.ScaleToFit,
                true, GUIOption.FixedHeight(owner.TileSize), GUIOption.FixedWidth(owner.TileSize));

            label = null;

            string name = PathEx.GetTail(path);
            if (owner.GridLayout)
            {
                int labelHeight = height - owner.TileSize;

                label = new GUILabel(name, EditorStyles.MultiLineLabelCentered,
                    GUIOption.FixedWidth(width), GUIOption.FixedHeight(labelHeight));

                switch (type)
                {
                    case LibraryGUIEntryType.Single:
                        break;
                    case LibraryGUIEntryType.MultiFirst:
                        groupUnderlay = new GUITexture(null, LibraryEntryFirstBg);
                        break;
                    case LibraryGUIEntryType.MultiElement:
                        groupUnderlay = new GUITexture(null, LibraryEntryBg);
                        break;
                    case LibraryGUIEntryType.MultiLast:
                        groupUnderlay = new GUITexture(null, LibraryEntryLastBg);
                        break;
                }
            }
            else
            {
                label = new GUILabel(name, GUIOption.FixedWidth(width - owner.TileSize), GUIOption.FixedHeight(height));

                switch (type)
                {
                    case LibraryGUIEntryType.Single:
                        break;
                    case LibraryGUIEntryType.MultiFirst:
                        groupUnderlay = new GUITexture(null, LibraryEntryVertFirstBg);
                        break;
                    case LibraryGUIEntryType.MultiElement:
                        groupUnderlay = new GUITexture(null, LibraryEntryVertBg);
                        break;
                    case LibraryGUIEntryType.MultiLast:
                        groupUnderlay = new GUITexture(null, LibraryEntryVertLastBg);
                        break;
                }
            }

            entryLayout.AddElement(icon);
            entryLayout.AddElement(label);

            if (groupUnderlay != null)
                owner.DeepUnderlay.AddElement(groupUnderlay);

            this.owner = owner;
            this.index = index;
            this.path = path;
            this.bounds = new Rect2I();
            this.underlay = null;
            this.type = type;
            this.width = width;
            this.height = height;
        }

        /// <summary>
        /// Positions the GUI elements. Must be called after construction, but only after all content area entries have
        /// been constructed so that entry's final bounds are known.
        /// </summary>
        public void Initialize()
        {
            Rect2I iconBounds = icon.Bounds;
            bounds = iconBounds;
            Rect2I labelBounds = label.Bounds;

            if (owner.GridLayout)
            {
                bounds.x = labelBounds.x;
                bounds.y -= VERT_PADDING;
                bounds.width = labelBounds.width;
                bounds.height = (labelBounds.y + labelBounds.height + VERT_PADDING) - bounds.y;
            }
            else
            {
                bounds.y -= VERT_PADDING;
                bounds.width = width;
                bounds.height += VERT_PADDING;
            }

            string hoistedPath = path;

            GUIButton overlayBtn = new GUIButton("", EditorStyles.Blank);
            overlayBtn.Bounds = bounds;
            overlayBtn.OnClick += () => OnEntryClicked(hoistedPath);
            overlayBtn.OnDoubleClick += () => OnEntryDoubleClicked(hoistedPath);
            overlayBtn.SetContextMenu(owner.Window.ContextMenu);

            owner.Overlay.AddElement(overlayBtn);

            if (groupUnderlay != null)
            {
                if (owner.GridLayout)
                {
                    int offsetToNext = BG_HORZ_PADDING + owner.HorzElementSpacing;
                    if (type == LibraryGUIEntryType.MultiLast)
                        offsetToNext = BG_HORZ_PADDING * 2;

                    Rect2I bgBounds = new Rect2I(bounds.x - BG_HORZ_PADDING, bounds.y, 
                        bounds.width + offsetToNext, bounds.height);
                    groupUnderlay.Bounds = bgBounds;
                }
                else
                {
                    int offsetToNext = BG_VERT_PADDING + LibraryGUIContent.LIST_ENTRY_SPACING;
                    if (type == LibraryGUIEntryType.MultiLast)
                        offsetToNext = BG_VERT_PADDING * 2;

                    Rect2I bgBounds = new Rect2I(bounds.x, bounds.y - BG_VERT_PADDING,
                        bounds.width, bounds.height + offsetToNext);

                    groupUnderlay.Bounds = bgBounds;
                }
            }
        }

        /// <summary>
        /// Called every frame.
        /// </summary>
        public void Update()
        {
            if (delayedSelect && Time.RealElapsed > delayedSelectTime)
            {
                owner.Window.Select(path);
                delayedSelect = false;
            }

            if (delayedOpenCodeEditorFrame == Time.FrameIdx)
            {
                LibraryEntry entry = ProjectLibrary.GetEntry(path);
                if (entry != null && entry.Type == LibraryEntryType.File)
                {
                    FileEntry resEntry = (FileEntry) entry;
                    CodeEditor.OpenFile(resEntry.Path, 0);
                }

                ProgressBar.Hide();
            }
        }

        /// <summary>
        /// Changes the visual representation of the element as being cut.
        /// </summary>
        /// <param name="enable">True if mark as cut, false to reset to normal.</param>
        public void MarkAsCut(bool enable)
        {
            if (enable)
                icon.SetTint(CUT_COLOR);
            else
                icon.SetTint(Color.White);
        }

        /// <summary>
        /// Changes the visual representation of the element as being selected.
        /// </summary>
        /// <param name="enable">True if mark as selected, false to reset to normal.</param>
        public void MarkAsSelected(bool enable)
        {
            if ((int)underlayState > (int)UnderlayState.Selected)
                return;

            if (enable)
            {
                CreateUnderlay();
                underlay.SetTint(SELECTION_COLOR);
                underlayState = UnderlayState.Selected;
            }
            else
            {
                ClearUnderlay();
                underlayState = UnderlayState.None;
            }
        }

        /// <summary>
        /// Changes the visual representation of the element as being pinged.
        /// </summary>
        /// <param name="enable">True if mark as pinged, false to reset to normal.</param>
        public void MarkAsPinged(bool enable)
        {
            if ((int)underlayState > (int)UnderlayState.Pinged)
                return;

            if (enable)
            {
                CreateUnderlay();
                underlay.SetTint(PING_COLOR);
                underlayState = UnderlayState.Pinged;
            }
            else
            {
                ClearUnderlay();
                underlayState = UnderlayState.None;
            }
        }

        /// <summary>
        /// Changes the visual representation of the element as being hovered over.
        /// </summary>
        /// <param name="enable">True if mark as hovered, false to reset to normal.</param>
        public void MarkAsHovered(bool enable)
        {
            if ((int)underlayState > (int)UnderlayState.Hovered)
                return;

            if (enable)
            {
                CreateUnderlay();
                underlay.SetTint(HOVER_COLOR);
                underlayState = UnderlayState.Hovered;
            }
            else
            {
                ClearUnderlay();
                underlayState = UnderlayState.None;
            }
        }

        /// <summary>
        /// Starts a rename operation over the entry, displaying the rename input box.
        /// </summary>
        public void StartRename()
        {
            if (renameTextBox != null)
                return;

            renameTextBox = new GUITextBox(true);
            Rect2I renameBounds = label.Bounds;

            // Rename box allows for less space for text than label, so adjust it slightly so it's more likely to be able
            // to display all visible text.
            renameBounds.x -= 4;
            renameBounds.width += 8;
            renameBounds.height += 8;

            renameTextBox.Bounds = renameBounds;
            owner.RenameOverlay.AddElement(renameTextBox);

            string name = Path.GetFileNameWithoutExtension(PathEx.GetTail(path));
            renameTextBox.Text = name;
            renameTextBox.Focus = true;
        }

        /// <summary>
        /// Stops a rename operation over the entry, hiding the rename input box.
        /// </summary>
        public void StopRename()
        {
            if (renameTextBox != null)
            {
                renameTextBox.Destroy();
                renameTextBox = null;
            }
        }

        /// <summary>
        /// Gets the new name of the entry. Only valid while a rename operation is in progress.
        /// </summary>
        /// <returns>New name of the entry currently entered in the rename input box.</returns>
        public string GetRenamedName()
        {
            if (renameTextBox != null)
                return renameTextBox.Text;

            return "";
        }

        /// <summary>
        /// Clears the underlay GUI element (for example ping, hover, select).
        /// </summary>
        private void ClearUnderlay()
        {
            if (underlay != null)
            {
                underlay.Destroy();
                underlay = null;
            }

            underlayState = UnderlayState.None;
        }

        /// <summary>
        /// Creates a GUI elements that may be used for underlay effects (for example ping, hover, select).
        /// </summary>
        private void CreateUnderlay()
        {
            if (underlay == null)
            {
                underlay = new GUITexture(Builtin.WhiteTexture);
                underlay.Bounds = Bounds;

                owner.Underlay.AddElement(underlay);
            }
        }

        /// <summary>
        /// Triggered when the user clicks on the entry.
        /// </summary>
        /// <param name="path">Project library path of the clicked entry.</param>
        private void OnEntryClicked(string path)
        {
            LibraryEntry entry = ProjectLibrary.GetEntry(path);
            if (entry != null && entry.Type == LibraryEntryType.Directory)
            {
                // If entry is a directory delay selection as it might be a double-click, in which case we want to keep
                // whatever selection is active currently so that user can perform drag and drop with its inspector
                // from the folder he is browsing to.

                delayedSelect = true;
                delayedSelectTime = Time.RealElapsed + 0.5f;
            }
            else
                owner.Window.Select(path);
        }

        /// <summary>
        /// Triggered when the user double-clicked on the entry.
        /// </summary>
        /// <param name="path">Project library path of the double-clicked entry.</param>
        private void OnEntryDoubleClicked(string path)
        {
            delayedSelect = false;

            LibraryEntry entry = ProjectLibrary.GetEntry(path);
            if (entry != null)
            {
                if (entry.Type == LibraryEntryType.Directory)
                    owner.Window.EnterDirectory(path);
                else
                {
                    ResourceMeta meta = ProjectLibrary.GetMeta(path);

                    FileEntry fileEntry = (FileEntry)entry;
                    if (meta.ResType == ResourceType.Prefab)
                    {
                        EditorApplication.LoadScene(fileEntry.Path);
                    }
                    else if (meta.ResType == ResourceType.ScriptCode)
                    {
                        ProgressBar.Show("Opening external code editor...", 1.0f);

                        delayedOpenCodeEditorFrame = Time.FrameIdx + 1;
                    }
                }
            }
        }

        /// <summary>
        /// Returns an icon that can be used for displaying a resource of the specified type.
        /// </summary>
        /// <param name="path">Path to the project library entry to display data for.</param>
        /// <param name="size">Size of the icon to retrieve, in pixels.</param>
        /// <returns>Icon to display for the specified entry.</returns>
        private static SpriteTexture GetIcon(string path, int size)
        {
            LibraryEntry entry = ProjectLibrary.GetEntry(path);
            if (entry.Type == LibraryEntryType.Directory)
            {
                return EditorBuiltin.GetLibraryItemIcon(LibraryItemIcon.Folder, size);
            }
            else
            {
                ResourceMeta meta = ProjectLibrary.GetMeta(path);
                switch (meta.ResType)
                {
                    case ResourceType.Font:
                        return EditorBuiltin.GetLibraryItemIcon(LibraryItemIcon.Font, size);
                    case ResourceType.Mesh:
                        return EditorBuiltin.GetLibraryItemIcon(LibraryItemIcon.Mesh, size);
                    case ResourceType.Texture:
                        return EditorBuiltin.GetLibraryItemIcon(LibraryItemIcon.Texture, size);
                    case ResourceType.PlainText:
                        return EditorBuiltin.GetLibraryItemIcon(LibraryItemIcon.PlainText, size);
                    case ResourceType.ScriptCode:
                        return EditorBuiltin.GetLibraryItemIcon(LibraryItemIcon.ScriptCode, size);
                    case ResourceType.SpriteTexture:
                        return EditorBuiltin.GetLibraryItemIcon(LibraryItemIcon.SpriteTexture, size);
                    case ResourceType.Shader:
                        return EditorBuiltin.GetLibraryItemIcon(LibraryItemIcon.Shader, size);
                    case ResourceType.ShaderInclude:
                        return EditorBuiltin.GetLibraryItemIcon(LibraryItemIcon.Shader, size);
                    case ResourceType.Material:
                        return EditorBuiltin.GetLibraryItemIcon(LibraryItemIcon.Material, size);
                    case ResourceType.Prefab:
                        return EditorBuiltin.GetLibraryItemIcon(LibraryItemIcon.Prefab, size);
                    case ResourceType.GUISkin:
                        return EditorBuiltin.GetLibraryItemIcon(LibraryItemIcon.GUISkin, size);
                    case ResourceType.PhysicsMaterial:
                        return EditorBuiltin.GetLibraryItemIcon(LibraryItemIcon.PhysicsMaterial, size);
                    case ResourceType.PhysicsMesh:
                        return EditorBuiltin.GetLibraryItemIcon(LibraryItemIcon.PhysicsMesh, size);
                    case ResourceType.AudioClip:
                        return EditorBuiltin.GetLibraryItemIcon(LibraryItemIcon.AudioClip, size);
                    case ResourceType.AnimationClip:
                        return EditorBuiltin.GetLibraryItemIcon(LibraryItemIcon.AnimationClip, size);
                }
            }

            return null;
        }
    }

    /// <summary>
    /// Type of <see cref="LibraryGUIEntry"/> that controls its look.
    /// </summary>
    internal enum LibraryGUIEntryType
    {
        /// <summary>
        /// Represents a single resource.
        /// </summary>
        Single,
        /// <summary>
        /// Represents the first entry in a multi-resource group.
        /// </summary>
        MultiFirst,
        /// <summary>
        /// Represents one of the mid entries in a multi-resource group.
        /// </summary>
        MultiElement,
        /// <summary>
        /// Represents the last entry in a multi-resource group.
        /// </summary>
        MultiLast
    }

    /** @} */
}
