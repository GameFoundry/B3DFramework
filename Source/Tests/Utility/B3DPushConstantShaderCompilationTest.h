//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Testing/B3DTestSuite.h"
#include "Material/B3DShaderCompiler.h"
#include "Material/B3DShader.h"
#include "Material/B3DVariation.h"
#include "Material/B3DPass.h"
#include "GpuBackend/B3DGpuProgram.h"
#include "GpuBackend/B3DGpuProgramParameterDescription.h"
#include "GpuBackend/B3DGpuPushConstants.h"
#include "String/B3DStringFormat.h"

namespace b3d
{
	/** Verifies compilation, size metadata and resource separation for every portable push-constant size. */
	inline void TestPushConstantShaderCompilation(TestSuite& suite, const String& targetLanguage)
	{
		const TShared<IShaderCompiler> compiler = ShaderCompilers::Instance().GetCompiler("bsl");
		B3D_TEST_ASSERT_EXTERNAL_MSG(suite, compiler != nullptr, "Host push-constant tests require the BSL compiler.")
		if(compiler == nullptr)
			return;

		for(u32 valueCount = 1; valueCount <= kMaxPushConstantValueCount; valueCount++)
		{
			String members;
			String valueSum;
			for(u32 valueIndex = 0; valueIndex < valueCount; valueIndex++)
			{
				members += "\t\t\tuint Value" + ToString(valueIndex) + ";\n";
				valueSum += (valueIndex == 0 ? "" : " + ") + String("Value") + ToString(valueIndex);
			}

			const String shaderName = "PushConstant" + ToString(valueCount * sizeof(u32)) + targetLanguage;
			const String source = "shader " + shaderName + "\n"
				"{\n"
				"\tcode\n"
				"\t{\n"
				"\t\t[pushConstant]\n"
				"\t\tcbuffer DrawConstants\n"
				"\t\t{\n" + members +
				"\t\t};\n"
				"\n"
				"\t\tfloat4 vsmain(uint vertexId : SV_VertexID) : SV_Position\n"
				"\t\t{\n"
				"\t\t\treturn float4((float)(" + valueSum + " + vertexId), 0.0, 0.0, 1.0);\n"
				"\t\t}\n"
				"\t};\n"
				"};\n";

			TShared<Shader> shader;
			const ShaderCompilerResult compileResult = compiler->Compile(shaderName, source, {}, { targetLanguage }, true, shader);
			B3D_TEST_ASSERT_EXTERNAL_MSG(suite, compileResult.ErrorMessage.empty(), shaderName + ": " + compileResult.ErrorMessage)
			B3D_TEST_ASSERT_EXTERNAL(suite, shader != nullptr)
			if(!compileResult.ErrorMessage.empty() || shader == nullptr)
				return;

			B3D_TEST_ASSERT_EXTERNAL(suite, shader->GetVariations().size() == 1)
			if(shader->GetVariations().size() != 1)
				return;

			const TShared<Variation>& variation = shader->GetVariations().front();
			B3D_TEST_ASSERT_EXTERNAL(suite, variation != nullptr)
			if(variation == nullptr)
				return;

			B3D_TEST_ASSERT_EXTERNAL(suite, variation->GetPassCount() == 1)
			if(variation->GetPassCount() != 1 || variation->GetPass(0) == nullptr)
			{
				B3D_TEST_ASSERT_EXTERNAL_MSG(suite, false, shaderName + ": missing compiled pass.")
				return;
			}

			const GpuProgramCreateInformation& program = variation->GetPass(0)->GetGpuProgramCreateInformation(GPT_VERTEX_PROGRAM);
			B3D_TEST_ASSERT_EXTERNAL(suite, program.GetEntryPointReflection().PushConstantBufferSize == valueCount * sizeof(u32))
			B3D_TEST_ASSERT_EXTERNAL(suite, program.Bytecode != nullptr)
			if(program.Bytecode == nullptr)
				return;

			B3D_TEST_ASSERT_EXTERNAL_MSG(suite, program.Bytecode->Instructions.Data != nullptr, shaderName + ": " + program.Bytecode->Messages)
			B3D_TEST_ASSERT_EXTERNAL(suite, program.Bytecode->ParameterDescription != nullptr)
			if(program.Bytecode->Instructions.Data == nullptr || program.Bytecode->ParameterDescription == nullptr)
				return;

			B3D_TEST_ASSERT_EXTERNAL(suite, program.Bytecode->ParameterDescription->PushConstantBufferSize == valueCount * sizeof(u32))
			B3D_TEST_ASSERT_EXTERNAL(suite, program.Bytecode->ParameterDescription->UniformBuffers.find("DrawConstants") == program.Bytecode->ParameterDescription->UniformBuffers.end())
			B3D_LOG(Info, LogGeneric, "Host push-constant compilation: {0}, {1} bytes.", targetLanguage, valueCount * sizeof(u32));
		}

		// The block is removed from a stage that never reads it. Only Metal also clears it from the shader reflection.
		const String unreadShaderName = "PushConstantUnread" + targetLanguage;
		const String unreadSource = "shader " + unreadShaderName + "\n"
			"{\n"
			"\tcode\n"
			"\t{\n"
			"\t\t[pushConstant]\n"
			"\t\tcbuffer DrawConstants\n"
			"\t\t{\n"
			"\t\t\tuint Value0;\n"
			"\t\t};\n"
			"\n"
			"\t\tfloat4 vsmain() : SV_Position\n"
			"\t\t{\n"
			"\t\t\treturn float4((float)Value0, 0.0, 0.0, 1.0);\n"
			"\t\t}\n"
			"\n"
			"\t\tfloat4 fsmain() : SV_Target0\n"
			"\t\t{\n"
			"\t\t\treturn float4(0.0, 0.0, 0.0, 1.0);\n"
			"\t\t}\n"
			"\t};\n"
			"};\n";

		TShared<Shader> unreadShader;
		const ShaderCompilerResult unreadResult = compiler->Compile(unreadShaderName, unreadSource, {}, { targetLanguage }, true, unreadShader);
		B3D_TEST_ASSERT_EXTERNAL_MSG(suite, unreadResult.ErrorMessage.empty(), unreadShaderName + ": " + unreadResult.ErrorMessage)
		if(!unreadResult.ErrorMessage.empty() || unreadShader == nullptr || unreadShader->GetVariations().empty() || unreadShader->GetVariations().front()->GetPassCount() == 0)
		{
			B3D_TEST_ASSERT_EXTERNAL_MSG(suite, false, unreadShaderName + ": missing compiled pass.")
			return;
		}

		const TShared<Pass>& unreadPass = unreadShader->GetVariations().front()->GetPass(0);
		const GpuProgramCreateInformation& readingProgram = unreadPass->GetGpuProgramCreateInformation(GPT_VERTEX_PROGRAM);
		const GpuProgramCreateInformation& unreadProgram = unreadPass->GetGpuProgramCreateInformation(GPT_FRAGMENT_PROGRAM);
		B3D_TEST_ASSERT_EXTERNAL(suite, unreadProgram.GetEntryPointReflection().PushConstantBufferSize == (targetLanguage == "msl" ? 0 : sizeof(u32)))
		if(readingProgram.Bytecode == nullptr || readingProgram.Bytecode->ParameterDescription == nullptr || unreadProgram.Bytecode == nullptr || unreadProgram.Bytecode->ParameterDescription == nullptr)
		{
			B3D_TEST_ASSERT_EXTERNAL_MSG(suite, false, unreadShaderName + ": missing reflection. " + (unreadProgram.Bytecode != nullptr ? unreadProgram.Bytecode->Messages : String()))
			return;
		}

		B3D_TEST_ASSERT_EXTERNAL_MSG(suite, unreadProgram.Bytecode->Instructions.Data != nullptr, unreadShaderName + ": " + unreadProgram.Bytecode->Messages)
		B3D_TEST_ASSERT_EXTERNAL(suite, readingProgram.Bytecode->ParameterDescription->PushConstantBufferSize == sizeof(u32))
		B3D_TEST_ASSERT_EXTERNAL(suite, unreadProgram.Bytecode->ParameterDescription->PushConstantBufferSize == 0)
	}
}
