//************************************ B3D Framework - Copyright 2019 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//

#include "B3DNetwork.h"
#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>
#include "Debug/B3DDebug.h"

using namespace b3d;

namespace
{
	/** Converts GameNetworkingSockets address to NetworkAddress. */
	void GNSToNetworkAddress(const SteamNetworkingIPAddr& gnsAddress, NetworkAddress& outAddress)
	{
		if(gnsAddress.IsIPv4())
		{
			outAddress.IPType = IPV4;
			outAddress.Port = gnsAddress.m_port;
			// Extract IPv4 address from IPv6-mapped format
			u32 ipv4 = gnsAddress.GetIPv4();
			memcpy(outAddress.IP, &ipv4, 4);
		}
		else
		{
			outAddress.IPType = IPV6;
			outAddress.Port = gnsAddress.m_port;
			memcpy(outAddress.IP, gnsAddress.m_ipv6, 16);
		}
	}

	/** Converts NetworkAddress to GameNetworkingSockets address. */
	SteamNetworkingIPAddr NetworkToGNSAddress(const NetworkAddress& address)
	{
		SteamNetworkingIPAddr gnsAddress;
		gnsAddress.Clear();

		if(address.IPType == IPV4)
		{
			u32 ipv4;
			memcpy(&ipv4, address.IP, 4);
			gnsAddress.SetIPv4(ipv4, address.Port);
		}
		else
		{
			gnsAddress.SetIPv6(address.IP, address.Port);
		}

		return gnsAddress;
	}

	/** Maps NetworkSendFlags to GameNetworkingSockets send flags. */
	int32 MapSendFlagsToGNS(NetworkSendFlags flags)
	{
		int32 gnsFlags = 0;

		if(flags.IsSet(NetworkSendFlagBits::Reliable))
			gnsFlags |= k_nSteamNetworkingSend_Reliable;

		if(flags.IsSet(NetworkSendFlagBits::NoNagle))
			gnsFlags |= k_nSteamNetworkingSend_NoNagle;

		if(flags.IsSet(NetworkSendFlagBits::NoDelay))
			gnsFlags |= k_nSteamNetworkingSend_NoDelay;

		if(flags.IsSet(NetworkSendFlagBits::UnreliableNoDelay))
			gnsFlags |= k_nSteamNetworkingSend_UnreliableNoDelay;

		if(flags.IsSet(NetworkSendFlagBits::UseCurrentThread))
			gnsFlags |= k_nSteamNetworkingSend_UseCurrentThread;

		return gnsFlags;
	}
}

struct NetworkPeer::Impl
{
	ISteamNetworkingSockets* Sockets = nullptr;
	HSteamListenSocket ListenSocket = k_HSteamListenSocket_Invalid;
	HSteamNetConnection ClientConnection = k_HSteamNetConnection_Invalid;
	HSteamNetPollGroup PollGroup = k_HSteamNetPollGroup_Invalid;

	bool IsServer = false;
	u32 MaxConnections = 0;

	// Connection tracking
	UnorderedMap<HSteamNetConnection, ConnectionID> ConnectionToID;
	UnorderedMap<u64, HSteamNetConnection> IDToConnection;
	u64 NextConnectionID = 1;

	// Message pool for received messages
	Vector<ISteamNetworkingMessage*> MessagePool;

	/** Maps a GNS connection handle to a ConnectionID, allocating if needed. */
	ConnectionID GetOrAllocateConnectionID(HSteamNetConnection handle)
	{
		auto it = ConnectionToID.find(handle);
		if(it != ConnectionToID.end())
			return it->second;

		u64 id = NextConnectionID++;
		ConnectionID connectionID(id);

		ConnectionToID[handle] = connectionID;
		IDToConnection[id] = handle;

		return connectionID;
	}

	/** Maps a ConnectionID to a GNS connection handle. */
	HSteamNetConnection GetConnectionHandle(ConnectionID id) const
	{
		auto it = IDToConnection.find(id.GetID());
		if(it != IDToConnection.end())
			return it->second;

		return k_HSteamNetConnection_Invalid;
	}

