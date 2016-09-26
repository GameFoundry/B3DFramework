//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.IO;
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup Windows
     *  @{
     */

    /// <summary>
    /// Project management window that allows the user to open and create new projects.
    /// </summary>
    internal sealed class ProjectWindow : ModalWindow
    {
        private GUITextField projectInputBox;
        private GUIScrollArea recentProjectsArea;
        private GUIToggle autoLoadToggle;

        /// <summary>
        /// Opens the project management window.
        /// </summary>
        /// <returns>Instance of the project window</returns>
        public static ProjectWindow Open()
        {
            return new ProjectWindow();
        }

        /// <summary>
        /// Constructs a new project window.
        /// </summary>
        private ProjectWindow()
            : base(false)
        {

        }

        private void OnInitialize()
        {
            Title = "Project Manager";

            Width = 500;
            Height = 290;

            GUILayout vertLayout = GUI.AddLayoutY();
            
            vertLayout.AddSpace(5);
            GUILayout firstRow = vertLayout.AddLayoutX();
            vertLayout.AddFlexibleSpace();
            GUILayout secondRow = vertLayout.AddLayoutX();
            vertLayout.AddSpace(5);
            GUILayout thirdRow = vertLayout.AddLayoutX();
            vertLayout.AddFlexibleSpace();
            GUILayout fourthRow = vertLayout.AddLayoutX();
            vertLayout.AddSpace(5);

            projectInputBox = new GUITextField(new LocEdString("Project path"), 70, false, "", GUIOption.FixedWidth(398));
            projectInputBox.Value = EditorSettings.LastOpenProject;

            GUIButton openBtn = new GUIButton(new LocEdString("Open"), GUIOption.FixedWidth(75));
            openBtn.OnClick += OpenProject;

            firstRow.AddSpace(5);
            firstRow.AddElement(projectInputBox);
            firstRow.AddSpace(15);
            firstRow.AddElement(openBtn);
            firstRow.AddSpace(5);

            GUILabel recentProjectsLabel = new GUILabel(new LocEdString("Recent projects:"));

            secondRow.AddSpace(5);
            secondRow.AddElement(recentProjectsLabel);
            secondRow.AddFlexibleSpace();
            GUIButton browseBtn = new GUIButton(new LocEdString("Browse"), GUIOption.FixedWidth(75));
            browseBtn.OnClick += BrowseClicked;
            secondRow.AddElement(browseBtn);
            secondRow.AddSpace(5);

            thirdRow.AddSpace(5);
            GUIPanel recentProjectsPanel = thirdRow.AddPanel();
            thirdRow.AddSpace(15);
            GUILayoutY thirdRowVertical = thirdRow.AddLayoutY();
            GUIButton createBtn = new GUIButton(new LocEdString("Create new"), GUIOption.FixedWidth(75));
            createBtn.OnClick += CreateClicked;
            thirdRowVertical.AddElement(createBtn);
            thirdRowVertical.AddFlexibleSpace();
            thirdRow.AddSpace(5);

            recentProjectsArea = new GUIScrollArea(GUIOption.FixedWidth(385), GUIOption.FixedHeight(170));
            GUILayoutX recentProjectsLayout = recentProjectsPanel.AddLayoutX();
            recentProjectsLayout.AddSpace(10);
            GUILayoutY recentProjectsPanelY = recentProjectsLayout.AddLayoutY();
            recentProjectsPanelY.AddSpace(5);
            recentProjectsPanelY.AddElement(recentProjectsArea);
            recentProjectsPanelY.AddSpace(5);
            recentProjectsLayout.AddFlexibleSpace();

            GUIPanel scrollAreaBgPanel = recentProjectsPanel.AddPanel(1);
            GUITexture scrollAreaBgTex = new GUITexture(null, true, EditorStylesInternal.ScrollAreaBg);
            scrollAreaBgPanel.AddElement(scrollAreaBgTex);

            autoLoadToggle = new GUIToggle("");
            autoLoadToggle.Value = EditorSettings.AutoLoadLastProject;

            GUILabel autoLoadLabel = new GUILabel(new LocEdString("Automatically load last open project"));

            GUIButton cancelBtn = new GUIButton(new LocEdString("Cancel"), GUIOption.FixedWidth(75));
            cancelBtn.OnClick += CancelClicked;

            fourthRow.AddSpace(5);
            fourthRow.AddElement(autoLoadToggle);
            fourthRow.AddElement(autoLoadLabel);
            fourthRow.AddFlexibleSpace();
            fourthRow.AddElement(cancelBtn);
            fourthRow.AddSpace(5);

            RefreshRecentProjects();
        }

        /// <summary>
        /// Attempts to open a project at the path currently entered in the project path input box.
        /// </summary>
        private void OpenProject()
        {
            string projectPath = projectInputBox.Value;

            if (EditorApplication.IsValidProject(projectPath))
            {
                EditorSettings.AutoLoadLastProject = autoLoadToggle.Value;

                Close();
                EditorApplication.LoadProject(projectPath);
            }
            else
            {
                // Remove invalid project from recent projects list
                RecentProject[] recentProjects = EditorSettings.RecentProjects;
                for (int i = 0; i < recentProjects.Length; i++)
                {
                    if (PathEx.Compare(recentProjects[i].path, projectPath))
                    {
                        RecentProject[] newRecentProjects = new RecentProject[recentProjects.Length - 1];
                        int idx = 0;
                        for (int j = 0; j < recentProjects.Length; j++)
                        {
                            if (i == j)
                                continue;

                            newRecentProjects[idx] = recentProjects[j];
                            idx++;
                        }

                        EditorSettings.RecentProjects = newRecentProjects;
                        EditorSettings.Save();
                        RefreshRecentProjects();

                        break;
                    }
                }

                // Warn user
                LocString message = new LocEdString("Provided project path \"{0}\" doesn't contain a valid project.");
                message.SetParameter(0, projectPath);

                DialogBox.Open(new LocEdString("Error"), message, DialogBox.Type.OK);
            }
        }

        /// <summary>
        /// Triggered when the user clicks on the browse button. Opens a browse dialog that allows the user to select
        /// a project folder anywhere on the file system.
        /// </summary>
        void BrowseClicked()
        {
            string projectPath = projectInputBox.Value;
            if (!Directory.Exists(projectPath))
                projectPath = Directory.GetCurrentDirectory();

            string selectedPath;
            if (BrowseDialog.OpenFolder(projectPath, out selectedPath))
            {
                projectInputBox.Value = selectedPath;
                OpenProject();
            }
        }

        /// <summary>
        /// Triggered when the user clicks on the create buttons. Opens a browse dialog that allows the user to select
        /// a folder to a new project to create. Project data will be initialized in the chosen folder and new project
        /// will be opened.
        /// </summary>
        void CreateClicked()
        {
            string projectPath = projectInputBox.Value;
            if (!Directory.Exists(projectPath))
                projectPath = Directory.GetCurrentDirectory();

            string selectedPath;
            if (BrowseDialog.OpenFolder(projectPath, out selectedPath))
            {
                projectInputBox.Value = selectedPath;

                EditorApplication.CreateProject(selectedPath);
                OpenProject();
            }
        }

        /// <summary>
        /// Triggered when the user clicks the cancel button.
        /// </summary>
        void CancelClicked()
        {
            if (EditorApplication.IsProjectLoaded)
                Close(); // Just close the window
            else
                EditorApplication.Quit(); // Close the application, we cannot do anything without a project
        }

        /// <summary>
        /// Updates GUI for the recent projects list.
        /// </summary>
        private void RefreshRecentProjects()
        {
            GUILayout scrollLayout = recentProjectsArea.Layout;
            while (scrollLayout.ChildCount > 0)
                scrollLayout.GetChild(0).Destroy();

            RecentProject[] recentProjects = EditorSettings.RecentProjects;
            Array.Sort(recentProjects, (a, b) => a.accessTimestamp.CompareTo(b.accessTimestamp));

            GUIToggleGroup grp = new GUIToggleGroup();
            for (int i = 0; i < recentProjects.Length; i++)
            {
                string projectPath = recentProjects[i].path;

                GUIToggle entryBtn = new GUIToggle(projectPath, grp, EditorStylesInternal.SelectableLabel);
                entryBtn.OnClick += () => OnEntryClicked(projectPath);
                entryBtn.OnDoubleClick += () => OnEntryDoubleClicked(projectPath);

                if (PathEx.Compare(projectPath, projectInputBox.Value))
                    entryBtn.Value = true;

                scrollLayout.AddElement(entryBtn);
            }
        }

        /// <summary>
        /// Triggered when an entry in the recent projects list was clicked.
        /// </summary>
        /// <param name="path">Absolute path to the project folder that was selected.</param>
        void OnEntryClicked(string path)
        {
            projectInputBox.Value = path;
        }

        /// <summary>
        /// Triggered when an entry in the recent projects list was double clicked. Opens the project at the selected path.
        /// </summary>
        /// <param name="path">Absolute path to the project folder that was selected.</param>
        void OnEntryDoubleClicked(string path)
        {
            projectInputBox.Value = path;
            OpenProject();
        }
    }

    /** @} */
}
