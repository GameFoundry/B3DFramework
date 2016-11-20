//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsMessageHandler.h"

namespace bs
{
	Map<String, UINT32> MessageId::UniqueMessageIds;
	UINT32 MessageId::NextMessageId = 0;

	MessageId::MessageId()
		: mMsgIdentifier(0)
	{ }

	MessageId::MessageId(const String& name)
	{
		auto findIter = UniqueMessageIds.find(name);

		if (findIter != UniqueMessageIds.end())
			mMsgIdentifier = findIter->second;
		else
		{
			mMsgIdentifier = NextMessageId;
			UniqueMessageIds[name] = NextMessageId++;
		}
	}

	HMessage::HMessage()
		:mId(0)
	{ }

	HMessage::HMessage(UINT32 id)
		:mId(id)
	{ }

	void HMessage::disconnect()
	{
		if (mId > 0)
			MessageHandler::instance().unsubscribe(mId);
	}

	MessageHandler::MessageHandler()
		:mNextCallbackId(1) // 0 is reserved
	{ }

	void MessageHandler::send(MessageId message)
	{
		auto iterFind = mMessageHandlers.find(message.mMsgIdentifier);
		if (iterFind != mMessageHandlers.end())
		{
			for (auto& handlerData : iterFind->second)
			{
				handlerData.callback();
			}
		}
	}

	HMessage MessageHandler::listen(MessageId message, std::function<void()> callback)
	{
		UINT32 callbackId = mNextCallbackId++;
		
		MessageHandlerData data;
		data.id = callbackId;
		data.callback = callback;

		mMessageHandlers[message.mMsgIdentifier].push_back(data);
		mHandlerIdToMessageMap[callbackId] = message.mMsgIdentifier;

		return HMessage(callbackId);
	}

	void MessageHandler::unsubscribe(UINT32 handleId)
	{
		UINT32 msgId = mHandlerIdToMessageMap[handleId];

		auto iterFind = mMessageHandlers.find(msgId);
		if (iterFind != mMessageHandlers.end())
		{
			Vector<MessageHandlerData>& handlerData = iterFind->second;

			handlerData.erase(
				std::remove_if(handlerData.begin(), handlerData.end(), 
				[&](MessageHandlerData& x)
				{
					return x.id == handleId;
				}));
		}

		mHandlerIdToMessageMap.erase(handleId);
	}

	void sendMessage(MessageId message)
	{
		MessageHandler::instance().send(message);
	}
}