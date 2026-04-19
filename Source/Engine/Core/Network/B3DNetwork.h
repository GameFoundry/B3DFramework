//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Utility/B3DModule.h"

namespace b3d
{
	/** @addtogroup Network
	 *  @{
	 */

	/** Supported versions of internet protocol (IP) and their representative address formats. */
	enum IPType
	{
		IPV4,
		IPV6
	};

	/** Represents an IPv4 or IPv6 network address. */
	struct NetworkAddress
	{
		/** Creates a null address. */
		NetworkAddress() = default;

		/**
		 * Creates the system address from a printable IP string, with an optional port component.
		 *
		 * @param		address		Null-terminated string such as "192.0.0.1" or "2001:db8:63b3:1::3490". The string
		 *							can optionally also contain a port component, delimited using "|", such as
		 *							"192.0.0.1|1234".
		 */
		NetworkAddress(const char* address);

		/**
		 * Creates the system address from a printable IP string, with a separately specified port component.
		 *
		 * @param		ip		Null-terminated string such as "192.0.0.1" or "2001:db8:63b3:1::3490".
		 * @param		port	Port in range [0, 65535].
		 */
		NetworkAddress(const char* ip, u16 port);

		/**
		 * Converts the network address into printable string, with an optional port component.
		 *
		 * @param		withPort	If true the returned string will contain a port component delimited with "|"
		 *							after the IP string. For example, "192.0.0.1|1234".
		 * @return					Printable IP string, such as "192.0.0.1" or "2001:db8:63b3:1::3490". If
		 *							@p withPort is true the string will also include a port component.
		 */
		String ToString(bool withPort = false) const;

		/**
		 * Compares the IP portion of a network address with another address (ignoring port).
		 *
		 * @param	other	Other address to compare with.
		 * @return			True if the IP addresses match, false otherwise.
		 */
		bool CompareIP(const NetworkAddress& other) const;

		NetworkAddress& operator=(const NetworkAddress& rhs);
		bool operator==(const NetworkAddress& rhs) const;
		bool operator!=(const NetworkAddress& rhs) const;

		/** Represents the default unassigned (null) network address. */
		static NetworkAddress UNASSIGNED;

		u8 IP[16] = { 0 };
		u16 Port = 0;
		IPType IPType = IPV4;
		u64 IP6FlowInfo = 0;
		u64 IP6ScopeId = 0;
	};

	/** ID uniquely representing a network connection. */
	struct ConnectionID
	{
		ConnectionID() = default;

		explicit ConnectionID(u64 id)
			: mID(id)
		{
		}

		bool IsValid() const
		{
			return mID != 0;
		}

		static ConnectionID Invalid()
		{
			return ConnectionID(0);
		}

		static ConnectionID Server()
		{
			return ConnectionID(~0ULL);
		}

		bool operator==(const ConnectionID& other) const
		{
			return mID == other.mID;
		}

		bool operator!=(const ConnectionID& other) const
		{
			return mID != other.mID;
		}

		u64 GetID() const
		{
			return mID;
		}

	private:
		u64 mID = 0;
	};

	/** Flags controlling how messages are sent over the network. */
	enum class NetworkSendFlagBits : u32
	{
		/** Default unreliable send. */
		None = 0,

		/** Ensure reliable delivery (message will be resent until acknowledged). */
		Reliable = 1 << 0,

		/** Disable Nagle's algorithm (send immediately, don't wait to coalesce). */
		NoNagle = 1 << 1,

		/** Bypass queue and send immediately (combines NoNagle with additional priority). */
		NoDelay = 1 << 2,

		/** Optimized unreliable + no delay combination. */
		UnreliableNoDelay = 1 << 3,

		/** Use calling thread for sending instead of background thread. */
		UseCurrentThread = 1 << 4
	};

	typedef Flags<NetworkSendFlagBits> NetworkSendFlags;
	B3D_FLAGS_OPERATORS(NetworkSendFlagBits);

