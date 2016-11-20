//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGLTimerQuery.h"
#include "BsGLCommandBuffer.h"
#include "BsMath.h"
#include "BsRenderStats.h"

namespace bs
{
	GLTimerQuery::GLTimerQuery(UINT32 deviceIdx)
		:mQueryStartObj(0), mQueryEndObj(0), mFinalized(false), mEndIssued(false), mTimeDelta(0.0f)
	{
		assert(deviceIdx == 0 && "Multiple GPUs not supported natively on OpenGL.");

		GLuint queries[2];
		glGenQueries(2, queries);

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

		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_Query);
	}

	void GLTimerQuery::begin(const SPtr<CommandBuffer>& cb)
	{
		auto execute = [&]()
		{
			glQueryCounter(mQueryStartObj, GL_TIMESTAMP);

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
		glGetQueryObjectui64v(mQueryEndObj, GL_QUERY_RESULT, &timeEnd);

		mTimeDelta = (timeEnd - timeStart) / 1000000.0f;
	}
}