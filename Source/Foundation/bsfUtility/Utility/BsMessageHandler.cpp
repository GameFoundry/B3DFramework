//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Utility/BsMessageHandler.h"

namespace bs
{
	Map<String, UINT32> MessageId::UniqueMessageIds;
	UINT32 MessageId::NextMessageId = 0;

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

	HMessage::HMessage(UINT32 id)
		:mId(id)
	{ }

	void HMessage::disconnect()
	{
		if (mId > 0)
			MessageHandler::instance().unsubscribe(mId);
	}

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
