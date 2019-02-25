//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsGLVertexArrayObjectManager.h"
#include "BsGLVertexBuffer.h"
#include "RenderAPI/BsVertexDeclaration.h"
#include "GLSL/BsGLSLGpuProgram.h"
#include "BsGLHardwareBufferManager.h"
#include "Profiling/BsRenderStats.h"

#define VBO_BUFFER_OFFSET(i) ((char *)NULL + (i))

namespace bs { namespace ct
{
	GLVertexArrayObject::GLVertexArrayObject(GLuint handle, UINT64 vertexProgramId, 
		GLVertexBuffer** attachedBuffers, UINT32 numBuffers)
		:mHandle(handle), mVertProgId(vertexProgramId), mAttachedBuffers(attachedBuffers), mNumBuffers(numBuffers)
	{ }

	::std::size_t GLVertexArrayObject::Hash::operator()(const GLVertexArrayObject &vao) const
	{
		std::size_t seed = 0;
		bs_hash_combine(seed, vao.mVertProgId);

		for (UINT32 i = 0; i < vao.mNumBuffers; i++)
			bs_hash_combine(seed, vao.mAttachedBuffers[i]->getGLBufferId());

		return seed;
	}

	bool GLVertexArrayObject::Equal::operator()(const GLVertexArrayObject &a, const GLVertexArrayObject &b) const
	{
		if (a.mVertProgId != b.mVertProgId)
			return false;

		if (a.mNumBuffers != b.mNumBuffers)
			return false;

		for (UINT32 i = 0; i < a.mNumBuffers; i++)
		{
			if (a.mAttachedBuffers[i]->getGLBufferId() != b.mAttachedBuffers[i]->getGLBufferId())
				return false;
		}

		return true;
	}

	bool GLVertexArrayObject::operator== (const GLVertexArrayObject& obj)
	{
		if (mVertProgId != obj.mVertProgId)
			return false;

		if (mNumBuffers != obj.mNumBuffers)
			return false;

		for (UINT32 i = 0; i < mNumBuffers; i++)
		{
			if (mAttachedBuffers[i]->getGLBufferId() != obj.mAttachedBuffers[i]->getGLBufferId())
				return false;
		}

		return true;
	}

	bool GLVertexArrayObject::operator!= (const GLVertexArrayObject& obj)
	{
		return !operator==(obj);
	}

	GLVertexArrayObjectManager::~GLVertexArrayObjectManager()
	{
		assert(mVAObjects.size() == 0 && "VertexArrayObjectManager getting shut down but not all VA objects were released.");
	}

