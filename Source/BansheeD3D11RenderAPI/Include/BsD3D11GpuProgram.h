//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsD3D11Prerequisites.h"
#include "BsGpuProgram.h"

namespace bs { namespace ct
{
	/** @addtogroup D3D11
	 *  @{
	 */

	/**	Abstraction of a DirectX 11 shader object. */
	class D3D11GpuProgram : public GpuProgram
	{
	public:
		virtual ~D3D11GpuProgram();

		/**	Returns compiled shader microcode. */
		const HLSLMicroCode& getMicroCode() const { return mMicrocode; }

		/**	Returns unique GPU program ID. */
		UINT32 getProgramId() const { return mProgramId; }

	protected:
		D3D11GpuProgram(const GPU_PROGRAM_DESC& desc, GpuDeviceFlags deviceMask);

		/** @copydoc GpuProgram::initialize */
		void initialize() override;

		/**	Loads the shader from microcode. */
		virtual void loadFromMicrocode(D3D11Device& device, ID3D10Blob* microcode) = 0;

		/**	Compiles the shader from source and generates the microcode. */
		ID3DBlob* compileMicrocode(const String& profile);

		/**
		 * Reflects the microcode and extracts input/output parameters, and constant buffer structures used by the program.
		 */
		void populateParametersAndConstants(ID3DBlob* microcode);

		/** Parses compiler error message and returns the line number at which the error occurred. */
		UINT32 parseErrorMessage(const char* message);

	protected:
		static UINT32 GlobalProgramId;

		bool mEnableBackwardsCompatibility;
		UINT32 mProgramId;

		HLSLMicroCode mMicrocode;
	};

	/**	Implementation of a DX11 vertex shader. */
	class D3D11GpuVertexProgram : public D3D11GpuProgram
	{
	public:
		~D3D11GpuVertexProgram();

		/**	Returns internal DX11 vertex shader object. */
		ID3D11VertexShader* getVertexShader() const;

	protected:
		friend class D3D11HLSLProgramFactory;

		D3D11GpuVertexProgram(const GPU_PROGRAM_DESC& desc, GpuDeviceFlags deviceMask);

		/** @copydoc D3D11GpuProgram::loadFromMicrocode */
		void loadFromMicrocode(D3D11Device& device, ID3D10Blob* microcode) override;

	protected:
		ID3D11VertexShader* mVertexShader;
	};

	/**	Implementation of a DX11 pixel shader. */
	class D3D11GpuFragmentProgram : public D3D11GpuProgram
	{
	public:
		~D3D11GpuFragmentProgram();

		/**	Returns internal DX11 pixel shader object. */
		ID3D11PixelShader* getPixelShader() const;

	protected:
		friend class D3D11HLSLProgramFactory;

		D3D11GpuFragmentProgram(const GPU_PROGRAM_DESC& desc, GpuDeviceFlags deviceMask);

		/** @copydoc D3D11GpuProgram::loadFromMicrocode */
		void loadFromMicrocode(D3D11Device& device, ID3D10Blob* microcode) override;

	protected:
		ID3D11PixelShader* mPixelShader;
	};

	/**	Implementation of a DX11 domain shader. */
	class D3D11GpuDomainProgram : public D3D11GpuProgram
	{
	public:
		~D3D11GpuDomainProgram();

		/**	Returns internal DX11 domain shader object. */
		ID3D11DomainShader* getDomainShader() const;

	protected:
		friend class D3D11HLSLProgramFactory;

		D3D11GpuDomainProgram(const GPU_PROGRAM_DESC& desc, GpuDeviceFlags deviceMask);

		/** @copydoc D3D11GpuProgram::loadFromMicrocode */
		void loadFromMicrocode(D3D11Device& device, ID3D10Blob* microcode) override;

	protected:
		ID3D11DomainShader* mDomainShader;
	};

	/**	Implementation of a DX11 hull shader. */
	class D3D11GpuHullProgram : public D3D11GpuProgram
	{
	public:
		~D3D11GpuHullProgram();

		/**	Returns internal DX11 hull shader object. */
		ID3D11HullShader* getHullShader() const;

	protected:
		friend class D3D11HLSLProgramFactory;

		D3D11GpuHullProgram(const GPU_PROGRAM_DESC& desc, GpuDeviceFlags deviceMask);

		/** @copydoc D3D11GpuProgram::loadFromMicrocode */
		void loadFromMicrocode(D3D11Device& device, ID3D10Blob* microcode) override;

	protected:
		ID3D11HullShader* mHullShader;
	};

	/**	Implementation of a DX11 geometry shader. */
	class D3D11GpuGeometryProgram : public D3D11GpuProgram
	{
	public:
		~D3D11GpuGeometryProgram();

		/**	Returns internal DX11 geometry shader object. */
		ID3D11GeometryShader* getGeometryShader() const;

	protected:
		friend class D3D11HLSLProgramFactory;

		D3D11GpuGeometryProgram(const GPU_PROGRAM_DESC& desc, GpuDeviceFlags deviceMask);

		/** @copydoc D3D11GpuProgram::loadFromMicrocode */
		void loadFromMicrocode(D3D11Device& device, ID3D10Blob* microcode) override;

	protected:
		ID3D11GeometryShader* mGeometryShader;
	};

	/**	Implementation of a DX11 compute shader. */
	class D3D11GpuComputeProgram : public D3D11GpuProgram
	{
	public:
		~D3D11GpuComputeProgram();

		/**	Returns internal DX11 compute shader object. */
		ID3D11ComputeShader* getComputeShader() const;

	protected:
		friend class D3D11HLSLProgramFactory;

		D3D11GpuComputeProgram(const GPU_PROGRAM_DESC& desc, GpuDeviceFlags deviceMask);

		/** @copydoc D3D11GpuProgram::loadFromMicrocode */
		void loadFromMicrocode(D3D11Device& device, ID3D10Blob* microcode) override;

	protected:
		ID3D11ComputeShader* mComputeShader;
	};

	/** @} */
}}