	/** Represents a message received from the network. */
	struct NetworkMessage
	{
		/** Type of message (user-defined). */
		u8 MessageType = 0;

		/** Connection that sent this message. */
		ConnectionID Sender;

		/** Pointer to message data (includes the MessageType byte as first byte). */
		const u8* Data = nullptr;

		/** Total length of message data in bytes. */
		u32 Length = 0;

		/** @name Internal
		 *  @{
		 */

		void* BackendData = nullptr;

		/** @} */
	};

	/** Connection state information and quality metrics. */
	struct ConnectionInformation
	{
		/** Unique identifier for this connection. */
		ConnectionID ID;

		/** Remote address of the peer. */
		NetworkAddress RemoteAddress;

		/** Current round-trip ping time in milliseconds. */
		float PingMS = 0.0f;

		/** Packet loss percentage (0.0 to 100.0). */
		float PacketLossPercent = 0.0f;

		/** Connection quality (jitter) in milliseconds. */
		float JitterMS = 0.0f;

		/** Outgoing bandwidth in bytes per second. */
		u64 BytesSentPerSecond = 0;

		/** Incoming bandwidth in bytes per second. */
		u64 BytesReceivedPerSecond = 0;
	};

	/** Connection state. */
	enum class ConnectionState
	{
		/** No connection. */
		None,

		/** Attempting to connect. */
		Connecting,

		/** Successfully connected. */
		Connected,

		/** Connection is being closed. */
		Disconnecting,

		/** Connection has been closed. */
		Disconnected,

		/** Connection problem detected. */
		Problem
	};

	/**
	 * Represents a single peer in a network hierarchy. Allows you to create a server that listens for incoming
	 * connections, or connect as a client to a remote server. Supports sending and receiving messages with
	 * configurable reliability and ordering guarantees.
	 */
	class B3D_EXPORT NetworkPeer
	{
	public:
		NetworkPeer();
		~NetworkPeer();

		/**
		 * Starts the peer as a server, listening for incoming connections on the specified port.
		 *
		 * @param	port				Port to listen on.
		 * @param	maxConnections		Maximum number of simultaneous connections allowed.
		 * @return						True if the server started successfully, false otherwise.
		 */
		bool StartServer(u16 port, u32 maxConnections);

		/**
		 * Connects to a remote server as a client.
		 *
		 * @param	address		Network address of the server to connect to.
		 * @return				Connection ID that will be valid once connection completes, or Invalid if the
		 *						connection attempt failed to start.
		 */
		ConnectionID Connect(const NetworkAddress& address);

		/**
		 * Disconnects from a specific peer.
		 *
		 * @param	connection	Connection ID to disconnect.
		 */
		void Disconnect(ConnectionID connection);

		/**
		 * Disconnects all connected peers and shuts down the peer.
		 */
		void DisconnectAll();

		/**
		 * Polls for new messages received from the network. Messages are appended to the provided container.
		 *
		 * @param	outMessages		Container to fill with received messages. Will be cleared before filling.
		 *							Users should maintain a persistent container to avoid per-frame allocations.
		 * @param	maxMessages		Maximum number of messages to retrieve in a single poll.
		 *
		 * @warning	You MUST call FreeMessage() for each message after processing to avoid memory leaks.
		 *			Failure to free messages will leak memory from the underlying network backend.
		 *
		 * Example:
		 * @code
		 * Vector<NetworkMessage> messages;
		 * peer.PollMessages(messages);
		 * for(auto& msg : messages)
		 * {
		 *     // Process message...
		 *     peer.FreeMessage(msg);  // Must free each message!
		 * }
		 * @endcode
		 */
		void PollMessages(Vector<NetworkMessage>& outMessages, u32 maxMessages = 32);

		/**
		 * Sends a message to a specific connection.
		 *
		 * @param	connection	Connection ID to send to.
		 * @param	data		Pointer to message data. First byte should be the message type.
		 * @param	size		Size of the message data in bytes.
		 * @param	flags		Flags controlling send behavior (reliability, priority, etc.).
		 */
		void SendMessage(ConnectionID connection, const u8* data, u32 size, NetworkSendFlags flags = NetworkSendFlagBits::Reliable);

