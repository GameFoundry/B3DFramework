//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsD3D11Prerequisites.h"

namespace bs { namespace ct
{
	/** @addtogroup D3D11
	 *  @{
	 */

	/**	Available DX11 error levels. */
	enum BS_D3D11_ERROR_LEVEL
	{
		D3D11ERR_NO_EXCEPTION,
		D3D11ERR_CORRUPTION,
		D3D11ERR_ERROR,
		D3D11ERR_WARNING,
		D3D11ERR_INFO
	};

	/**	Wrapper around DirectX 11 device object. */
	class D3D11Device
	{
	public:
		/**	Constructs the object with a previously created DX11 device. */
		D3D11Device(ID3D11Device* device);
		~D3D11Device();

		/**	Shuts down the device any releases any internal resources. */
		void shutdown();

		/**	Returns DX11 immediate context object. */
		ID3D11DeviceContext* getImmediateContext() const { return mImmediateContext; }

		/**	Returns DX11 class linkage object. */
		ID3D11ClassLinkage* getClassLinkage() const { return mClassLinkage; }

		/**	Returns internal DX11 device. */
		ID3D11Device* getD3D11Device() const { return mD3D11Device; }		

		/** Returns information about optional features supported by the device. */
		const D3D11_FEATURE_DATA_D3D11_OPTIONS& getFeatureOptions() const { return mD3D11FeatureOptions; }

		/**	Resets error state & error messages. */
		void clearErrors();

		/**
		 * Query if error occurred at any point since last clearErrors() call. Use getErrorDescription to get a string
		 * describing the error.
		 */
		bool hasError() const;

		/**	Returns a string describing an error if one occurred. */
		String getErrorDescription(bool clearErrors = true);
		
		/**
		 * Sets the level for which we want to receive errors for. Errors will be reported for the provided level and any
		 * higher priority level.
		 */
		void setExceptionsErrorLevel(const BS_D3D11_ERROR_LEVEL exceptionsErrorLevel);

	private:
		D3D11Device() = default;

		ID3D11Device* mD3D11Device = nullptr;
		ID3D11DeviceContext* mImmediateContext = nullptr;
		ID3D11InfoQueue* mInfoQueue = nullptr; 
		ID3D11ClassLinkage* mClassLinkage = nullptr;
		D3D11_FEATURE_DATA_D3D11_OPTIONS mD3D11FeatureOptions;
	};

	/** @} */
}}