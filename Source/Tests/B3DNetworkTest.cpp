//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Testing/B3DTestSuite.h"
#include "Network/B3DNetwork.h"
#include "Threading/B3DThreadUtility.h"
#include <thread>
#include <chrono>

using namespace b3d;

class NetworkTestSuite : public TestSuite
{
public:
	NetworkTestSuite();

private:
	void TestNetworkAddress();
	void TestServerCreation();
	void TestClientConnection();
	void TestMessageSending();
	void TestReliableMessages();
	void TestUnreliableMessages();
	void TestBroadcastMessages();
	void TestConnectionInfo();
	void TestGracefulDisconnect();
	void TestMultipleClients();

	static constexpr u16 TEST_PORT = 7777;
	static constexpr u32 TEST_TIMEOUT_MS = 5000;
};

NetworkTestSuite::NetworkTestSuite()
{
	B3D_ADD_TEST(NetworkTestSuite::TestNetworkAddress)
	B3D_ADD_TEST(NetworkTestSuite::TestServerCreation)
	B3D_ADD_TEST(NetworkTestSuite::TestClientConnection)
	B3D_ADD_TEST(NetworkTestSuite::TestMessageSending)
	B3D_ADD_TEST(NetworkTestSuite::TestReliableMessages)
	B3D_ADD_TEST(NetworkTestSuite::TestUnreliableMessages)
	B3D_ADD_TEST(NetworkTestSuite::TestBroadcastMessages)
	B3D_ADD_TEST(NetworkTestSuite::TestConnectionInfo)
	B3D_ADD_TEST(NetworkTestSuite::TestGracefulDisconnect)
	B3D_ADD_TEST(NetworkTestSuite::TestMultipleClients)
}

void NetworkTestSuite::TestNetworkAddress()
{
	// Test IPv4 address parsing
	NetworkAddress addr1("127.0.0.1", 8080);
	B3D_TEST_ASSERT(addr1.IPType == IPV4);
	B3D_TEST_ASSERT(addr1.Port == 8080);

	// Test address with port string parsing
	NetworkAddress addr2("192.168.1.1|9000");
	B3D_TEST_ASSERT(addr2.IPType == IPV4);
	B3D_TEST_ASSERT(addr2.Port == 9000);

	// Test ToString
	String str = addr1.ToString(true);
	B3D_TEST_ASSERT(str.find("127.0.0.1") != String::npos);
	B3D_TEST_ASSERT(str.find("8080") != String::npos);

	// Test comparison
	NetworkAddress addr3("127.0.0.1", 8080);
	B3D_TEST_ASSERT(addr1 == addr3);
	B3D_TEST_ASSERT(addr1.CompareIP(addr3));

	NetworkAddress addr4("127.0.0.1", 9000);
	B3D_TEST_ASSERT(addr1 != addr4);
	B3D_TEST_ASSERT(addr1.CompareIP(addr4)); // Same IP, different port
}

void NetworkTestSuite::TestServerCreation()
{
	NetworkPeer server;

	// Test successful server startup
	bool result = server.StartServer(TEST_PORT, 32);
	B3D_TEST_ASSERT(result);

	// Test that we can't start another server on same peer
	bool result2 = server.StartServer(TEST_PORT + 1, 32);
	B3D_TEST_ASSERT(!result2);

	server.DisconnectAll();
}

