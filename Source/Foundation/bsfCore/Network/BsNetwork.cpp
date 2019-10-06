//************************************ bs::framework - Copyright 2019 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//

#include "BsNetwork.h"
#include "RakNet/RakPeer.h"
#include "RakNet/MessageIdentifiers.h"
#include "Allocators/BsPoolAlloc.h"
#include "Serialization/BsSerializedObject.h"
#include "Serialization/BsBinarySerializer.h"
#include "FileSystem/BsDataStream.h"
#include "Math/BsMath.h"
#include "Serialization/BsBinaryDiff.h"
#include "Reflection/BsRTTIType.h"

using namespace RakNet;

namespace bs
{
	static_assert(NETWORK_BACKEND_FIRST_FREE_ID == ID_USER_PACKET_ENUM, "");

	/** Converts a RakNet SystemAddress into the framework's NetworkAddress type. */
	void systemToNetworkAddress(const SystemAddress& address, NetworkAddress& output)
	{
		if(address.address.addr4.sin_family == AF_INET)
		{
			output.ipType = IPV4;
			output.port = address.address.addr4.sin_port;
			memcpy(output.ip, &address.address.addr4.sin_addr, 4);
		}
		else
		{
			output.ipType = IPV6;
			output.port = address.address.addr6.sin6_port;
			output.ip6FlowInfo = address.address.addr6.sin6_flowinfo;
			output.ip6ScopeId = address.address.addr6.sin6_scope_id;
			memcpy(output.ip, &address.address.addr6.sin6_addr, 16);
		}
	}

	/**
	 * Same as systemToNetworkAddress(const SystemAddress&, NetworkAddress&) except it returns a brand new NetworkAddress
	 * object.
	 */
	NetworkAddress systemToNetworkAddress(const SystemAddress& address)
	{
		NetworkAddress output;
		systemToNetworkAddress(address, output);
		return output;
	}

	/** Converts the framework's NetworkAddress into RakNet's SystemAddres type. */
	SystemAddress networkToSystemAddress(const NetworkAddress& address)
	{
		SystemAddress output;
		output.address.addr4.sin_port = address.port;

		if(address.ipType == IPV4)
		{
			output.address.addr4.sin_family = AF_INET;
			memcpy(&output.address.addr4.sin_addr, address.ip, 4);
		}
		else
		{
			output.address.addr6.sin6_family = AF_INET6;
			output.address.addr6.sin6_flowinfo = (ULONG)address.ip6FlowInfo;
			output.address.addr6.sin6_scope_id = (ULONG)address.ip6ScopeId;
			memcpy(&output.address.addr6.sin6_addr, address.ip, 16);
		}

		return output;
	}

	/** Converts PacketChannel reliability, priority and ordering into equivalent RakNet enums. */
	void mapChannelToRakNet(const PacketChannel& channel, ::PacketReliability& reliability, ::PacketPriority& priority)
	{
		switch(channel.priority)
		{
		case PacketPriority::Immediate:
			priority = IMMEDIATE_PRIORITY;
			break;
		case PacketPriority::High:
			priority = HIGH_PRIORITY;
			break;
		default:
		case PacketPriority::Medium:
			priority = MEDIUM_PRIORITY;
			break;
		case PacketPriority::Low:
			priority = LOW_PRIORITY;
			break;
		}

		if(channel.reliability == PacketReliability::Unreliable)
		{
			if(channel.ordering == PacketOrdering::Ordered || channel.ordering == PacketOrdering::Sequenced)
				reliability = UNRELIABLE_SEQUENCED;
			else
				reliability = UNRELIABLE;
		}
		else
		{
			switch(channel.ordering)
			{
			default:
			case PacketOrdering::Unordered:
				reliability = RELIABLE;
				break;
			case PacketOrdering::Ordered:
				reliability = RELIABLE_ORDERED;
				break;
			case PacketOrdering::Sequenced:
				reliability = RELIABLE_SEQUENCED;
				break;
			}
		}
	}

	NetworkAddress NetworkAddress::UNASSIGNED;

	NetworkAddress::NetworkAddress(const char* address)
	{
		SystemAddress sysAddress(address);
		systemToNetworkAddress(sysAddress, *this);
	}