	const GLVertexArrayObject& GLVertexArrayObjectManager::getVAO(const SPtr<GLSLGpuProgram>& vertexProgram,
		const SPtr<VertexDeclaration>& vertexDecl, const std::array<SPtr<VertexBuffer>, 32>& boundBuffers)
	{
		UINT16 maxStreamIdx = 0;
		const Vector<VertexElement>& decl = vertexDecl->getProperties().getElements();
		for (auto& elem : decl)
			maxStreamIdx = std::max(maxStreamIdx, elem.getStreamIdx());

		UINT32 numStreams = maxStreamIdx + 1;
		UINT32 numUsedBuffers = 0;
		INT32* streamToSeqIdx = bs_stack_alloc<INT32>(numStreams);
		GLVertexBuffer** usedBuffers = bs_stack_alloc<GLVertexBuffer*>((UINT32)boundBuffers.size());
		
		memset(usedBuffers, 0, (UINT32)boundBuffers.size() * sizeof(GLVertexBuffer*));

		for (UINT32 i = 0; i < numStreams; i++)
			streamToSeqIdx[i] = -1;

		for (auto& elem : decl)
		{
			UINT16 streamIdx = elem.getStreamIdx();
			if (streamIdx >= (UINT32)boundBuffers.size())
				continue;

			if (streamToSeqIdx[streamIdx] != -1) // Already visited
				continue;

			SPtr<VertexBuffer> vertexBuffer = boundBuffers[streamIdx];
			streamToSeqIdx[streamIdx] = (INT32)numUsedBuffers;

			if (vertexBuffer != nullptr)
				usedBuffers[numUsedBuffers] = static_cast<GLVertexBuffer*>(vertexBuffer.get()); 
			else
				usedBuffers[numUsedBuffers] = nullptr;

			numUsedBuffers++;
		}
		
		GLVertexArrayObject wantedVAO(0, vertexProgram->getGLHandle(), usedBuffers, numUsedBuffers);

		auto findIter = mVAObjects.find(wantedVAO);
		if (findIter != mVAObjects.end())
		{
			bs_stack_free(usedBuffers);
			bs_stack_free(streamToSeqIdx);

			return *findIter; // Found existing, return that
		}

		// Need to create new VAO
		const Vector<VertexElement>& inputAttributes = vertexProgram->getInputDeclaration()->getProperties().getElements();

		glGenVertexArrays(1, &wantedVAO.mHandle);
		BS_CHECK_GL_ERROR();

		glBindVertexArray(wantedVAO.mHandle);
		BS_CHECK_GL_ERROR();

		for (auto& elem : decl)
		{
			UINT16 streamIdx = elem.getStreamIdx();
			INT32 seqIdx = streamToSeqIdx[streamIdx];

			if (seqIdx == -1)
				continue;

			bool foundSemantic = false;
			GLint attribLocation = 0;
			for (auto iter = inputAttributes.begin(); iter != inputAttributes.end(); ++iter)
			{
				if (iter->getSemantic() == elem.getSemantic() && iter->getSemanticIdx() == elem.getSemanticIdx())
				{
					foundSemantic = true;
					attribLocation = iter->getOffset();
					break;
				}
			}

			if (!foundSemantic) // Shader needs to have a matching input attribute, otherwise we skip it
				continue;

			// TODO - We might also want to check the size of input and buffer, and make sure they match? Or does OpenGL handle that internally?

			GLVertexBuffer* vertexBuffer = usedBuffers[seqIdx];
			const VertexBufferProperties& vbProps = vertexBuffer->getProperties();

			glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer->getGLBufferId());
			BS_CHECK_GL_ERROR();

			void* bufferData = VBO_BUFFER_OFFSET(elem.getOffset());

			UINT16 typeCount = VertexElement::getTypeCount(elem.getType());
			GLenum glType = GLHardwareBufferManager::getGLType(elem.getType());
			bool isInteger = glType == GL_SHORT || glType == GL_UNSIGNED_SHORT || glType == GL_INT 
				|| glType == GL_UNSIGNED_INT || glType == GL_UNSIGNED_BYTE;

			GLboolean normalized = GL_FALSE;
			switch (elem.getType())
			{
			case VET_COLOR:
			case VET_COLOR_ABGR:
			case VET_COLOR_ARGB:
			case VET_UBYTE4_NORM:
				normalized = GL_TRUE;
				isInteger = false;
				break;
			default:
				break;
			}

			GLsizei vertexSize = static_cast<GLsizei>(vbProps.getVertexSize());
			if(isInteger)
			{
				glVertexAttribIPointer(attribLocation, typeCount, glType, vertexSize, bufferData);
				BS_CHECK_GL_ERROR();
			}
			else
			{
				glVertexAttribPointer(attribLocation, typeCount, glType, normalized, vertexSize, bufferData);
				BS_CHECK_GL_ERROR();
			}

			glVertexAttribDivisor(attribLocation, elem.getInstanceStepRate());
			BS_CHECK_GL_ERROR();

			glEnableVertexAttribArray(attribLocation);
			BS_CHECK_GL_ERROR();
		}

		wantedVAO.mAttachedBuffers = (GLVertexBuffer**)bs_alloc(numUsedBuffers * sizeof(GLVertexBuffer*));
		for (UINT32 i = 0; i < numUsedBuffers; i++)
		{
			wantedVAO.mAttachedBuffers[i] = usedBuffers[i];
			usedBuffers[i]->registerVAO(wantedVAO);
		}

		bs_stack_free(usedBuffers);
		bs_stack_free(streamToSeqIdx);

		auto iter = mVAObjects.insert(wantedVAO);

		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_VertexArrayObject);
		return *iter.first;
	}

	// Note: This must receieve a copy and not a ref because original will be destroyed
	void GLVertexArrayObjectManager::notifyBufferDestroyed(GLVertexArrayObject vao)
	{
		mVAObjects.erase(vao);

		for (UINT32 i = 0; i < vao.mNumBuffers; i++)
		{
			vao.mAttachedBuffers[i]->unregisterVAO(vao);
		}

		glDeleteVertexArrays(1, &vao.mHandle);
		BS_CHECK_GL_ERROR();

		bs_free(vao.mAttachedBuffers);

		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_VertexArrayObject);
	}
}}