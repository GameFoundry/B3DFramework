//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DIReflectable.h"
#include "Utility/B3DDataBlob.h"
#include "RenderAPI/B3DVertexDescription.h"

namespace b3d
{
	/** @addtogroup RenderAPI
	 *  @{
	 */

	struct GpuProgramBytecode;

	/** Descriptor structure used for initialization of a GpuProgram. */
	struct B3D_EXPORT GpuProgramCreateInformation : public IReflectable
	{
		String Name; /**< Name of the program. Used primarily for debugging. */
		String Source; /**< Source code to compile the program from. */
		String EntryPoint; /**< Name of the entry point function, for example "main". */
		String Language; /**< Language the source is written in, for example "hlsl" or "glsl". */
		GpuProgramType Type = GPT_VERTEX_PROGRAM; /**< Type of the program, for example vertex or fragment. */
		bool RequiresAdjacency = false; /**< If true then adjacency information will be provided when rendering. */

		/**
		 * Optional intermediate version of the GPU program. Can significantly speed up GPU program compilation/creation
		 * when supported by the render backend. Call render::GpuProgram::CompileBytecode to generate it.
		 */
		SPtr<GpuProgramBytecode> Bytecode;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
		friend class GpuProgramCreateInformationRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/**
	 * A GPU program compiled to an intermediate bytecode format, as well as any relevant meta-data that could be
	 * extracted from that format.
	 */
	struct B3D_EXPORT GpuProgramBytecode : IReflectable
	{
		~GpuProgramBytecode();

		/** Instructions (compiled code) for the GPU program. Contains no data if compilation was not succesful. */
		DataBlob Instructions;

		/** Reflected information about GPU program parameters. */
		SPtr<GpuProgramParameterDescription> ParameterDescription;

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
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/**
	 * Contains a GPU program such as vertex or fragment program which gets compiled from the provided source code.
	 *
	 * @note	Thread safe (Immutable).
	 */
	class B3D_EXPORT GpuProgram : public IReflectable
	{
	public:
		virtual ~GpuProgram();

		/** Initializes the object. The object should not be used before this is called. */
		virtual void Initialize() { }

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

		/** Returns description of all parameters in this GPU program. */
		SPtr<GpuProgramParameterDescription> GetParameterDescription() const { return mParametersDescription; }

		/**	Returns a list of vertex elements that a vertex program expects as inputs. Only relevant for vertex programs. */
		SPtr<VertexDescription> GetVertexInputDescription() const { return mVertexInputDescription; }

		/** Returns the compiled bytecode of this program. */
		SPtr<GpuProgramBytecode> GetBytecode() const { return mBytecode; }

	protected:
		GpuProgram(const GpuProgramCreateInformation& createInformation);

		bool mNeedsAdjacencyInfo;

		bool mIsCompiled = false;
		String mCompileMessages;

		SPtr<GpuProgramParameterDescription> mParametersDescription;
		SPtr<VertexDescription> mVertexInputDescription;

		GpuProgramType mType;
		String mLanguage;
		String mName;
		String mEntryPoint;
		String mSource;

		SPtr<GpuProgramBytecode> mBytecode;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class GpuProgramRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** @} */
} // namespace b3d