	NetworkAddress::NetworkAddress(const char* ip, UINT16 port)
	{
		SystemAddress sysAddress(ip, port);
		systemToNetworkAddress(sysAddress, *this);
	}

	String NetworkAddress::toString(bool withPort) const
	{
		SystemAddress sysAddress = networkToSystemAddress(*this);
		return String(sysAddress.ToString(withPort, '|'));
	}

	bool NetworkAddress::compareIP(const NetworkAddress& other) const
	{
		if(ipType != other.ipType)
			return false;

		if(ipType == IPV4)
			return memcmp(ip, other.ip, 4) == 0;

		return (memcmp(ip, other.ip, sizeof(ip)) == 0 && ip6FlowInfo == other.ip6FlowInfo && ip6ScopeId == other.ip6ScopeId);
	}

	NetworkAddress& NetworkAddress::operator= (const NetworkAddress& rhs)
	{
		ipType = rhs.ipType;
		port = rhs.port;
		ip6FlowInfo = rhs.ip6FlowInfo;
		ip6ScopeId = rhs.ip6ScopeId;

		memcpy(&ip, &rhs.ip, sizeof(ip));
		return *this;
	}

	bool NetworkAddress::operator==(const NetworkAddress& rhs) const
	{
		return port == rhs.port && compareIP(rhs);
	}

	bool NetworkAddress::operator!=(const NetworkAddress& rhs) const
	{
		return !(*this==rhs);
	}

	/** Contains information associated with a single connected peer. */
	struct NetworkConnection
	{
		NetworkId id;
		NetworkAddress networkAddress;
		SystemAddress systemAddress;
		RakNetGUID guid;
	};

	struct NetworkPeer::Pimpl
	{
		RakPeerInterface* peer;
		Vector<NetworkConnection> networkIdMapping;
		PoolAlloc<sizeof(NetworkEvent)> eventPool;

		/** Maps a network ID into a RakNet system address. Returns null if the ID is not valid. */
		const SystemAddress* getSystemAddress(const NetworkId& id)
		{
			if(id.id < 0 || id.id >= (INT32)networkIdMapping.size())
				return nullptr;

			return &networkIdMapping[id.id].systemAddress;
		}

		/** Maps a network ID into a RakNet GUID. Returns null if the ID is not valid. */
		const RakNetGUID* getGUID(const NetworkId& id)
		{
			if(id.id < 0 || id.id >= (INT32)networkIdMapping.size())
				return nullptr;

			return &networkIdMapping[id.id].guid;
		}

		/**
		 * Using a RakNet address or a GUID, attempts to retrieve a network ID, or allocates a new ID if one cannot be
		 * found.
		 */
		NetworkId getOrRegisterNetworkId(const SystemAddress& address, const RakNetGUID& guid)
		{
			INT32 systemIndex = address.systemIndex;
			assert(systemIndex >= 0 && systemIndex < (INT32)networkIdMapping.size());

			NetworkConnection& connection = networkIdMapping[systemIndex];
			if(connection.systemAddress != address || connection.guid != guid)
			{
				connection.networkAddress = systemToNetworkAddress(address);
				connection.systemAddress = address;
				connection.guid = guid;
			}

			return connection.id;
		}

		/** Allocates a new network event, responsible for reporting data received in @p packet. */
		NetworkEvent* allocNetworkEvent(Packet* packet)
		{
			NetworkEvent* event = eventPool.construct<NetworkEvent>();
			event->_backendData = packet;
			event->sender = getOrRegisterNetworkId(packet->systemAddress, packet->guid);

			switch (packet->data[0])
			{
			case ID_CONNECTION_REQUEST_ACCEPTED:
				event->type = NetworkEventType::ConnectingDone;
				break;
			case ID_CONNECTION_ATTEMPT_FAILED:
				event->type = NetworkEventType::ConnectingFailed;
				break;
			case ID_ALREADY_CONNECTED:
				event->type = NetworkEventType::AlreadyConnected;
				break;
			case ID_NEW_INCOMING_CONNECTION:
				event->type = NetworkEventType::IncomingNew;
				break;
			case ID_NO_FREE_INCOMING_CONNECTIONS:
				event->type = NetworkEventType::IncomingNoFree;
				break;
			case ID_DISCONNECTION_NOTIFICATION:
				event->type = NetworkEventType::Disconnected;
				break;
			case ID_CONNECTION_LOST:
				event->type = NetworkEventType::LostConnection;
				break;
			default:
				event->type = NetworkEventType::Data;
				event->data.bytes = packet->data;
				event->data.length = packet->length;
				break;
			}

			return event;
		}

