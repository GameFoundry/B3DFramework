//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsCoreObject.h"
#include "BsIReflectable.h"

namespace BansheeEngine 
{
	/** @addtogroup RenderAPI
	 *  @{
	 */

	/**	GPU program profiles representing supported feature sets. */
	enum GpuProgramProfile
	{
		GPP_NONE, /**< No profile. */
		GPP_FS_1_1, /**< Fragment program 1.1 profile. */
		GPP_FS_1_2, /**< Fragment program 1.2 profile. */
		GPP_FS_1_3, /**< Fragment program 1.3 profile. */
		GPP_FS_1_4, /**< Fragment program 1.4 profile. */
		GPP_FS_2_0, /**< Fragment program 2.0 profile. */
		GPP_FS_2_x, /**< Fragment program 2.x profile. */
		GPP_FS_2_a, /**< Fragment program 2.a profile. */
		GPP_FS_2_b, /**< Fragment program 2.b profile. */
		GPP_FS_3_0, /**< Fragment program 3.0 profile. */
		GPP_FS_3_x, /**< Fragment program 3.x profile. */
		GPP_FS_4_0, /**< Fragment program 4.0 profile. */
		GPP_FS_4_1, /**< Fragment program 4.1 profile. */
		GPP_FS_5_0, /**< Fragment program 5.0 profile. */
		GPP_VS_1_1, /**< Vertex program 1.1 profile. */
		GPP_VS_2_0, /**< Vertex program 2.0 profile. */
		GPP_VS_2_x, /**< Vertex program 2.x profile. */
		GPP_VS_2_a, /**< Vertex program 2.a profile. */
		GPP_VS_3_0, /**< Vertex program 3.0 profile. */
		GPP_VS_4_0, /**< Vertex program 4.0 profile. */
		GPP_VS_4_1, /**< Vertex program 4.1 profile. */
		GPP_VS_5_0, /**< Vertex program 5.0 profile. */
		GPP_GS_4_0, /**< Geometry program 4.0 profile. */
		GPP_GS_4_1, /**< Geometry program 4.1 profile. */
		GPP_GS_5_0, /**< Geometry program 5.0 profile. */
		GPP_HS_5_0, /**< Hull program 5.0 profile. */
		GPP_DS_5_0, /**< Domain program 5.0 profile. */
		GPP_CS_5_0 /**< Compute program 5.0 profile. */
	};

	/** Descriptor structure used for initialization of a GpuProgram. */
	struct GPU_PROGRAM_DESC
	{
		String source; /**< Source code to compile the program from. */
		String entryPoint; /**< Name of the entry point function, for example "main". */
		String language; /**< Language the source is written in, for example "hlsl" or "glsl". */
		GpuProgramType type = GPT_VERTEX_PROGRAM; /**< Type of the program, for example vertex or fragment. */
		GpuProgramProfile profile = GPP_NONE; /**< Program profile specifying supported feature-set. Must match the type. */
		bool requiresAdjacency = false; /**< If true then adjacency information will be provided when rendering. */
	};

	/** Data describing a GpuProgram. */
	class BS_CORE_EXPORT GpuProgramProperties
	{
	public:
		GpuProgramProperties(const String& source, const String& entryPoint,
			GpuProgramType gptype, GpuProgramProfile profile);

		virtual ~GpuProgramProperties() { }

        /** Source used for creating this program. */
        const String& getSource() const { return mSource; }
        
		/**	Type of GPU program (for example fragment, vertex). */
        GpuProgramType getType() const { return mType; }

		/**	Profile of the GPU program (for example VS_4_0, VS_5_0). */
		GpuProgramProfile getProfile() const { return mProfile; }

		/**	Name of the program entry method (for example "main"). */
		const String& getEntryPoint() const { return mEntryPoint; }

	protected:
		friend class GpuProgramRTTI;

		GpuProgramType mType;
		String mEntryPoint;
		GpuProgramProfile mProfile;
		String mSource;
	};

	/**
	 * Contains a GPU program such as vertex or fragment program which gets compiled from the provided source code.
	 *
	 * @note	Sim thread only.
	 */
	class BS_CORE_EXPORT GpuProgram : public IReflectable, public CoreObject
	{
	public:
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
		SPtr<GpuProgramCore> getCore() const;

		/** Returns properties that contain information about the GPU program. */
		const GpuProgramProperties& getProperties() const { return mProperties; }

		/**
		 * Creates a new GPU program using the provided source code. If compilation fails or program is not supported
		 * isCompiled() with return false, and you will be able to retrieve the error message via getCompileErrorMessage().
		 *
		 * @param[in]	desc				Description of the program to create.
		 */
		static SPtr<GpuProgram> create(const GPU_PROGRAM_DESC& desc);

	protected:
		friend class GpuProgramManager;

		GpuProgram(const GPU_PROGRAM_DESC& desc);

		/** @copydoc CoreObject::createCore */
		SPtr<CoreObjectCore> createCore() const override;

	protected:
		bool mNeedsAdjacencyInfo;
		String mLanguage;
		GpuProgramProperties mProperties;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class GpuProgramRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** @} */

	/** @addtogroup RenderAPI-Internal
	 *  @{
	 */

	/**
	 * Core thread version of a GpuProgram.
	 *
	 * @note	Core thread only.
	 */
	class BS_CORE_EXPORT GpuProgramCore : public CoreObjectCore
	{
	public:
		virtual ~GpuProgramCore() { }

		/** Returns whether this program can be supported on the current renderer and hardware. */
        virtual bool isSupported() const;

		/** Returns true if program was successfully compiled. */
		virtual bool isCompiled() const { return mIsCompiled; }

		/**	Returns an error message returned by the compiler, if the compilation failed. */
		virtual String getCompileErrorMessage() const { return mCompileError; }

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

		/** @copydoc GpuProgram::getParamDesc */
		SPtr<GpuParamDesc> getParamDesc() const { return mParametersDesc; }

		/**	Returns GPU program input declaration. Only relevant for vertex programs. */
		SPtr<VertexDeclarationCore> getInputDeclaration() const { return mInputDeclaration; }

		/**	Returns properties that contain information about the GPU program. */
		const GpuProgramProperties& getProperties() const { return mProperties; }

		/** 
		 * @copydoc GpuProgram::create 
		 * @param[in]	deviceMask		Mask that determines on which GPU devices should the object be created on.
		 */
		static SPtr<GpuProgramCore> create(const GPU_PROGRAM_DESC& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT);

	protected:
		GpuProgramCore(const GPU_PROGRAM_DESC& desc, GpuDeviceFlags deviceMask);

		/** Returns whether required capabilities for this program is supported. */
		bool isRequiredCapabilitiesSupported() const;

		bool mNeedsAdjacencyInfo;

		bool mIsCompiled;
		String mCompileError;

		SPtr<GpuParamDesc> mParametersDesc;
		SPtr<VertexDeclarationCore> mInputDeclaration;
		GpuProgramProperties mProperties;
	};

	/** @} */
}
