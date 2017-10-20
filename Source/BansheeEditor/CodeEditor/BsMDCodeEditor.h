//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsEditorPrerequisites.h"
#include "CodeEditor/BsCodeEditor.h"

namespace bs
{
	/** @addtogroup CodeEditor-Internal
	 *  @{
	 */

	/**	Code editor implementation that handles interacting with MonoDevelop. */
	class BS_ED_EXPORT MDCodeEditor : public CodeEditor
	{
	public:
		MDCodeEditor(const Path& execPath);

		/** @copydoc CodeEditor::openFile */
		void openFile(const Path& solutionPath, const Path& path, UINT32 lineNumber) const override;

		/** @copydoc CodeEditor::syncSolution */
		void syncSolution(const CodeSolutionData& data, const Path& outputPath) const override;

	private:
		Path mExecPath;
	};

	/** Code editor factory used for creating specific instances of the MonoDevelop code editor object. */
	class BS_ED_EXPORT MDCodeEditorFactory : public CodeEditorFactory
	{
	public:
		MDCodeEditorFactory();

		/** @copydoc CodeEditorFactory::getAvailableEditors */
		const Vector<CodeEditorType>& getAvailableEditors() const override { return mSupportedEditors; }

		/** @copydoc CodeEditorFactory::create */
		CodeEditor* create(CodeEditorType editor) const override;

	private:
		Path mInstallPath;
		Vector<CodeEditorType> mSupportedEditors;
	};

	/** @} */
}
