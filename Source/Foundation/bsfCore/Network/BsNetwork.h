//************************************ bs::framework - Copyright 2019 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"

namespace bs
{
	struct SerializationContext;
	/** @addtogroup Network-Internal
	 *  @{
	 */

	static constexpr UINT8 NETWORK_BACKEND_FIRST_FREE_ID = 134;

	// TODO - Doc
	enum NetworkMessageType
	{
		NWM_ReplicationSync = NETWORK_BACKEND_FIRST_FREE_ID,

		NWM_User
	};


	/** @} */

	/** @addtogroup Network
	 *  @{
	 */

	static constexpr UINT8 NETWORK_USER_MESSAGE_ID = NWM_User;

	/** Supported versions of internet protocol (IP) and they're representative address formats. */
	enum IPType
	{
		IPV4, IPV6
	};

	/** Represents an IPv4 or IPv6 network address. */
	struct NetworkAddress
	{
		/** Creates a null address. */
		NetworkAddress() = default;

		/**
		 * Creates the system address from a printable IP string, with an optional port component.
		 *
		 * @param[in]		address		Null-terminated string such as "192.0.0.1" or "2001:db8:63b3:1::3490". The string
		 *								can optionally also contain a port component, delimited using "|", such as
		 *								"192.0.0.1|1234".
		 */
		NetworkAddress(const char* address);

		/**
		 * Creates the system address from a printable IP string, with a separately specified port component.
		 *
		 * @param[in]		ip		Null-terminated string such as "192.0.0.1" or "2001:db8:63b3:1::3490".
		 * @param[in]		port	Port in range [0, 65535].
		 */
		NetworkAddress(const char* ip, UINT16 port);

		/**
		 * Converts the network address into printable string, with an optional port component.
		 *
		 * @param[in]		withPort		If true the returned string will contain a port component delimited with "|"
		 *									after the IP string. For example, "192.0.0.1|1234".
		 * @return							Printable IP string, such as "192.0.0.1" or "2001:db8:63b3:1::3490". If
		 *									@p withPort is true the string will also include a port component.
		 */
		String toString(bool withPort = false) const;

		/**
		 * Compares the IP portion of a network address with another address (ignoring port).
		 *
		 * @param[in]	other		Other address to compare with.
		 * @return					True if the IP addresses match, false otherwise.
		 */
		bool compareIP(const NetworkAddress& other) const;

		NetworkAddress& operator=(const NetworkAddress& rhs);
		bool operator==(const NetworkAddress& rhs) const;
		bool operator!=(const NetworkAddress& rhs) const;

		/** Represents the default unassigned (null) network address. */
		static NetworkAddress UNASSIGNED;

		UINT8 ip[16] = { 0 };
		UINT16 port = 0;
		IPType ipType = IPV4;
		UINT64 ip6FlowInfo = 0;
		UINT64 ip6ScopeId = 0;
	};

	/** ID uniquely representing a network connection. */
	struct NetworkId
	{
		NetworkId() = default;
		NetworkId(INT32 id) : id(id) { }

		INT32 id = -1;
	};

	/** List of possible events that can be returned by the networking system. */
	enum class NetworkEventType
	{
		/** Requested connection has been established. */
		ConnectingDone,

		/** Failed establishing a connection. */
		ConnectingFailed,

		/** Attempting to connect to a peer we're already connected to. */
		AlreadyConnected,

		/** New incoming connection request has been received and accepted. */
		IncomingNew,

		/**
		 * New incoming connection request has been received but cannot be fulfilled as we have reached the maximum
		 * incoming connection capacity.
		 */
		IncomingNoFree,

		/**
		 * A peer has gracefully disconnected from us. This can be a client disconnecting from a server, or a server going
		 * offline.
		 */
		Disconnected,

		/** A peer can no longer be reached and the connection has been lost. */
		LostConnection,

		/** Event contains a user-specified message type and data. */
		Data
	};

	/** Determines when are packets sent. */
	enum class PacketPriority
	{
		/**
		 * Packets will be send immediately (not waiting for the next network thread update). This also means the packets
		 * will not be aggregated, resulting in higher per-packet overhead at the benefit to latency.
		 */
		Immediate,

		/** More of these packets will be sent compared to medium priority ones. */
		High,

