//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsD3D11Prerequisites.h"

namespace bs
{
	/** @addtogroup D3D11
	 *  @{
	 */

	/**	Provides information about a driver (for example hardware GPU driver or software emulated). */
	class D3D11Driver
	{
	public:
		/** Constructs a new object from the adapter number provided by DX11 runtime, and DXGI adapter object. */
		D3D11Driver(UINT32 adapterNumber, IDXGIAdapter* dxgiAdapter);
		D3D11Driver(const D3D11Driver &ob);
		~D3D11Driver();

		D3D11Driver& operator=(const D3D11Driver& r);

		/**	Returns the name of the driver. */
		String getDriverName() const;

		/**	Returns the description of the driver. */
		String getDriverDescription() const;

		/**	Returns adapter index of the adapter the driver is managing. */
		UINT32 getAdapterNumber() const { return mAdapterNumber; }

		/**	Returns number of outputs connected to the adapter the driver is managing. */
		UINT32 getNumAdapterOutputs() const { return mNumOutputs; }

		/**	Returns a description of the adapter the driver is managing. */
		const DXGI_ADAPTER_DESC& getAdapterIdentifier() const { return mAdapterIdentifier; }

		/**	Returns internal DXGI adapter object for the driver. */
		IDXGIAdapter* getDeviceAdapter() const { return mDXGIAdapter; }

		/**	Returns description of an output device at the specified index. */
		DXGI_OUTPUT_DESC getOutputDesc(UINT32 adapterOutputIdx) const;

		/**	Returns a list of all available video modes for all output devices. */
		SPtr<VideoModeInfo> getVideoModeInfo() const { return mVideoModeInfo; }

	private:
		/**	Initializes the internal data. */
		void construct();

	private:
		UINT32 mAdapterNumber;
		UINT32 mNumOutputs;
		DXGI_ADAPTER_DESC mAdapterIdentifier;
		IDXGIAdapter* mDXGIAdapter;
		SPtr<VideoModeInfo> mVideoModeInfo;
	};

	/** @} */
}