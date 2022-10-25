//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Managers/BsRenderAPIFactory.h"
#include "Managers/BsRenderAPIManager.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup NullRenderAPI
		 *  @{
		 */
		/**	Handles creation of the Null render system. */
		class NullRenderAPIFactory final : public RenderAPIFactory
		{
		public:
			static constexpr const char* SystemName = "bsfNullRenderAPI";

			/** @copydoc RenderAPIFactory::create */
			void Create();

			/** @copydoc RenderAPIFactory::name */
			const char* Name() const { return SystemName; }

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
						newFactory = bs_shared_ptr_new<NullRenderAPIFactory>();
						RenderAPIManager::Instance().RegisterFactory(newFactory);
					}
				}
			};

			static InitOnStart initOnStart; // Makes sure factory is registered on program start
		};

		/** @} */
	} // namespace ct
} // namespace bs