		/** More of these packets will be sent compared to low priority ones. */
		Medium,

		/** Lowest packet priority, resulting in least packets sent comapred to other priorities. */
		Low
	};

	/** Determines how should packets that were failed to be delivered be handled. */
	enum class PacketReliability
	{
		/**
		 * Lost packets will send again until successfuly delivered. Internally this comes with an overhead as each
		 * packet delivery must be confirmed, as well as the overhead for retransmisson itself.
		 */
		Reliable,

		/**
		 * Lost packets will be ignored and never received by the remote. Internally this comes with low overhead as
		 * packets can be sent using a 'fire-and-forget' approach.
		 */
		Unreliable
	};

	/** Determines in what order are sent packets allowed to arrive in. */
	enum class PacketOrdering
	{
		/** Packets are allowed to be received in order different from the order they were sent in. */
		Unordered,

		/**
		 * Packets will arrive in the order you have sent them. This means events might be delayed waiting for
		 * out-of-order packets. When used with 'Unreliable' reliability this is the same as 'Sequenced'.
		 */
		Ordered,

		/**
		 * Packets will arrive in the order you have sent them, but ones older than the last received packet will
		 * be ignored.
		 */
		Sequenced
	};

	/** Controls reliability, ordering and priority of sent packets. */
	struct PacketChannel
	{		
		PacketPriority priority;
		PacketReliability reliability;
		PacketOrdering ordering;

		static PacketChannel DEFAULT;

		// TODO - Use template or create static versions of common channels
	};

	/** Raw bytes representing data sent or received over a network. First byte always contains the message ID. */
	struct PacketData
	{
		UINT8* bytes = nullptr;
		UINT32 length = 0;
	};

	/** Represents a piece of information received by the network peer. */
	struct NetworkEvent
	{
		/** Type of the network event. */
		NetworkEventType type;

		/** Identifier of the peer that sent the event. */
		NetworkId sender;

		/** User-specified data attached to the event. Only relevant if event type is 'Data'. */
		PacketData data;

		/** @name Internal
		 *  @{
		 */

		void* _backendData = nullptr;

		/** @} */
	};

	/** Information required for initializing a new network peer. */
	struct NETWORK_PEER_DESC
	{
		/**
		 * A list of addresses and ports the peer should be listening on. For clients this can usually be a single
		 * null network address. For servers this can usually be a single network address with a port to listen on.
		 */
		SmallVector<NetworkAddress, 4> listenAddresses;

		/**
		 * Maximum number of connections (both incoming and outgoing) allowed to be established by the peer. In a regular
		 * client-server model, this should be 1 for client, and same as @p maxNumIncomingConnections for server.
		 */
		UINT32 maxNumConnections = 1;

		/**
		 * Maximum number of incoming connections allowed to be accepted by the peer. In a regular client-server model
		 * this should be 0 for the client, and N for server, where N is the maximum number of clients allowed to be
		 * connected at once.
		 */
		UINT32 maxNumIncomingConnections = 0;
	};

	/**
	 * Represents a single peer in a network hierarchy. Allows you to connect to other peers on the network by sending
	 * explicit connection requests, or listening for incoming requests. Allows you to send and receive data from
	 * connected peers.
	 */
	class BS_CORE_EXPORT NetworkPeer
	{
	public:
		/** Constructs a new network peer using the provided initial setting descriptor. */
		NetworkPeer(const NETWORK_PEER_DESC& desc);
		~NetworkPeer();

		// TODO Low Priority - Perhaps disallow domain-name here and use NetworkAddress directly?
		//  - Add a separate async method for resolving domain name beforehand

		/**
		 * Attempts to connect to a new peer. This will execute asynchronously and you must query @p receive() for a
		 * network event of type 'ConnectingDone' to confirm the connection has been established.
		 *
		 * @param[in]		host		Host name of the peer to connect to. This can be an IP address or a domain name.
		 * @param[in]		port		Port on which to try connecting to.
		 * @return						True if the connection attempt succesfully started, and false otherwise. If false
		 *								the relevant error message will be printed in the log.
		 */
		bool connect(const char* host, UINT16 port);

