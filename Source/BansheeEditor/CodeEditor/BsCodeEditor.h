//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsEditorPrerequisites.h"
#include "Utility/BsModule.h"

namespace bs
{
	/** @addtogroup CodeEditor-Internal
	 *  @{
	 */

	class CodeEditor;
	class CodeEditorFactory;

	/** Contains data about a reference to a project in an external editor solution. */
	struct BS_ED_EXPORT CodeProjectReference
	{
		WString name;
		Path path;
	};

	/**	Contains data about a single project in an external editor solution. */
	struct BS_ED_EXPORT CodeProjectData
	{
		WString name;
		Vector<Path> codeFiles;
		Vector<Path> nonCodeFiles;
		WString defines;
		Vector<CodeProjectReference> assemblyReferences;
		Vector<CodeProjectReference> projectReferences;
	};

	/**	Contains data about an external editor solution, including all projects contained. */
	struct BS_ED_EXPORT CodeSolutionData
	{
		WString name;
		Vector<CodeProjectData> projects;
	};

	/** @} */
	/** @addtogroup CodeEditor
	 *  @{
	 */

	/**
	 * Handles connectivity of the editor with external code editing tools. The system provides methods for interacting with
	 * external tools but the exact tool used depends on the currently active setting.
	 */
	class BS_ED_EXPORT CodeEditorManager : public Module<CodeEditorManager>
	{
	public:
		CodeEditorManager();
		~CodeEditorManager();

		/**	Returns a list of all available code editors for this platform. */
		const Vector<CodeEditorType>& getAvailableEditors() const { return mEditors; }

		/**
		 * Changes the active code editor. All further operations on this object will be executed using this editor. If the
		 * specified editor is not valid for this platform, no change will be made.
		 */
		void setActive(CodeEditorType editor);

		/**	Returns the currently active code editor. */
		CodeEditorType getActive() const { return mActiveEditorType; }

		/**
		 * Opens a code file in the active external editor. 
		 *
		 * @param[in]	path		Path to the code file to open, can be absolute or relative to project resources folder.
		 *							The file should be part of a solution in the active editor.
		 * @param[in]	lineNumber	Line number to focus on once the file is opened. Might not be supported by all editors.
		 */
		void openFile(const Path& path, UINT32 lineNumber) const;

		/**
		 * Synchronizes all code files and assemblies in the active project and updates the project solution for the active
		 * editor. Each project can only have one solution per editor.
		 */
		void syncSolution() const;

	private:
		/**	Returns the absolute path at which the external editor solution file should be stored. */
		Path getSolutionPath() const;

		CodeEditor* mActiveEditor;
		CodeEditorType mActiveEditorType;
		Map<CodeEditorType, CodeEditorFactory*> mFactoryPerEditor;
		Vector<CodeEditorType> mEditors;
		Vector<CodeEditorFactory*> mFactories;
	};

	/** @} */
	/** @addtogroup CodeEditor-Internal
	 *  @{
	 */

	/**
	 * Interface that classes interacting with external code editors needs to implement.
	 *
	 * @see		CodeEditorManager
	 */
	class BS_ED_EXPORT CodeEditor
	{
	public:
		virtual ~CodeEditor() { }

		/** 
		 * @copydoc CodeEditorManager::openFile 
		 *
		 * @param[in]	solutionPath	Path to the solution file the file is a part of.
		 */
		virtual void openFile(const Path& solutionPath, const Path& path, UINT32 lineNumber) const = 0;

		/** 
		 * @copydoc CodeEditorManager::syncSolution 
		 *
		 * @param[in]	data		Information about the solution and the files it contains.
		 * @param[in]	outputPath	Path to the file into which to output the solution.
		 */
		virtual void syncSolution(const CodeSolutionData& data, const Path& outputPath) const = 0;
	};

	/**
	 * Interface for factory that creates a specific implementation(s) of a code editor.
	 *
	 * @see		CodeEditor
	 */
	class BS_ED_EXPORT CodeEditorFactory
	{
	public:
		virtual ~CodeEditorFactory() { }

		/**	Returns a list of code editors supported by this factory. */
		virtual const Vector<CodeEditorType>& getAvailableEditors() const = 0;

		/**
		 * Creates a specific implementation of a code editor.
		 *
		 * @param[in]	editor	Type of editor to create. Make sure to provide a valid value returned by 
		 *						getAvailableEditors().
		 */
		virtual CodeEditor* create(CodeEditorType editor) const = 0;
	};

	/** Different versions of .csproj files, depending on which Microsoft Visual Studio version generated them. */
	enum class CSProjectVersion
	{
		VS2008,
		VS2010,
		VS2012,
		VS2013,
		VS2015,
		VS2017,
		MonoDevelop
	};

	/** 
	 * Contains helper functionality for the generation of .csproj files, as well as the .sln file. Those are used by C# IDE's like Visual Studio
	 * and MonoDevelop, and build systems like msbuild or xbuild.
	 */
	class CSProject
	{
	public:
		/**
		 * Builds the .sln text for the provided version, using the provided solution data.
		 *
		 * @param[in]	version	Visual Studio version for which we're generating the solution file.
		 * @param[in]	data	Data containing a list of projects and other information required to build the solution text.
		 * @return				Generated text of the solution file.
		 */
		static String writeSolution(CSProjectVersion version, const CodeSolutionData& data);

		/**
		 * Builds the .csproj text for the provided version, using the provided project data.
		 *
		 * @param[in]	version		Visual Studio version for which we're generating the project file.
		 * @param[in]	projectData	Data containing a list of files, references and other information required to 
		 *							build the project text.
		 * @return					Generated text of the project file.
		 */
		static String writeProject(CSProjectVersion version, const CodeProjectData& projectData);

	private:
		static const String SLN_TEMPLATE; /**< Template text used for a solution file. */
		static const String PROJ_ENTRY_TEMPLATE; /**< Template text used for a project entry in a solution file. */
		static const String PROJ_PLATFORM_TEMPLATE; /**< Template text used for platform specific information for a project entry in a solution file. */

		static const String PROJ_TEMPLATE; /**< Template XML used for a project file. */
		static const String REFERENCE_ENTRY_TEMPLATE; /**< Template XML used for a reference to another assembly entry by name. */
		static const String REFERENCE_PROJECT_ENTRY_TEMPLATE; /**< Template XML used for a reference to another project entry. */
		static const String REFERENCE_PATH_ENTRY_TEMPLATE; /**< Template XML used for a reference to another assembly entry by name and path. */
		static const String CODE_ENTRY_TEMPLATE; /**< Template XML used for a single code file entry in a project. */
		static const String NON_CODE_ENTRY_TEMPLATE; /**< Template XML used for a single non-code file entry in a project. */
	};

	/** @} */
}