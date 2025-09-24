//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Managers/B3DRenderAPIFactory.h"
#include "Managers/B3DRenderAPIManager.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup NullRenderAPI
		 *  @{
		 */
		/**	Handles creation of the Null render system. */
		class NullRenderAPIFactory final : public RenderAPIFactory
		{
		public:
			static constexpr const char* SystemName = "bsfNullRenderAPI";

			void Create() override;
			const char* Name() const override { return SystemName; }

		private:
			/**	Registers the factory with the render system manager when constructed. */
			class InitOnStart
			{
			public:
				InitOnStart()
				{
					static SPtr<RenderAPIFactory> newFactory;
					if(newFactory == nullptr)
					{
						newFactory = B3DMakeShared<NullRenderAPIFactory>();
						RenderAPIManager::Instance().RegisterFactory(newFactory);
					}
				}
			};

			static InitOnStart initOnStart; // Makes sure factory is registered on program start
		};

		/** @} */
	} // namespace render
} // namespace b3d
