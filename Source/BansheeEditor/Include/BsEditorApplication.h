//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsEditorPrerequisites.h"
#include "BsApplication.h"

namespace BansheeEngine
{
	/** @addtogroup Application-Editor
	 *  @{
	 */

	/**	Primary editor class containing the editor entry point. */
	class BS_ED_EXPORT EditorApplication : public Application
	{
	public:
		EditorApplication();
		virtual ~EditorApplication();

		/**	Starts the editor with the specified render and audio systems. */
		static void startUp();

		/**	Checks whether the editor currently has a project loaded. */
		bool isProjectLoaded() const { return mIsProjectLoaded; }

		/**	Returns the path to the currently loaded project. */
		const Path& getProjectPath() const { return mProjectPath; }

		/**	Returns the name of the currently loaded project. */
		const WString& getProjectName() const { return mProjectName; }

		/**	Returns the absolute path to the built-in managed editor assembly file. */
		Path getEditorAssemblyPath() const;

		/**	Returns the absolute path of the managed editor script assembly file. */
		Path getEditorScriptAssemblyPath() const;

		/** @copydoc	Application::getScriptAssemblyFolder */
		Path getScriptAssemblyFolder() const override;

		/** Returns a set of serializable editor settings that contain every globally customizable editor property. */
		SPtr<EditorSettings> getEditorSettings() const { return mEditorSettings; }

		/** Returns a set of serializable project settings that contain every customizable property specific to a project. */
		SPtr<ProjectSettings> getProjectSettings() const { return mProjectSettings; }

		/**	Saves the current editor settings at the default location. */
		void saveEditorSettings();

		/** Saves the current project settings at the default location. Does nothing if no project is loaded. */
		void saveProjectSettings();

		/**	Saves any project specific data, if a project is currently loaded. */
		void saveProject();

		/**	Unloads the currently loaded project, if any. */
		void unloadProject();

		/**
		 * Loads a new project, unloading the current one. 
		 *
		 * @param[in]	path	Absolute path to the root project folder. Must be pointing to a valid project.
		 */
		void loadProject(const Path& path);

		/**
		 * Creates a new project at the specified path.
		 *
		 * @param[in]	path	Path to the folder where to create the project in. Name of this folder will be the name of
		 *						the project.
		 */
		void createProject(const Path& path);

		/**
		 * Checks is the provided folder a valid project.
		 *
		 * @param[in]	path	Absolute path to the root project folder.
		 */
		bool isValidProjectPath(const Path& path);

		/** @copydoc Application::isEditor */
		bool isEditor() const override { return true; }
	private:
		/** @copydoc Module::onStartUp */
		void onStartUp() override;

		/** @copydoc Module::onShutDown */
		void onShutDown() override;

		/** @copydoc CoreApplication::preUpdate */
		void preUpdate() override;

		/** @copydoc CoreApplication::postUpdate */
		void postUpdate() override;

		/** @copydoc CoreApplication::quitRequested */
		void quitRequested() override;

		/** @copydoc Application::loadScriptSystem */
		void loadScriptSystem() override;

		/**
		 * Loads the previously saved editor widget layout from the default location. Can return null if no layout was 
		 * previously saved.
		 */
		SPtr<EditorWidgetLayout> loadWidgetLayout();

		/**	Saves the provided widget layout at the default layout location. */
		void saveWidgetLayout(const SPtr<EditorWidgetLayout>& layout);

		/** Saves the default widget layout in the current project folder. */
		void saveDefaultWidgetLayout();

		/** Loads the previously saved editor settings from the default location. Overwrites any current settings. */
		void loadEditorSettings();

		/**
		 * Loads the previously saved project settings from the default location within the active project. Loads default
		 * settings if no project is active. Overwrites any current settings.
		 */
		void loadProjectSettings();

		/** @copydoc Application::getShaderIncludeHandler */
		SPtr<IShaderIncludeHandler> getShaderIncludeHandler() const override;

	private:
		static const Path WIDGET_LAYOUT_PATH;
		static const Path BUILD_DATA_PATH;
		static const Path PROJECT_SETTINGS_PATH;

		SPtr<EditorSettings> mEditorSettings;
		SPtr<ProjectSettings> mProjectSettings;

		bool mIsProjectLoaded;
		Path mProjectPath;
		WString mProjectName;

		DynLib* mSBansheeEditorPlugin;
	};

	/**	Easy way to access EditorApplication. */
	BS_ED_EXPORT EditorApplication& gEditorApplication();

	/** @} */
}