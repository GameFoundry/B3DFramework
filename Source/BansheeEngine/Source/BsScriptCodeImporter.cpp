//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptCodeImporter.h"
#include "BsScriptCode.h"
#include "BsDataStream.h"
#include "BsFileSystem.h"
#include "BsScriptCodeImportOptions.h"

namespace bs
{
	ScriptCodeImporter::ScriptCodeImporter()
		:SpecificImporter()
	{

	}

	ScriptCodeImporter::~ScriptCodeImporter()
	{

	}

	bool ScriptCodeImporter::isExtensionSupported(const WString& ext) const
	{
		WString lowerCaseExt = ext;
		StringUtil::toLowerCase(lowerCaseExt);

		return lowerCaseExt == L"cs";
	}

	bool ScriptCodeImporter::isMagicNumberSupported(const UINT8* magicNumPtr, UINT32 numBytes) const
	{
		return true; // Plain-text so we don't even check for magic number
	}

	SPtr<Resource> ScriptCodeImporter::import(const Path& filePath, SPtr<const ImportOptions> importOptions)
	{
		SPtr<DataStream> stream = FileSystem::openFile(filePath);
		WString textData = stream->getAsWString();

		bool editorScript = false;
		if (importOptions != nullptr)
		{
			SPtr<const ScriptCodeImportOptions> scriptIO = std::static_pointer_cast<const ScriptCodeImportOptions>(importOptions);
			editorScript = scriptIO->isEditorScript();
		}
			
		SPtr<ScriptCode> scriptCode = ScriptCode::_createPtr(textData, editorScript);

		WString fileName = filePath.getWFilename(false);
		scriptCode->setName(fileName);

		return scriptCode;
	}

	SPtr<ImportOptions> ScriptCodeImporter::createImportOptions() const
	{
		return bs_shared_ptr_new<ScriptCodeImportOptions>();
	}
}