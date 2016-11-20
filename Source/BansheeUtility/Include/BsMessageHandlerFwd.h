//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsTypes.h"
#include "BsString.h"

namespace bs
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
	 * This class is not thread safe and should only be used on the sim thread.
	 */
	class BS_UTILITY_EXPORT MessageId
	{
	public:
		MessageId();
		MessageId(const String& name);

		bool operator== (const MessageId& rhs) const
		{
			return (mMsgIdentifier == rhs.mMsgIdentifier);
		}
	private:
		friend class MessageHandler;

		static Map<String, UINT32> UniqueMessageIds;
		static UINT32 NextMessageId;

		UINT32 mMsgIdentifier;
	};

	/** Handle to a subscription for a specific message in the global messaging system. */
	class BS_UTILITY_EXPORT HMessage
	{
	public:
		HMessage();

		/** Disconnects the message listener so it will no longer receive events from the messaging system. */
		void disconnect();

	private:
		friend class MessageHandler;

		HMessage(UINT32 id);

		UINT32 mId;
	};

	/**
	 * Sends a message using the global messaging system.
	 *
	 * @note	Sim thread only.
	 */
	void BS_UTILITY_EXPORT sendMessage(MessageId message);

	class MessageHandler;

	/** @} */
}
