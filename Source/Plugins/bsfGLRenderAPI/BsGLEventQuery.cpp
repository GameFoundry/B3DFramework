//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsGLEventQuery.h"
#include "BsGLCommandBuffer.h"
#include "Profiling/BsRenderStats.h"

using namespace bs;
using namespace bs::ct;

GLEventQuery::GLEventQuery(u32 deviceIdx)
{
	B3D_ASSERT(deviceIdx == 0 && "Multiple GPUs not supported natively on OpenGL.");

	glGenQueries(1, &mQueryObj);
	B3D_CHECK_GL_ERROR();

	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResCreated, RenderStatObject_Query);
}

GLEventQuery::~GLEventQuery()
{
	glDeleteQueries(1, &mQueryObj);
	B3D_CHECK_GL_ERROR();

	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResDestroyed, RenderStatObject_Query);
}

void GLEventQuery::Begin(const SPtr<CommandBuffer>& cb)
{
	auto execute = [&]()
	{
		glQueryCounter(mQueryObj, GL_TIMESTAMP);
		B3D_CHECK_GL_ERROR();

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

bool GLEventQuery::IsReady() const
{
	GLint done = 0;
	glGetQueryObjectiv(mQueryObj, GL_QUERY_RESULT_AVAILABLE, &done);
	B3D_CHECK_GL_ERROR();

	return done == GL_TRUE;
}