void NetworkTestSuite::TestClientConnection()
{
	NetworkPeer server;
	NetworkPeer client;

	// Start server
	B3D_TEST_ASSERT(server.StartServer(TEST_PORT, 32));

	// Connect client
	NetworkAddress serverAddress("127.0.0.1", TEST_PORT);
	ConnectionID clientConnection = client.Connect(serverAddress);
	B3D_TEST_ASSERT(clientConnection.IsValid());

	// Wait for connection to establish (poll for up to TEST_TIMEOUT_MS)
	bool connected = false;
	auto startTime = std::chrono::steady_clock::now();
	Vector<NetworkMessage> messages;

	while(!connected)
	{
		auto now = std::chrono::steady_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count();

		if(elapsed > TEST_TIMEOUT_MS)
			break;

		// Poll both server and client
		server.PollMessages(messages);
		for(auto& msg : messages)
			server.FreeMessage(msg);

		client.PollMessages(messages);
		for(auto& msg : messages)
			client.FreeMessage(msg);

		// Check connection state
		if(client.IsConnected(clientConnection))
		{
			connected = true;
			break;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	B3D_TEST_ASSERT(connected);
	B3D_TEST_ASSERT(client.GetConnectionState(clientConnection) == ConnectionState::Connected);

	client.DisconnectAll();
	server.DisconnectAll();
}

void NetworkTestSuite::TestMessageSending()
{
	NetworkPeer server;
	NetworkPeer client;

	// Setup connection
	B3D_TEST_ASSERT(server.StartServer(TEST_PORT, 32));
	NetworkAddress serverAddress("127.0.0.1", TEST_PORT);
	ConnectionID clientConnection = client.Connect(serverAddress);
	B3D_TEST_ASSERT(clientConnection.IsValid());

	// Wait for connection
	bool connected = false;
	auto startTime = std::chrono::steady_clock::now();
	Vector<NetworkMessage> messages;
	ConnectionID serverSideClientConnection;

	while(!connected)
	{
		auto now = std::chrono::steady_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count();

		if(elapsed > TEST_TIMEOUT_MS)
			break;

		server.PollMessages(messages);
		for(auto& msg : messages)
		{
			serverSideClientConnection = msg.Sender;
			server.FreeMessage(msg);
		}

		client.PollMessages(messages);
		for(auto& msg : messages)
			client.FreeMessage(msg);

		if(client.IsConnected(clientConnection))
		{
			connected = true;
			break;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	B3D_TEST_ASSERT(connected);

	// Send message from client to server
	u8 testMessage[] = { 42, 'H', 'e', 'l', 'l', 'o' };
	client.SendMessage(clientConnection, testMessage, sizeof(testMessage), NetworkSendFlagBits::Reliable);

	// Wait for message to arrive at server
	bool messageReceived = false;
	startTime = std::chrono::steady_clock::now();

	while(!messageReceived)
	{
		auto now = std::chrono::steady_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count();

		if(elapsed > TEST_TIMEOUT_MS)
			break;

		server.PollMessages(messages);

		for(auto& msg : messages)
		{
			if(msg.Length == sizeof(testMessage) && msg.MessageType == 42)
			{
				B3D_TEST_ASSERT(memcmp(msg.Data, testMessage, sizeof(testMessage)) == 0);
				messageReceived = true;
			}
			server.FreeMessage(msg);
		}

		if(messageReceived)
			break;

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	B3D_TEST_ASSERT(messageReceived);

	client.DisconnectAll();
	server.DisconnectAll();
}

void NetworkTestSuite::TestReliableMessages()
{
	NetworkPeer server;
	NetworkPeer client;

	// Setup connection
	B3D_TEST_ASSERT(server.StartServer(TEST_PORT, 32));
	NetworkAddress serverAddress("127.0.0.1", TEST_PORT);
	ConnectionID clientConnection = client.Connect(serverAddress);

	// Wait for connection
	bool connected = false;
	auto startTime = std::chrono::steady_clock::now();
	Vector<NetworkMessage> messages;

	while(!connected && (std::chrono::steady_clock::now() - startTime) < std::chrono::milliseconds(TEST_TIMEOUT_MS))
	{
		server.PollMessages(messages);
		for(auto& msg : messages)
			server.FreeMessage(msg);

		client.PollMessages(messages);
		for(auto& msg : messages)
			client.FreeMessage(msg);

		if(client.IsConnected(clientConnection))
			connected = true;

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	B3D_TEST_ASSERT(connected);

	// Send multiple reliable messages
	constexpr u32 messageCount = 100;
	for(u32 i = 0; i < messageCount; ++i)
	{
		u8 testMessage[] = { 1, (u8)(i >> 24), (u8)(i >> 16), (u8)(i >> 8), (u8)i };
		client.SendMessage(clientConnection, testMessage, sizeof(testMessage), NetworkSendFlagBits::Reliable);
	}

	// Verify all messages arrive
	u32 receivedCount = 0;
	startTime = std::chrono::steady_clock::now();

	while(receivedCount < messageCount && (std::chrono::steady_clock::now() - startTime) < std::chrono::milliseconds(TEST_TIMEOUT_MS))
	{
		server.PollMessages(messages);

		for(auto& msg : messages)
		{
			if(msg.MessageType == 1)
				receivedCount++;
			server.FreeMessage(msg);
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	B3D_TEST_ASSERT(receivedCount == messageCount);

	client.DisconnectAll();
	server.DisconnectAll();
}

void NetworkTestSuite::TestUnreliableMessages()
{
	NetworkPeer server;
	NetworkPeer client;

	// Setup connection
	B3D_TEST_ASSERT(server.StartServer(TEST_PORT, 32));
	NetworkAddress serverAddress("127.0.0.1", TEST_PORT);
	ConnectionID clientConnection = client.Connect(serverAddress);

	// Wait for connection
	bool connected = false;
	auto startTime = std::chrono::steady_clock::now();
	Vector<NetworkMessage> messages;

	while(!connected && (std::chrono::steady_clock::now() - startTime) < std::chrono::milliseconds(TEST_TIMEOUT_MS))
	{
		server.PollMessages(messages);
		for(auto& msg : messages)
			server.FreeMessage(msg);

		client.PollMessages(messages);
		for(auto& msg : messages)
			client.FreeMessage(msg);

		if(client.IsConnected(clientConnection))
			connected = true;

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	B3D_TEST_ASSERT(connected);

	// Send unreliable messages (we expect most to arrive on localhost)
	constexpr u32 messageCount = 100;
	for(u32 i = 0; i < messageCount; ++i)
	{
		u8 testMessage[] = { 2, (u8)i };
		client.SendMessage(clientConnection, testMessage, sizeof(testMessage), NetworkSendFlagBits::None);
	}

	// Give time for messages to arrive
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	// Count received messages (we expect most but not necessarily all)
	u32 receivedCount = 0;
	server.PollMessages(messages);

	for(auto& msg : messages)
	{
		if(msg.MessageType == 2)
			receivedCount++;
		server.FreeMessage(msg);
	}

	// On localhost, we should receive most unreliable messages (at least 80%)
	B3D_TEST_ASSERT(receivedCount >= (messageCount * 8 / 10));

	client.DisconnectAll();
	server.DisconnectAll();
}

void NetworkTestSuite::TestBroadcastMessages()
{
	NetworkPeer server;
	NetworkPeer client1;
	NetworkPeer client2;

	// Setup server
	B3D_TEST_ASSERT(server.StartServer(TEST_PORT, 32));
	NetworkAddress serverAddress("127.0.0.1", TEST_PORT);

	// Connect both clients
	ConnectionID conn1 = client1.Connect(serverAddress);
	ConnectionID conn2 = client2.Connect(serverAddress);
	B3D_TEST_ASSERT(conn1.IsValid());
	B3D_TEST_ASSERT(conn2.IsValid());

	// Wait for both to connect
	bool client1Connected = false;
	bool client2Connected = false;
	auto startTime = std::chrono::steady_clock::now();
	Vector<NetworkMessage> messages;

	while((!client1Connected || !client2Connected) && (std::chrono::steady_clock::now() - startTime) < std::chrono::milliseconds(TEST_TIMEOUT_MS))
	{
		server.PollMessages(messages);
		for(auto& msg : messages)
			server.FreeMessage(msg);

		client1.PollMessages(messages);
		for(auto& msg : messages)
			client1.FreeMessage(msg);

		client2.PollMessages(messages);
		for(auto& msg : messages)
			client2.FreeMessage(msg);

		client1Connected = client1.IsConnected(conn1);
		client2Connected = client2.IsConnected(conn2);

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	B3D_TEST_ASSERT(client1Connected);
	B3D_TEST_ASSERT(client2Connected);

	// Broadcast message from server
	u8 broadcastMessage[] = { 99, 'B', 'r', 'o', 'a', 'd', 'c', 'a', 's', 't' };
	server.BroadcastMessage(broadcastMessage, sizeof(broadcastMessage), NetworkSendFlagBits::Reliable);

	// Wait for both clients to receive
	bool client1Received = false;
	bool client2Received = false;
	startTime = std::chrono::steady_clock::now();

	while((!client1Received || !client2Received) && (std::chrono::steady_clock::now() - startTime) < std::chrono::milliseconds(TEST_TIMEOUT_MS))
	{
		client1.PollMessages(messages);
		for(auto& msg : messages)
		{
			if(msg.MessageType == 99)
				client1Received = true;
			client1.FreeMessage(msg);
		}

		client2.PollMessages(messages);
		for(auto& msg : messages)
		{
			if(msg.MessageType == 99)
				client2Received = true;
			client2.FreeMessage(msg);
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	B3D_TEST_ASSERT(client1Received);
	B3D_TEST_ASSERT(client2Received);

	client1.DisconnectAll();
	client2.DisconnectAll();
	server.DisconnectAll();
}

void NetworkTestSuite::TestConnectionInfo()
{
	NetworkPeer server;
	NetworkPeer client;

	// Setup connection
	B3D_TEST_ASSERT(server.StartServer(TEST_PORT, 32));
	NetworkAddress serverAddress("127.0.0.1", TEST_PORT);
	ConnectionID clientConnection = client.Connect(serverAddress);

	// Wait for connection
	bool connected = false;
	auto startTime = std::chrono::steady_clock::now();
	Vector<NetworkMessage> messages;

	while(!connected && (std::chrono::steady_clock::now() - startTime) < std::chrono::milliseconds(TEST_TIMEOUT_MS))
	{
		server.PollMessages(messages);
		for(auto& msg : messages)
			server.FreeMessage(msg);

		client.PollMessages(messages);
		for(auto& msg : messages)
			client.FreeMessage(msg);

		if(client.IsConnected(clientConnection))
			connected = true;

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	B3D_TEST_ASSERT(connected);

	// Get connection info
	ConnectionInformation info;
	bool gotInfo = client.GetConnectionInfo(clientConnection, info);
	B3D_TEST_ASSERT(gotInfo);
	B3D_TEST_ASSERT(info.ID == clientConnection);
	B3D_TEST_ASSERT(info.PingMS >= 0.0f); // Ping should be very low on localhost

	client.DisconnectAll();
	server.DisconnectAll();
}

void NetworkTestSuite::TestGracefulDisconnect()
{
	NetworkPeer server;
	NetworkPeer client;

	// Setup connection
	B3D_TEST_ASSERT(server.StartServer(TEST_PORT, 32));
	NetworkAddress serverAddress("127.0.0.1", TEST_PORT);
	ConnectionID clientConnection = client.Connect(serverAddress);

	// Wait for connection
	bool connected = false;
	auto startTime = std::chrono::steady_clock::now();
	Vector<NetworkMessage> messages;

	while(!connected && (std::chrono::steady_clock::now() - startTime) < std::chrono::milliseconds(TEST_TIMEOUT_MS))
	{
		server.PollMessages(messages);
		for(auto& msg : messages)
			server.FreeMessage(msg);

		client.PollMessages(messages);
		for(auto& msg : messages)
			client.FreeMessage(msg);

		if(client.IsConnected(clientConnection))
			connected = true;

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	B3D_TEST_ASSERT(connected);

	// Disconnect client
	client.Disconnect(clientConnection);

	// Give time for disconnect to propagate
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	// Verify disconnection
	B3D_TEST_ASSERT(!client.IsConnected(clientConnection));

	server.DisconnectAll();
}

void NetworkTestSuite::TestMultipleClients()
{
	NetworkPeer server;
	constexpr u32 clientCount = 10;
	NetworkPeer clients[clientCount];
	ConnectionID connections[clientCount];

	// Start server
	B3D_TEST_ASSERT(server.StartServer(TEST_PORT, 32));
	NetworkAddress serverAddress("127.0.0.1", TEST_PORT);

	// Connect all clients
	for(u32 i = 0; i < clientCount; ++i)
	{
		connections[i] = clients[i].Connect(serverAddress);
		B3D_TEST_ASSERT(connections[i].IsValid());
	}

	// Wait for all to connect
	bool allConnected = false;
	auto startTime = std::chrono::steady_clock::now();
	Vector<NetworkMessage> messages;

	while(!allConnected && (std::chrono::steady_clock::now() - startTime) < std::chrono::milliseconds(TEST_TIMEOUT_MS))
	{
		server.PollMessages(messages);
		for(auto& msg : messages)
			server.FreeMessage(msg);

		allConnected = true;
		for(u32 i = 0; i < clientCount; ++i)
		{
			clients[i].PollMessages(messages);
			for(auto& msg : messages)
				clients[i].FreeMessage(msg);

			if(!clients[i].IsConnected(connections[i]))
				allConnected = false;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	B3D_TEST_ASSERT(allConnected);

	// Verify all connections are active
	for(u32 i = 0; i < clientCount; ++i)
	{
		B3D_TEST_ASSERT(clients[i].IsConnected(connections[i]));
	}

	// Cleanup
	for(u32 i = 0; i < clientCount; ++i)
	{
		clients[i].DisconnectAll();
	}
	server.DisconnectAll();
}
