//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGLOcclusionQuery.h"
#include "BsGLCommandBuffer.h"
#include "BsMath.h"
#include "BsRenderStats.h"

namespace bs
{
	GLOcclusionQuery::GLOcclusionQuery(bool binary, UINT32 deviceIdx)
		:OcclusionQuery(binary), mQueryObj(0), mFinalized(false), mEndIssued(false), mNumSamples(0)
	{
		assert(deviceIdx == 0 && "Multiple GPUs not supported natively on OpenGL.");

		glGenQueries(1, &mQueryObj);
		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_Query);
	}

	GLOcclusionQuery::~GLOcclusionQuery()
	{
		glDeleteQueries(1, &mQueryObj);
		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_Query);
	}

	void GLOcclusionQuery::begin(const SPtr<CommandBuffer>& cb)
	{
		auto execute = [&]()
		{
			glBeginQuery(mBinary ? GL_ANY_SAMPLES_PASSED : GL_SAMPLES_PASSED, mQueryObj);

			mNumSamples = 0;
			mEndIssued = false;
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

	void GLOcclusionQuery::end(const SPtr<CommandBuffer>& cb)
	{
		auto execute = [&]()
		{
			glEndQuery(mBinary ? GL_ANY_SAMPLES_PASSED : GL_SAMPLES_PASSED);

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

	bool GLOcclusionQuery::isReady() const
	{
		if (!mEndIssued)
			return false;

		GLint done = 0;
		glGetQueryObjectiv(mQueryObj, GL_QUERY_RESULT_AVAILABLE, &done);

		return done == GL_TRUE;
	}

	UINT32 GLOcclusionQuery::getNumSamples()
	{
		if (!mFinalized && isReady())
		{
			finalize();
		}

		return mNumSamples;
	}

	void GLOcclusionQuery::finalize()
	{
		mFinalized = true;

		if (mBinary)
		{
			GLboolean anyPassed = GL_FALSE;
			glGetQueryObjectuiv(mQueryObj, GL_QUERY_RESULT_ARB, (GLuint*)&anyPassed);

			mNumSamples = anyPassed == GL_TRUE ? 1 : 0;
		}
		else
		{
			GLuint numSamples = 0;
			glGetQueryObjectuiv(mQueryObj, GL_QUERY_RESULT_ARB, (GLuint*)&numSamples);

			mNumSamples = (UINT32)numSamples;
		}
	}
}
