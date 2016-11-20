//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGLVertexArrayObjectManager.h"
#include "BsGLVertexBuffer.h"
#include "BsVertexDeclaration.h"
#include "BsGLSLGpuProgram.h"
#include "BsGLHardwareBufferManager.h"
#include "BsRenderStats.h"

#define VBO_BUFFER_OFFSET(i) ((char *)NULL + (i))

namespace bs
{
	GLVertexArrayObject::GLVertexArrayObject()
		:mHandle(0), mVertProgId(0), mAttachedBuffers(nullptr), mNumBuffers(0)
	{ }

	GLVertexArrayObject::GLVertexArrayObject(GLuint handle, UINT64 vertexProgramId, 
		GLVertexBufferCore** attachedBuffers, UINT32 numBuffers)
		:mHandle(handle), mVertProgId(vertexProgramId), mAttachedBuffers(attachedBuffers), mNumBuffers(numBuffers)
	{ }

	::std::size_t GLVertexArrayObject::Hash::operator()(const GLVertexArrayObject &vao) const
	{
		std::size_t seed = 0;
		hash_combine(seed, vao.mVertProgId);

		for (UINT32 i = 0; i < vao.mNumBuffers; i++)
			hash_combine(seed, vao.mAttachedBuffers[i]->getGLBufferId());

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

	const GLVertexArrayObject& GLVertexArrayObjectManager::getVAO(const SPtr<GLSLGpuProgramCore>& vertexProgram,
		const SPtr<VertexDeclarationCore>& vertexDecl, const std::array<SPtr<VertexBufferCore>, 32>& boundBuffers)
	{
		UINT16 maxStreamIdx = 0;
		const List<VertexElement>& decl = vertexDecl->getProperties().getElements();
		for (auto& elem : decl)
			maxStreamIdx = std::max(maxStreamIdx, elem.getStreamIdx());

		UINT32 numStreams = maxStreamIdx + 1;
		UINT32 numUsedBuffers = 0;
		INT32* streamToSeqIdx = bs_stack_alloc<INT32>(numStreams);
		GLVertexBufferCore** usedBuffers = bs_stack_alloc<GLVertexBufferCore*>((UINT32)boundBuffers.size());
		
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

			SPtr<VertexBufferCore> vertexBuffer = boundBuffers[streamIdx];
			streamToSeqIdx[streamIdx] = (INT32)numUsedBuffers;

			if (vertexBuffer != nullptr)
				usedBuffers[numUsedBuffers] = static_cast<GLVertexBufferCore*>(vertexBuffer.get()); 
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
		const List<VertexElement>& inputAttributes = vertexProgram->getInputDeclaration()->getProperties().getElements();

		glGenVertexArrays(1, &wantedVAO.mHandle);
		glBindVertexArray(wantedVAO.mHandle);

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

			GLVertexBufferCore* vertexBuffer = usedBuffers[seqIdx];
			const VertexBufferProperties& vbProps = vertexBuffer->getProperties();

			glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer->getGLBufferId());
			void* bufferData = VBO_BUFFER_OFFSET(elem.getOffset());

			UINT16 typeCount = VertexElement::getTypeCount(elem.getType());
			GLenum glType = GLHardwareBufferCoreManager::getGLType(elem.getType());
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
			}
			else
			{
				glVertexAttribPointer(attribLocation, typeCount, glType, normalized,
					vertexSize, bufferData);
			}

			glVertexAttribDivisor(attribLocation, elem.getInstanceStepRate());
			glEnableVertexAttribArray(attribLocation);
		}

		wantedVAO.mAttachedBuffers = (GLVertexBufferCore**)bs_alloc(numUsedBuffers * sizeof(GLVertexBufferCore*));
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
		bs_free(vao.mAttachedBuffers);

		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_VertexArrayObject);
	}
}