	/** Removes a connection from tracking. */
	void RemoveConnection(ConnectionID id)
	{
		auto it = IDToConnection.find(id.GetID());
		if(it != IDToConnection.end())
		{
			HSteamNetConnection handle = it->second;
			IDToConnection.erase(it);
			ConnectionToID.erase(handle);
		}
	}

	/** Process connection state changes. */
	void OnConnectionStateChanged(SteamNetConnectionStatusChangedCallback_t* info)
	{
		switch(info->m_info.m_eState)
		{
		case k_ESteamNetworkingConnectionState_None:
			break;

		case k_ESteamNetworkingConnectionState_Connecting:
			// Incoming connection request (server only)
			if(IsServer)
			{
				if(ConnectionToID.size() < MaxConnections)
				{
					if(Sockets->AcceptConnection(info->m_hConn) == k_EResultOK)
					{
						GetOrAllocateConnectionID(info->m_hConn);
					}
				}
				else
				{
					Sockets->CloseConnection(info->m_hConn, 0, "Server full", false);
				}
			}
			break;

		case k_ESteamNetworkingConnectionState_Connected:
			// Connection established
			GetOrAllocateConnectionID(info->m_hConn);
			// Add to poll group if server
			if(IsServer && PollGroup != k_HSteamNetPollGroup_Invalid)
			{
				Sockets->SetConnectionPollGroup(info->m_hConn, PollGroup);
			}
			break;

		case k_ESteamNetworkingConnectionState_ClosedByPeer:
		case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
			{
				ConnectionID id = GetOrAllocateConnectionID(info->m_hConn);
				Sockets->CloseConnection(info->m_hConn, 0, nullptr, false);
				RemoveConnection(id);
			}
			break;

		default:
			break;
		}
	}
};

NetworkPeer::NetworkPeer()
	: m(B3DNew<Impl>())
{
	// Initialize GameNetworkingSockets
	SteamDatagramErrMsg errMsg;
	if(!GameNetworkingSockets_Init(nullptr, errMsg))
	{
		B3D_LOG(Error, LogNetwork, "Failed to initialize GameNetworkingSockets: {0}", errMsg);
		return;
	}

	m->Sockets = SteamNetworkingSockets();
}

NetworkPeer::~NetworkPeer()
{
	DisconnectAll();

	// Cleanup
	GameNetworkingSockets_Kill();

	B3DDelete(m);
}

bool NetworkPeer::StartServer(u16 port, u32 maxConnections)
{
	if(!m->Sockets)
	{
		B3D_LOG(Error, LogNetwork, "Cannot start server, sockets not initialized.");
		return false;
	}

	if(m->ListenSocket != k_HSteamListenSocket_Invalid || m->ClientConnection != k_HSteamNetConnection_Invalid)
	{
		B3D_LOG(Error, LogNetwork, "Cannot start server, peer already active.");
		return false;
	}

	// Create listen socket
	SteamNetworkingIPAddr serverAddress;
	serverAddress.Clear();
	serverAddress.m_port = port;

	m->ListenSocket = m->Sockets->CreateListenSocketIP(serverAddress, 0, nullptr);
	if(m->ListenSocket == k_HSteamListenSocket_Invalid)
	{
		B3D_LOG(Error, LogNetwork, "Failed to create listen socket on port {0}.", port);
		return false;
	}

	// Create poll group for efficient message polling
	m->PollGroup = m->Sockets->CreatePollGroup();
	if(m->PollGroup == k_HSteamNetPollGroup_Invalid)
	{
		B3D_LOG(Error, LogNetwork, "Failed to create poll group.");
		m->Sockets->CloseListenSocket(m->ListenSocket);
		m->ListenSocket = k_HSteamListenSocket_Invalid;
		return false;
	}

	m->IsServer = true;
	m->MaxConnections = maxConnections;

	B3D_LOG(Info, LogNetwork, "Server started on port {0}, max connections: {1}", port, maxConnections);
	return true;
}

