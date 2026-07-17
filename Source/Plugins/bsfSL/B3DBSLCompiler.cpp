//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DBSLCompiler.h"
#include "GpuBackend/B3DGpuProgram.h"
#include "Material/B3DShader.h"
#include "Material/B3DVariation.h"
#include "Material/B3DPass.h"
#include "Math/B3DMatrix4.h"
#include "Resources/B3DBuiltinResources.h"
#include "Material/B3DShaderVariation.h"

#include "B3DBSLParser.h"
#include "B3DHLSLCrossCompiler.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "Xsc/Backend.h"

using namespace std;
using namespace b3d;

BSLCompiler::BSLCompiler()
{
	// Load any optional output backends
	const String binariesPath = Paths::GetBinariesPath().ToString();
	std::string backendLoadError;
	Xsc::LoadBackendsInDirectory(std::string(binariesPath.c_str()), &backendLoadError);
	if(!backendLoadError.empty())
		B3D_LOG(Warning, LogBSLCompiler, "XShaderCompiler backend load reported: {0}", String(backendLoadError.c_str()));

	HLSLCrossCompiler::RegisterSupportedTargets();
}

BSLCompiler::~BSLCompiler()
{
	HLSLCrossCompiler::UnregisterSupportedTargets();
}

static TShared<Shader> CreateShader(const String& name, const ShaderCreateInformation& shaderCreateInformation, const Vector<String>& includes)
{
	TShared<Shader> shader = Shader::CreateShared(name, shaderCreateInformation);
	shader->SetIncludeFiles(includes);

	const TShared<ShaderCompilerMetaData> compilerMetaData = shaderCreateInformation.CompilerMetaData;
	if(compilerMetaData != nullptr)
	{
		for(const auto& includePath : includes)
			compilerMetaData->IncludeHashes[includePath] = Shader::ComputeIncludeHash(includePath);
	}

	return shader;
}

static TShared<render::Shader> CreateShader(const String& name, const render::ShaderCreateInformation& shaderCreateInformation, const Vector<String>& includes)
{
	TShared<render::Shader> shader = render::Shader::Create(name, shaderCreateInformation);

	const TShared<ShaderCompilerMetaData> compilerMetaData = shaderCreateInformation.CompilerMetaData;
	if(compilerMetaData != nullptr)
	{
		for(const auto& includePath : includes)
			compilerMetaData->IncludeHashes[includePath] = Shader::ComputeIncludeHash(includePath);
	}

	return shader;
}