		/** Frees an event allocated with allocNetworkEvent(). */
		void freeNetworkEvent(NetworkEvent* event)
		{
			Packet* packet = (Packet*)event->_backendData;

			peer->DeallocatePacket(packet);
			eventPool.free(event);
		}
	};

	NetworkPeer::NetworkPeer(const NETWORK_PEER_DESC& desc)
		:m(bs_new<Pimpl>())
	{
		m->peer = RakPeerInterface::GetInstance();
		m->networkIdMapping.resize(desc.maxNumConnections);

		for(INT32 i = 0; i < (INT32)m->networkIdMapping.size(); i++)
			m->networkIdMapping[i].id = NetworkId(i);

		UINT32 numDescriptors = (UINT32)desc.listenAddresses.size();
		SocketDescriptor* descriptors = bs_stack_alloc<SocketDescriptor>(numDescriptors);

		for(UINT32 i = 0; i < numDescriptors; i++)
		{
			const NetworkAddress& address = desc.listenAddresses[i];

			if(address.compareIP(NetworkAddress::UNASSIGNED))
			{
				if(address.port == 0)
					descriptors[i] = SocketDescriptor();
				else
					descriptors[i] = SocketDescriptor(address.port, nullptr);
			}
			else
			{
				if(address.ipType == IPV6)
				{
					BS_LOG(Error, Network, "IPV6 not supported for listener addreses on this backend");
					descriptors[i] = SocketDescriptor();
				}
				else // IPV4
				{
					String addressStr = address.toString();
					descriptors[i] = SocketDescriptor(address.port, addressStr.c_str());
				}
			}
		}

		StartupResult result = m->peer->Startup(desc.maxNumConnections, descriptors, numDescriptors);

		bs_stack_free(descriptors);

		switch(result)
		{
		case RAKNET_ALREADY_STARTED:
			BS_LOG(Warning, Network, "Failed to start RakNet peer, RakNet already started.");
			break;
		case INVALID_SOCKET_DESCRIPTORS:
			BS_LOG(Error, Network, "Failed to start RakNet peer, invalid socket descriptors provided.");
			break;
		case INVALID_MAX_CONNECTIONS:
			BS_LOG(Error, Network, "Failed to start RakNet peer, invalid max. connection count provided.");
			break;
		case SOCKET_FAMILY_NOT_SUPPORTED:
			BS_LOG(Error, Network, "Failed to start RakNet peer, socket family not supported.");
			break;
		case SOCKET_PORT_ALREADY_IN_USE:
			BS_LOG(Error, Network, "Failed to start RakNet peer, port already in use.");
			break;
		case SOCKET_FAILED_TO_BIND:
			BS_LOG(Error, Network, "Failed to start RakNet peer, socket failed to bind.");
			break;
		case SOCKET_FAILED_TEST_SEND:
			BS_LOG(Error, Network, "Failed to start RakNet peer, socket failed to test send.");
			break;
		case PORT_CANNOT_BE_ZERO:
			BS_LOG(Error, Network, "Failed to start RakNet peer, port cannot be zero.");
			break;
		case FAILED_TO_CREATE_NETWORK_THREAD:
			BS_LOG(Error, Network, "Failed to start RakNet peer, failed to create the network thread.");
			break;
		case COULD_NOT_GENERATE_GUID:
			BS_LOG(Error, Network, "Failed to start RakNet peer, failed to generate GUID.");
			break;
		case STARTUP_OTHER_FAILURE:
			BS_LOG(Error, Network, "Failed to start RakNet peer, unknown failure.");
			break;
		default:
			break;
		}

		if(desc.maxNumIncomingConnections > 0)
			m->peer->SetMaximumIncomingConnections(desc.maxNumIncomingConnections);
	}

	NetworkPeer::~NetworkPeer()
	{
		RakPeerInterface::DestroyInstance(m->peer);

		bs_delete(m);
	}

