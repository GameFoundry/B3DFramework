//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DSLPrerequisites.h"
#include "B3DBSLCompiler.h"
#include "B3DSLImporter.h"
#include "Importer/B3DImporter.h"
#include "Material/B3DShaderCompiler.h"

using namespace b3d;


/**	Returns a name of the plugin. */
extern "C" B3D_PLUGIN_EXPORT const char* GetPluginName()
{
	static constexpr const char* kSystemName = "bsfSL";
	return kSystemName;
}

/**	Entry point to the plugin. Called by the engine when the plugin is loaded. */
extern "C" B3D_PLUGIN_EXPORT void* LoadPlugin()
{
	SLImporter* importer = B3DNew<SLImporter>();
	Importer::Instance().RegisterAssetImporter(importer);

	const TShared<IShaderCompiler> compiler = B3DMakeShared<BSLCompiler>();
	ShaderCompilers::Instance().RegisterCompiler(SLImporter::kShaderExtensionWithoutLeadingDot, compiler);

	return nullptr;
}

/**	Called by the engine when the plugin is about to be unloaded. */
extern "C" B3D_PLUGIN_EXPORT void UnloadPlugin()
{
	ShaderCompilers::Instance().UnregisterCompiler(SLImporter::kShaderExtensionWithoutLeadingDot);
}
