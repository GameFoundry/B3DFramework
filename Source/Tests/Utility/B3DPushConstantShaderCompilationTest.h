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
	}
}