	bool NetworkPeer::connect(const char* host, UINT16 port)
	{
		ConnectionAttemptResult result = m->peer->Connect(host, port, nullptr, 0);
		
		if(result != CONNECTION_ATTEMPT_STARTED)
		{
			switch(result)
			{
			case INVALID_PARAMETER:
				BS_LOG(Error, Network, "Unable to connect to {0}|{1}, invalid parameter.", host, port);
				break;
			case CANNOT_RESOLVE_DOMAIN_NAME:
				BS_LOG(Error, Network, "Unable to connect to {0}|{1}, domain name cannot be resolved.", host, port);
				break;
			case ALREADY_CONNECTED_TO_ENDPOINT:
				BS_LOG(Warning, Network, "Unable to connect to {0}|{1}, already connected.", host, port);
				break;
			case CONNECTION_ATTEMPT_ALREADY_IN_PROGRESS:
				BS_LOG(Warning, Network, "Unable to connect to {0}|{1}, connection attempt already in progress.", host, port);
				break;
			case SECURITY_INITIALIZATION_FAILED:
				BS_LOG(Error, Network, "Unable to connect to {0}|{1}, security initialization failed.", host, port);
				break;
			default:
				break;
			}

			return false;
		}

		return true;
	}

	void NetworkPeer::disconnect(const NetworkAddress& address, bool silent)
	{
		m->peer->CloseConnection(networkToSystemAddress(address), !silent);
	}

	void NetworkPeer::disconnect(const NetworkId& id, bool silent)
	{
		const RakNetGUID* guid = m->getGUID(id);
		if(!guid)
		{
			BS_LOG(Error, Network, "Cannot disconnect from {0}, invalid network ID provided.", id.id);
			return;
		}

		m->peer->CloseConnection(*guid, !silent);
	}

	NetworkEvent* NetworkPeer::receive() const
	{
		Packet* packet = m->peer->Receive();
		if(!packet || packet->length == 0)
			return nullptr;

		return m->allocNetworkEvent(packet);
	}

	void NetworkPeer::free(NetworkEvent* event)
	{
		if(!event)
			return;

		m->freeNetworkEvent(event);
	}

	void NetworkPeer::send(const PacketData& data, const NetworkAddress& address, const PacketChannel& channel)
	{
		::PacketReliability reliability;
		::PacketPriority priority;

		mapChannelToRakNet(channel, reliability, priority);

		m->peer->Send((const char*)data.bytes, (INT32)data.length, priority, reliability, 0,
			networkToSystemAddress(address), false);
	}

	void NetworkPeer::send(const PacketData& data, const NetworkId& id, const PacketChannel& channel)
	{
		const RakNetGUID* guid = m->getGUID(id);
		if(!guid)
		{
			BS_LOG(Error, Network, "Cannot send to {0}, invalid network ID provided.", id.id);
			return;
		}

		::PacketReliability reliability;
		::PacketPriority priority;

		mapChannelToRakNet(channel, reliability, priority);

		m->peer->Send((const char*)data.bytes, (INT32)data.length, priority, reliability, 0,
			*guid, false);
	}

	void NetworkPeer::broadcast(const PacketData& data, const PacketChannel& channel)
	{
		::PacketReliability reliability;
		::PacketPriority priority;

		mapChannelToRakNet(channel, reliability, priority);

		m->peer->Send((const char*)data.bytes, (INT32)data.length, priority, reliability, 0,
			UNASSIGNED_RAKNET_GUID, true);
	}

	NetworkObject::~NetworkObject()
	{
		Network::instance()._notifyNetworkObjectDestroyed(this);
	}

	NetworkObjectState NetworkObject::getNetworkState() const
	{
		// We know serialization will maintain object state, so safely remove the const
		NetworkObject* thisObj = const_cast<NetworkObject*>(this);

		NetworkObjectState state;
		state.state = SerializedObject::create(*thisObj,
			SerializedObjectEncodeFlag::Shallow | SerializedObjectEncodeFlag::ReplicableOnly);

		return state;
	}

	void NetworkObject::networkSpawn()
	{
		if(mState != NotReplicated || !Network::instance().isHost())
			return;

		Network::instance()._notifyNetworkObjectSpawned(this);
		mState = Replicated;
		// TODO
	}

