//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Utility/BsModule.h"

namespace bs
{
	/** @addtogroup General
	 *  @{
	 */

	/**
	 * Allows you to transparently pass messages between different systems.
	 *
	 * @note Sim thread only.
	 */
	class BS_UTILITY_EXPORT MessageHandler : public Module<MessageHandler>
	{
		struct MessageHandlerData
		{
			UINT32 id;
			std::function<void()> callback;
		};

	public:
		MessageHandler() = default;

		/** Sends a message to all subscribed listeners. */
		void send(MessageId message);

		/**
		 * Subscribes a message listener for the specified message. Provided callback will be triggered whenever that 
		 * message gets sent.
		 *
		 * @return	A handle to the message subscription that you can use to unsubscribe from listening.
		 */
		HMessage listen(MessageId message, std::function<void()> callback);

	private:
		friend class HMessage;
		void unsubscribe(UINT32 handleId);

		Map<UINT32, Vector<MessageHandlerData>> mMessageHandlers;
		Map<UINT32, UINT32> mHandlerIdToMessageMap;

		UINT32 mNextCallbackId = 1;
	};

	/** @} */
}
