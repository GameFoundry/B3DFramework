//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DBSLCompiler.h"
#include "RenderAPI/B3DGpuProgram.h"
#include <regex>
#include "Material/B3DShader.h"
#include "Material/B3DVariation.h"
#include "Material/B3DPass.h"
#include "Math/B3DMatrix4.h"
#include "Resources/B3DBuiltinResources.h"
#include "Material/B3DShaderVariation.h"

#include "B3DBSLParser.h"
#include "B3DHLSLCrossCompiler.h"
#include "RenderAPI/B3DGpuDevice.h"

using namespace std;
using namespace b3d;

static SPtr<Shader> CreateShader(const String& name, const ShaderCreateInformation& shaderCreateInformation, const Vector<String>& includes)
{
	SPtr<Shader> shader = Shader::CreateShared(name, shaderCreateInformation);
	shader->SetIncludeFiles(includes);

	const SPtr<ShaderCompilerMetaData> compilerMetaData = shaderCreateInformation.CompilerMetaData;
	if(compilerMetaData != nullptr)
	{
		for(const auto& includePath : includes)
			compilerMetaData->IncludeHashes[includePath] = Shader::ComputeIncludeHash(includePath);
	}

	return shader;
}

static SPtr<render::Shader> CreateShader(const String& name, const render::ShaderCreateInformation& shaderCreateInformation, const Vector<String>& includes)
{
	SPtr<render::Shader> shader = render::Shader::Create(name, shaderCreateInformation);

	const SPtr<ShaderCompilerMetaData> compilerMetaData = shaderCreateInformation.CompilerMetaData;
	if(compilerMetaData != nullptr)
	{
		for(const auto& includePath : includes)
			compilerMetaData->IncludeHashes[includePath] = Shader::ComputeIncludeHash(includePath);
	}

	return shader;
}