	void NetworkObject::networkDespawn()
	{
		if(mState != Replicated || !Network::instance().isHost())
			return;

		Network::instance()._notifyNetworkObjectDespawned(this);
		mState = NotReplicated;
		// TODO
	}

	NetworkEncoder::NetworkEncoder()
	{
		mWriteBuffer = (UINT8*)bs_alloc(WRITE_BUFFER_SIZE);
	}

	NetworkEncoder::~NetworkEncoder()
	{
		assert(mBufferPieces.empty());

		for (auto& entry : mBufferPiecePool)
			bs_free(entry.buffer);

		if (mResultBuffer)
			bs_free(mResultBuffer);

		bs_free(mWriteBuffer);
	}

	void NetworkEncoder::encode(UINT8 type, const UUID& uuid, IReflectable* object, SerializationContext* context)
	{
		BinarySerializer bs;
		
		auto flushToBuffer = [this](UINT8* bufferStart, UINT32 bytesWritten, UINT32& newBufferSize)
		{
			if(mBufferPieces.empty())
				allocBufferPiece();

			assert(bytesWritten <= WRITE_BUFFER_SIZE);

			UINT32 bufferSize = WRITE_BUFFER_SIZE - mBufferPieces.back().size;

			UINT32 amountToCopy = std::min(bufferSize, bytesWritten);
			memcpy(mBufferPieces.back().buffer + mBufferPieces.back().size, bufferStart, amountToCopy);
			mBufferPieces.back().size += amountToCopy;

			if(bufferSize < bytesWritten)
			{
				allocBufferPiece();
				memcpy(mBufferPieces.back().buffer, bufferStart + bufferSize, bytesWritten - bufferSize);
				mBufferPieces.back().size = bytesWritten - bufferSize;
			}

			mWriteBufferOffset = 0;
			newBufferSize = WRITE_BUFFER_SIZE;
			return mWriteBuffer;
		};

		auto writeToBuffer = [this, &flushToBuffer](auto data)
		{
			UINT32 size = rttiGetElemSize(data);
			if ((mWriteBufferOffset + size) > WRITE_BUFFER_SIZE)
			{
				UINT32 newBufferSize;
				flushToBuffer(mWriteBuffer, mWriteBufferOffset, newBufferSize);
			}

			rttiWriteElem(data, (char*)mWriteBuffer + mWriteBufferOffset);

			mWriteBufferOffset += size;
			mBytesWritten += size;
		};

		auto writeToOffset = [this](UINT32 offset, auto data)
		{
			UINT32 curOffset = 0;
			for(auto& entry : mBufferPieces)
			{
				if(offset >= curOffset && offset < (curOffset + entry.size))
				{
					UINT32 localOffset = offset - curOffset;
					rttiWriteElem(data, (char*)entry.buffer + localOffset);
					break;
				}
			}
		};

		writeToBuffer(type);
		writeToBuffer(uuid);

		UINT32 sizeWriteOffset = mBytesWritten;
		writeToBuffer(0);

		if(object)
		{
			UINT8* writeStart = mWriteBuffer + mWriteBufferOffset;
			UINT32 remainingBufferSize = WRITE_BUFFER_SIZE - mWriteBufferOffset;
			UINT32 objBytesWritten = 0;
			bs.encode(object, writeStart, remainingBufferSize, &objBytesWritten, flushToBuffer, false, context);
			mBytesWritten += objBytesWritten;

			writeToOffset(sizeWriteOffset, objBytesWritten);
		}
	}

	UINT8* NetworkEncoder::getOutput(UINT32& size)
	{
		UINT32 bytesRequired = mBytesWritten + 1; // Reserve first byte for message type

		if(mResultBufferSize < bytesRequired)
		{
			if(mResultBuffer)
				bs_free(mResultBuffer);

			UINT32 bufferSize = (UINT32)(bytesRequired * 1.25f); // 25% extra

			mResultBuffer = (UINT8*)bs_alloc(bufferSize);
			mResultBufferSize = bufferSize;
		}

		UINT32 offset = 1; // First byte reserved for message type, set externally
		for(auto iter = mBufferPieces.begin(); iter != mBufferPieces.end(); ++iter)
		{
			if(iter->size > 0)
			{
				memcpy(mResultBuffer + offset, iter->buffer, iter->size);
				offset += iter->size;
			}
		}

		for(auto iter = mBufferPieces.rbegin(); iter != mBufferPieces.rend(); ++iter)
			mBufferPiecePool.push_back(*iter);

		mBufferPieces.clear();

		size = bytesRequired;
		return mResultBuffer;
	}