		/**
		 * Disconnects from a previously connected remote peer.
		 *
		 * @param[in]	address		Address of the peer to disconnect from.
		 * @param[in]	silent		If true the peer will neatly disconnect from the remote by first sending a disconnect
		 *							message. If false the peer will immediately close the connection without notifying the
		 *							remote.
		 */
		void disconnect(const NetworkAddress& address, bool silent = false);

		/**
		 * Disconnects from a previously connected remote peer.
		 *
		 * @param[in]	id			Unique network id of the peer to disconnect from.
		 * @param[in]	silent		If true the peer will neatly disconnect from the remote by first sending a disconnect
		 *							message. If false the peer will immediately close the connection without notifying the
		 *							remote.
		 */
		void disconnect(const NetworkId& id, bool silent = false);

		// TODO Low priority - Maybe make use of UPtr to free the event? Instead on relying on the user to manually
		// free them

		/**
		 * Checks if there are any available network events and returns the first available event.

		 * @return		First available event, or null of none available. Received events must be freed by calling
		 *				free().
		 */
		NetworkEvent* receive() const;

		// TODO Low priority - Hide NETWORK_USER_MESSAGE_ID from the outside world. Allow user to use an ID starting at 0.

		/**
		 * Attempts to send some data to the specified remote peer.
		 *
		 * @param[in]	data		Data to send in the form of raw bytes. The first byte of your message /must/ contain
		 *							the message identifier, starting with NETWORK_USER_MESSAGE_ID (lower identifiers are
		 *							reserved).
		 * @param[in]	destination	Network address to send the message to. You must be currently connected to the
		 *							peer at the specified address.
		 * @param[in]	channel		Channel determining reliability, ordering and priority of the sent data.
		 *							
		 * @note		Whenever possible prefer to use the variant of this method that accepts NetworkId for the
		 *				@p destination parameter, as it is faster.
		 */
		void send(const PacketData& data, const NetworkAddress& destination,
			const PacketChannel& channel = PacketChannel::DEFAULT);

		/**
		 * Attempts to send some data to the specified remote peer.
		 *
		 * @param[in]	data		Data to send in the form of raw bytes. The first byte of your message /must/ contain
		 *							the message identifier, starting with NETWORK_USER_MESSAGE_ID (lower identifiers are
		 *							reserved).
		 * @param[in]	destination	Network id of the peer to send the message to.
		 * @param[in]	channel		Channel determining reliability, ordering and priority of the sent data.
		 */
		void send(const PacketData& data, const NetworkId& destination,
			const PacketChannel& channel = PacketChannel::DEFAULT);

		/**
		 * Broadcasts some data to all currently connected peers.
		 *
		 * @param[in]	data		Data to send in the form of raw bytes. The first byte of your message /must/ contain
		 *							the message identifier, starting with NETWORK_USER_MESSAGE_ID (lower identifiers are
		 *							reserved).
		 * @param[in]	channel		Channel determining reliability, ordering and priority of the sent data.
		 */
		void broadcast(const PacketData& data, const PacketChannel& channel);

		/** Frees a network event received though a call to @p receive(). */
		void free(NetworkEvent* event);

		// TODO - Other methods needed:
		// AveragePing(AddrOrId)
		// LastPing(AddOrId)
		// GetConnectionList
		// GetConnectionState(AddrOrId)
		// GetMyId
		// GetMyInternalIp(AddrOrId)
		// GetMyExternalIp(AddrOrId)
		// Ping(AddrOrId)
		// NetworkId getNetworkId(const NetworkAddress& address);
		// NetworkAddress getNetworkAddress(const NetworkId& id);

		// TODO - create() methods
		// - Simple ones for server (port only), and client (no listeners or incoming connections)

	private:
		struct Pimpl;
		Pimpl* m;
	};

	/** Represents a particular state of a network object at a certain point in time. */
	struct BS_CORE_EXPORT NetworkObjectState
	{
		SPtr<SerializedObject> state;
	};

	/** Base class for all objects that can be manipulated using the high-level networking system. */
	class BS_CORE_EXPORT NetworkObject : public virtual IReflectable
	{
		enum State
		{
			NotReplicated,
			Replicated,
		};

		// TODO - Doc
		const UUID& getNetworkUUID() const { return mNetworkUUID; }