ConnectionID NetworkPeer::Connect(const NetworkAddress& address)
{
	if(!m->Sockets)
	{
		B3D_LOG(Error, LogNetwork, "Cannot connect, sockets not initialized.");
		return ConnectionID::Invalid();
	}

	if(m->ListenSocket != k_HSteamListenSocket_Invalid || m->ClientConnection != k_HSteamNetConnection_Invalid)
	{
		B3D_LOG(Error, LogNetwork, "Cannot connect, peer already active.");
		return ConnectionID::Invalid();
	}

	SteamNetworkingIPAddr serverAddress = NetworkToGNSAddress(address);

	m->ClientConnection = m->Sockets->ConnectByIPAddress(serverAddress, 0, nullptr);
	if(m->ClientConnection == k_HSteamNetConnection_Invalid)
	{
		B3D_LOG(Error, LogNetwork, "Failed to initiate connection to {0}", address.ToString(true));
		return ConnectionID::Invalid();
	}

	m->IsServer = false;
	return m->GetOrAllocateConnectionID(m->ClientConnection);
}

void NetworkPeer::Disconnect(ConnectionID connection)
{
	if(!m->Sockets)
		return;

	HSteamNetConnection handle = m->GetConnectionHandle(connection);
	if(handle == k_HSteamNetConnection_Invalid)
		return;

	m->Sockets->CloseConnection(handle, 0, "Disconnected", true);
	m->RemoveConnection(connection);
}

void NetworkPeer::DisconnectAll()
{
	if(!m->Sockets)
		return;

	// Close all connections
	for(auto& pair : m->IDToConnection)
	{
		m->Sockets->CloseConnection(pair.second, 0, "Shutdown", true);
	}

	m->ConnectionToID.clear();
	m->IDToConnection.clear();

	// Close listen socket
	if(m->ListenSocket != k_HSteamListenSocket_Invalid)
	{
		m->Sockets->CloseListenSocket(m->ListenSocket);
		m->ListenSocket = k_HSteamListenSocket_Invalid;
	}

	// Destroy poll group
	if(m->PollGroup != k_HSteamNetPollGroup_Invalid)
	{
		m->Sockets->DestroyPollGroup(m->PollGroup);
		m->PollGroup = k_HSteamNetPollGroup_Invalid;
	}

	// Close client connection
	if(m->ClientConnection != k_HSteamNetConnection_Invalid)
	{
		m->Sockets->CloseConnection(m->ClientConnection, 0, "Shutdown", true);
		m->ClientConnection = k_HSteamNetConnection_Invalid;
	}
}

void NetworkPeer::PollMessages(Vector<NetworkMessage>& outMessages, u32 maxMessages)
{
	outMessages.clear();

	if(!m->Sockets)
		return;

	// Poll connection state changes
	m->Sockets->RunCallbacks();

	// Allocate temporary buffer for messages
	m->MessagePool.clear();
	m->MessagePool.resize(maxMessages);

	int32 messageCount = 0;

	if(m->IsServer)
	{
		// Server: receive from poll group
		if(m->PollGroup != k_HSteamNetPollGroup_Invalid)
		{
			messageCount = m->Sockets->ReceiveMessagesOnPollGroup(
				m->PollGroup,
				m->MessagePool.data(),
				maxMessages);
		}
	}
	else
	{
		// Client: receive from server connection
		if(m->ClientConnection != k_HSteamNetConnection_Invalid)
		{
			messageCount = m->Sockets->ReceiveMessagesOnConnection(
				m->ClientConnection,
				m->MessagePool.data(),
				maxMessages);
		}
	}

	// Convert GNS messages to NetworkMessage
	for(int32 i = 0; i < messageCount; ++i)
	{
		ISteamNetworkingMessage* gnsMessage = m->MessagePool[i];

		NetworkMessage message;
		message.Sender = m->GetOrAllocateConnectionID(gnsMessage->m_conn);
		message.Data = (const u8*)gnsMessage->GetData();
		message.Length = gnsMessage->GetSize();
		message.MessageType = (message.Length > 0) ? message.Data[0] : 0;
		message.BackendData = gnsMessage;

		outMessages.push_back(message);
	}
}