	void NetworkEncoder::clear()
	{
		mWriteBufferOffset = 0;
		mBytesWritten = 0;
	}

	NetworkEncoder::BufferPiece NetworkEncoder::allocBufferPiece()
	{
		BufferPiece piece;
		if (!mBufferPiecePool.empty())
		{
			piece = mBufferPiecePool.back();
			mBufferPiecePool.pop_back();
		}
		else
			piece.buffer = (UINT8*)bs_alloc(WRITE_BUFFER_SIZE);

		piece.size = 0;
		mBufferPieces.push_back(piece);
		return piece;
	}

	NetworkDecoder::NetworkDecoder(const SPtr<MemoryDataStream>& data)
		:mInputStream(data)
	{
		mInputStream->skip(1); // Skip the network message type byte
	}

	SPtr<IReflectable> NetworkDecoder::decode(UINT8& type, UUID& uuid, SerializationContext* context)
	{
		if (mInputStream->eof())
			return nullptr;

		char* data = (char*)mInputStream->getCurrentPtr();
		UINT32 offset = 0;
		data = rtti_read(type, data, offset);
		data = rtti_read(uuid, data, offset);

		mInputStream->skip(offset);

		UINT32 objectSize = 0;
		mInputStream->read(&objectSize, sizeof(objectSize));

		if(objectSize > 0)
		{
			BinarySerializer bs;
			SPtr<IReflectable> object = bs.decode(mInputStream, objectSize, context);

			return object;
		}

		return nullptr;
	}

	PacketChannel CHANNEL_RELIABLE_ORDERED = { PacketPriority::Medium, PacketReliability::Reliable, PacketOrdering::Ordered };
	PacketChannel CHANNEL_UNRELIABLE_ORDERED = { PacketPriority::Medium, PacketReliability::Unreliable, PacketOrdering::Ordered };

	void Network::_notifyNetworkObjectSpawned(NetworkObject* object)
	{
		object->mNetworkUUID = UUIDGenerator::generateRandom();
		mActions.emplace_back(object->mNetworkUUID, Spawning);

		ObjectInfo objInfo;
		objInfo.obj = object;
		objInfo.state = object->getNetworkState();

		mNetworkObjects[object->mNetworkUUID] = objInfo;

		// TODO - This queues sync on next tick. Allow caller to force sync immediately though some flag, and/or
		// change the tick rate. The same applies to other _notify functions.
	}

	void Network::_notifyNetworkObjectDespawned(NetworkObject* object)
	{
		object->mNetworkUUID = UUID::EMPTY;
		mActions.emplace_back(object->mNetworkUUID, Despawning);

		mNetworkObjects[object->mNetworkUUID].obj = nullptr;
	}

	void Network::_notifyNetworkObjectDestroyed(NetworkObject* object)
	{
		if(object->mState == NetworkObject::Replicated)
			_notifyNetworkObjectDespawned(object);
		else
			mNetworkObjects[object->mNetworkUUID].obj = nullptr;
	}

	// TODO - Need a method that receives state updates and handles spawn, despawn and sync

	enum class NetworkActionType
	{
		Spawn,
		Sync,
		Despawn
	};

	void Network::host(const SmallVector<NetworkAddress, 4>& listenAddresses, UINT32 tickRate, UINT32 maxConnections)
	{
		if(mPeer)
		{
			BS_LOG(Error, Network, "Cannot start hosting when an existing network connection is active.");
			return;
		}

		NETWORK_PEER_DESC desc;
		desc.listenAddresses = listenAddresses;
		desc.maxNumConnections = maxConnections;
		desc.maxNumIncomingConnections = maxConnections;

		mPeer = bs_unique_ptr_new<NetworkPeer>(desc);
		mState = NetworkState::Hosting;
		mTickRate = tickRate;
		mTimeAccumulator = 0.0f;
	}

	void Network::connect(const char* host, UINT16 port)
	{
		if(mPeer)
		{
			BS_LOG(Error, Network, "Cannot connect when an existing network connection is active.");
			return;
		}

		NETWORK_PEER_DESC desc;
		desc.maxNumConnections = 1;

		mPeer = bs_unique_ptr_new<NetworkPeer>(desc);
		mPeer->connect(host, port);
		mState = NetworkState::Connecting;
	}

