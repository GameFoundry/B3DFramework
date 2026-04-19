//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Resources/B3DScriptCodeImporter.h"
#include "Resources/B3DScriptCode.h"
#include "FileSystem/B3DDataStream.h"
#include "FileSystem/B3DFileSystem.h"
#include "Resources/B3DScriptCodeImportOptions.h"

using namespace b3d;

bool ScriptCodeImporter::IsExtensionSupported(const String& ext) const
{
	String lowerCaseExt = ext;
	StringUtility::ToLowerCase(lowerCaseExt);

	return lowerCaseExt == u8"cs";
}

bool ScriptCodeImporter::IsMagicNumberSupported(const u8* magicNumPtr, u32 numBytes) const
{
	return true; // Plain-text so we don't even check for magic number
}

SPtr<Resource> ScriptCodeImporter::Import(const Path& filePath, SPtr<const ImportOptions> importOptions)
{
	WString textData;
	{
		SPtr<DataStream> stream = FileSystem::OpenFile(filePath);
		textData = stream->GetAsWString();
	}

	bool editorScript = false;
	if(importOptions != nullptr)
	{
		SPtr<const ScriptCodeImportOptions> scriptIO = std::static_pointer_cast<const ScriptCodeImportOptions>(importOptions);
		editorScript = scriptIO->EditorScript;
	}

	SPtr<ScriptCode> scriptCode = ScriptCode::CreatePtrInternal(textData, editorScript);

	const String fileName = filePath.GetFilename(false);
	scriptCode->SetName(fileName);

	return scriptCode;
}

SPtr<ImportOptions> ScriptCodeImporter::CreateImportOptions() const
{
	return B3DMakeShared<ScriptCodeImportOptions>();
}