		// TODO - Special case for spawning prefabs, to avoid syncing all their state
		// (Probably not part of this class, but keeping the comment here so I don't forget)

		/** 
		 * Spawns the network object across all relevant clients. Only usable on the server. If the object is already
		 * spawned, no change is made.
		 */
		void networkSpawn();

		/**
		 * Destroys the object across all clients it was spawned on. Only usable on the server. If the object is already
		 * destroyed, or hasn't been spawned at all, no change is made.
		 */
		void networkDespawn();

		/** Gets the current state of all replicable fields on an object. */
		NetworkObjectState getNetworkState() const;
		
	public:
		NetworkObject() = default;
		~NetworkObject();

	private:
		friend class Network;

		UUID mNetworkUUID;
		State mState = NotReplicated;
	};

	// TODO - Refactor this class so it uses more optimal data layout and uses Bitstream, instead of just calling the
	// generic BinarySerializer. When serializing entire state there is no need to write field information, as long as the
	// peers keep their data scheme in sync.
	// TODO - Doc
	class NetworkEncoder
	{
		static constexpr const UINT32 WRITE_BUFFER_SIZE = 16384;

		struct BufferPiece
		{
			UINT8* buffer = nullptr;
			UINT32 size = 0;
		};
	public:
		NetworkEncoder();
		~NetworkEncoder();

		void encode(UINT8 type, const UUID& uuid, IReflectable* object, SerializationContext* context = nullptr);
		UINT8* getOutput(UINT32& size);
		void clear();

	private:
		BufferPiece allocBufferPiece();

		Vector<BufferPiece> mBufferPieces;
		Vector<BufferPiece> mBufferPiecePool;

		UINT8* mWriteBuffer = nullptr;
		UINT32 mWriteBufferOffset = 0;

		UINT32 mResultBufferSize = 0;
		UINT8* mResultBuffer = nullptr;
		UINT32 mBytesWritten = 0;
	};

	class NetworkDecoder
	{
	public:
		NetworkDecoder(const SPtr<MemoryDataStream>& data);

		SPtr<IReflectable> decode(UINT8& type, UUID& uuid, SerializationContext* context = nullptr);

	private:
		SPtr<MemoryDataStream> mInputStream;
	};

	/** 
	 * High-level networking class that utilizes the low-level networking systems to easily host a server, connect to
	 * a server, and handle high level concepts such as object data replication and remote procedure calls.
	 */
	class BS_CORE_EXPORT Network : public Module<Network>
	{
	public:
		// TODO- Doc

		bool isHost() const { return mState == NetworkState::Hosting; }
		bool isClient() const { return mState == NetworkState::Connected || mState == NetworkState::Connecting; }

		// TODO - Handle cases when network is already in host or client state when one of these is called again
		void host(const SmallVector<NetworkAddress, 4>& listenAddresses, UINT32 tickRate = 30, UINT32 maxConnections = 64);
		void connect(const char* host, UINT16 port);
		void disconnect();

		void update(float dt);

		void _notifyNetworkObjectSpawned(NetworkObject* object);
		void _notifyNetworkObjectDespawned(NetworkObject* object);
		void _notifyNetworkObjectDestroyed(NetworkObject* object);

	private:
		enum ObjectActionType
		{
			Spawning,
			Spawned,
			Despawning
		};

		enum class NetworkState
		{
			Disconnected,
			Connected,
			Hosting,

			// Intermediate states
			Connecting,
		};

		struct ObjectAction
		{
			ObjectAction(const UUID& uuid, ObjectActionType type)
				:uuid(uuid), type(type)
			{ }

			ObjectActionType type;
			UUID uuid;
		};

		struct ObjectInfo
		{
			NetworkObject* obj;
			NetworkObjectState state;
		};

		NetworkState mState = NetworkState::Disconnected;
		UINT32 mTickRate = 30; // TODO - Allow different objects to have different tick rates (globally perhaps provide a multiplier? and a default rate?)

		Vector<ObjectAction> mActions;
		UnorderedMap<UUID, ObjectInfo> mNetworkObjects;

		float mTimeAccumulator = 0.0f;
		NetworkEncoder mEncoder;
		NetworkDecoder mDecoder;

		UPtr<NetworkPeer> mPeer;
	};

	/** @} */
}
