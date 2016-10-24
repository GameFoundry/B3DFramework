//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGLEventQuery.h"
#include "BsRenderStats.h"

namespace BansheeEngine
{
	GLEventQuery::GLEventQuery(UINT32 deviceIdx)
		:mQueryObj(0)
	{
		assert(deviceIdx == 0 && "Multiple GPUs not supported natively on OpenGL.");

		glGenQueries(1, &mQueryObj);
		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_Query);
	}

	GLEventQuery::~GLEventQuery()
	{
		glDeleteQueries(1, &mQueryObj);
		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_Query);
	}

	void GLEventQuery::begin()
	{
		glQueryCounter(mQueryObj, GL_TIMESTAMP);
		setActive(true);
	}

	bool GLEventQuery::isReady() const
	{
		GLint done = 0;
		glGetQueryObjectiv(mQueryObj, GL_QUERY_RESULT_AVAILABLE, &done);

		return done == GL_TRUE;
	}
}