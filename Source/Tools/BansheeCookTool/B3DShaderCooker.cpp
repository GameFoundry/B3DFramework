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

namespace
{
	/** Human-readable name for a GPU program type, for cook diagnostics. */
	const char* GetProgramTypeName(GpuProgramType type)
	{
		switch(type)
		{
		case GPT_VERTEX_PROGRAM: return "vertex";
		case GPT_FRAGMENT_PROGRAM: return "fragment";
		case GPT_GEOMETRY_PROGRAM: return "geometry";
		case GPT_DOMAIN_PROGRAM: return "domain";
		case GPT_HULL_PROGRAM: return "hull";
		case GPT_COMPUTE_PROGRAM: return "compute";
		default: return "unknown";
		}
	}

	/** How a single cooked variation should be handled, based on the state of its compiled programs. */
	enum class VariationBytecodeState
	{
		Complete, /**< Every program of the variation carries bytecode; the variation can be shipped. */
		UnsupportedStage, /**< A program uses a stage the language's bytecode compiler cannot handle; skip the variation. */
		MissingBytecode /**< A supported program produced no bytecode; the variation (and the cook) failed. */
	};

	/**
	 * Validates that every program of the cooked variation carries the bytecode the target platform will consume.
	 * @p outProgramType receives the offending program's type unless the returned state is Complete.
	 */
	VariationBytecodeState DetermineVariationBytecodeState(const PrecompiledVariationData& variationData,
		const IGpuBytecodeCompiler& bytecodeCompiler, GpuProgramType& outProgramType)
	{
		for(const PassInformation& pass : variationData.Passes)
		{
			const GpuProgramCreateInformation* programs[] =
			{
				&pass.VertexProgramCreateInformation,
				&pass.FragmentProgramCreateInformation,
				&pass.GeometryProgramCreateInformation,
				&pass.HullProgramCreateInformation,
				&pass.DomainProgramCreateInformation,
				&pass.ComputeProgramCreateInformation,
			};

			for(const GpuProgramCreateInformation* program : programs)
			{
				if(program->Source.empty())
					continue;

				outProgramType = program->Type;

				if(!bytecodeCompiler.IsProgramTypeSupported(program->Type))
					return VariationBytecodeState::UnsupportedStage;

				if(program->Bytecode == nullptr || program->Bytecode->Instructions.Size == 0)
					return VariationBytecodeState::MissingBytecode;
			}
		}

		return VariationBytecodeState::Complete;
	}
}

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

