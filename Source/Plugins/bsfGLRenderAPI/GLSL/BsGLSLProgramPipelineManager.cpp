//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GLSL/BsGLSLProgramPipelineManager.h"
#include "GLSL/BsGLSLGpuProgram.h"
#include "Profiling/BsRenderStats.h"

using namespace bs;
using namespace bs::ct;

::std::size_t GLSLProgramPipelineManager::ProgramPipelineKeyHashFunction::operator()(const GLSLProgramPipelineManager::ProgramPipelineKey& key) const
{
	std::size_t seed = 0;
	B3DCombineHash(seed, key.VertexProgKey);
	B3DCombineHash(seed, key.FragmentProgKey);
	B3DCombineHash(seed, key.GeometryProgKey);
	B3DCombineHash(seed, key.HullProgKey);
	B3DCombineHash(seed, key.DomainProgKey);

	return seed;
}

bool GLSLProgramPipelineManager::ProgramPipelineKeyEqual::operator()(const GLSLProgramPipelineManager::ProgramPipelineKey& a, const GLSLProgramPipelineManager::ProgramPipelineKey& b) const
{
	return a.VertexProgKey == b.VertexProgKey && a.FragmentProgKey == b.FragmentProgKey && a.GeometryProgKey == b.GeometryProgKey &&
		a.HullProgKey == b.HullProgKey && a.DomainProgKey == b.DomainProgKey;
}

GLSLProgramPipelineManager::~GLSLProgramPipelineManager()
{
	for(auto& pipeline : mPipelines)
	{
		glDeleteProgramPipelines(1, &pipeline.second.GlHandle);
		B3D_CHECK_GL_ERROR();

		B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResDestroyed, RenderStatObject_PipelineObject);
	}
}

const GLSLProgramPipeline* GLSLProgramPipelineManager::GetPipeline(GLSLGpuProgram* vertexProgram, GLSLGpuProgram* fragmentProgram, GLSLGpuProgram* geometryProgram, GLSLGpuProgram* hullProgram, GLSLGpuProgram* domainProgram)
{
	ProgramPipelineKey key;
	key.VertexProgKey = vertexProgram != nullptr ? vertexProgram->GetProgramId() : 0;
	key.FragmentProgKey = fragmentProgram != nullptr ? fragmentProgram->GetProgramId() : 0;
	key.GeometryProgKey = geometryProgram != nullptr ? geometryProgram->GetProgramId() : 0;
	key.HullProgKey = hullProgram != nullptr ? hullProgram->GetProgramId() : 0;
	key.DomainProgKey = domainProgram != nullptr ? domainProgram->GetProgramId() : 0;

	auto iterFind = mPipelines.find(key);

	if(iterFind == mPipelines.end())
	{
		GLSLProgramPipeline newPipeline;

		glGenProgramPipelines(1, &newPipeline.GlHandle);
		B3D_CHECK_GL_ERROR();

		if(vertexProgram != nullptr)
		{
			glUseProgramStages(newPipeline.GlHandle, GL_VERTEX_SHADER_BIT, vertexProgram->GetGlHandle());
			B3D_CHECK_GL_ERROR();
		}

		if(fragmentProgram != nullptr)
		{
			glUseProgramStages(newPipeline.GlHandle, GL_FRAGMENT_SHADER_BIT, fragmentProgram->GetGlHandle());
			B3D_CHECK_GL_ERROR();
		}

		if(geometryProgram != nullptr)
		{
			glUseProgramStages(newPipeline.GlHandle, GL_GEOMETRY_SHADER_BIT, geometryProgram->GetGlHandle());
			B3D_CHECK_GL_ERROR();
		}

		if(hullProgram != nullptr)
		{
			glUseProgramStages(newPipeline.GlHandle, GL_TESS_CONTROL_SHADER_BIT, hullProgram->GetGlHandle());
			B3D_CHECK_GL_ERROR();
		}

		if(domainProgram != nullptr)
		{
			glUseProgramStages(newPipeline.GlHandle, GL_TESS_EVALUATION_SHADER_BIT, domainProgram->GetGlHandle());
			B3D_CHECK_GL_ERROR();
		}

		mPipelines[key] = newPipeline;

		B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResCreated, RenderStatObject_PipelineObject);
		return &mPipelines[key];
	}
	else
		return &iterFind->second;
}