void NetworkPeer::SendMessage(ConnectionID connection, const u8* data, u32 size, NetworkSendFlags flags)
{
	if(!m->Sockets)
		return;

	HSteamNetConnection handle = m->GetConnectionHandle(connection);
	if(handle == k_HSteamNetConnection_Invalid)
	{
		B3D_LOG(Error, LogNetwork, "Cannot send message, invalid connection ID.");
		return;
	}

	int32 gnsFlags = MapSendFlagsToGNS(flags);

	EResult result = m->Sockets->SendMessageToConnection(
		handle,
		data,
		size,
		gnsFlags,
		nullptr);

	if(result != k_EResultOK)
	{
		B3D_LOG(Warning, LogNetwork, "Failed to send message, error code: {0}", (int32)result);
	}
}

void NetworkPeer::BroadcastMessage(const u8* data, u32 size, NetworkSendFlags flags)
{
	if(!m->Sockets || !m->IsServer)
		return;

	int32 gnsFlags = MapSendFlagsToGNS(flags);

	// Send to all connected clients
	for(auto& pair : m->IDToConnection)
	{
		m->Sockets->SendMessageToConnection(
			pair.second,
			data,
			size,
			gnsFlags,
			nullptr);
	}
}

bool NetworkPeer::GetConnectionInfo(ConnectionID connection, ConnectionInformation& outInfo) const
{
	if(!m->Sockets)
		return false;

	HSteamNetConnection handle = m->GetConnectionHandle(connection);
	if(handle == k_HSteamNetConnection_Invalid)
		return false;

	SteamNetConnectionRealTimeStatus_t status;
	if(m->Sockets->GetConnectionRealTimeStatus(handle, &status, 0, nullptr) != k_EResultOK)
		return false;

	SteamNetConnectionInfo_t connectionInfo;
	if(!m->Sockets->GetConnectionInfo(handle, &connectionInfo))
		return false;

	outInfo.ID = connection;
	GNSToNetworkAddress(connectionInfo.m_addrRemote, outInfo.RemoteAddress);
	outInfo.PingMS = (float)status.m_nPing;
	// GNS doesn't provide packet loss directly, use connection quality instead (0-1, lower is worse)
	outInfo.PacketLossPercent = (1.0f - status.m_flConnectionQualityLocal) * 100.0f;
	outInfo.JitterMS = status.m_usecMaxJitter / 1000.0f; // Convert microseconds to milliseconds
	outInfo.BytesSentPerSecond = (u64)status.m_flOutBytesPerSec;
	outInfo.BytesReceivedPerSecond = (u64)status.m_flInBytesPerSec;

	return true;
}

bool NetworkPeer::IsConnected(ConnectionID connection) const
{
	return GetConnectionState(connection) == ConnectionState::Connected;
}

ConnectionState NetworkPeer::GetConnectionState(ConnectionID connection) const
{
	if(!m->Sockets)
		return ConnectionState::None;

	HSteamNetConnection handle = m->GetConnectionHandle(connection);
	if(handle == k_HSteamNetConnection_Invalid)
		return ConnectionState::None;

	SteamNetConnectionInfo_t info;
	if(!m->Sockets->GetConnectionInfo(handle, &info))
		return ConnectionState::None;

	switch(info.m_eState)
	{
	case k_ESteamNetworkingConnectionState_None:
		return ConnectionState::None;
	case k_ESteamNetworkingConnectionState_Connecting:
		return ConnectionState::Connecting;
	case k_ESteamNetworkingConnectionState_FindingRoute:
		return ConnectionState::Connecting;
	case k_ESteamNetworkingConnectionState_Connected:
		return ConnectionState::Connected;
	case k_ESteamNetworkingConnectionState_ClosedByPeer:
		return ConnectionState::Disconnected;
	case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
		return ConnectionState::Problem;
	default:
		return ConnectionState::None;
	}
}

