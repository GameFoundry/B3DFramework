//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsGLPrerequisites.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup GL
		 *  @{
		 */

		/**	Wrapper around OpenGL pipeline object. */
		struct GLSLProgramPipeline
		{
			GLuint GlHandle;
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
			const GLSLProgramPipeline* GetPipeline(GLSLGpuProgram* vertexProgram, GLSLGpuProgram* fragmentProgram, GLSLGpuProgram* geometryProgram, GLSLGpuProgram* hullProgram, GLSLGpuProgram* domainProgram);

		private:
			/**	Key that uniquely identifies a pipeline object. */
			struct ProgramPipelineKey
			{
				u32 VertexProgKey;
				u32 FragmentProgKey;
				u32 GeometryProgKey;
				u32 HullProgKey;
				u32 DomainProgKey;
			};

			/**	Used for calculating a hash code from pipeline object key. */
			class ProgramPipelineKeyHashFunction
			{
			public:
				::std::size_t operator()(const ProgramPipelineKey& key) const;
			};

			/**	Used for comparing two pipeline objects for equality. */
			class ProgramPipelineKeyEqual
			{
			public:
				bool operator()(const ProgramPipelineKey& a, const ProgramPipelineKey& b) const;
			};

			typedef UnorderedMap<ProgramPipelineKey, GLSLProgramPipeline, ProgramPipelineKeyHashFunction, ProgramPipelineKeyEqual> ProgramPipelineMap;
			ProgramPipelineMap mPipelines;
		};

		/** @} */
	} // namespace ct
} // namespace bs
