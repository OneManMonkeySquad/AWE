#pragma once

namespace yojimbo {
	class Server;
	class Client;
	class Message;
	class Adapter;
}
class engine;

namespace events {
	struct client_connected {
		int client_idx;
	};

	struct client_disconnected {
		int client_idx;
	};
}

class network_adapter : public yojimbo::Adapter {
public:
	network_adapter(engine& engine);

	void OnServerClientConnected(int client_idx) final;
	void OnServerClientDisconnected(int client_idx) final;

private:
	engine& _engine;
};

class yojimbo_network {
public:
	std::vector<std::pair<int, yojimbo::Message*>> server_messages;
	std::vector<yojimbo::Message*> client_messages;

	yojimbo_network();
	~yojimbo_network();

	void start_server(std::unique_ptr<network_adapter> adapter, uint16_t port);

	void start_client(std::unique_ptr<network_adapter> adapter);
	void connect(const char* host, uint16_t port);
	void disconnect();

	void begin_frame();

	bool is_client() const { return _client != nullptr; }
	bool is_server() const { return _server != nullptr; }

	yojimbo::Client& get_client() { return *_client; }
	yojimbo::Server& get_server() { return *_server; }

private:
	static constexpr int max_players = 32;
	static constexpr int num_channels = 1;

	yojimbo::Server* _server = nullptr;
	yojimbo::Client* _client = nullptr;
	double time = 0;
	std::unique_ptr<network_adapter> _adapter;

	void update_server();
	void update_client();

	void server_process_messages();
	void client_process_messages();
};