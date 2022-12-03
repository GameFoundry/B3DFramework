//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsD3D11Prerequisites.h"
#include "RenderAPI/BsGpuProgram.h"

namespace bs
{
	namespace ct
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
			const DataBlob& GetMicroCode() const { return mBytecode->Instructions; }

			/**	Returns unique GPU program ID. */
			u32 GetProgramId() const { return mProgramId; }

		protected:
			D3D11GpuProgram(const GpuProgramCreateInformation& desc, GpuDeviceFlags deviceMask);

			void Initialize() override;

			/**	Loads the shader from microcode. */
			virtual void LoadFromMicrocode(D3D11Device& device, const DataBlob& microcode) = 0;

		protected:
			static u32 GlobalProgramId;

			u32 mProgramId = 0;
		};

		/**	Implementation of a DX11 vertex shader. */
		class D3D11GpuVertexProgram : public D3D11GpuProgram
		{
		public:
			~D3D11GpuVertexProgram();

			/**	Returns internal DX11 vertex shader object. */
			ID3D11VertexShader* GetVertexShader() const;

		protected:
			friend class D3D11HLSLProgramFactory;

			D3D11GpuVertexProgram(const GpuProgramCreateInformation& desc, GpuDeviceFlags deviceMask);

			void LoadFromMicrocode(D3D11Device& device, const DataBlob& microcode) override;

		protected:
			ID3D11VertexShader* mVertexShader;
		};

		/**	Implementation of a DX11 pixel shader. */
		class D3D11GpuFragmentProgram : public D3D11GpuProgram
		{
		public:
			~D3D11GpuFragmentProgram();

			/**	Returns internal DX11 pixel shader object. */
			ID3D11PixelShader* GetPixelShader() const;

		protected:
			friend class D3D11HLSLProgramFactory;

			D3D11GpuFragmentProgram(const GpuProgramCreateInformation& desc, GpuDeviceFlags deviceMask);

			void LoadFromMicrocode(D3D11Device& device, const DataBlob& microcode) override;

		protected:
			ID3D11PixelShader* mPixelShader;
		};

		/**	Implementation of a DX11 domain shader. */
		class D3D11GpuDomainProgram : public D3D11GpuProgram
		{
		public:
			~D3D11GpuDomainProgram();

			/**	Returns internal DX11 domain shader object. */
			ID3D11DomainShader* GetDomainShader() const;

		protected:
			friend class D3D11HLSLProgramFactory;

			D3D11GpuDomainProgram(const GpuProgramCreateInformation& desc, GpuDeviceFlags deviceMask);

			void LoadFromMicrocode(D3D11Device& device, const DataBlob& microcode) override;

		protected:
			ID3D11DomainShader* mDomainShader;
		};

		/**	Implementation of a DX11 hull shader. */
		class D3D11GpuHullProgram : public D3D11GpuProgram
		{
		public:
			~D3D11GpuHullProgram();

			/**	Returns internal DX11 hull shader object. */
			ID3D11HullShader* GetHullShader() const;

		protected:
			friend class D3D11HLSLProgramFactory;

			D3D11GpuHullProgram(const GpuProgramCreateInformation& desc, GpuDeviceFlags deviceMask);

			void LoadFromMicrocode(D3D11Device& device, const DataBlob& microcode) override;

		protected:
			ID3D11HullShader* mHullShader;
		};

		/**	Implementation of a DX11 geometry shader. */
		class D3D11GpuGeometryProgram : public D3D11GpuProgram
		{
		public:
			~D3D11GpuGeometryProgram();

			/**	Returns internal DX11 geometry shader object. */
			ID3D11GeometryShader* GetGeometryShader() const;

		protected:
			friend class D3D11HLSLProgramFactory;

			D3D11GpuGeometryProgram(const GpuProgramCreateInformation& desc, GpuDeviceFlags deviceMask);

			void LoadFromMicrocode(D3D11Device& device, const DataBlob& microcode) override;

		protected:
			ID3D11GeometryShader* mGeometryShader;
		};

		/**	Implementation of a DX11 compute shader. */
		class D3D11GpuComputeProgram : public D3D11GpuProgram
		{
		public:
			~D3D11GpuComputeProgram();

			/**	Returns internal DX11 compute shader object. */
			ID3D11ComputeShader* GetComputeShader() const;

		protected:
			friend class D3D11HLSLProgramFactory;

			D3D11GpuComputeProgram(const GpuProgramCreateInformation& desc, GpuDeviceFlags deviceMask);

			void LoadFromMicrocode(D3D11Device& device, const DataBlob& microcode) override;

		protected:
			ID3D11ComputeShader* mComputeShader;
		};

		/** Identifier of the compiler used for compiling DirectX 11 GPU programs. */
		static constexpr const char* DIRECTX_COMPILER_ID = "DirectX11";

		/** @} */
	} // namespace ct
} // namespace bs
