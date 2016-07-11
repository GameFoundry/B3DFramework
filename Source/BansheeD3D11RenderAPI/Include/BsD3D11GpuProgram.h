//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsD3D11Prerequisites.h"
#include "BsGpuProgram.h"

namespace BansheeEngine
{
	/** @addtogroup D3D11
	 *  @{
	 */

	/**	Abstraction of a DirectX 11 shader object. */
	class BS_D3D11_EXPORT D3D11GpuProgramCore : public GpuProgramCore
	{
	public:
		virtual ~D3D11GpuProgramCore();

		/**	Returns compiled shader microcode. */
		const HLSLMicroCode& getMicroCode() const { return mMicrocode; }

		/**	Returns unique GPU program ID. */
		UINT32 getProgramId() const { return mProgramId; }

	protected:
		D3D11GpuProgramCore(const String& source, const String& entryPoint, GpuProgramType gptype,
			GpuProgramProfile profile, bool isAdjacencyInfoRequired);

		/** @copydoc GpuProgramCore::initialize */
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
	class BS_D3D11_EXPORT D3D11GpuVertexProgramCore : public D3D11GpuProgramCore
	{
	public:
		~D3D11GpuVertexProgramCore();

		/**	Returns internal DX11 vertex shader object. */
		ID3D11VertexShader* getVertexShader() const;

	protected:
		friend class D3D11HLSLProgramFactory;

		D3D11GpuVertexProgramCore(const String& source, const String& entryPoint,
			GpuProgramProfile profile);

		/** @copydoc D3D11GpuProgramCore::loadFromMicrocode */
		void loadFromMicrocode(D3D11Device& device, ID3D10Blob* microcode) override;

	protected:
		ID3D11VertexShader* mVertexShader;
	};

	/**	Implementation of a DX11 pixel shader. */
	class BS_D3D11_EXPORT D3D11GpuFragmentProgramCore : public D3D11GpuProgramCore
	{
	public:
		~D3D11GpuFragmentProgramCore();

		/**	Returns internal DX11 pixel shader object. */
		ID3D11PixelShader* getPixelShader() const;

	protected:
		friend class D3D11HLSLProgramFactory;

		D3D11GpuFragmentProgramCore(const String& source, const String& entryPoint,
			GpuProgramProfile profile);

		/** @copydoc D3D11GpuProgramCore::loadFromMicrocode */
		void loadFromMicrocode(D3D11Device& device, ID3D10Blob* microcode) override;

	protected:
		ID3D11PixelShader* mPixelShader;
	};

	/**	Implementation of a DX11 domain shader. */
	class BS_D3D11_EXPORT D3D11GpuDomainProgramCore : public D3D11GpuProgramCore
	{
	public:
		~D3D11GpuDomainProgramCore();

		/**	Returns internal DX11 domain shader object. */
		ID3D11DomainShader* getDomainShader() const;

	protected:
		friend class D3D11HLSLProgramFactory;

		D3D11GpuDomainProgramCore(const String& source, const String& entryPoint,
			GpuProgramProfile profile);

		/** @copydoc D3D11GpuProgramCore::loadFromMicrocode */
		void loadFromMicrocode(D3D11Device& device, ID3D10Blob* microcode) override;

	protected:
		ID3D11DomainShader* mDomainShader;
	};

	/**	Implementation of a DX11 hull shader. */
	class BS_D3D11_EXPORT D3D11GpuHullProgramCore : public D3D11GpuProgramCore
	{
	public:
		~D3D11GpuHullProgramCore();

		/**	Returns internal DX11 hull shader object. */
		ID3D11HullShader* getHullShader() const;

	protected:
		friend class D3D11HLSLProgramFactory;

		D3D11GpuHullProgramCore(const String& source, const String& entryPoint,
			GpuProgramProfile profile);

		/** @copydoc D3D11GpuProgramCore::loadFromMicrocode */
		void loadFromMicrocode(D3D11Device& device, ID3D10Blob* microcode) override;

	protected:
		ID3D11HullShader* mHullShader;
	};

	/**	Implementation of a DX11 geometry shader. */
	class BS_D3D11_EXPORT D3D11GpuGeometryProgramCore : public D3D11GpuProgramCore
	{
	public:
		~D3D11GpuGeometryProgramCore();

		/**	Returns internal DX11 geometry shader object. */
		ID3D11GeometryShader* getGeometryShader() const;

	protected:
		friend class D3D11HLSLProgramFactory;

		D3D11GpuGeometryProgramCore(const String& source, const String& entryPoint,
			GpuProgramProfile profile, bool isAdjacencyInfoRequired);

		/** @copydoc D3D11GpuProgramCore::loadFromMicrocode */
		void loadFromMicrocode(D3D11Device& device, ID3D10Blob* microcode) override;

	protected:
		ID3D11GeometryShader* mGeometryShader;
	};

	/**	Implementation of a DX11 compute shader. */
	class BS_D3D11_EXPORT D3D11GpuComputeProgramCore : public D3D11GpuProgramCore
	{
	public:
		~D3D11GpuComputeProgramCore();

		/**	Returns internal DX11 compute shader object. */
		ID3D11ComputeShader* getComputeShader() const;

	protected:
		friend class D3D11HLSLProgramFactory;

		D3D11GpuComputeProgramCore(const String& source, const String& entryPoint,
			GpuProgramProfile profile);

		/** @copydoc D3D11GpuProgramCore::loadFromMicrocode */
		void loadFromMicrocode(D3D11Device& device, ID3D10Blob* microcode) override;

	protected:
		ID3D11ComputeShader* mComputeShader;
	};

	/** @} */
}