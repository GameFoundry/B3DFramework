//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsBSLCompiler.h"
#include "RenderAPI/BsGpuProgram.h"
#include <regex>
#include "Material/BsShader.h"
#include "Material/BsTechnique.h"
#include "Material/BsPass.h"
#include "RenderAPI/BsRenderAPI.h"
#include "Math/BsMatrix4.h"
#include "Resources/BsBuiltinResources.h"
#include "Material/BsShaderVariation.h"

#include "BsBSLParser.h"
#include "BsHLSLCrossCompiler.h"

using namespace std;
using namespace bs;

BSLResult BSLCompiler::Compile(const String& name, const String& source, const UnorderedMap<String, String>& defines, ShadingLanguageFlags languages, SPtr<Shader>& outShader)
{
	SPtr<BSLFXShaderMetaData> shaderMetaData;
	BSLResult compileStatus = CompileMetaData(source, defines, shaderMetaData);
	if(!compileStatus.ErrorMessage.empty())
		return compileStatus;

	B3D_ASSERT(shaderMetaData != nullptr);

		BSLResult compileResult;

	SmallVector<ShadingLanguageFlag, (u32)ShadingLanguageFlag::Count> requiredLanguageSet;
	for(u32 shadingLanguageIndex = 0; shadingLanguageIndex < (u32)ShadingLanguageFlag::Count; shadingLanguageIndex++)
	{
		if(languages.IsSet((ShadingLanguageFlag)(1 << shadingLanguageIndex)))
			requiredLanguageSet.Add((ShadingLanguageFlag)(1 << shadingLanguageIndex));
	}

	// For every variation, re-parse the file with relevant defines
	for(auto& variation : shaderMetaData->Variations)
	{
		BSLParsedShaderData parsedNode;
		compileResult = BSLParser::ParseVariation(shaderMetaData->Name, source, variation, defines, parsedNode);

		if(!compileResult.ErrorMessage.empty())
			return compileResult;

		for(u32 languageIndex = 0; languageIndex < requiredLanguageSet.size(); ++languageIndex)
		{
			const auto passCount = (u32)parsedNode.Passes.size();
			for(u32 passIndex = 0; passIndex < passCount; passIndex++)
			{
				const BSLParsedShaderPassData& parsedShaderPassNode = parsedNode.Passes[passIndex];

				// Find valid entry points and parameters
				// Note: Ideally we don't need to do a full reflection pass for each GPU program type (i.e. by adding some kind of AST caching to XShaderCompiler)
				compileStatus = HLSLCrossCompiler::Reflect(parsedShaderPassNode.Code, shaderMetaData->ShaderInformation, shaderMetaData->GPUProgramTypes);
			}

			SPtr<Technique> compiledVariation;
			compileStatus = CompileVariation(shaderMetaData->Name, parsedNode, *shaderMetaData, variation, requiredLanguageSet[languageIndex], compiledVariation);

			if(!compileStatus.ErrorMessage.empty())
				return compileStatus;

			shaderMetaData->ShaderInformation.Techniques.push_back(compiledVariation);
		}
	}

	// Verify techniques compile correctly
	bool hasError = false;
	StringStream gpuProgError;
	for(auto& technique : shaderMetaData->ShaderInformation.Techniques)
	{
		if(!technique->IsSupported())
			continue;

		u32 numPasses = technique->GetNumPasses();
		technique->Compile();

		for(u32 i = 0; i < numPasses; i++)
		{
			SPtr<Pass> pass = technique->GetPass(i);

			auto checkCompileStatus = [&](const String& prefix, const SPtr<GpuProgram>& prog)
			{
				if(prog != nullptr)
				{
					prog->BlockUntilCoreInitialized();

					if(!prog->IsCompiled())
					{
						hasError = true;
						gpuProgError << prefix << ": " << prog->GetCompileErrorMessage() << std::endl;
					}
				}
			};

			const SPtr<GraphicsPipelineState>& graphicsPipeline = pass->GetGraphicsPipelineState();
			if(graphicsPipeline)
			{
				checkCompileStatus("Vertex program", graphicsPipeline->GetVertexProgram());
				checkCompileStatus("Fragment program", graphicsPipeline->GetFragmentProgram());
				checkCompileStatus("Geometry program", graphicsPipeline->GetGeometryProgram());
				checkCompileStatus("Hull program", graphicsPipeline->GetHullProgram());
				checkCompileStatus("Domain program", graphicsPipeline->GetDomainProgram());
			}

			const SPtr<ComputePipelineState>& computePipeline = pass->GetComputePipelineState();
			if(computePipeline)
				checkCompileStatus("Compute program", computePipeline->GetProgram());
		}
	}

	if(hasError)
	{
		compileResult.ErrorMessage = "Failed compiling GPU program(s): " + gpuProgError.str();
		compileResult.ErrorLine = 0;
		compileResult.ErrorColumn = 0;
	}

	if(compileStatus.ErrorMessage.empty())
	{
		outShader = Shader::CreateShared(name, shaderMetaData->ShaderInformation);
		outShader->SetIncludeFiles(shaderMetaData->Includes);
	}

	return compileResult;
}

BSLResult BSLCompiler::CompileMetaData(const String& source, const UnorderedMap<String, String>& defines, SPtr<BSLFXShaderMetaData>& outShaderMetaData)
{
	SPtr<BSLFXShaderMetaData> shaderMetaData = B3DMakeShared<BSLFXShaderMetaData>();

	BSLParsedShaderMetaData parsedShaderMetaData;
	BSLResult compileResult = BSLParser::ParseMetaData(source, defines, shaderMetaData->ShaderInformation, parsedShaderMetaData, shaderMetaData->Includes);

	if(!compileResult.ErrorMessage.empty())
		return compileResult;

	shaderMetaData->Variations = CreateShaderVariations(parsedShaderMetaData);
	shaderMetaData->Name = parsedShaderMetaData.Name;
	shaderMetaData->Defines = defines;

	outShaderMetaData = shaderMetaData;
	return compileResult;
}

