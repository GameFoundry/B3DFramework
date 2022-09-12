//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GLSL/BsGLSLProgramPipelineManager.h"
#include "GLSL/BsGLSLGpuProgram.h"
#include "Profiling/BsRenderStats.h"

namespace bs { namespace ct
{
	::std::size_t GLSLProgramPipelineManager::ProgramPipelineKeyHashFunction::operator()
		(const GLSLProgramPipelineManager::ProgramPipelineKey &key) const
	{
		std::size_t seed = 0;
		bs_hash_combine(seed, key.vertexProgKey);
		bs_hash_combine(seed, key.fragmentProgKey);
		bs_hash_combine(seed, key.geometryProgKey);
		bs_hash_combine(seed, key.hullProgKey);
		bs_hash_combine(seed, key.domainProgKey);

		return seed;
	}

	bool GLSLProgramPipelineManager::ProgramPipelineKeyEqual::operator()
		(const GLSLProgramPipelineManager::ProgramPipelineKey &a, const GLSLProgramPipelineManager::ProgramPipelineKey &b) const
	{
		return a.vertexProgKey == b.vertexProgKey && a.fragmentProgKey == b.fragmentProgKey && a.geometryProgKey == b.geometryProgKey &&
			a.hullProgKey == b.hullProgKey && a.domainProgKey == b.domainProgKey;
	}

	GLSLProgramPipelineManager::~GLSLProgramPipelineManager()
	{
		for (auto& pipeline : mPipelines)
		{
			glDeleteProgramPipelines(1, &pipeline.second.glHandle);
			BS_CHECK_GL_ERROR();

			BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_PipelineObject);
		}
	}

	const GLSLProgramPipeline* GLSLProgramPipelineManager::getPipeline(GLSLGpuProgram* vertexProgram, GLSLGpuProgram* fragmentProgram,
		GLSLGpuProgram* geometryProgram, GLSLGpuProgram* hullProgram, GLSLGpuProgram* domainProgram)
	{
		ProgramPipelineKey key;
		key.vertexProgKey = vertexProgram != nullptr ? vertexProgram->GetProgramID() : 0;
		key.fragmentProgKey = fragmentProgram != nullptr ? fragmentProgram->GetProgramID() : 0;
		key.geometryProgKey = geometryProgram != nullptr ? geometryProgram->GetProgramID() : 0;
		key.hullProgKey = hullProgram != nullptr ? hullProgram->GetProgramID() : 0;
		key.domainProgKey = domainProgram != nullptr ? domainProgram->GetProgramID() : 0;

		auto iterFind = mPipelines.Find(key);

		if(iterFind == mPipelines.End())
		{
			GLSLProgramPipeline newPipeline;

			glGenProgramPipelines(1, &newPipeline.glHandle);
			BS_CHECK_GL_ERROR();

			if(vertexProgram != nullptr)
			{
				glUseProgramStages(newPipeline.glHandle, GL_VERTEX_SHADER_BIT, vertexProgram->GetGLHandle());
				BS_CHECK_GL_ERROR();
			}

			if(fragmentProgram != nullptr)
			{
				glUseProgramStages(newPipeline.glHandle, GL_FRAGMENT_SHADER_BIT, fragmentProgram->GetGLHandle());
				BS_CHECK_GL_ERROR();
			}

			if(geometryProgram != nullptr)
			{
				glUseProgramStages(newPipeline.glHandle, GL_GEOMETRY_SHADER_BIT, geometryProgram->GetGLHandle());
				BS_CHECK_GL_ERROR();
			}

			if(hullProgram != nullptr)
			{
				glUseProgramStages(newPipeline.glHandle, GL_TESS_CONTROL_SHADER_BIT, hullProgram->GetGLHandle());
				BS_CHECK_GL_ERROR();
			}

			if(domainProgram != nullptr)
			{
				glUseProgramStages(newPipeline.glHandle, GL_TESS_EVALUATION_SHADER_BIT, domainProgram->GetGLHandle());
				BS_CHECK_GL_ERROR();
			}

			mPipelines[key] = newPipeline;

			BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_PipelineObject);
			return &mPipelines[key];
		}
		else
			return &iterFind->second;
	}
}}
