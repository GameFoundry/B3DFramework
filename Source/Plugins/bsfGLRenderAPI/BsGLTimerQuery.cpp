//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsGLTimerQuery.h"
#include "BsGLCommandBuffer.h"
#include "Math/BsMath.h"
#include "Profiling/BsRenderStats.h"

using namespace bs;
using namespace bs::ct;

GLTimerQuery::GLTimerQuery(u32 deviceIdx)
{
	B3D_ASSERT(deviceIdx == 0 && "Multiple GPUs not supported natively on OpenGL.");

	GLuint queries[2];
	glGenQueries(2, queries);
	B3D_CHECK_GL_ERROR();

	mQueryStartObj = queries[0];
	mQueryEndObj = queries[1];

	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResCreated, RenderStatObject_Query);
}

GLTimerQuery::~GLTimerQuery()
{
	GLuint queries[2];
	queries[0] = mQueryStartObj;
	queries[1] = mQueryEndObj;

	glDeleteQueries(2, queries);
	B3D_CHECK_GL_ERROR();

	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResDestroyed, RenderStatObject_Query);
}

void GLTimerQuery::Begin(const SPtr<CommandBuffer>& cb)
{
	auto execute = [&]()
	{
		glQueryCounter(mQueryStartObj, GL_TIMESTAMP);
		B3D_CHECK_GL_ERROR();

		SetActive(true);
		mEndIssued = false;
	};

	if(cb == nullptr)
		execute();
	else
	{
		SPtr<GLCommandBuffer> glCB = std::static_pointer_cast<GLCommandBuffer>(cb);
		glCB->QueueCommand(execute);
	}
}

void GLTimerQuery::End(const SPtr<CommandBuffer>& cb)
{
	auto execute = [&]()
	{
		glQueryCounter(mQueryEndObj, GL_TIMESTAMP);
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

bool GLTimerQuery::IsReady() const
{
	if(!mEndIssued)
		return false;

	GLint done = 0;
	glGetQueryObjectiv(mQueryEndObj, GL_QUERY_RESULT_AVAILABLE, &done);
	B3D_CHECK_GL_ERROR();

	return done == GL_TRUE;
}

float GLTimerQuery::GetTimeMs()
{
	if(!mFinalized && IsReady())
	{
		Finalize();
	}

	return mTimeDelta;
}

void GLTimerQuery::Finalize()
{
	mFinalized = true;

	GLuint64 timeStart;
	GLuint64 timeEnd;

	glGetQueryObjectui64v(mQueryStartObj, GL_QUERY_RESULT, &timeStart);
	B3D_CHECK_GL_ERROR();

	glGetQueryObjectui64v(mQueryEndObj, GL_QUERY_RESULT, &timeEnd);
	B3D_CHECK_GL_ERROR();

	mTimeDelta = (timeEnd - timeStart) / 1000000.0f;
}