template<bool IsRenderProxy>
ShaderCompilerResult BSLCompiler::TCompile(const String& name, const String& source, const UnorderedMap<String, String>& defines, const Vector<String>& languages, bool compileVariations, TShared<CoreVariantType<Shader, IsRenderProxy>>& outShader)
{
	CoreVariantType<ShaderCreateInformation, IsRenderProxy> shaderCreateInformation;
	Vector<String> shaderIncludes;

	BSLParsedShaderMetaData parsedShaderMetaData;
	ShaderCompilerResult compileResult = BSLParser::ParseMetaData(source, defines, shaderCreateInformation, parsedShaderMetaData, shaderIncludes);

	if(!compileResult.ErrorMessage.empty())
		return compileResult;

	TShared<ShaderCompilerMetaData> compilerMetaData = B3DMakeShared<ShaderCompilerMetaData>();
	compilerMetaData->Source = source;
	compilerMetaData->CompilerVersion = kCompilerVersion;
	compilerMetaData->Variations = CreateShaderVariations(parsedShaderMetaData);
	compilerMetaData->Defines = defines;

	TShared<CoreVariantType<Shader, IsRenderProxy>> shader;
	for(auto& variationParameters : compilerMetaData->Variations)
	{
		for(u32 languageIndex = 0; languageIndex < languages.size(); ++languageIndex)
		{
			const String& languageName = languages[languageIndex];
			TShared<CoreVariantType<Variation, IsRenderProxy>> variation = CoreVariantType<Variation, IsRenderProxy>::Create(shader, languageName, variationParameters);

			shaderCreateInformation.Variations.push_back(std::move(variation));
		}
	}

	if(!compileVariations)
	{
		shaderCreateInformation.CompilerMetaData = compilerMetaData;
	}

	// For every variation, re-parse the file with relevant defines
	u32 variationIndex = 0;
	bool wasShaderCreationAttempted = false;
	for(auto& variationParameters : compilerMetaData->Variations)
	{
		BSLParsedShaderData parsedNode;

		if(compileVariations || !wasShaderCreationAttempted)
		{
			compileResult = BSLParser::ParseVariation(parsedShaderMetaData.Name, source, variationParameters, defines, parsedNode);

			if(!compileResult.ErrorMessage.empty())
				return compileResult;
		}

		for(u32 languageIndex = 0; languageIndex < languages.size(); ++languageIndex)
		{
			if(!wasShaderCreationAttempted)
			{
				const auto passCount = (u32)parsedNode.Passes.size();
				for(u32 passIndex = 0; passIndex < passCount; passIndex++)
				{
					const BSLParsedShaderPassData& parsedShaderPassNode = parsedNode.Passes[passIndex];

					// Find valid entry points and parameters
					// Note: Ideally we don't need to do a full reflection pass for each GPU program type (i.e. by adding some kind of AST caching to XShaderCompiler)
					compileResult = HLSLCrossCompiler::Reflect(parsedShaderPassNode.Code, shaderCreateInformation, compilerMetaData->GPUProgramTypes);
				}

				if(compileResult.ErrorMessage.empty())
					shader = CreateShader(parsedShaderMetaData.Name, shaderCreateInformation, shaderIncludes);

				wasShaderCreationAttempted = true;
			}

			const TShared<CoreVariantType<Variation, IsRenderProxy>>& variation = shaderCreateInformation.Variations[variationIndex];

			if(compileVariations)
			{
				compileResult = TCompileVariation<IsRenderProxy>(parsedShaderMetaData.Name, parsedNode, *compilerMetaData, languages[languageIndex], *variation);

				if(!compileResult.ErrorMessage.empty())
					return compileResult;
			}

			variation->SetOwner(shader);
			variationIndex++;
		}
	}

	if(compileResult.ErrorMessage.empty())
	{
		outShader = shader;
	}

	return compileResult;
}

template ShaderCompilerResult BSLCompiler::TCompile<false>(const String&, const String&, const UnorderedMap<String, String>&, const Vector<String>&, bool, TShared<CoreVariantType<Shader, false>>&);
template ShaderCompilerResult BSLCompiler::TCompile<true>(const String&, const String&, const UnorderedMap<String, String>&, const Vector<String>&, bool, TShared<CoreVariantType<Shader, true>>&);

template<bool IsRenderProxy>
ShaderCompilerResult BSLCompiler::TCompileVariation(const CoreVariantType<Shader, IsRenderProxy>& shader, const ShaderVariationParameters& variationParameters, const String& language, CoreVariantType<Variation, IsRenderProxy>& outVariation)
{
	TShared<ShaderCompilerMetaData> compilerMetaData = shader.GetCompilerMetaData();
	if(compilerMetaData == nullptr)
	{
		ShaderCompilerResult returnStatus;
		returnStatus.ErrorMessage = "Cannot compile variation as parent shader does not contain compilation meta-data.";

		return returnStatus;
	}

	BSLParsedShaderData parsedNode;
	ShaderCompilerResult compileResult = BSLParser::ParseVariation(shader.GetShaderName(), compilerMetaData->Source, variationParameters, compilerMetaData->Defines, parsedNode);

	if(!compileResult.ErrorMessage.empty())
		return compileResult;

	return TCompileVariation<IsRenderProxy>(shader.GetShaderName(), parsedNode, *compilerMetaData, language, outVariation);
}

template ShaderCompilerResult BSLCompiler::TCompileVariation<false>(const CoreVariantType<Shader, false>&, const ShaderVariationParameters&, const String&, CoreVariantType<Variation, false>& outVariation);
template ShaderCompilerResult BSLCompiler::TCompileVariation<true>(const CoreVariantType<Shader, true>&, const ShaderVariationParameters&, const String&, CoreVariantType<Variation, true>& outVariation);

