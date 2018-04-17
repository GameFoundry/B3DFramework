//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Resources/BsScriptCodeImporter.h"
#include "Resources/BsScriptCode.h"
#include "FileSystem/BsDataStream.h"
#include "FileSystem/BsFileSystem.h"
#include "Resources/BsScriptCodeImportOptions.h"

namespace bs
{
	bool ScriptCodeImporter::isExtensionSupported(const String& ext) const
	{
		String lowerCaseExt = ext;
		StringUtil::toLowerCase(lowerCaseExt);

		return lowerCaseExt == u8"cs";
	}

	bool ScriptCodeImporter::isMagicNumberSupported(const UINT8* magicNumPtr, UINT32 numBytes) const
	{
		return true; // Plain-text so we don't even check for magic number
	}

	SPtr<Resource> ScriptCodeImporter::import(const Path& filePath, SPtr<const ImportOptions> importOptions)
	{
		WString textData;
		{
			Lock fileLock = FileScheduler::getLock(filePath);

			SPtr<DataStream> stream = FileSystem::openFile(filePath);
			textData = stream->getAsWString();
		}

		bool editorScript = false;
		if (importOptions != nullptr)
		{
			SPtr<const ScriptCodeImportOptions> scriptIO = std::static_pointer_cast<const ScriptCodeImportOptions>(importOptions);
			editorScript = scriptIO->isEditorScript();
		}
			
		SPtr<ScriptCode> scriptCode = ScriptCode::_createPtr(textData, editorScript);

		const String fileName = filePath.getFilename(false);
		scriptCode->setName(fileName);

		return scriptCode;
	}

	SPtr<ImportOptions> ScriptCodeImporter::createImportOptions() const
	{
		return bs_shared_ptr_new<ScriptCodeImportOptions>();
	}
}