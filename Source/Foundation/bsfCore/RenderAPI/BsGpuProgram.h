//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "CoreThread/BsCoreObject.h"
#include "Reflection/BsIReflectable.h"
#include "Utility/BsDataBlob.h"
#include "BsVertexDeclaration.h"

namespace bs 
{
	/** @addtogroup RenderAPI
	 *  @{
	 */

	struct GpuProgramBytecode;

	/** Descriptor structure used for initialization of a GpuProgram. */
	struct GPU_PROGRAM_DESC
	{
		String source; /**< Source code to compile the program from. */
		String entryPoint; /**< Name of the entry point function, for example "main". */
		String language; /**< Language the source is written in, for example "hlsl" or "glsl". */
		GpuProgramType type = GPT_VERTEX_PROGRAM; /**< Type of the program, for example vertex or fragment. */
		bool requiresAdjacency = false; /**< If true then adjacency information will be provided when rendering. */

		/** 
		 * Optional intermediate version of the GPU program. Can significantly speed up GPU program compilation/creation.
		 * when supported by the render backend. Call ct::GpuProgram::compileBytecode to generate it.
		 */
		SPtr<GpuProgramBytecode> bytecode;
	};

	/**
	 * Contains a GPU program such as vertex or fragment program which gets compiled from the provided source code.
	 *
	 * @note	Sim thread only.
	 */
	class BS_CORE_EXPORT GpuProgram : public IReflectable, public CoreObject
	{
	public:
		/** Information returned when compiling a GPU program. */
		struct CompileStatus
		{
			bool success = false;
			String messages;
		};

		virtual ~GpuProgram() { }

		/**
		 * Returns true if the program was successfully compiled. 
		 *
		 * @note	Only valid after core thread has initialized the program.
		 */
		bool isCompiled() const;

		/**
		 * Returns an error message returned by the compiler, if the compilation failed.
		 *
		 * @note	Only valid after core thread has initialized the program.
		 */
		String getCompileErrorMessage() const;

		/**
		 * Returns description of all parameters in this GPU program.
		 *
		 * @note	Only valid after core thread has initialized the program.
		 */
		SPtr<GpuParamDesc> getParamDesc() const;

		/** Retrieves a core implementation of a gpu program usable only from the core thread. */
		SPtr<ct::GpuProgram> getCore() const;

		/**
		 * Creates a new GPU program using the provided source code. If compilation fails or program is not supported
		 * isCompiled() with return false, and you will be able to retrieve the error message via getCompileErrorMessage().
		 *
		 * @param[in]	desc		Description of the program to create.
		 */
		static SPtr<GpuProgram> create(const GPU_PROGRAM_DESC& desc);

	protected:
		friend class GpuProgramManager;

		GpuProgram(const GPU_PROGRAM_DESC& desc);

		/** @copydoc CoreObject::createCore */
		SPtr<ct::CoreObject> createCore() const override;

	protected:
		bool mNeedsAdjacencyInfo;
		String mLanguage;
		GpuProgramType mType;
		String mEntryPoint;
		String mSource;

		SPtr<GpuProgramBytecode> mCachedBytecode;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class GpuProgramRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** 
	 * A GPU program compiled to an intermediate bytecode format, as well as any relevant meta-data that could be
	 * extracted from that format. 
	 */
	struct BS_CORE_EXPORT GpuProgramBytecode : IReflectable
	{
		~GpuProgramBytecode();

		/** Instructions (compiled code) for the GPU program. Contains no data if compilation was not succesful. */
		DataBlob instructions;

		/** Reflected information about GPU program parameters. */
		SPtr<GpuParamDesc> paramDesc;

		/** Input parameters for a vertex GPU program. */
		Vector<VertexElement> vertexInput;

		/** Messages output during the compilation process. Includes errors in case compilation failed. */
		String messages;

		/** Identifier of the compiler that compiled the bytecode. */
		String compilerId;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class GpuProgramBytecodeRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** @} */

	namespace ct
	{
	/** @addtogroup RenderAPI-Internal
	 *  @{
	 */

	/**
	 * Core thread version of a bs::GpuProgram.
	 *
	 * @note	Core thread only.
	 */
	class BS_CORE_EXPORT GpuProgram : public CoreObject
	{
	public:
		virtual ~GpuProgram();

		/** Returns whether this program can be supported on the current renderer and hardware. */
		virtual bool isSupported() const;

		/** Returns true if program was successfully compiled. */
		virtual bool isCompiled() const { return mIsCompiled; }

		/**	Returns an error message returned by the compiler, if the compilation failed. */
		virtual String getCompileErrorMessage() const { return mCompileMessages; }

		/**
		 * Sets whether this geometry program requires adjacency information from the input primitives.
		 *
		 * @note	Only relevant for geometry programs.
		 */
		virtual void setAdjacencyInfoRequired(bool required) { mNeedsAdjacencyInfo = required; }

		/**
		 * Returns whether this geometry program requires adjacency information from the input primitives.
		 *
		 * @note	Only relevant for geometry programs.
		 */
		virtual bool isAdjacencyInfoRequired() const { return mNeedsAdjacencyInfo; }

		/**	Type of GPU program (for example fragment, vertex). */
		GpuProgramType getType() const { return mType; }

		/** @copydoc bs::GpuProgram::getParamDesc */
		SPtr<GpuParamDesc> getParamDesc() const { return mParametersDesc; }

		/**	Returns GPU program input declaration. Only relevant for vertex programs. */
		SPtr<VertexDeclaration> getInputDeclaration() const { return mInputDeclaration; }

		/** 
		 * @copydoc bs::GpuProgram::create(const GPU_PROGRAM_DESC&, GpuDeviceFlags)
		 * @param[in]	deviceMask		Mask that determines on which GPU devices should the object be created on.
		 */
		static SPtr<GpuProgram> create(const GPU_PROGRAM_DESC& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT);

		/** 
		 * Compiles the GPU program to an intermediate bytecode format. The bytecode can be cached and used for
		 * quicker compilation/creation of GPU programs.
		 */
		static SPtr<GpuProgramBytecode> compileBytecode(const GPU_PROGRAM_DESC& desc);

	protected:
		friend class GpuProgramRTTI;

		GpuProgram(const GPU_PROGRAM_DESC& desc, GpuDeviceFlags deviceMask);

		bool mNeedsAdjacencyInfo;

		bool mIsCompiled = false;
		String mCompileMessages;

		SPtr<GpuParamDesc> mParametersDesc;
		SPtr<VertexDeclaration> mInputDeclaration;

		GpuProgramType mType;
		String mEntryPoint;
		String mSource;

		SPtr<GpuProgramBytecode> mCachedBytecode;
	};

	/** @} */
	}
}