template<bool IsRenderProxy>
ShaderCompilerResult BSLCompiler::TCompile(const String& name, const String& source, const UnorderedMap<String, String>& defines, ShadingLanguageFlags languages, bool compileVariations, SPtr<CoreVariantType<Shader, IsRenderProxy>>& outShader)
{
	CoreVariantType<ShaderCreateInformation, IsRenderProxy> shaderCreateInformation;
	Vector<String> shaderIncludes;

	BSLParsedShaderMetaData parsedShaderMetaData;
	ShaderCompilerResult compileResult = BSLParser::ParseMetaData(source, defines, shaderCreateInformation, parsedShaderMetaData, shaderIncludes);

	if(!compileResult.ErrorMessage.empty())
		return compileResult;

	SPtr<ShaderCompilerMetaData> compilerMetaData = B3DMakeShared<ShaderCompilerMetaData>();
	compilerMetaData->Source = source;
	compilerMetaData->Variations = CreateShaderVariations(parsedShaderMetaData);
	compilerMetaData->Defines = defines;

	TInlineArray<ShadingLanguageFlag, (u32)ShadingLanguageFlag::Count> requiredLanguageSet;
	for(u32 shadingLanguageIndex = 0; shadingLanguageIndex < (u32)ShadingLanguageFlag::Count; shadingLanguageIndex++)
	{
		if(languages.IsSet((ShadingLanguageFlag)(1 << shadingLanguageIndex)))
			requiredLanguageSet.Add((ShadingLanguageFlag)(1 << shadingLanguageIndex));
	}

	SPtr<CoreVariantType<Shader, IsRenderProxy>> shader;
	for(auto& variationParameters : compilerMetaData->Variations)
	{
		for(u32 languageIndex = 0; languageIndex < requiredLanguageSet.size(); ++languageIndex)
		{
			const String languageName = ShaderCompilers::GetShadingLanguageName(requiredLanguageSet[languageIndex]);
			SPtr<CoreVariantType<Variation, IsRenderProxy>> variation = CoreVariantType<Variation, IsRenderProxy>::Create(shader, languageName, variationParameters);

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

		for(u32 languageIndex = 0; languageIndex < requiredLanguageSet.size(); ++languageIndex)
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

			const String languageName = ShaderCompilers::GetShadingLanguageName(requiredLanguageSet[languageIndex]);
			const SPtr<CoreVariantType<Variation, IsRenderProxy>>& variation = shaderCreateInformation.Variations[variationIndex];

			if(compileVariations)
			{
				compileResult = TCompileVariation<IsRenderProxy>(parsedShaderMetaData.Name, parsedNode, *compilerMetaData, requiredLanguageSet[languageIndex], *variation);

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

template ShaderCompilerResult BSLCompiler::TCompile<false>(const String&, const String&, const UnorderedMap<String, String>&, ShadingLanguageFlags, bool, SPtr<CoreVariantType<Shader, false>>&);
template ShaderCompilerResult BSLCompiler::TCompile<true>(const String&, const String&, const UnorderedMap<String, String>&, ShadingLanguageFlags, bool, SPtr<CoreVariantType<Shader, true>>&);

template<bool IsRenderProxy>
ShaderCompilerResult BSLCompiler::TCompileVariation(const CoreVariantType<Shader, IsRenderProxy>& shader, const ShaderVariationParameters& variationParameters, ShadingLanguageFlag language, CoreVariantType<Variation, IsRenderProxy>& outVariation)
{
	SPtr<ShaderCompilerMetaData> compilerMetaData = shader.GetCompilerMetaData();
	if(compilerMetaData == nullptr)
	{
		ShaderCompilerResult returnStatus;
		returnStatus.ErrorMessage = "Cannot compile technique as parent shader does not contain compilation meta-data.";

		return returnStatus;
	}

	BSLParsedShaderData parsedNode;
	ShaderCompilerResult compileResult = BSLParser::ParseVariation(shader.GetShaderName(), compilerMetaData->Source, variationParameters, compilerMetaData->Defines, parsedNode);

	if(!compileResult.ErrorMessage.empty())
		return compileResult;

	return TCompileVariation<IsRenderProxy>(shader.GetShaderName(), parsedNode, *compilerMetaData, language, outVariation);
}

template ShaderCompilerResult BSLCompiler::TCompileVariation<false>(const CoreVariantType<Shader, false>&, const ShaderVariationParameters&, ShadingLanguageFlag, CoreVariantType<Variation, false>& outVariation);
template ShaderCompilerResult BSLCompiler::TCompileVariation<true>(const CoreVariantType<Shader, true>&, const ShaderVariationParameters&, ShadingLanguageFlag, CoreVariantType<Variation, true>& outVariation);

template<bool IsRenderProxy>
ShaderCompilerResult BSLCompiler::TCompileVariation(const String& name, const BSLParsedShaderData& parsedShader, const ShaderCompilerMetaData& shaderMetaData, ShadingLanguageFlag language, CoreVariantType<Variation, IsRenderProxy>& outVariation)
{
	B3D_ASSERT(!parsedShader.MetaData.IsMixin);
	B3D_ASSERT(shaderMetaData.GPUProgramTypes.size() > 0);

	ShaderCompilerResult compileResult;

	HLSLCrossCompileOutput crossCompileOutputLanguage = HLSLCrossCompileOutput::VKSL45;
	const String crossCompileOutputLanguageName = ShaderCompilers::GetShadingLanguageName(language);
	if(language == ShadingLanguageFlag::GLSL)
	{
		crossCompileOutputLanguage = HLSLCrossCompileOutput::GLSL45;
	}
	else if(language == ShadingLanguageFlag::VKSL)
	{
		crossCompileOutputLanguage = HLSLCrossCompileOutput::VKSL45;
	}
	else if(language == ShadingLanguageFlag::MSL)
	{
		crossCompileOutputLanguage = HLSLCrossCompileOutput::MVKSL;
	}

	struct CrossCompilePassOutput
	{
		String ProgramCodePerType[GPT_COUNT];
	};

	using PassType = CoreVariantType<Pass, IsRenderProxy>;

	Map<u32, SPtr<CoreVariantType<Pass, IsRenderProxy>>, std::greater<u32>> passes;
	const auto passCount = (u32)parsedShader.Passes.size();
	for(u32 passIndex = 0; passIndex < passCount; passIndex++)
	{
		const BSLParsedShaderPassData& parsedShaderPass = parsedShader.Passes[passIndex];

		auto fnCrossCompilePass = [&shaderMetaData, &compileResult](const BSLParsedShaderPassData& parsedShaderPass, HLSLCrossCompileOutput language, CrossCompilePassOutput& crossCompiledOutput)
		{
			for(auto& type : shaderMetaData.GPUProgramTypes)
			{
				B3D_ASSERT((i32)type < GPT_COUNT);
				u32 binding = 0;
				compileResult = HLSLCrossCompiler::CrossCompile(parsedShaderPass.Code, type, language, binding, crossCompiledOutput.ProgramCodePerType[(i32)type]);

				if(!compileResult.ErrorMessage.empty())
					return;
			}
		};

		CrossCompilePassOutput crossCompilePassOutput;
		if(language == ShadingLanguageFlag::HLSL)
		{
			// Clean non-standard HLSL
			// Note: Ideally we add a full HLSL output module to XShaderCompiler, instead of using simple regex. This
			// way the syntax could be enhanced with more complex features, while still being able to output pure
			// HLSL.
			static const std::regex attrRegex(
				R"(\[\s*layout\s*\(.*\)\s*\]|\[\s*internal\s*\]|\[\s*color\s*\]|\[\s*alias\s*\(.*\)\s*\]|\[\s*spriteuv\s*\(.*\)\s*\])");
			String parsedCode = regex_replace(parsedShaderPass.Code, attrRegex, "");

			static const std::regex attr2Regex(
				R"(\[\s*hideInInspector\s*\]|\[\s*name\s*\(".*"\)\s*\]|\[\s*hdr\s*\])");
			parsedCode = regex_replace(parsedCode, attr2Regex, "");

			static const std::regex initializerRegex(
				R"((Texture2D|Texture3D)\s*(\S*)\s*=.*;)");
			parsedCode = regex_replace(parsedCode, initializerRegex, "$1 $2;");

			static const std::regex warpWithSyncRegex(
				R"(Warp(Group|Device|All)MemoryBarrierWithWarpSync)");
			parsedCode = regex_replace(parsedCode, warpWithSyncRegex, "$1MemoryBarrierWithGroupSync");

			static const std::regex warpNoSyncRegex(
				R"(Warp(Group|Device|All)MemoryBarrier)");
			parsedCode = regex_replace(parsedCode, warpNoSyncRegex, "$1MemoryBarrier");

			// Note: I'm just copying HLSL code as-is. This code will contain all entry points which could have
			// an effect on compile time. It would be ideal to remove dead code depending on program Type. This would
			// involve adding a HLSL code generator to XShaderCompiler.
			for(auto& type : shaderMetaData.GPUProgramTypes)
			{
				B3D_ASSERT((i32)type < GPT_COUNT);
				crossCompilePassOutput.ProgramCodePerType[(i32)type] = parsedCode;
			}
		}
		else // Need to cross compile to correct low-level language
		{
			fnCrossCompilePass(parsedShaderPass, crossCompileOutputLanguage, crossCompilePassOutput);

			if(!compileResult.ErrorMessage.empty())
				return compileResult;
		}

		PassCreateInformation shaderPassInformation;
		shaderPassInformation.BlendStateDesc = parsedShaderPass.BlendStateInformation;
		shaderPassInformation.RasterizerStateDesc = parsedShaderPass.RasterizerStateInformation;
		shaderPassInformation.DepthStencilStateDesc = parsedShaderPass.DepthStencilStateInformation;

		auto fnBuildGpuProgramCreateInformation = [&name](const String& language, const String& entry, const String& code, GpuProgramType type) -> GpuProgramCreateInformation
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
			gpuProgramCreateInformation.Name = StringUtil::Format("{0} ({1} Program)", name, typeString);
			gpuProgramCreateInformation.Language = language;
			gpuProgramCreateInformation.EntryPoint = entry;
			gpuProgramCreateInformation.Source = code;
			gpuProgramCreateInformation.Type = type;

			const SPtr<GpuDevice> gpuDevice = GetApplication().GetPrimaryGpuDevice();

			if(gpuDevice != nullptr)
				gpuProgramCreateInformation.Bytecode = gpuDevice->CompileGpuProgramBytecode(gpuProgramCreateInformation);

			return gpuProgramCreateInformation;
		};

		const bool isHLSL = language == ShadingLanguageFlag::HLSL;
		shaderPassInformation.VertexProgramDesc = fnBuildGpuProgramCreateInformation(
			crossCompileOutputLanguageName,
			isHLSL ? "vsmain" : "main",
			crossCompilePassOutput.ProgramCodePerType[GPT_VERTEX_PROGRAM],
			GPT_VERTEX_PROGRAM);

		shaderPassInformation.FragmentProgramDesc = fnBuildGpuProgramCreateInformation(
			crossCompileOutputLanguageName,
			isHLSL ? "fsmain" : "main",
			crossCompilePassOutput.ProgramCodePerType[GPT_FRAGMENT_PROGRAM],
			GPT_FRAGMENT_PROGRAM);

		shaderPassInformation.GeometryProgramDesc = fnBuildGpuProgramCreateInformation(
			crossCompileOutputLanguageName,
			isHLSL ? "gsmain" : "main",
			crossCompilePassOutput.ProgramCodePerType[GPT_GEOMETRY_PROGRAM],
			GPT_GEOMETRY_PROGRAM);

		shaderPassInformation.HullProgramDesc = fnBuildGpuProgramCreateInformation(
			crossCompileOutputLanguageName,
			isHLSL ? "hsmain" : "main",
			crossCompilePassOutput.ProgramCodePerType[GPT_HULL_PROGRAM],
			GPT_HULL_PROGRAM);

		shaderPassInformation.DomainProgramDesc = fnBuildGpuProgramCreateInformation(
			crossCompileOutputLanguageName,
			isHLSL ? "dsmain" : "main",
			crossCompilePassOutput.ProgramCodePerType[GPT_DOMAIN_PROGRAM],
			GPT_DOMAIN_PROGRAM);

		shaderPassInformation.ComputeProgramDesc = fnBuildGpuProgramCreateInformation(
			crossCompileOutputLanguageName,
			isHLSL ? "csmain" : "main",
			crossCompilePassOutput.ProgramCodePerType[GPT_COMPUTE_PROGRAM],
			GPT_COMPUTE_PROGRAM);

		shaderPassInformation.StencilRefValue = parsedShaderPass.StencilReferenceValue;

		const SPtr<PassType> pass = PassType::Create(shaderPassInformation);
		if(pass != nullptr)
		{
			passes[parsedShaderPass.SequentialIndex] = pass;
		}
	}

	TInlineArray<SPtr<PassType>, 1> orderedPasses;
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
