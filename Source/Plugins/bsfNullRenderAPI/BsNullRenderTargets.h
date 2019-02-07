//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsNullPrerequisites.h"
#include "Managers/BsRenderWindowManager.h"
#include "RenderAPI/BsRenderTexture.h"

namespace bs
{
	namespace ct { class NullRenderWindow; }

	/** @addtogroup NullRenderAPI
	 *  @{
	 */

	/** @copydoc RenderWindowManager */
	class NullRenderWindowManager final : public RenderWindowManager
	{
	protected:
		/** @copydoc RenderWindowManager::createImpl */
		SPtr<RenderWindow> createImpl(RENDER_WINDOW_DESC& desc, UINT32 windowId, 
			const SPtr<RenderWindow>& parentWindow) override;
	};

	/** Null implementation of a render texture. */
	class NullRenderTexture : public RenderTexture
	{
	public:
		NullRenderTexture(const RENDER_TEXTURE_DESC& desc)
			:RenderTexture(desc), mProperties(desc, false)
		{ }

		virtual ~NullRenderTexture() = default;

	protected:
		/** @copydoc RenderTexture::getProperties */
		const RenderTargetProperties& getPropertiesInternal() const override { return mProperties; }

		RenderTextureProperties mProperties;	
	};

	/** Render window implementation for null render backend. */
	class NullRenderWindow final : public RenderWindow
	{
	public:
		~NullRenderWindow() = default;

		/** @copydoc RenderWindow::screenToWindowPos */
		Vector2I screenToWindowPos(const Vector2I& screenPos) const override { return screenPos; }

		/** @copydoc RenderWindow::windowToScreenPos */
		Vector2I windowToScreenPos(const Vector2I& windowPos) const override { return windowPos; }

		/** @copydoc RenderWindow::getCore */
		SPtr<ct::NullRenderWindow> getCore() const;

		/** @copydoc RenderWindow::getCustomAttribute */
		void getCustomAttribute(const String& name, void* pData) const override;


	protected:
		friend class NullRenderWindowManager;
		friend class ct::NullRenderWindow;

		NullRenderWindow(const RENDER_WINDOW_DESC& desc, UINT32 windowId);

		/** @copydoc RenderWindow::getProperties */
		const RenderTargetProperties& getPropertiesInternal() const override { return mProperties; }

		/** @copydoc RenderWindow::syncProperties */
		void syncProperties() override;

		/** @copydoc RenderWindow::createCore */
		SPtr<ct::CoreObject> createCore() const override;

	private:
		RenderWindowProperties mProperties;
	};

	namespace ct
	{
		/** Null implementation of a render texture. */
		class NullRenderTexture : public RenderTexture
		{
		public:
			NullRenderTexture(const RENDER_TEXTURE_DESC& desc, UINT32 deviceIdx)
				:RenderTexture(desc, deviceIdx), mProperties(desc, false)
			{ }

			virtual ~NullRenderTexture() = default;

		protected:
			/** @copydoc RenderTexture::getProperties */
			const RenderTargetProperties& getPropertiesInternal() const override { return mProperties; }

			RenderTextureProperties mProperties;	
		};

		/** Render window implementation for null render backend. */
		class NullRenderWindow final : public RenderWindow
		{
		public:
			NullRenderWindow(const RENDER_WINDOW_DESC& desc, UINT32 windowId);

			/** @copydoc RenderWindow::move */
			void move(INT32 left, INT32 top) override { }

			/** @copydoc RenderWindow::resize */
			void resize(UINT32 width, UINT32 height) override { }

			/** @copydoc RenderWindow::setVSync */
			void setVSync(bool enabled, UINT32 interval = 1) override { }

			/** @copydoc RenderWindow::getCustomAttribute */
			void getCustomAttribute(const String& name, void* pData) const override;

		protected:
			friend class bs::NullRenderWindow;

			/** @copydoc RenderWindow::getProperties */
			const RenderTargetProperties& getPropertiesInternal() const override { return mProperties; }

			/** @copydoc RenderWindow::getSyncedProperties */
			RenderWindowProperties& getSyncedProperties() override { return mSyncedProperties; }

			/** @copydoc RenderWindow::syncProperties */
			void syncProperties() override;

		protected:
			RenderWindowProperties mProperties;
			RenderWindowProperties mSyncedProperties;
		};
	}

	/** @} */
}
