//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsGLTimerQuery.h"
#include "BsGLCommandBuffer.h"
#include "Math/BsMath.h"
#include "Profiling/BsRenderStats.h"

namespace bs { namespace ct
{
	GLTimerQuery::GLTimerQuery(UINT32 deviceIdx)
	{
		assert(deviceIdx == 0 && "Multiple GPUs not supported natively on OpenGL.");

		GLuint queries[2];
		glGenQueries(2, queries);
		BS_CHECK_GL_ERROR();

		mQueryStartObj = queries[0];
		mQueryEndObj = queries[1];

		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_Query);
	}

	GLTimerQuery::~GLTimerQuery()
	{
		GLuint queries[2];
		queries[0] = mQueryStartObj;
		queries[1] = mQueryEndObj;

		glDeleteQueries(2, queries);
		BS_CHECK_GL_ERROR();

		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_Query);
	}

	void GLTimerQuery::begin(const SPtr<CommandBuffer>& cb)
	{
		auto execute = [&]()
		{
			glQueryCounter(mQueryStartObj, GL_TIMESTAMP);
			BS_CHECK_GL_ERROR();

			setActive(true);
			mEndIssued = false;
		};

		if (cb == nullptr)
			execute();
		else
		{
			SPtr<GLCommandBuffer> glCB = std::static_pointer_cast<GLCommandBuffer>(cb);
			glCB->queueCommand(execute);
		}
	}

	void GLTimerQuery::end(const SPtr<CommandBuffer>& cb)
	{
		auto execute = [&]()
		{
			glQueryCounter(mQueryEndObj, GL_TIMESTAMP);
			BS_CHECK_GL_ERROR();

			mEndIssued = true;
			mFinalized = false;
		};

		if (cb == nullptr)
			execute();
		else
		{
			SPtr<GLCommandBuffer> glCB = std::static_pointer_cast<GLCommandBuffer>(cb);
			glCB->queueCommand(execute);
		}
	}

	bool GLTimerQuery::isReady() const
	{
		if (!mEndIssued)
			return false;

		GLint done = 0;
		glGetQueryObjectiv(mQueryEndObj, GL_QUERY_RESULT_AVAILABLE, &done);
		BS_CHECK_GL_ERROR();

		return done == GL_TRUE;
	}

	float GLTimerQuery::getTimeMs() 
	{
		if(!mFinalized && isReady())
		{
			finalize();
		}

		return mTimeDelta;
	}

	void GLTimerQuery::finalize()
	{
		mFinalized = true;

		GLuint64 timeStart;
		GLuint64 timeEnd;

		glGetQueryObjectui64v(mQueryStartObj, GL_QUERY_RESULT, &timeStart);
		BS_CHECK_GL_ERROR();

		glGetQueryObjectui64v(mQueryEndObj, GL_QUERY_RESULT, &timeEnd);
		BS_CHECK_GL_ERROR();

		mTimeDelta = (timeEnd - timeStart) / 1000000.0f;
	}
}}