void NetworkPeer::FreeMessage(NetworkMessage& message)
{
	if(message.BackendData)
	{
		ISteamNetworkingMessage* gnsMessage = (ISteamNetworkingMessage*)message.BackendData;
		gnsMessage->Release();
		message.BackendData = nullptr;
	}
}

NetworkAddress::NetworkAddress(const char* address)
{
	// Parse address string (may contain port after '|')
	String addressString(address);
	size_t pipePos = addressString.find('|');

	if(pipePos != String::npos)
	{
		String ipString = addressString.substr(0, pipePos);
		String portString = addressString.substr(pipePos + 1);
		Port = (u16)atoi(portString.c_str());

		// Parse IP portion
		SteamNetworkingIPAddr gnsAddress;
		if(gnsAddress.ParseString(ipString.c_str()))
		{
			GNSToNetworkAddress(gnsAddress, *this);
		}
	}
	else
	{
		// No port specified
		SteamNetworkingIPAddr gnsAddress;
		if(gnsAddress.ParseString(address))
		{
			GNSToNetworkAddress(gnsAddress, *this);
		}
	}
}

NetworkAddress::NetworkAddress(const char* ip, u16 port)
{
	SteamNetworkingIPAddr gnsAddress;
	if(gnsAddress.ParseString(ip))
	{
		GNSToNetworkAddress(gnsAddress, *this);
		Port = port;
	}
}

String NetworkAddress::ToString(bool withPort) const
{
	SteamNetworkingIPAddr gnsAddress = NetworkToGNSAddress(*this);
	char buffer[128];
	gnsAddress.ToString(buffer, sizeof(buffer), withPort);
	return String(buffer);
}

bool NetworkAddress::CompareIP(const NetworkAddress& other) const
{
	if(IPType != other.IPType)
		return false;

	if(IPType == IPV4)
		return memcmp(IP, other.IP, 4) == 0;

	return (memcmp(IP, other.IP, sizeof(IP)) == 0 && IP6FlowInfo == other.IP6FlowInfo && IP6ScopeId == other.IP6ScopeId);
}

NetworkAddress& NetworkAddress::operator=(const NetworkAddress& rhs)
{
	IPType = rhs.IPType;
	Port = rhs.Port;
	IP6FlowInfo = rhs.IP6FlowInfo;
	IP6ScopeId = rhs.IP6ScopeId;
	memcpy(&IP, &rhs.IP, sizeof(IP));
	return *this;
}

bool NetworkAddress::operator==(const NetworkAddress& rhs) const
{
	return Port == rhs.Port && CompareIP(rhs);
}

bool NetworkAddress::operator!=(const NetworkAddress& rhs) const
{
	return !(*this == rhs);
}

NetworkAddress NetworkAddress::UNASSIGNED;

void Network::Host(u16 port, u32 maxConnections)
{
	if(mPeer)
	{
		B3D_LOG(Error, LogNetwork, "Cannot start hosting when an existing network connection is active.");
		return;
	}

	mPeer = B3DMakeUnique<NetworkPeer>();
	if(!mPeer->StartServer(port, maxConnections))
	{
		mPeer = nullptr;
		return;
	}

	mState = NetworkState::Hosting;
}

void Network::Connect(const char* host, u16 port)
{
	if(mPeer)
	{
		B3D_LOG(Error, LogNetwork, "Cannot connect when an existing network connection is active.");
		return;
	}

	NetworkAddress serverAddress(host, port);

	mPeer = B3DMakeUnique<NetworkPeer>();
	ConnectionID connection = mPeer->Connect(serverAddress);

	if(!connection.IsValid())
	{
		mPeer = nullptr;
		return;
	}

	mState = NetworkState::Connecting;
}

void Network::Disconnect()
{
	mPeer = nullptr;
	mState = NetworkState::Disconnected;
}

void Network::Update(float dt)
{
	// Placeholder for future high-level networking features
	// For Phase 1.1, users interact directly with NetworkPeer
}
