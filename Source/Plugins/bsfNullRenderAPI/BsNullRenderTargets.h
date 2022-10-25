//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsNullPrerequisites.h"
#include "Managers/BsRenderWindowManager.h"
#include "RenderAPI/BsRenderTexture.h"

namespace bs
{
	namespace ct
	{
		class NullRenderWindow;
	}

	/** @addtogroup NullRenderAPI
	 *  @{
	 */

	/** @copydoc RenderWindowManager */
	class NullRenderWindowManager final : public RenderWindowManager
	{
	protected:
		/** @copydoc RenderWindowManager::createImpl */
		SPtr<RenderWindow> CreateImpl(RENDER_WINDOW_DESC& desc, u32 windowId, const SPtr<RenderWindow>& parentWindow);
	};

	/** Null implementation of a render texture. */
	class NullRenderTexture : public RenderTexture
	{
	public:
		NullRenderTexture(const RENDER_TEXTURE_DESC& desc)
			: RenderTexture(desc), mProperties(desc, false)
		{}

		virtual ~NullRenderTexture() = default;

	protected:
		/** @copydoc RenderTexture::getProperties */
		const RenderTargetProperties& GetPropertiesInternal() const { return mProperties; }

		RenderTextureProperties mProperties;
	};

	/** Render window implementation for null render backend. */
	class NullRenderWindow final : public RenderWindow
	{
	public:
		~NullRenderWindow() = default;

		/** @copydoc RenderWindow::screenToWindowPos */
		Vector2I ScreenToWindowPos(const Vector2I& screenPos) const { return screenPos; }

		/** @copydoc RenderWindow::windowToScreenPos */
		Vector2I WindowToScreenPos(const Vector2I& windowPos) const { return windowPos; }

		/** @copydoc RenderWindow::getCore */
		SPtr<ct::NullRenderWindow> GetCore() const;

		/** @copydoc RenderWindow::getCustomAttribute */
		void GetCustomAttribute(const String& name, void* pData) const;

	protected:
		friend class NullRenderWindowManager;
		friend class ct::NullRenderWindow;

		NullRenderWindow(const RENDER_WINDOW_DESC& desc, u32 windowId);

		/** @copydoc RenderWindow::getProperties */
		const RenderTargetProperties& GetPropertiesInternal() const override { return mProperties; }

		/** @copydoc RenderWindow::syncProperties */
		void SyncProperties() override;

		/** @copydoc RenderWindow::createCore */
		SPtr<ct::CoreObject> CreateCore() const;

	private:
		RenderWindowProperties mProperties;
	};

	namespace ct
	{
		/** Null implementation of a render texture. */
		class NullRenderTexture : public RenderTexture
		{
		public:
			NullRenderTexture(const RENDER_TEXTURE_DESC& desc, u32 deviceIdx)
				: RenderTexture(desc, deviceIdx), mProperties(desc, false)
			{}

			virtual ~NullRenderTexture() = default;

		protected:
			/** @copydoc RenderTexture::getProperties */
			const RenderTargetProperties& GetPropertiesInternal() const override { return mProperties; }

			RenderTextureProperties mProperties;
		};

		/** Render window implementation for null render backend. */
		class NullRenderWindow final : public RenderWindow
		{
		public:
			NullRenderWindow(const RENDER_WINDOW_DESC& desc, u32 windowId);

			/** @copydoc RenderWindow::move */
			void Move(i32 left, i32 top) override {}

			/** @copydoc RenderWindow::resize */
			void Resize(u32 width, u32 height) override {}

			/** @copydoc RenderWindow::setVSync */
			void SetVSync(bool enabled, u32 interval = 1) override {}

			/** @copydoc RenderWindow::getCustomAttribute */
			void GetCustomAttribute(const String& name, void* pData) const override;

		protected:
			friend class bs::NullRenderWindow;

			/** @copydoc RenderWindow::getProperties */
			const RenderTargetProperties& GetPropertiesInternal() const override { return mProperties; }

			/** @copydoc RenderWindow::getSyncedProperties */
			RenderWindowProperties& GetSyncedProperties() override { return mSyncedProperties; }

			/** @copydoc RenderWindow::syncProperties */
			void SyncProperties() override;

		protected:
			RenderWindowProperties mProperties;
			RenderWindowProperties mSyncedProperties;
		};
	} // namespace ct

	/** @} */
} // namespace bs