	void Network::disconnect()
	{
		mPeer = nullptr;
		mState = NetworkState::Disconnected;
	}

	void Network::update(float dt)
	{
		if(mPeer)
		{
			NetworkEvent* event = mPeer->receive();
			
			while(event)
			{
				switch(event->type)
				{
				case NetworkEventType::ConnectingDone: 
					if(mState == NetworkState::Connecting)
						mState = NetworkState::Connected;
					
					break;
				case NetworkEventType::ConnectingFailed: 
					if(mState == NetworkState::Connecting)
						mState = NetworkState::Disconnected;

					break;
				case NetworkEventType::AlreadyConnected: break;
				case NetworkEventType::IncomingNew: 
					// TODO - Need to send state for all current objects to the newly connected peer
					break;
				case NetworkEventType::IncomingNoFree: 
					BS_LOG(Warning, Network, "Refused incoming connection due to maximum connection count being reached.");
					break;
				case NetworkEventType::Disconnected: break;
				case NetworkEventType::LostConnection: break;
				case NetworkEventType::Data:
					if(event->data.bytes[0] == NWM_ReplicationSync)
					{
						// TODO - Spawn/despawn objects, sync state
					}

					break;
				default: 
					break;
				}

				mPeer->free(event);
				event = mPeer->receive();
			}
		}

		if(isHost())
		{
			// TODO - Support setting different tick times for different objects (or object types)
			if (mTimeAccumulator >= 0.0f)
			{
				for (auto& entry : mActions)
				{
					auto iterFind = mNetworkObjects.find(entry.uuid);
					assert(iterFind != mNetworkObjects.end());

					ObjectInfo& objInfo = iterFind->second;

					switch (entry.type)
					{
					case Spawning:
						mEncoder.encode((UINT32)NetworkActionType::Spawn, entry.uuid, objInfo.state.state.get());
						break;
					case Spawned:
						// TODO - No purpose. Remove this?
						break;
					case Despawning:
						mEncoder.encode((UINT32)NetworkActionType::Despawn, entry.uuid, nullptr);
						mNetworkObjects.erase(iterFind);
						break;
					default:
						break;
					}
				}

				mActions.clear();

				{
					PacketData data;
					data.bytes = mEncoder.getOutput(data.length);
					data.bytes[0] = NWM_ReplicationSync;

					mPeer->broadcast(data, CHANNEL_RELIABLE_ORDERED);
					mEncoder.clear();
				}

				for (auto& entry : mNetworkObjects)
				{
					// TODO - This needs to be rewritten as a specialized method that avoids doing a SerializedObject
					// conversion. The method should check previous SerializedObject state with actual state in IReflectable.
					// It should also auto-update the previous state to new state, if modifications are detected.

					// TODO - Add a manual overridable method to a NetworkObject that allows the user to determine if
					// a network object needs sync or not

					SPtr<SerializedObject> newState = SerializedObject::create(*entry.second.obj,
						SerializedObjectEncodeFlag::Shallow | SerializedObjectEncodeFlag::ReplicableOnly);

					IDiff& diffHandler = entry.second.obj->getRTTI()->getDiffHandler();
					SPtr<SerializedObject> diff = diffHandler.generateDiff(entry.second.state.state, newState);
					if (diff == nullptr)
						continue;

					entry.second.state.state = newState;

					mEncoder.encode((UINT32)NetworkActionType::Sync, entry.first, diff.get());
				}

				// TODO - Perhaps allow an object to force sync to be reliable?
				{
					PacketData data;
					data.bytes = mEncoder.getOutput(data.length);
					data.bytes[0] = NWM_ReplicationSync;

					mPeer->broadcast(data, CHANNEL_UNRELIABLE_ORDERED);
					mEncoder.clear();
				}

				// TODO - Add relevant sets. Don't send all messages to every connected client

				mEncoder.clear();

				float tickLength = 1.0f / mTickRate;
				mTimeAccumulator = Math::repeat(mTimeAccumulator, tickLength) - tickLength;
			}

			mTimeAccumulator += dt;
		}

	}
}
