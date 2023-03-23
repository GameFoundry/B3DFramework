//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "CoreThread/BsCoreObject.h"
#include "Reflection/BsIReflectable.h"
#include "Utility/BsDataBlob.h"
#include "RenderAPI/BsVertexDescription.h"

namespace bs
{
	/** @addtogroup RenderAPI
	 *  @{
	 */

	struct GpuProgramBytecode;

	/** Descriptor structure used for initialization of a GpuProgram. */
	struct B3D_CORE_EXPORT GpuProgramCreateInformation : public IReflectable
	{
		String Name; /**< Name of the program. Used primarily for debugging. */
		String Source; /**< Source code to compile the program from. */
		String EntryPoint; /**< Name of the entry point function, for example "main". */
		String Language; /**< Language the source is written in, for example "hlsl" or "glsl". */
		GpuProgramType Type = GPT_VERTEX_PROGRAM; /**< Type of the program, for example vertex or fragment. */
		bool RequiresAdjacency = false; /**< If true then adjacency information will be provided when rendering. */

		/**
		 * Optional intermediate version of the GPU program. Can significantly speed up GPU program compilation/creation
		 * when supported by the render backend. Call ct::GpuProgram::CompileBytecode to generate it.
		 */
		SPtr<GpuProgramBytecode> Bytecode;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
		friend class GpuProgramCreateInformationRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;
	};

	/**
	 * Contains a GPU program such as vertex or fragment program which gets compiled from the provided source code.
	 *
	 * @note	Sim thread only.
	 */
	class B3D_CORE_EXPORT GpuProgram : public IReflectable, public CoreObject
	{
	public:
		/** Information returned when compiling a GPU program. */
		struct CompileStatus
		{
			bool Success = false;
			String Messages;
		};

		virtual ~GpuProgram() = default;

		/**
		 * Returns true if the program was successfully compiled.
		 *
		 * @note	Only valid after core thread has initialized the program.
		 */
		bool IsCompiled() const;

		/**
		 * Returns an error message returned by the compiler, if the compilation failed.
		 *
		 * @note	Only valid after core thread has initialized the program.
		 */
		String GetCompileErrorMessage() const;

		/**
		 * Returns description of all parameters in this GPU program.
		 *
		 * @note	Only valid after core thread has initialized the program.
		 */
		SPtr<GpuParamDesc> GetParamDesc() const;

		/** Retrieves a core implementation of a gpu program usable only from the core thread. */
		SPtr<ct::GpuProgram> GetCore() const;

		/**
		 * Creates a new GPU program using the provided source code. If compilation fails or program is not supported
		 * isCompiled() with return false, and you will be able to retrieve the error message via getCompileErrorMessage().
		 *
		 * @param[in]	desc		Description of the program to create.
		 */
		static SPtr<GpuProgram> Create(const GpuProgramCreateInformation& desc);

	protected:
		friend class GpuProgramManager;

		GpuProgram(const GpuProgramCreateInformation& createInformation);

		SPtr<ct::CoreObject> CreateCore() const override;

	protected:
		bool mNeedsAdjacencyInfo;
		String mLanguage;
		String mName;
		GpuProgramType mType;
		String mEntryPoint;
		String mSource;

		SPtr<GpuProgramBytecode> mBytecode;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class GpuProgramRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;
	};

	/**
	 * A GPU program compiled to an intermediate bytecode format, as well as any relevant meta-data that could be
	 * extracted from that format.
	 */
	struct B3D_CORE_EXPORT GpuProgramBytecode : IReflectable
	{
		~GpuProgramBytecode();

		/** Instructions (compiled code) for the GPU program. Contains no data if compilation was not succesful. */
		DataBlob Instructions;

		/** Reflected information about GPU program parameters. */
		SPtr<GpuParamDesc> ParamDesc;

		/** Input parameters for a vertex GPU program. */
		Vector<VertexElement> VertexInput;

		/** Messages output during the compilation process. Includes errors in case compilation failed. */
		String Messages;

		/** Identifier of the compiler that compiled the bytecode. */
		String CompilerId;

		/** Version of the compiler that compiled the bytecode. */
		u32 CompilerVersion = 0;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class GpuProgramBytecodeRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;
	};

	/** @} */

	class CoreGpuProgramRTTI;

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
		class B3D_CORE_EXPORT GpuProgram : public CoreObject
		{
		public:
			virtual ~GpuProgram();

			/** Assigns an name to the image, primarily used for easier debugging. */
			virtual void SetName(const StringView& name) { mName = name; }

			/** Returns whether this program can be supported on the current renderer and hardware. */
			virtual bool IsSupported() const;

			/** Returns true if program was successfully compiled. */
			virtual bool IsCompiled() const { return mIsCompiled; }

			/**	Returns an error message returned by the compiler, if the compilation failed. */
			virtual String GetCompileErrorMessage() const { return mCompileMessages; }

			/**
			 * Sets whether this geometry program requires adjacency information from the input primitives.
			 *
			 * @note	Only relevant for geometry programs.
			 */
			virtual void SetAdjacencyInfoRequired(bool required) { mNeedsAdjacencyInfo = required; }

			/**
			 * Returns whether this geometry program requires adjacency information from the input primitives.
			 *
			 * @note	Only relevant for geometry programs.
			 */
			virtual bool IsAdjacencyInfoRequired() const { return mNeedsAdjacencyInfo; }

			/**	Type of GPU program (for example fragment, vertex). */
			GpuProgramType GetType() const { return mType; }

			/** @copydoc bs::GpuProgram::GetParamDesc */
			SPtr<GpuParamDesc> GetParamDesc() const { return mParametersDesc; }

			/**	Returns a list of vertex elements that a vertex program expects as inputs. Only relevant for vertex programs. */
			SPtr<VertexDescription> GetVertexInputDescription() const { return mVertexInputDescription; }

			/** Returns the compiled bytecode of this program. */
			SPtr<GpuProgramBytecode> GetBytecode() const { return mBytecode; }

			/**
			 * @copydoc bs::GpuProgram::Create(const GPU_PROGRAM_DESC&)
			 * @param[in]	deviceMask		Mask that determines on which GPU devices should the object be created on.
			 */
			static SPtr<GpuProgram> Create(const GpuProgramCreateInformation& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT);

			/**
			 * Compiles the GPU program to an intermediate bytecode format. The bytecode can be cached and used for
			 * quicker compilation/creation of GPU programs.
			 */
			static SPtr<GpuProgramBytecode> CompileBytecode(const GpuProgramCreateInformation& desc);

		protected:
			GpuProgram(const GpuProgramCreateInformation& desc, GpuDeviceFlags deviceMask);

			bool mNeedsAdjacencyInfo;

			bool mIsCompiled = false;
			String mCompileMessages;

			SPtr<GpuParamDesc> mParametersDesc;
			SPtr<VertexDescription> mVertexInputDescription;

			GpuProgramType mType;
			String mName;
			String mEntryPoint;
			String mSource;

			SPtr<GpuProgramBytecode> mBytecode;
		};

		/** @} */
	} // namespace ct
} // namespace bs