		/**
		 * Broadcasts a message to all connected peers.
		 *
		 * @param	data	Pointer to message data. First byte should be the message type.
		 * @param	size	Size of the message data in bytes.
		 * @param	flags	Flags controlling send behavior (reliability, priority, etc.).
		 */
		void BroadcastMessage(const u8* data, u32 size, NetworkSendFlags flags = NetworkSendFlagBits::Reliable);

		/**
		 * Retrieves connection quality information for a specific connection.
		 *
		 * @param	connection	Connection to query.
		 * @param	outInfo		Structure to fill with connection information.
		 * @return				True if connection info was retrieved successfully, false if connection is invalid.
		 */
		bool GetConnectionInfo(ConnectionID connection, ConnectionInformation& outInfo) const;

		/**
		 * Checks if a connection is currently active.
		 *
		 * @param	connection	Connection to check.
		 * @return				True if connected, false otherwise.
		 */
		bool IsConnected(ConnectionID connection) const;

		/**
		 * Retrieves the current state of a connection.
		 *
		 * @param	connection	Connection to query.
		 * @return				Current connection state.
		 */
		ConnectionState GetConnectionState(ConnectionID connection) const;

		/**
		 * Frees a message received via PollMessages. Must be called for each message after processing.
		 *
		 * @param	message		Message to free.
		 */
		void FreeMessage(NetworkMessage& message);

	private:
		struct Impl;
		Impl* m;
	};

	/**
	 * High-level networking class that wraps NetworkPeer to provide simple server/client functionality.
	 *
	 * This is a convenience wrapper around NetworkPeer for applications that need basic networking
	 * without manual peer management. For Phase 1.1, this class provides only basic hosting/connecting
	 * functionality. High-level features like entity replication will be added in future phases.
	 *
	 * Example usage as server:
	 * @code
	 * Network::Instance().Host(7777, 32);  // Host on port 7777, max 32 clients
	 * @endcode
	 *
	 * Example usage as client:
	 * @code
	 * Network::Instance().Connect("127.0.0.1", 7777);
	 * @endcode
	 *
	 * For full control over networking, use NetworkPeer directly instead of this wrapper.
	 */
	class B3D_EXPORT Network : public Module<Network>
	{
	public:
		/**
		 * Checks if this instance is currently hosting a server.
		 *
		 * @return True if hosting, false otherwise.
		 */
		bool IsHost() const
		{
			return mState == NetworkState::Hosting;
		}

		/**
		 * Checks if this instance is a connected or connecting client.
		 *
		 * @return True if client connection is active or being established, false otherwise.
		 */
		bool IsClient() const
		{
			return mState == NetworkState::Connected || mState == NetworkState::Connecting;
		}

		/**
		 * Starts hosting a server on the specified port.
		 *
		 * @param port			Port to listen on for incoming connections.
		 * @param maxConnections	Maximum number of simultaneous client connections allowed.
		 */
		void Host(u16 port, u32 maxConnections = 64);

		/**
		 * Connects to a remote server.
		 *
		 * @param host	Hostname or IP address of the server (e.g., "127.0.0.1" or "example.com").
		 * @param port	Port of the server to connect to.
		 */
		void Connect(const char* host, u16 port);

		/**
		 * Disconnects from the current network session (server or client).
		 */
		void Disconnect();

		/**
		 * Updates the network system. Currently a stub for Phase 1.1.
		 *
		 * @param dt	Time elapsed since last update, in seconds.
		 */
		void Update(float dt);

	private:
		enum class NetworkState
		{
			Disconnected,
			Connected,
			Hosting,
			Connecting,
		};

		NetworkState mState = NetworkState::Disconnected;
		UPtr<NetworkPeer> mPeer;
	};

	/** @} */
}
