//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsGLPrerequisites.h"
#include "BsModule.h"

namespace bs { namespace ct
{
	/** @addtogroup GL
	 *  @{
	 */

	/**
	 * Vertex array object that contains vertex buffer object bindings and vertex attribute pointers for a specific set of
	 * vertex buffers and a vertex declaration.
	 */
	class BS_RSGL_EXPORT GLVertexArrayObject
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

		GLVertexArrayObject();
		GLVertexArrayObject(GLuint handle, UINT64 vertexProgramId, GLVertexBuffer** attachedBuffers, UINT32 numBuffers);

		GLuint mHandle;
		UINT64 mVertProgId;
		GLVertexBuffer** mAttachedBuffers;
		UINT32 mNumBuffers;
	};

	/**	Manager that handles creation and destruction of vertex array objects. */
	class BS_RSGL_EXPORT GLVertexArrayObjectManager : public Module<GLVertexArrayObjectManager>
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
			const SPtr<VertexDeclarationCore>& vertexDecl, const std::array<SPtr<VertexBufferCore>, 32>& boundBuffers);

		/**	Called when a vertex buffer containing the provided VAO is destroyed. */
		void notifyBufferDestroyed(GLVertexArrayObject vao);
	private:
		typedef UnorderedSet<GLVertexArrayObject, GLVertexArrayObject::Hash, GLVertexArrayObject::Equal> VAOMap;

		VAOMap mVAObjects;
	};

	/** @} */
}}