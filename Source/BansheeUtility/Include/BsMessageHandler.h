//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisitesUtil.h"
#include "BsModule.h"

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
		MessageHandler();

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

		UINT32 mNextCallbackId;
	};

	/** @} */
}