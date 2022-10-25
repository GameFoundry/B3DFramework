//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsD3D11Prerequisites.h"
#include "RenderAPI/BsRenderWindow.h"

namespace bs
{
	/** @addtogroup D3D11
	 *  @{
	 */

	/**
	 * Render window implementation for Windows and DirectX 11.
	 *
	 * @note	Sim thread only.
	 */
	class D3D11RenderWindow : public RenderWindow
	{
	public:
		~D3D11RenderWindow() = default;

		/** @copydoc RenderWindow::screenToWindowPos */
		void GetCustomAttribute(const String& name, void* pData) const;

		/** @copydoc RenderWindow::screenToWindowPos */
		Vector2I ScreenToWindowPos(const Vector2I& screenPos) const;

		/** @copydoc RenderWindow::windowToScreenPos */
		Vector2I WindowToScreenPos(const Vector2I& windowPos) const;

		/** @copydoc RenderWindow::getCore */
		SPtr<ct::D3D11RenderWindow> GetCore() const;

	protected:
		friend class D3D11RenderWindowManager;
		friend class ct::D3D11RenderWindow;

		D3D11RenderWindow(const RENDER_WINDOW_DESC& desc, u32 windowId);

		/** @copydoc RenderWindow::getProperties */
		const RenderTargetProperties& GetPropertiesInternal() const { return mProperties; }

		/** @copydoc RenderWindow::syncProperties */
		void SyncProperties();

		/** @copydoc RenderWindow::createCore */
		SPtr<ct::CoreObject> CreateCore() const;

		/**	Retrieves internal window handle. */
		HWND GetHWnd() const;

	private:
		RenderWindowProperties mProperties;
	};

	namespace ct
	{
		/**
		 * Render window implementation for Windows and DirectX 11.
		 *
		 * @note	Core thread only.
		 */
		class D3D11RenderWindow : public RenderWindow
		{
		public:
			D3D11RenderWindow(const RENDER_WINDOW_DESC& desc, u32 windowId, D3D11Device& device, IDXGIFactory1* DXGIFactory);
			~D3D11RenderWindow();

			/** @copydoc RenderWindow::move */
			void Move(i32 left, i32 top);

			/** @copydoc RenderWindow::resize */
			void Resize(u32 width, u32 height);

			/** @copydoc RenderWindow::setHidden */
			void SetHidden(bool hidden);

			/** @copydoc RenderWindow::setActive */
			void SetActive(bool state);

			/** @copydoc RenderWindow::minimize */
			void Minimize();

			/** @copydoc RenderWindow::maximize */
			void Maximize();

			/** @copydoc RenderWindow::restore */
			void Restore();

			/** @copydoc RenderWindow::setFullscreen(u32, u32, float, u32) */
			void SetFullscreen(u32 width, u32 height, float refreshRate = 60.0f, u32 monitorIdx = 0);

			/** @copydoc RenderWindow::setFullscreen(const VideoMode&) */
			void SetFullscreen(const VideoMode& videoMode) override;

			/** @copydoc RenderWindow::setWindowed */
			void SetWindowed(u32 width, u32 height) override;

			/** @copydoc RenderWindow::setVSync */
			void SetVSync(bool enabled, u32 interval = 1) override;

			/**
			 * Copies the contents of a frame buffer into the pre-allocated buffer.
			 *
			 * @param[out]	dst		Previously allocated buffer to read the contents into. Must be of valid size.
			 * @param[in]	buffer	Frame buffer to read the contents from.
			 */
			void CopyToMemory(PixelData& dst, FrameBuffer buffer);

			/** @copydoc RenderWindow::swapBuffers */
			void SwapBuffers(u32 syncMask = 0xFFFFFFFF) override;

			/** @copydoc RenderWindow::getCustomAttribute */
			void GetCustomAttribute(const String& name, void* pData) const override;

			/** @copydoc RenderWindow::_windowMovedOrResized */
			void WindowMovedOrResizedInternal() override;

			/**	Returns presentation parameters used for creating the window swap chain. */
			DXGI_SWAP_CHAIN_DESC* GetPresentationParametersInternal() { return &mSwapChainDesc; }

			/**	Returns internal window handle. */
			HWND GetWindowHandleInternal() const;

		protected:
			friend class bs::D3D11RenderWindow;

			/** @copydoc CoreObject::initialize */
			void Initialize() override;

			/**	Creates internal resources dependent on window size. */
			void CreateSizeDependedD3DResources();

			/**	Destroys internal resources dependent on window size. */
			void DestroySizeDependedD3DResources();

			/**	Queries the current DXGI device. Make sure to release the returned object when done with it. */
			IDXGIDevice* QueryDxgiDevice();

			/**	Creates a swap chain for the window. */
			void CreateSwapChain();

			/**	Resizes all buffers attached to the swap chain to the specified size. */
			void ResizeSwapChainBuffers(u32 width, u32 height);

			/** @copydoc RenderWindow::getProperties */
			const RenderTargetProperties& GetPropertiesInternal() const override { return mProperties; }

			/** @copydoc RenderWindow::getSyncedProperties */
			RenderWindowProperties& GetSyncedProperties() override { return mSyncedProperties; }

			/** @copydoc RenderWindow::syncProperties */
			void SyncProperties() override;

		protected:
			D3D11Device& mDevice;
			IDXGIFactory1* mDXGIFactory;
			bool mSizing = false;
			bool mIsChild = false;
			bool mShowOnSwap = false;

			DXGI_SAMPLE_DESC mMultisampleType;
			u32 mRefreshRateNumerator = 0;
			u32 mRefreshRateDenominator = 0;

			ID3D11Texture2D* mBackBuffer = nullptr;
			ID3D11RenderTargetView* mRenderTargetView = nullptr;
			SPtr<TextureView> mDepthStencilView = nullptr;
			SPtr<Texture> mDepthStencilBuffer;

			IDXGISwapChain* mSwapChain = nullptr;
			DXGI_SWAP_CHAIN_DESC mSwapChainDesc;
			Win32Window* mWindow = nullptr;

			RenderWindowProperties mProperties;
			RenderWindowProperties mSyncedProperties;
		};

		/** @} */
	} // namespace ct
} // namespace bs
