//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsEditorPrerequisites.h"
#include "BsModule.h"

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

	/** @} */
}