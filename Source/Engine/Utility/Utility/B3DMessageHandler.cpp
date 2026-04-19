//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Utility/B3DMessageHandler.h"

using namespace b3d;

Map<String, u32> MessageId::UniqueMessageIds;
u32 MessageId::NextMessageId = 0;

MessageId::MessageId(const String& name)
{
	auto findIter = UniqueMessageIds.find(name);

	if(findIter != UniqueMessageIds.end())
		mMsgIdentifier = findIter->second;
	else
	{
		mMsgIdentifier = NextMessageId;
		UniqueMessageIds[name] = NextMessageId++;
	}
}

HMessage::HMessage(u32 id)
	: mId(id)
{}

void HMessage::Disconnect()
{
	if(mId > 0)
		MessageHandler::Instance().Unsubscribe(mId);
}

void MessageHandler::Send(MessageId message)
{
	auto iterFind = mMessageHandlers.find(message.mMsgIdentifier);
	if(iterFind != mMessageHandlers.end())
	{
		for(auto& handlerData : iterFind->second)
		{
			handlerData.Callback();
		}
	}
}

HMessage MessageHandler::Listen(MessageId message, std::function<void()> callback)
{
	u32 callbackId = mNextCallbackId++;

	MessageHandlerData data;
	data.Id = callbackId;
	data.Callback = callback;

	mMessageHandlers[message.mMsgIdentifier].push_back(data);
	mHandlerIdToMessageMap[callbackId] = message.mMsgIdentifier;

	return HMessage(callbackId);
}

void MessageHandler::Unsubscribe(u32 handleId)
{
	u32 msgId = mHandlerIdToMessageMap[handleId];

	auto iterFind = mMessageHandlers.find(msgId);
	if(iterFind != mMessageHandlers.end())
	{
		Vector<MessageHandlerData>& handlerData = iterFind->second;

		handlerData.erase(
			std::remove_if(handlerData.begin(), handlerData.end(), [handleId](MessageHandlerData& x)
						   { return x.Id == handleId; }),
			handlerData.end());
	}

	mHandlerIdToMessageMap.erase(handleId);
}

namespace b3d
{
void SendMessage(MessageId message)
{
	MessageHandler::Instance().Send(message);
}
} // namespace b3d
