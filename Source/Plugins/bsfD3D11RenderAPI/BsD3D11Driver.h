//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsD3D11Prerequisites.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup D3D11
		 *  @{
		 */

		/**	Provides information about a driver (for example hardware GPU driver or software emulated). */
		class D3D11Driver
		{
		public:
			/** Constructs a new object from the adapter number provided by DX11 runtime, and DXGI adapter object. */
			D3D11Driver(u32 adapterNumber, IDXGIAdapter* dxgiAdapter);
			D3D11Driver(const D3D11Driver& ob);
			~D3D11Driver();

			D3D11Driver& operator=(const D3D11Driver& r);

			/**	Returns the name of the driver. */
			String GetDriverName() const;

			/**	Returns the description of the driver. */
			String GetDriverDescription() const;

			/**	Returns adapter index of the adapter the driver is managing. */
			u32 GetAdapterNumber() const { return mAdapterNumber; }

			/**	Returns number of outputs connected to the adapter the driver is managing. */
			u32 GetNumAdapterOutputs() const { return mNumOutputs; }

			/**	Returns a description of the adapter the driver is managing. */
			const DXGI_ADAPTER_DESC& GetAdapterIdentifier() const { return mAdapterIdentifier; }

			/**	Returns internal DXGI adapter object for the driver. */
			IDXGIAdapter* GetDeviceAdapter() const { return mDXGIAdapter; }

			/**	Returns description of an output device at the specified index. */
			DXGI_OUTPUT_DESC GetOutputDesc(u32 adapterOutputIdx) const;

			/**	Returns a list of all available video modes for all output devices. */
			SPtr<VideoModeInfo> GetVideoModeInfo() const { return mVideoModeInfo; }

		private:
			/**	Initializes the internal data. */
			void Construct();

		private:
			u32 mAdapterNumber;
			u32 mNumOutputs;
			DXGI_ADAPTER_DESC mAdapterIdentifier;
			IDXGIAdapter* mDXGIAdapter;
			SPtr<VideoModeInfo> mVideoModeInfo;
		};

		/** @} */
	} // namespace ct
} // namespace bs
