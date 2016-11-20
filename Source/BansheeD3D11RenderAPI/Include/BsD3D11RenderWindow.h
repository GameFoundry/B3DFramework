//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsD3D11Prerequisites.h"
#include "BsRenderWindow.h"

namespace bs
{
	/** @addtogroup D3D11
	 *  @{
	 */

	class D3D11RenderWindow;

	/**	Contains various properties that describe a render window. */
	class BS_D3D11_EXPORT D3D11RenderWindowProperties : public RenderWindowProperties
	{
	public:
		D3D11RenderWindowProperties(const RENDER_WINDOW_DESC& desc);
		virtual ~D3D11RenderWindowProperties() { }

	private:
		friend class D3D11RenderWindowCore;
		friend class D3D11RenderWindow;
	};

	/**
	 * Render window implementation for Windows and DirectX 11.
	 *
	 * @note	Core thread only.
	 */
	class BS_D3D11_EXPORT D3D11RenderWindowCore : public RenderWindowCore
	{
	public:
		D3D11RenderWindowCore(const RENDER_WINDOW_DESC& desc, UINT32 windowId,
			D3D11Device& device, IDXGIFactory* DXGIFactory);

		~D3D11RenderWindowCore();

		/** @copydoc RenderWindowCore::move */
		void move(INT32 left, INT32 top) override;

		/** @copydoc RenderWindowCore::resize */
		void resize(UINT32 width, UINT32 height) override;

		/** @copydoc RenderWindowCore::setHidden */
		void setHidden(bool hidden) override;

		/** @copydoc RenderWindowCore::setActive */
		void setActive(bool state) override;

		/** @copydoc RenderWindowCore::minimize */
		void minimize() override;

		/** @copydoc RenderWindowCore::maximize */
		void maximize() override;

		/** @copydoc RenderWindowCore::restore */
		void restore() override;

		/** @copydoc RenderWindowCore::setFullscreen(UINT32, UINT32, float, UINT32) */
		void setFullscreen(UINT32 width, UINT32 height, float refreshRate = 60.0f, UINT32 monitorIdx = 0) override;

		/** @copydoc RenderWindowCore::setFullscreen(const VideoMode&) */
		void setFullscreen(const VideoMode& videoMode) override;

		/** @copydoc RenderWindowCore::setWindowed */
		void setWindowed(UINT32 width, UINT32 height) override;

		/** 
		 * Copies the contents of a frame buffer into the pre-allocated buffer. 
		 *
		 * @param[out]	dst		Previously allocated buffer to read the contents into. Must be of valid size.
		 * @param[in]	buffer	Frame buffer to read the contents from.
		 */
		void copyToMemory(PixelData &dst, FrameBuffer buffer);

		/** @copydoc RenderWindowCore::swapBuffers */
		void swapBuffers(UINT32 syncMask = 0xFFFFFFFF) override;

		/** @copydoc RenderWindowCore::getCustomAttribute */
		void getCustomAttribute(const String& name, void* pData) const override;

		/** @copydoc RenderWindowCore::_windowMovedOrResized */
		void _windowMovedOrResized() override;

		/**	Returns presentation parameters used for creating the window swap chain. */
		DXGI_SWAP_CHAIN_DESC* _getPresentationParameters() { return &mSwapChainDesc; }

		/**	Returns internal window handle. */
		HWND _getWindowHandle() const;

	protected:
		friend class D3D11RenderWindow;

		/** @copydoc CoreObjectCore::initialize */
		void initialize() override;

		/**	Creates internal resources dependent on window size. */
		void createSizeDependedD3DResources();

		/**	Destroys internal resources dependent on window size. */
		void destroySizeDependedD3DResources();

		/**	Queries the current DXGI device. Make sure to release the returned object when done with it. */
		IDXGIDevice* queryDxgiDevice(); 

		/**	Creates a swap chain for the window. */
		void createSwapChain();

		/**	Resizes all buffers attached to the swap chain to the specified size. */
		void resizeSwapChainBuffers(UINT32 width, UINT32 height);

		/** @copydoc RenderWindowCore::getProperties */
		const RenderTargetProperties& getPropertiesInternal() const override { return mProperties; }

		/** @copydoc RenderWindowCore::getSyncedProperties */
		RenderWindowProperties& getSyncedProperties() override { return mSyncedProperties; }

		/** @copydoc RenderWindowCore::syncProperties */
		void syncProperties() override;

	protected:
		D3D11Device& mDevice;
		IDXGIFactory* mDXGIFactory;
		bool mSizing;
		bool mIsChild;
		bool mShowOnSwap;

		DXGI_SAMPLE_DESC mMultisampleType;
		UINT32 mRefreshRateNumerator;
		UINT32 mRefreshRateDenominator;

		ID3D11Texture2D* mBackBuffer;
		ID3D11RenderTargetView*	mRenderTargetView;
		SPtr<TextureView> mDepthStencilView;
		SPtr<TextureCore> mDepthStencilBuffer;

		IDXGISwapChain*	mSwapChain;
		DXGI_SWAP_CHAIN_DESC mSwapChainDesc;
		Win32Window* mWindow;

		D3D11RenderWindowProperties mProperties;
		D3D11RenderWindowProperties mSyncedProperties;
	};

	/**
	 * Render window implementation for Windows and DirectX 11.
	 *
	 * @note	Sim thread only.
	 */
	class BS_D3D11_EXPORT D3D11RenderWindow : public RenderWindow
	{
	public:
		~D3D11RenderWindow() { }

		/** @copydoc RenderWindow::screenToWindowPos */
		void getCustomAttribute(const String& name, void* pData) const override;

		/** @copydoc RenderWindow::screenToWindowPos */
		Vector2I screenToWindowPos(const Vector2I& screenPos) const override;

		/** @copydoc RenderWindow::windowToScreenPos */
		Vector2I windowToScreenPos(const Vector2I& windowPos) const override;

		/** @copydoc RenderWindow::getCore */
		SPtr<D3D11RenderWindowCore> getCore() const;

	protected:
		friend class D3D11RenderWindowManager;
		friend class D3D11RenderWindowCore;

		D3D11RenderWindow(const RENDER_WINDOW_DESC& desc, UINT32 windowId, 
			D3D11Device& device, IDXGIFactory* DXGIFactory);

		/** @copydoc RenderWindowCore::getProperties */
		const RenderTargetProperties& getPropertiesInternal() const override { return mProperties; }

		/** @copydoc RenderWindow::syncProperties */
		void syncProperties() override;

		/**	Retrieves internal window handle. */
		HWND getHWnd() const;

	private:
		D3D11Device& mDevice;
		IDXGIFactory* mDXGIFactory;
		D3D11RenderWindowProperties mProperties;
	};
	
	/** @} */
}