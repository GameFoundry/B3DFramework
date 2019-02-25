//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsGLPrerequisites.h"
#include "Utility/BsModule.h"

namespace bs { namespace ct
{
	/** @addtogroup GL
	 *  @{
	 */

	/**
	 * Vertex array object that contains vertex buffer object bindings and vertex attribute pointers for a specific set of
	 * vertex buffers and a vertex declaration.
	 */
	class GLVertexArrayObject
	{
	private:
		/**	Generates hash value for the VAO object. */
		class Hash
		{
		public:
			::std::size_t operator()(const GLVertexArrayObject& vao) const;
		};

		/**	Checks if two VAO objects are equal. */
		class Equal
		{
		public:
			bool operator()(const GLVertexArrayObject &a, const GLVertexArrayObject &b) const;
		};

	public:
		bool operator== (const GLVertexArrayObject& obj);
		bool operator!= (const GLVertexArrayObject& obj);

		/**	Returns internal OpenGL VBO handle. */
		GLuint getGLHandle() const { return mHandle;  }

	private:
		friend class GLVertexArrayObjectManager;

		GLVertexArrayObject() = default;
		GLVertexArrayObject(GLuint handle, UINT64 vertexProgramId, GLVertexBuffer** attachedBuffers, UINT32 numBuffers);

		GLuint mHandle = 0;
		UINT64 mVertProgId = 0;
		GLVertexBuffer** mAttachedBuffers = nullptr;
		UINT32 mNumBuffers = 0;
	};

	/**	Manager that handles creation and destruction of vertex array objects. */
	class GLVertexArrayObjectManager : public Module<GLVertexArrayObjectManager>
	{
	public:
		~GLVertexArrayObjectManager();

		/**
		 * Attempts to find an existing vertex array object matching the provided set of vertex buffers, vertex declaration,
		 * and vertex shader input parameters. If one cannot be found new one is created and returned.
		 *
		 * Lifetime of returned VAO is managed by the vertex buffers that it binds.
		 */
		const GLVertexArrayObject& getVAO(const SPtr<GLSLGpuProgram>& vertexProgram,
			const SPtr<VertexDeclaration>& vertexDecl, const std::array<SPtr<VertexBuffer>, 32>& boundBuffers);

		/**	Called when a vertex buffer containing the provided VAO is destroyed. */
		void notifyBufferDestroyed(GLVertexArrayObject vao);
	private:
		typedef UnorderedSet<GLVertexArrayObject, GLVertexArrayObject::Hash, GLVertexArrayObject::Equal> VAOMap;

		VAOMap mVAObjects;
	};

	/** @} */
}}
