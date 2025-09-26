//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Utility/B3DModule.h"

namespace b3d
{
	/** @addtogroup General
	 *  @{
	 */

	/**
	 * Allows you to transparently pass messages between different systems.
	 *
	 * @note Main thread only.
	 */
	class B3D_EXPORT MessageHandler : public Module<MessageHandler>
	{
		struct MessageHandlerData
		{
			u32 Id;
			std::function<void()> Callback;
		};

	public:
		MessageHandler() = default;

		/** Sends a message to all subscribed listeners. */
		void Send(MessageId message);

		/**
		 * Subscribes a message listener for the specified message. Provided callback will be triggered whenever that
		 * message gets sent.
		 *
		 * @return	A handle to the message subscription that you can use to unsubscribe from listening.
		 */
		HMessage Listen(MessageId message, std::function<void()> callback);

	private:
		friend class HMessage;
		void Unsubscribe(u32 handleId);

		Map<u32, Vector<MessageHandlerData>> mMessageHandlers;
		Map<u32, u32> mHandlerIdToMessageMap;

		u32 mNextCallbackId = 1;
	};

	/** @} */
} // namespace b3d
