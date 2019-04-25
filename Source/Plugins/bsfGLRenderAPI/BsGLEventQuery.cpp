//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsGLEventQuery.h"
#include "BsGLCommandBuffer.h"
#include "Profiling/BsRenderStats.h"

namespace bs { namespace ct
{
	GLEventQuery::GLEventQuery(UINT32 deviceIdx)
	{
		assert(deviceIdx == 0 && "Multiple GPUs not supported natively on OpenGL.");

		glGenQueries(1, &mQueryObj);
		BS_CHECK_GL_ERROR();

		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_Query);
	}

	GLEventQuery::~GLEventQuery()
	{
		glDeleteQueries(1, &mQueryObj);
		BS_CHECK_GL_ERROR();

		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_Query);
	}

	void GLEventQuery::begin(const SPtr<CommandBuffer>& cb)
	{
		auto execute = [&]()
		{
			glQueryCounter(mQueryObj, GL_TIMESTAMP);
			BS_CHECK_GL_ERROR();

			setActive(true);
		};

		if (cb == nullptr)
			execute();
		else
		{
			SPtr<GLCommandBuffer> glCB = std::static_pointer_cast<GLCommandBuffer>(cb);
			glCB->queueCommand(execute);
		}
	}

	bool GLEventQuery::isReady() const
	{
		GLint done = 0;
		glGetQueryObjectiv(mQueryObj, GL_QUERY_RESULT_AVAILABLE, &done);
		BS_CHECK_GL_ERROR();

		return done == GL_TRUE;
	}
}}