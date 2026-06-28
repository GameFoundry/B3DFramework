//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DShaderCooker.h"

#include "Material/B3DShaderCompiler.h"
#include "Material/B3DShaderRegistry.h"
#include "Material/B3DShader.h"
#include "Material/B3DVariation.h"
#include "Material/B3DPass.h"
#include "GpuBackend/B3DGpuProgram.h"
#include "Resources/B3DPackage.h"
#include "Resources/B3DPackageManager.h"
#include "FileSystem/B3DFileSystem.h"
#include "Debug/B3DDebug.h"

using namespace b3d;

void ShaderCooker::StripVariationSource(PrecompiledVariationData& variationData)
{
	for(PassInformation& pass : variationData.Passes)
	{
		GpuProgramCreateInformation* programs[] =
		{
			&pass.VertexProgramCreateInformation,
			&pass.FragmentProgramCreateInformation,
			&pass.GeometryProgramCreateInformation,
			&pass.HullProgramCreateInformation,
			&pass.DomainProgramCreateInformation,
			&pass.ComputeProgramCreateInformation,
		};

		for(GpuProgramCreateInformation* program : programs)
			program->Source.clear();
	}
}

bool ShaderCooker::CookItem(const ShaderCookItem& item, const String& language, Package& package, u32& outVariationCount)
{
	// 1) Compile the shader meta-data for this language. This does not compile the variations (that happens below),
	//    it only produces the variation parameter sets and the compiler meta-data.
	const TShared<Shader> shader = ShaderCompilers::Instance().CompileShader<false>(item.Name, item.Source, item.Defines, { language });
	if(shader == nullptr)
	{
		B3D_LOG(Error, LogResources, "Shader cook failed for \"{0}\" (language \"{1}\"): compilation produced no shader.", item.Name, language);
		return false;
	}

	const TShared<ShaderCompilerMetaData>& metaData = shader->GetCompilerMetaData();
	if(metaData == nullptr)
	{
		B3D_LOG(Error, LogResources, "Shader cook failed for \"{0}\" (language \"{1}\"): compiled shader has no compiler meta-data.", item.Name, language);
		return false;
	}

	// Replicate the runtime resolver's post-compile bookkeeping (B3DShaderRegistry.cpp) so the cooked keys line up
	// byte-for-byte with the ShaderRegistry lookup. GetVariationPath folds NameInCache, ShaderHash and the include
	// hashes into the variation key, so these must be set before any variation path is derived.
	metaData->NameInCache = ShaderRegistry::GetShaderCacheName(item.CachePrefix, item.Name);
	metaData->ShaderHash = Shader::ComputeHash(item.Source);

	const TShared<IShaderCompiler> bslCompiler = ShaderCompilers::Instance().GetCompiler("bsl");
	if(bslCompiler == nullptr)
	{
		B3D_LOG(Error, LogResources, "Shader cook failed for \"{0}\": the BSL shader compiler is not available.", item.Name);
		return false;
	}

	// 2) Cook every variation. The shader's source must stay intact for these compiles, so the meta-data Source is only
	//    stripped afterwards in step 3.
	for(const ShaderVariationParameters& variationParameters : metaData->Variations)
	{
		const TShared<Variation> variation = Variation::Create(shader, language, variationParameters);

		const ShaderCompilerResult compileResult = bslCompiler->CompileVariation(*shader, variationParameters, language, *variation);
		if(!compileResult.ErrorMessage.empty())
		{
			B3D_LOG(Error, LogResources, "Shader cook failed for a variation of \"{0}\" (language \"{1}\"): {2}", item.Name, language, compileResult.ErrorMessage);
			continue;
		}

		const TShared<PrecompiledVariationData> variationData = variation->GetPrecompiledData();
		if(variationData == nullptr)
		{
			B3D_LOG(Warning, LogResources, "Shader cook skipped a variation of \"{0}\" (language \"{1}\"): no precompiled data was produced.", item.Name, language);
			continue;
		}

		// Keep source in development builds, for easier debugging
#if !B3D_BUILD_TYPE_DEVELOPMENT
		StripVariationSource(*variationData);
#endif

		const Path variationPath = ShaderRegistry::GetVariationPath(*metaData, language, variationParameters.CreateVariationName());
		package.AddResource(variationPath, PrebuiltShader::Create(variationData));
		++outVariationCount;
	}

	// 3) Shader meta-data artifact. Take a snapshot, clone the compiler meta-data so the shipped copy can drop its
	//    high-level Source (never read by Material/reflection) without disturbing the live shader, then add it.
	const TShared<PrecompiledShaderData> shaderData = shader->GetPrecompiledData();

	// Keep source in development builds, for easier debugging
#if !B3D_BUILD_TYPE_DEVELOPMENT
	if(shaderData != nullptr && shaderData->CompilerMetaData != nullptr)
	{
		const TShared<ShaderCompilerMetaData> strippedMetaData = B3DMakeShared<ShaderCompilerMetaData>(*shaderData->CompilerMetaData);
		strippedMetaData->Source.clear();
		shaderData->CompilerMetaData = strippedMetaData;
	}
#endif

	const Path metaDataPath = ShaderRegistry::GetShaderMetaDataPath(metaData->NameInCache, language);
	package.AddResource(metaDataPath, PrebuiltShader::Create(shaderData));

	return true;
}

bool ShaderCooker::Cook(const Vector<ShaderCookItem>& items, const CookOptions& options)
{
	const String packageName = options.OutputPath.GetFilename(false);
	const TShared<Package> package = Package::Create(packageName);

	u32 cookedShaderCount = 0;
	u32 cookedVariationCount = 0;

	for(const ShaderCookItem& item : items)
	{
		if(CookItem(item, options.Language, *package, cookedVariationCount))
			++cookedShaderCount;
	}

	// Ensure the destination folder exists, then write the package. A brand-new package not registered with the
	// package manager can be saved as-is (see PackageManager::SavePackage).
	FileSystem::CreateFolder(options.OutputPath.GetParent());

	PackageManagerSavePackageOptions saveOptions;
	PackageManager::Instance().SavePackage(package, options.OutputPath, saveOptions);

	B3D_LOG(Info, LogResources, "Shader cook complete: wrote {0} shader(s) and {1} variation(s) to \"{2}\".",
		cookedShaderCount, cookedVariationCount, options.OutputPath.ToString());

	return true;
}
