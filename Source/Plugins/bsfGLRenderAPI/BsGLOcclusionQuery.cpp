//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsGLOcclusionQuery.h"
#include "BsGLCommandBuffer.h"
#include "Math/BsMath.h"
#include "Profiling/BsRenderStats.h"

namespace bs { namespace ct
{
	GLOcclusionQuery::GLOcclusionQuery(bool binary, UINT32 deviceIdx)
		:OcclusionQuery(binary)
	{
		assert(deviceIdx == 0 && "Multiple GPUs not supported natively on OpenGL.");

		glGenQueries(1, &mQueryObj);
		BS_CHECK_GL_ERROR();

		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_Query);
	}

	GLOcclusionQuery::~GLOcclusionQuery()
	{
		glDeleteQueries(1, &mQueryObj);
		BS_CHECK_GL_ERROR();

		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_Query);
	}

	void GLOcclusionQuery::begin(const SPtr<CommandBuffer>& cb)
	{
		auto execute = [&]()
		{
			glBeginQuery(mBinary ? GL_ANY_SAMPLES_PASSED : GL_SAMPLES_PASSED, mQueryObj);
			BS_CHECK_GL_ERROR();

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

	bool GLOcclusionQuery::isReady() const
	{
		if (!mEndIssued)
			return false;

		GLint done = 0;
		glGetQueryObjectiv(mQueryObj, GL_QUERY_RESULT_AVAILABLE, &done);
		BS_CHECK_GL_ERROR();

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
			GLuint anyPassed = GL_FALSE;
			glGetQueryObjectuiv(mQueryObj, GL_QUERY_RESULT, &anyPassed);
			BS_CHECK_GL_ERROR();

			mNumSamples = anyPassed == GL_TRUE ? 1 : 0;
		}
		else
		{
			GLuint numSamples = 0;
			glGetQueryObjectuiv(mQueryObj, GL_QUERY_RESULT, &numSamples);
			BS_CHECK_GL_ERROR();

			mNumSamples = (UINT32)numSamples;
		}
	}
}}