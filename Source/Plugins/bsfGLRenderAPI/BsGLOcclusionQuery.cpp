//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsGLOcclusionQuery.h"
#include "BsGLCommandBuffer.h"
#include "Math/BsMath.h"
#include "Profiling/BsRenderStats.h"

using namespace bs;
using namespace bs::ct;

GLOcclusionQuery::GLOcclusionQuery(bool binary, u32 deviceIdx)
	: OcclusionQuery(binary)
{
	B3D_ASSERT(deviceIdx == 0 && "Multiple GPUs not supported natively on OpenGL.");

	glGenQueries(1, &mQueryObj);
	B3D_CHECK_GL_ERROR();

	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResCreated, RenderStatObject_Query);
}

GLOcclusionQuery::~GLOcclusionQuery()
{
	glDeleteQueries(1, &mQueryObj);
	B3D_CHECK_GL_ERROR();

	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResDestroyed, RenderStatObject_Query);
}

void GLOcclusionQuery::Begin(const SPtr<CommandBuffer>& cb)
{
	auto execute = [&]()
	{
		glBeginQuery(mBinary ? GL_ANY_SAMPLES_PASSED : GL_SAMPLES_PASSED, mQueryObj);
		B3D_CHECK_GL_ERROR();

		mNumSamples = 0;
		mEndIssued = false;
		SetActive(true);
	};

	if(cb == nullptr)
		execute();
	else
	{
		SPtr<GLCommandBuffer> glCB = std::static_pointer_cast<GLCommandBuffer>(cb);
		glCB->QueueCommand(execute);
	}
}

void GLOcclusionQuery::End(const SPtr<CommandBuffer>& cb)
{
	auto execute = [&]()
	{
		glEndQuery(mBinary ? GL_ANY_SAMPLES_PASSED : GL_SAMPLES_PASSED);
		B3D_CHECK_GL_ERROR();

		mEndIssued = true;
		mFinalized = false;
	};

	if(cb == nullptr)
		execute();
	else
	{
		SPtr<GLCommandBuffer> glCB = std::static_pointer_cast<GLCommandBuffer>(cb);
		glCB->QueueCommand(execute);
	}
}

bool GLOcclusionQuery::IsReady() const
{
	if(!mEndIssued)
		return false;

	GLint done = 0;
	glGetQueryObjectiv(mQueryObj, GL_QUERY_RESULT_AVAILABLE, &done);
	B3D_CHECK_GL_ERROR();

	return done == GL_TRUE;
}

u32 GLOcclusionQuery::GetSampleCount()
{
	if(!mFinalized && IsReady())
	{
		Finalize();
	}

	return mNumSamples;
}

void GLOcclusionQuery::Finalize()
{
	mFinalized = true;

	if(mBinary)
	{
		GLuint anyPassed = GL_FALSE;
		glGetQueryObjectuiv(mQueryObj, GL_QUERY_RESULT, &anyPassed);
		B3D_CHECK_GL_ERROR();

		mNumSamples = anyPassed == GL_TRUE ? 1 : 0;
	}
	else
	{
		GLuint numSamples = 0;
		glGetQueryObjectuiv(mQueryObj, GL_QUERY_RESULT, &numSamples);
		B3D_CHECK_GL_ERROR();

		mNumSamples = (u32)numSamples;
	}
}
