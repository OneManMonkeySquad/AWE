
#include "pch.h"
#include "yojimbo_network.h"
#include "utils.h"
#include "engine.h"
#include "event_hub.h"


network_adapter::network_adapter(engine& engine)
	: _engine{ engine } {}

void network_adapter::OnServerClientConnected(int client_idx) {
	_engine.get_event_hub().emit<events::client_connected>(client_idx);
}


void network_adapter::OnServerClientDisconnected(int client_idx) {
	_engine.get_event_hub().emit<events::client_disconnected>(client_idx);
}





yojimbo_network::yojimbo_network() {}

yojimbo_network::~yojimbo_network() {
	if (_client != nullptr) {
		_client->Disconnect();
		delete _client;
	}
	if (_server != nullptr) {
		_server->Stop();
		delete _server;
	}
}

void yojimbo_network::start_server(std::unique_ptr<network_adapter> adapter, uint16_t port) {
	time = yojimbo_time();
	_adapter = std::move(adapter);

	uint8_t privateKey[yojimbo::KeyBytes];
	memset(privateKey, 0, yojimbo::KeyBytes);

	yojimbo::ClientServerConfig config;
	_server = new yojimbo::Server(yojimbo::GetDefaultAllocator(), privateKey, yojimbo::Address("127.0.0.1", port), config, *_adapter, time);

	_server->Start(max_players);

	if (!_server->IsRunning())
		panic("Failed to start server");
}

void yojimbo_network::start_client(std::unique_ptr<network_adapter> adapter) {
	time = yojimbo_time();
	_adapter = std::move(adapter);

	yojimbo::ClientServerConfig config;
	_client = new yojimbo::Client(yojimbo::GetDefaultAllocator(), yojimbo::Address("0.0.0.0"), config, *_adapter, time);
}

void yojimbo_network::connect(const char* host, uint16_t port) {
	yojimbo::Address serverAddress("127.0.0.1", port);

	uint8_t privateKey[yojimbo::KeyBytes];
	memset(privateKey, 0, yojimbo::KeyBytes);

	uint64_t clientId = 0;
	yojimbo::random_bytes((uint8_t*)&clientId, 8);

	_client->InsecureConnect(privateKey, clientId, serverAddress);
}

void yojimbo_network::disconnect() {}

void yojimbo_network::begin_frame() {
	time = yojimbo_time();

	if (_server != nullptr) {
		update_server();
	}
	if (_client != nullptr) {
		update_client();
	}
}

void yojimbo_network::update_server() {
	_server->AdvanceTime(time);

	_server->ReceivePackets();
	server_process_messages();

	_server->SendPackets();
}

void yojimbo_network::update_client() {
	_client->AdvanceTime(time);

	_client->ReceivePackets();
	client_process_messages();

	_client->SendPackets();
}

void yojimbo_network::server_process_messages() {
	for (auto msg_pair : server_messages) {
		_server->ReleaseMessage(msg_pair.first, msg_pair.second);
	}
	server_messages.clear();

	for (int client_idx = 0; client_idx < max_players; client_idx++) {
		if (!_server->IsClientConnected(client_idx))
			continue;

		for (int j = 0; j < num_channels; j++) {
			yojimbo::Message* message = _server->ReceiveMessage(client_idx, j);
			while (message != nullptr) {
				server_messages.emplace_back(client_idx, message);

				message = _server->ReceiveMessage(client_idx, j);
			}
		}
	}
}

void yojimbo_network::client_process_messages() {
	for (auto msg : client_messages) {
		_client->ReleaseMessage(msg);
	}
	client_messages.clear();

	if (!_client->IsConnected())
		return;

	for (int j = 0; j < num_channels; j++) {
		yojimbo::Message* message = _client->ReceiveMessage(j);
		while (message != nullptr) {
			client_messages.emplace_back(message);

			message = _client->ReceiveMessage(j);
		}
	}
}