ShaderCooker::CookItemResult ShaderCooker::CookItem(const ShaderCookItem& item, const String& language, Package& package,
	u32& outVariationCount, u32& outSkippedVariationCount)
{
	// 1) Compile the shader meta-data for this language. This does not compile the variations (that happens below),
	//    it only produces the variation parameter sets and the compiler meta-data.
	const TShared<Shader> shader = ShaderCompilers::Instance().CompileShader<false>(item.Name, item.Source, item.Defines, { language });
	if(shader == nullptr)
	{
		B3D_LOG(Error, LogResources, "Shader cook failed for \"{0}\" (language \"{1}\"): compilation produced no shader.", item.Name, language);
		return CookItemResult::Failed;
	}

	const TShared<ShaderCompilerMetaData>& metaData = shader->GetCompilerMetaData();
	if(metaData == nullptr)
	{
		B3D_LOG(Error, LogResources, "Shader cook failed for \"{0}\" (language \"{1}\"): compiled shader has no compiler meta-data.", item.Name, language);
		return CookItemResult::Failed;
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
		return CookItemResult::Failed;
	}

	// The cook only runs for languages with a registered bytecode compiler (see the tool's language check), and every
	// shipped variation must carry bytecode - the store is the only shader source on platforms without a runtime compiler.
	const TShared<IGpuBytecodeCompiler> bytecodeCompiler = ShaderCompilers::Instance().GetBytecodeCompiler(language);
	if(bytecodeCompiler == nullptr)
	{
		B3D_LOG(Error, LogResources, "Shader cook failed for \"{0}\": no bytecode compiler is registered for language \"{1}\".", item.Name, language);
		return CookItemResult::Failed;
	}

	bool anyVariationFailed = false;

	// 2) Cook every variation. The shader's source must stay intact for these compiles, so the meta-data Source is only
	//    stripped afterwards in step 3.
	for(const ShaderVariationParameters& variationParameters : metaData->Variations)
	{
		const TShared<Variation> variation = Variation::Create(shader, language, variationParameters);

		const ShaderCompilerResult compileResult = bslCompiler->CompileVariation(*shader, variationParameters, language, *variation);
		if(!compileResult.ErrorMessage.empty())
		{
			B3D_LOG(Error, LogResources, "Shader cook failed for a variation of \"{0}\" (language \"{1}\"): {2}", item.Name, language, compileResult.ErrorMessage);
			anyVariationFailed = true;
			continue;
		}

		const TShared<PrecompiledVariationData> variationData = variation->GetPrecompiledData();
		if(variationData == nullptr)
		{
			B3D_LOG(Warning, LogResources, "Shader cook skipped a variation of \"{0}\" (language \"{1}\"): no precompiled data was produced.", item.Name, language);
			++outSkippedVariationCount;
			continue;
		}

		GpuProgramType offendingProgramType = GPT_VERTEX_PROGRAM;
		switch(DetermineVariationBytecodeState(*variationData, *bytecodeCompiler, offendingProgramType))
		{
		case VariationBytecodeState::UnsupportedStage:
			B3D_LOG(Warning, LogResources, "Shader cook skipped variation \"{0}\" of \"{1}\": it uses a {2} program, which the \"{3}\" bytecode compiler does not support. The variation will be unavailable on this platform.",
				variationParameters.CreateVariationName(), item.Name, GetProgramTypeName(offendingProgramType), language);
			++outSkippedVariationCount;
			continue;
		case VariationBytecodeState::MissingBytecode:
			B3D_LOG(Error, LogResources, "Shader cook failed for variation \"{0}\" of \"{1}\" (language \"{2}\"): its {3} program produced no bytecode.",
				variationParameters.CreateVariationName(), item.Name, language, GetProgramTypeName(offendingProgramType));
			anyVariationFailed = true;
			continue;
		case VariationBytecodeState::Complete:
			break;
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

	return anyVariationFailed ? CookItemResult::Failed : CookItemResult::Cooked;
}

bool ShaderCooker::Cook(const Vector<ShaderCookItem>& items, const CookOptions& options)
{
	const String packageName = options.OutputPath.GetFilename(false);
	const TShared<Package> package = Package::Create(packageName);

	u32 cookedShaderCount = 0;
	u32 failedShaderCount = 0;
	u32 cookedVariationCount = 0;
	u32 skippedVariationCount = 0;

	for(const ShaderCookItem& item : items)
	{
		switch(CookItem(item, options.Language, *package, cookedVariationCount, skippedVariationCount))
		{
		case CookItemResult::Cooked:
			++cookedShaderCount;
			break;
		case CookItemResult::Failed:
			++failedShaderCount;
			break;
		}
	}

	// Ensure the destination folder exists, then write the package. A brand-new package not registered with the
	// package manager can be saved as-is (see PackageManager::SavePackage).
	FileSystem::CreateFolder(options.OutputPath.GetParent());

	PackageManagerSavePackageOptions saveOptions;
	PackageManager::Instance().SavePackage(package, options.OutputPath, saveOptions);

	B3D_LOG(Info, LogResources, "Shader cook complete: wrote {0} shader(s) and {1} variation(s) to \"{2}\" ({3} variation(s) skipped).",
		cookedShaderCount, cookedVariationCount, options.OutputPath.ToString(), skippedVariationCount);

	if(failedShaderCount > 0)
	{
		B3D_LOG(Error, LogResources, "Shader cook FAILED: {0} shader(s) did not cook cleanly. See the errors above.", failedShaderCount);
		return false;
	}

	return true;
}