template<bool IsRenderProxy>
ShaderCompilerResult BSLCompiler::TCompileVariation(const String& name, const BSLParsedShaderData& parsedShader, const ShaderCompilerMetaData& shaderMetaData, const String& language, CoreVariantType<Variation, IsRenderProxy>& outVariation)
{
	B3D_ASSERT(!parsedShader.MetaData.IsMixin);
	B3D_ASSERT(shaderMetaData.GPUProgramTypes.size() > 0);

	ShaderCompilerResult compileResult;

	const HLSLCrossCompileTarget* crossCompileTarget = HLSLCrossCompiler::GetTarget(language);
	const String& crossCompileOutputLanguageName = language;

	struct CrossCompilePassOutput
	{
		CrossCompilePassOutput()
		{
			for(Array<u32, 3>& threadGroupSize : ThreadGroupSizePerType)
				threadGroupSize = { 1, 1, 1 };
		}

		Array<String, GPT_COUNT> ProgramCodePerType;
		Array<Array<u32, 3>, GPT_COUNT> ThreadGroupSizePerType;
	};

	using PassType = CoreVariantType<Pass, IsRenderProxy>;

	Map<u32, TShared<CoreVariantType<Pass, IsRenderProxy>>, std::greater<u32>> passes;
	const auto passCount = (u32)parsedShader.Passes.size();
	for(u32 passIndex = 0; passIndex < passCount; passIndex++)
	{
		const BSLParsedShaderPassData& parsedShaderPass = parsedShader.Passes[passIndex];

		auto fnCrossCompilePass = [&shaderMetaData, &compileResult](const BSLParsedShaderPassData& parsedShaderPass, const HLSLCrossCompileTarget& target, CrossCompilePassOutput& crossCompiledOutput)
		{
			for(auto& type : shaderMetaData.GPUProgramTypes)
			{
				B3D_ASSERT((i32)type < GPT_COUNT);
				u32 binding = 0;
				compileResult = HLSLCrossCompiler::CrossCompile(parsedShaderPass.Code, type, target, binding,
					crossCompiledOutput.ProgramCodePerType[(i32)type],
					crossCompiledOutput.ThreadGroupSizePerType[(i32)type]);

				if(!compileResult.ErrorMessage.empty())
					return;
			}
		};

		CrossCompilePassOutput crossCompilePassOutput;
		if(language == kGpuProgramLanguageNullsl)
		{
			// Null backend: no compilation needed, leave all ProgramCodePerType entries as empty strings.
			// The null GPU device accepts empty source and produces empty bytecode.
		}
		else // Need to cross compile to correct low-level language
		{
			if(crossCompileTarget == nullptr)
			{
				compileResult.ErrorMessage = StringUtility::Format("Unsupported shading language \"{0}\".", language);
				return compileResult;
			}

			fnCrossCompilePass(parsedShaderPass, *crossCompileTarget, crossCompilePassOutput);

			if(!compileResult.ErrorMessage.empty())
				return compileResult;
		}

		PassCreateInformation shaderPassInformation;
		shaderPassInformation.BlendStateInformation = parsedShaderPass.BlendStateInformation;
		shaderPassInformation.RasterizerStateInformation = parsedShaderPass.RasterizerStateInformation;
		shaderPassInformation.DepthStencilStateInformation = parsedShaderPass.DepthStencilStateInformation;

		auto fnBuildGpuProgramCreateInformation = [&name](const String& language, const String& entry, const String& code, GpuProgramType type, const Array<u32, 3>& threadGroupSize) -> GpuProgramCreateInformation
		{
			const char* typeString;
			switch(type)
			{
			case GPT_VERTEX_PROGRAM:
				typeString = "Vertex";
				break;
			case GPT_FRAGMENT_PROGRAM:
				typeString = "Fragment";
				break;
			case GPT_GEOMETRY_PROGRAM:
				typeString = "Geometry";
				break;
			case GPT_DOMAIN_PROGRAM:
				typeString = "Domain";
				break;
			case GPT_HULL_PROGRAM:
				typeString = "Hull";
				break;
			case GPT_COMPUTE_PROGRAM:
				typeString = "Compute";
				break;
			default:
				typeString = "Unknown";
				break;
			}

			GpuProgramCreateInformation gpuProgramCreateInformation;
			gpuProgramCreateInformation.Name = StringUtility::Format("{0} ({1} Program)", name, typeString);
			gpuProgramCreateInformation.Language = language;
			gpuProgramCreateInformation.EntryPoint = entry;
			gpuProgramCreateInformation.Source = code;
			gpuProgramCreateInformation.Type = type;
			gpuProgramCreateInformation.ThreadGroupSize = threadGroupSize;

			// Bake bytecode for the target language if a compiler is registered for it. Program types the compiler cannot
			// handle (for example geometry programs for some platforms) are left bytecode-less; consumers that require bytecode (the
			// offline shader cook) detect and report these, while runtime backends fall back to compiling from source.
			if(const TShared<IGpuBytecodeCompiler> bytecodeCompiler = ShaderCompilers::Instance().GetBytecodeCompiler(gpuProgramCreateInformation.Language))
			{
				if(bytecodeCompiler->IsProgramTypeSupported(type))
					gpuProgramCreateInformation.Bytecode = bytecodeCompiler->CompileBytecode(gpuProgramCreateInformation);
			}

			return gpuProgramCreateInformation;
		};

		// HLSL-family cross-compile outputs keep their per-stage entry names intact; GLSL-family outputs have
		// their entry function renamed to "main" by the cross compiler.
		const bool usesStageEntryNames = crossCompileTarget != nullptr && crossCompileTarget->KeepsEntryPointNames;

		shaderPassInformation.VertexProgramCreateInformation = fnBuildGpuProgramCreateInformation(
			crossCompileOutputLanguageName,
			usesStageEntryNames ? "vsmain" : "main",
			crossCompilePassOutput.ProgramCodePerType[GPT_VERTEX_PROGRAM],
			GPT_VERTEX_PROGRAM,
			crossCompilePassOutput.ThreadGroupSizePerType[GPT_VERTEX_PROGRAM]);

		shaderPassInformation.FragmentProgramCreateInformation = fnBuildGpuProgramCreateInformation(
			crossCompileOutputLanguageName,
			usesStageEntryNames ? "fsmain" : "main",
			crossCompilePassOutput.ProgramCodePerType[GPT_FRAGMENT_PROGRAM],
			GPT_FRAGMENT_PROGRAM,
			crossCompilePassOutput.ThreadGroupSizePerType[GPT_FRAGMENT_PROGRAM]);

		shaderPassInformation.GeometryProgramCreateInformation = fnBuildGpuProgramCreateInformation(
			crossCompileOutputLanguageName,
			usesStageEntryNames ? "gsmain" : "main",
			crossCompilePassOutput.ProgramCodePerType[GPT_GEOMETRY_PROGRAM],
			GPT_GEOMETRY_PROGRAM,
			crossCompilePassOutput.ThreadGroupSizePerType[GPT_GEOMETRY_PROGRAM]);

		shaderPassInformation.HullProgramCreateInformation = fnBuildGpuProgramCreateInformation(
			crossCompileOutputLanguageName,
			usesStageEntryNames ? "hsmain" : "main",
			crossCompilePassOutput.ProgramCodePerType[GPT_HULL_PROGRAM],
			GPT_HULL_PROGRAM,
			crossCompilePassOutput.ThreadGroupSizePerType[GPT_HULL_PROGRAM]);

		shaderPassInformation.DomainProgramCreateInformation = fnBuildGpuProgramCreateInformation(
			crossCompileOutputLanguageName,
			usesStageEntryNames ? "dsmain" : "main",
			crossCompilePassOutput.ProgramCodePerType[GPT_DOMAIN_PROGRAM],
			GPT_DOMAIN_PROGRAM, crossCompilePassOutput.ThreadGroupSizePerType[GPT_DOMAIN_PROGRAM]);

		shaderPassInformation.ComputeProgramCreateInformation = fnBuildGpuProgramCreateInformation(
			crossCompileOutputLanguageName,
			usesStageEntryNames ? "csmain" : "main",
			crossCompilePassOutput.ProgramCodePerType[GPT_COMPUTE_PROGRAM],
			GPT_COMPUTE_PROGRAM, crossCompilePassOutput.ThreadGroupSizePerType[GPT_COMPUTE_PROGRAM]);

		shaderPassInformation.StencilRefValue = parsedShaderPass.StencilReferenceValue;

		const TShared<PassType> pass = PassType::Create(shaderPassInformation);
		if(pass != nullptr)
		{
			passes[parsedShaderPass.SequentialIndex] = pass;
		}
	}

	TInlineArray<TShared<PassType>, 1> orderedPasses;
	for(auto& KVP : passes)
		orderedPasses.Add(KVP.second);

	outVariation.SetCompiledPassData(orderedPasses);

	return compileResult;
}