BSLResult BSLCompiler::CompileVariation(const String& source, const ShaderVariation& variation, ShadingLanguageFlag language, BSLFXShaderMetaData& inOutShaderMetaData, SPtr<Technique>& outVariation)
{
	BSLParsedShaderData parsedNode;
	BSLResult compileResult = BSLParser::ParseVariation(inOutShaderMetaData.Name, source, variation, inOutShaderMetaData.Defines, parsedNode);

	if(!compileResult.ErrorMessage.empty())
		return compileResult;

	if(!inOutShaderMetaData.HasGPUProgramMetaData)
	{
		const auto passCount = (u32)parsedNode.Passes.size();
		for(u32 passIndex = 0; passIndex < passCount; passIndex++)
		{
			const BSLParsedShaderPassData& parsedShaderPassNode = parsedNode.Passes[passIndex];

			// Find valid entry points and parameters
			// Note: Ideally we don't need to do a full reflection pass for each GPU program type (i.e. by adding some kind of AST caching to XShaderCompiler)
			compileResult = HLSLCrossCompiler::Reflect(parsedShaderPassNode.Code, inOutShaderMetaData.ShaderInformation, inOutShaderMetaData.GPUProgramTypes);
		}

		inOutShaderMetaData.HasGPUProgramMetaData = true;
	}

	return CompileVariation(inOutShaderMetaData.Name, parsedNode, inOutShaderMetaData, variation, language, outVariation);
}

BSLResult BSLCompiler::CompileVariation(const String& name, const BSLParsedShaderData& parsedShader, const BSLFXShaderMetaData& shaderMetaData, const ShaderVariation& variation, ShadingLanguageFlag language, SPtr<Technique>& outVariation)
{
	B3D_ASSERT(!parsedShader.MetaData.IsMixin);
	B3D_ASSERT(shaderMetaData.GPUProgramTypes.size() > 0);

	BSLResult compileResult;

	HLSLCrossCompileOutput crossCompileOutputLanguage = HLSLCrossCompileOutput::VKSL45;
	String crossCompileOutputLanguageName;
	if(language == ShadingLanguageFlag::GLSL)
	{
		crossCompileOutputLanguage = HLSLCrossCompileOutput::GLSL45;
		crossCompileOutputLanguageName = "glsl";
	}
	else if(language == ShadingLanguageFlag::VKSL)
	{
		crossCompileOutputLanguage = HLSLCrossCompileOutput::VKSL45;
		crossCompileOutputLanguageName = "vksl";
	}
	else if(language == ShadingLanguageFlag::MSL)
	{
		crossCompileOutputLanguage = HLSLCrossCompileOutput::MVKSL;
		crossCompileOutputLanguageName = "mvksl";
	}
	else if(language == ShadingLanguageFlag::HLSL)
	{
		// No cross compile needed
		crossCompileOutputLanguageName = "hlsl";
	}

	struct CrossCompilePassOutput
	{
		String ProgramCodePerType[GPT_COUNT];
	};

	Map<u32, SPtr<Pass>, std::greater<u32>> passes;
	const auto passCount = (u32)parsedShader.Passes.size();
	for(u32 passIndex = 0; passIndex < passCount; passIndex++)
	{
		const BSLParsedShaderPassData& parsedShaderPass = parsedShader.Passes[passIndex];

		auto fnCrossCompilePass = [&shaderMetaData, &compileResult](const BSLParsedShaderPassData& parsedShaderPass, HLSLCrossCompileOutput language, CrossCompilePassOutput& crossCompiledOutput)
		{
			u32 binding = 0;
			for(auto& type : shaderMetaData.GPUProgramTypes)
			{
				B3D_ASSERT((i32)type < GPT_COUNT);
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

		PASS_DESC shaderPassInformation;
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

		const SPtr<Pass> pass = Pass::Create(shaderPassInformation);
		if(pass != nullptr)
		{
			passes[parsedShaderPass.SequentialIndex] = pass;
		}
	}

	Vector<SPtr<Pass>> orderedPasses;
	for(auto& KVP : passes)
		orderedPasses.push_back(KVP.second);

	if(!orderedPasses.empty())
		outVariation = Technique::Create(crossCompileOutputLanguageName, parsedShader.MetaData.Tags, variation, orderedPasses);

	return compileResult;
}

Vector<ShaderVariation> BSLCompiler::CreateShaderVariations(const BSLParsedShaderMetaData& shaderMetaData)
{
	if(shaderMetaData.Variations.empty())
		return { ShaderVariation() };

	Vector<ShaderVariation> variations;

	FrameScope frameScope;
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
				ShaderVariation a;
				ShaderVariation b;

				b.AddParam(ShaderVariation::Param(currentVariation->Identifier, 1));

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
					variations[variationIndex].AddParam(ShaderVariation::Param(currentVariation->Identifier, 1));
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
					ShaderVariation variation;
					variation.AddParam(ShaderVariation::Param(currentVariation->Identifier, currentVariation->Values[variationValueIndex].Value));

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
						ShaderVariation copy = variations[variationIndex];
						copy.AddParam(ShaderVariation::Param(currentVariation->Identifier, currentVariation->Values[variationValueIndex].Value));

						variations.push_back(copy);
					}

					variations[variationIndex].AddParam(ShaderVariation::Param(currentVariation->Identifier, currentVariation->Values[0].Value));
				}
			}
		}
	}

	return variations;
}
