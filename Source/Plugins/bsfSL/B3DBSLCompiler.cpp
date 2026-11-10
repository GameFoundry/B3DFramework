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
	ShaderCompilerResult compileResult = BSLParser::ParseMetaData(source, defines, *shaderCreateInformation.Description, parsedShaderMetaData, shaderIncludes);

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
			// Only reflect first variation of each language
			if(variationIndex < languages.size())
			{
				const auto passCount = (u32)parsedNode.Passes.size();
				for(u32 passIndex = 0; passIndex < passCount; passIndex++)
				{
					const BSLParsedShaderPassData& parsedShaderPassNode = parsedNode.Passes[passIndex];

					// Find valid entry points and parameters
					ShaderReflection reflection;
					compileResult = HLSLCrossCompiler::Reflect(parsedShaderPassNode.Code, reflection, HLSLCrossCompiler::GetTarget(languages[languageIndex]));
					if(!compileResult.ErrorMessage.empty())
						return compileResult;

					if(languageIndex == 0 && passIndex == 0)
						shaderCreateInformation.Description->Parameters = std::move(reflection.Parameters);
					else if(languageIndex == 0)
					{
						const Result combined = shaderCreateInformation.Description->Parameters->TryCombine(*reflection.Parameters);
						if(!combined.IsSuccessful())
						{
							compileResult.ErrorMessage = combined.GetFullErrorMessage();
							return compileResult;
						}
					}

					String rayTracingEntryPoints;
					bool hasRasterComputeEntryPoint = false;
					for(const auto& entry : reflection.EntryPoints)
					{
						const u32 passSequentialIndex = parsedShaderPassNode.SequentialIndex;
						const auto found = std::find_if(compilerMetaData->EntryPoints.begin(), compilerMetaData->EntryPoints.end(),
							[passSequentialIndex, &entry](const ShaderEntryPoint& existing) { return existing.PassIndex == passSequentialIndex && existing.Name == entry.first; });

						if(found == compilerMetaData->EntryPoints.end())
						{
							ShaderEntryPoint entryPoint;
							entryPoint.PassIndex = passSequentialIndex;
							entryPoint.Type = entry.second.Type;
							entryPoint.Name = entry.first;
							compilerMetaData->EntryPoints.push_back(std::move(entryPoint));
						}

						if(!IsRayTracingProgramType(entry.second.Type))
							hasRasterComputeEntryPoint = true;
						else
						{
							if(!rayTracingEntryPoints.empty())
								rayTracingEntryPoints += ", ";

							rayTracingEntryPoints += entry.first;
						}
					}

					if(hasRasterComputeEntryPoint && !rayTracingEntryPoints.empty())
					{
						compileResult.ErrorMessage = StringUtility::Format("Pass {0} mixes ray tracing entry points ({1}) with vsmain/fsmain/gsmain/hsmain/dsmain/csmain.",
							parsedShaderPassNode.SequentialIndex, rayTracingEntryPoints);
						return compileResult;
					}
				}

				if(!wasShaderCreationAttempted)
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
	B3D_ASSERT(!shaderMetaData.EntryPoints.empty());

	ShaderCompilerResult compileResult;

	// The null backend needs no compilation and leaves every generated source empty: the null GPU device accepts empty
	// source and produces empty bytecode
	const bool crossCompiles = language != kGpuProgramLanguageNullsl;
	const HLSLCrossCompileTarget* crossCompileTarget = HLSLCrossCompiler::GetTarget(language);
	if(crossCompiles && crossCompileTarget == nullptr)
	{
		compileResult.ErrorMessage = StringUtility::Format("Unsupported shading language \"{0}\".", language);
		return compileResult;
	}

	const TShared<IGpuBytecodeCompiler> bytecodeCompiler = ShaderCompilers::Instance().GetBytecodeCompiler(language);

	using PassType = CoreVariantType<Pass, IsRenderProxy>;

	Map<u32, TShared<CoreVariantType<Pass, IsRenderProxy>>, std::greater<u32>> passes;
	const auto passCount = (u32)parsedShader.Passes.size();
	for(u32 passIndex = 0; passIndex < passCount; passIndex++)
	{
		const BSLParsedShaderPassData& parsedShaderPass = parsedShader.Passes[passIndex];

		PassCreateInformation shaderPassInformation;
		shaderPassInformation.BlendStateInformation = parsedShaderPass.BlendStateInformation;
		shaderPassInformation.RasterizerStateInformation = parsedShaderPass.RasterizerStateInformation;
		shaderPassInformation.DepthStencilStateInformation = parsedShaderPass.DepthStencilStateInformation;
		shaderPassInformation.StencilRefValue = parsedShaderPass.StencilReferenceValue;

		TShared<ShaderReflection> passReflection;
		TShared<ShaderReflection> sourceReflection;
		for(const ShaderEntryPoint& entryPoint : shaderMetaData.EntryPoints)
		{
			if(entryPoint.PassIndex != parsedShaderPass.SequentialIndex)
				continue;

			// A shader has to leave out the entry points its language can't compile. Entry points are gathered across languages
			// and variations, so only the ones this variation's source still has are an error.
			if(crossCompiles && bytecodeCompiler != nullptr && !bytecodeCompiler->IsProgramTypeSupported(entryPoint.Type))
			{
				if(sourceReflection == nullptr)
				{
					sourceReflection = B3DMakeShared<ShaderReflection>();
					compileResult = HLSLCrossCompiler::Reflect(parsedShaderPass.Code, *sourceReflection, crossCompileTarget);
					if(!compileResult.ErrorMessage.empty())
						return compileResult;
				}

				if(sourceReflection->EntryPoints.find(entryPoint.Name) != sourceReflection->EntryPoints.end())
				{
					compileResult.ErrorMessage = StringUtility::Format("Shading language \"{0}\" can't compile {1} programs, but the shader has the \"{2}\" entry point. Leave the entry point out for this language with a preprocessor condition.",
						language, GetProgramTypeName(entryPoint.Type), entryPoint.Name);
					return compileResult;
				}

				continue;
			}

			GpuProgramCreateInformation program;
			program.Name = StringUtility::Format("{0} ({1} Program {2})", name, GetProgramTypeName(entryPoint.Type), entryPoint.Name);
			program.Language = language;
			program.EntryPoint = entryPoint.Name;
			program.Type = entryPoint.Type;

			if(entryPoint.Type == GPT_FRAGMENT_PROGRAM)
				program.RenderTargetFormats = parsedShaderPass.RenderTargetFormats;

			if(crossCompiles)
			{
				u32 binding = 0;
				ShaderCrossCompileOutput output;
				compileResult = HLSLCrossCompiler::CrossCompile(parsedShaderPass.Code, entryPoint.Type, entryPoint.Name, *crossCompileTarget, binding, output, passReflection.get());
				if(!compileResult.ErrorMessage.empty())
					return compileResult;

				// The variation's defines can leave out an entry point of the first variation
				if(output.Source.empty())
					continue;

				if(passReflection == nullptr)
					passReflection = output.Reflection;

				program.Source = std::move(output.Source);
				program.ShaderReflection = std::move(output.Reflection);

				if(bytecodeCompiler != nullptr)
					program.Bytecode = bytecodeCompiler->CompileBytecode(program);
			}

			shaderPassInformation.Programs.push_back(std::move(program));
		}

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