Vector<ShaderVariationParameters> BSLCompiler::CreateShaderVariations(const BSLParsedShaderMetaData& shaderMetaData)
{
	if(shaderMetaData.Variations.empty())
		return { ShaderVariationParameters() };

	Vector<ShaderVariationParameters> variations;

	FrameAllocatorScope frameScope;
	FrameVector<const BSLParsedVariationData*> variationsToProcess;
	for(u32 variationIndex = 0; variationIndex < (u32)shaderMetaData.Variations.size(); variationIndex++)
		variationsToProcess.push_back(&shaderMetaData.Variations[variationIndex]);

	while(!variationsToProcess.empty())
	{
		const BSLParsedVariationData* currentVariation = variationsToProcess.back();
		variationsToProcess.erase(variationsToProcess.end() - 1);

		// Variation parameter that's either defined or isn't
		if(currentVariation->Values.empty())
		{
			// This is the first variation parameter, register new variations
			if(variations.empty())
			{
				ShaderVariationParameters a;
				ShaderVariationParameters b;

				b.AddParameter(ShaderVariationParameter(currentVariation->Identifier, 1));

				variations.push_back(a);
				variations.push_back(b);
			}
			else // Duplicate existing variations, and add the parameter
			{
				const u32 variationCount = (u32)variations.size();
				for(u32 variationIndex = 0; variationIndex < variationCount; variationIndex++)
				{
					// Make a copy
					variations.push_back(variations[variationIndex]);

					// Add the parameter to existing variation
					variations[variationIndex].AddParameter(ShaderVariationParameter(currentVariation->Identifier, 1));
				}
			}
		}
		else // Variation parameter with multiple values
		{
			// This is the first variation parameter, register new variations
			if(variations.empty())
			{
				for(u32 variationValueIndex = 0; variationValueIndex < (u32)currentVariation->Values.size(); variationValueIndex++)
				{
					ShaderVariationParameters variation;
					variation.AddParameter(ShaderVariationParameter(currentVariation->Identifier, currentVariation->Values[variationValueIndex].Value));

					variations.push_back(variation);
				}
			}
			else // Duplicate existing variations, and add the parameter
			{
				const u32 variationCount = (u32)variations.size();
				for(u32 variationIndex = 0; variationIndex < variationCount; variationIndex++)
				{
					for(u32 variationValueIndex = 1; variationValueIndex < (u32)currentVariation->Values.size(); variationValueIndex++)
					{
						ShaderVariationParameters copy = variations[variationIndex];
						copy.AddParameter(ShaderVariationParameter(currentVariation->Identifier, currentVariation->Values[variationValueIndex].Value));

						variations.push_back(copy);
					}

					variations[variationIndex].AddParameter(ShaderVariationParameter(currentVariation->Identifier, currentVariation->Values[0].Value));
				}
			}
		}
	}

	return variations;
}
