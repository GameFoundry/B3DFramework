//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"

namespace bs
{
	struct GpuPipelineParameterLayoutCreateInformation;
	struct GpuProgramBytecode;
	struct GpuBufferCreateInformation;
	struct GpuProgramCreateInformation;
	struct GpuComputePipelineStateCreateInformation;
	struct GpuGraphicsPipelineStateCreateInformation;

	/** @addtogroup RenderAPI
	 *  @{
	 */

	/**
	 * Provides access to a particular GPU device.
	 *
	 * @note	Thread safe.
	 */
	class B3D_CORE_EXPORT GpuDevice
	{
	public:
		virtual ~GpuDevice() = default;

		// TODO - Doc
		virtual bool IsInitialized() const = 0;
		virtual bool Initialize() = 0;

		virtual const GpuDeviceCapabilities& GetCapabilities() = 0;

		/** Returns information about available output devices and their video modes. */
		virtual const VideoModeInfo& GetVideoModeInfo() const = 0;

		/** Query if a GPU program language is supported (for example "hlsl", "glsl"). Thread safe. */
		virtual bool IsGpuProgramLanguageSupported(const StringView& language) const = 0;

		/**
		 * Compiles the GPU program to an intermediate bytecode format. The bytecode can be cached and used for
		 * quicker compilation/creation of GPU programs.
		 */
		virtual SPtr<GpuProgramBytecode> CompileGpuProgramBytecode(const GpuProgramCreateInformation& createInformation) const = 0;

		/**
		 * Creates a new GPU buffer.
		 *
		 * @param	createInformation		Object describing the buffer to create.
		 * @param	deferredInitialize		If true, Initialize() will not be called on the returned object, and the caller is expected to call it himself, before first using the object.
		 */
		virtual SPtr<ct::GpuBuffer> CreateGpuBuffer(const GpuBufferCreateInformation& createInformation, bool deferredInitialize = false) = 0;

		/** Create a new event query. */
		virtual SPtr<ct::EventQuery> CreateEventQuery() = 0;

		/** Creates a new timer query. */
		virtual SPtr<ct::TimerQuery> CreateTimerQuery() = 0;

		/**
		 * Creates a new occlusion query.
		 *
		 * @param isBinary		If query is binary it will not give you an exact count of samples rendered, but will
		 *						instead just return 0 (no samples were rendered) or 1 (one or more samples were
		 *						rendered). Binary queries can return sooner as they potentially do not need to wait
		 *						until all of the geometry is rendered.
		 */
		virtual SPtr<ct::OcclusionQuery> CreateOcclusionQuery(bool isBinary) = 0;

		/**
		 * Creates a new GPU program using the provided source code. If compilation fails or program is not supported
		 * GpuProgram::IsCompiled() will return false, and you will be able to retrieve the error message via GpuProgram::GetCompileErrorMessage().
		 *
		 * @param	createInformation		Object describing the program to create.
		 * @param	deferredInitialize		If true, Initialize() will not be called on the returned object, and the caller is expected to call it himself, before first using the object.
		 */
		virtual SPtr<GpuProgram> CreateGpuProgram(const GpuProgramCreateInformation& createInformation, bool deferredInitialize = false) = 0;

		/**
		 * Creates a set of GPU parameters that allow you to bind values to be used as input to GPU programs within a GPU pipeline with the provided layout.
		 *
		 * @param	parameterLayout			Layout that describes the GPU parameters, as retrieved from the GPU pipeline.
		 * @param	deferredInitialize		If true, Initialize() will not be called on the returned object, and the caller is expected to call it himself, before first using the object.
		 */
		virtual SPtr<ct::GpuParameters> CreateGpuParameters(const SPtr<GpuPipelineParameterLayout>& parameterLayout, bool deferredInitialize = false) = 0;

		/**
		 * Creates a graphics pipeline.
		 *
		 * @param	createInformation		Object describing the pipeline to create.
		 * @param	deferredInitialize		If true, Initialize() will not be called on the returned object, and the caller is expected to call it himself, before first using the object.
		 */
		virtual SPtr<GpuGraphicsPipelineState> CreateGpuGraphicsPipelineState(const GpuGraphicsPipelineStateCreateInformation& createInformation, bool deferredInitialize = false) = 0;

		/**
		 * Creates a compute pipeline.
		 *
		 * @param	createInformation		Object describing the pipeline to create.
		 * @param	deferredInitialize		If true, Initialize() will not be called on the returned object, and the caller is expected to call it himself, before first using the object.
		 */
		virtual SPtr<GpuComputePipelineState> CreateGpuComputePipelineState(const GpuComputePipelineStateCreateInformation& createInformation, bool deferredInitialize = false) = 0;

		/**
		 * Creates a pipeline layout from a set of GPU program parameter descriptions. 
		 *
		 * @param	createInformation		Object describing the layout to create.
		 * @param	deferredInitialize		If true, Initialize() will not be called on the returned object, and the caller is expected to call it himself, before first using the object.
		 */
		virtual SPtr<GpuPipelineParameterLayout> CreateGpuPipelineParameterLayout(const GpuPipelineParameterLayoutCreateInformation& createInformation, bool deferredInitialize = false) = 0;
	};

	/** @} */

} // namespace bs
