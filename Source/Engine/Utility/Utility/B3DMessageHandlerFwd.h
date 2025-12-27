//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/B3DTypes.h"
#include "String/B3DString.h"

namespace b3d
{
	/** @addtogroup General
	 *  @{
	 */

	/**
	 * Identifier for message used with the global messaging system.
	 *
	 * @note
	 * Primary purpose of this class is to avoid expensive string compare, and instead use a unique message identifier for
	 * compare. Generally you want to create one of these using the message name, and then store it for later use.
	 * @note
	 * This class is not thread safe and should only be used on the main thread.
	 */
	class B3D_EXPORT MessageId
	{
	public:
		MessageId() = default;
		MessageId(const String& name);

		bool operator==(const MessageId& rhs) const
		{
			return (mMsgIdentifier == rhs.mMsgIdentifier);
		}

	private:
		friend class MessageHandler;

		static Map<String, u32> UniqueMessageIds;
		static u32 NextMessageId;

		u32 mMsgIdentifier = 0;
	};

	/** Handle to a subscription for a specific message in the global messaging system. */
	class B3D_EXPORT HMessage
	{
	public:
		HMessage() = default;

		/** Disconnects the message listener so it will no longer receive events from the messaging system. */
		void Disconnect();

	private:
		friend class MessageHandler;

		HMessage(u32 id);

		u32 mId = 0;
	};

	/**
	 * Sends a message using the global messaging system.
	 *
	 * @note	Main thread only.
	 */
	void B3D_EXPORT SendMessage(MessageId message);

	class MessageHandler;

	/** @} */
} // namespace b3d
