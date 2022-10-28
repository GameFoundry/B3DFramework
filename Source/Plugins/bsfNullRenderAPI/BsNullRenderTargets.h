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
		SPtr<RenderWindow> CreateImpl(RENDER_WINDOW_DESC& desc, u32 windowId, const SPtr<RenderWindow>& parentWindow) override;
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
		const RenderTargetProperties& GetPropertiesInternal() const override { return mProperties; }

		RenderTextureProperties mProperties;
	};

	/** Render window implementation for null render backend. */
	class NullRenderWindow final : public RenderWindow
	{
	public:
		~NullRenderWindow() = default;

		Vector2I ScreenToWindowPos(const Vector2I& screenPos) const override { return screenPos; }
		Vector2I WindowToScreenPos(const Vector2I& windowPos) const override { return windowPos; }

		void GetCustomAttribute(const String& name, void* pData) const override;

		/** @copydoc RenderWindow::GetCore */
		SPtr<ct::NullRenderWindow> GetCore() const;

	protected:
		friend class NullRenderWindowManager;
		friend class ct::NullRenderWindow;

		NullRenderWindow(const RENDER_WINDOW_DESC& desc, u32 windowId);

		const RenderTargetProperties& GetPropertiesInternal() const override { return mProperties; }
		void SyncProperties() override;
		SPtr<ct::CoreObject> CreateCore() const override;

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
			const RenderTargetProperties& GetPropertiesInternal() const override { return mProperties; }

			RenderTextureProperties mProperties;
		};

		/** Render window implementation for null render backend. */
		class NullRenderWindow final : public RenderWindow
		{
		public:
			NullRenderWindow(const RENDER_WINDOW_DESC& desc, u32 windowId);

			void Move(i32 left, i32 top) override {}
			void Resize(u32 width, u32 height) override {}
			void SetVSync(bool enabled, u32 interval = 1) override {}
			void GetCustomAttribute(const String& name, void* pData) const override;

		protected:
			friend class bs::NullRenderWindow;

			const RenderTargetProperties& GetPropertiesInternal() const override { return mProperties; }
			RenderWindowProperties& GetSyncedProperties() override { return mSyncedProperties; }
			void SyncProperties() override;

		protected:
			RenderWindowProperties mProperties;
			RenderWindowProperties mSyncedProperties;
		};
	} // namespace ct

	/** @} */
} // namespace bs
