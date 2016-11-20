//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsGLPrerequisites.h"

namespace bs
{
	/** @addtogroup GL
	 *  @{
	 */

	/**	Wrapper around OpenGL pipeline object. */
	struct GLSLProgramPipeline
	{
		GLuint glHandle;
	};

	/**
	 * Managed OpenGL pipeline objects that are used for binding a certain combination of GPU programs to the render system.
	 *
	 * @note	
	 * In OpenGL you cannot bind GPU programs to the pipeline manually. Instead as a preprocessing step you create a
	 * pipeline object containing the programs you plan on using, and then later you bind the previously created pipeline
	 * object. 
	 */
	class GLSLProgramPipelineManager
	{
	public:
		~GLSLProgramPipelineManager();

		/**
		 * Creates or returns an existing pipeline that uses the provided combination of GPU programs. Provide null for
		 * unused programs.
		 */
		const GLSLProgramPipeline* getPipeline(GLSLGpuProgramCore* vertexProgram, GLSLGpuProgramCore* fragmentProgram,
			GLSLGpuProgramCore* geometryProgram, GLSLGpuProgramCore* hullProgram, GLSLGpuProgramCore* domainProgram);

	private:
		/**	Key that uniquely identifies a pipeline object. */
		struct ProgramPipelineKey
		{
			UINT32 vertexProgKey;
			UINT32 fragmentProgKey;
			UINT32 geometryProgKey;
			UINT32 hullProgKey;
			UINT32 domainProgKey;
		};

		/**	Used for calculating a hash code from pipeline object key. */
		class ProgramPipelineKeyHashFunction 
		{
		public:
			::std::size_t operator()(const ProgramPipelineKey &key) const;
		};

		/**	Used for comparing two pipeline objects for equality. */
		class ProgramPipelineKeyEqual 
		{
		public:
			bool operator()(const ProgramPipelineKey &a, const ProgramPipelineKey &b) const;
		};

		typedef UnorderedMap<ProgramPipelineKey, GLSLProgramPipeline, ProgramPipelineKeyHashFunction, ProgramPipelineKeyEqual> ProgramPipelineMap;
		ProgramPipelineMap mPipelines;
	};

	/** @} */
}