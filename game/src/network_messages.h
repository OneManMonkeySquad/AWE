
#pragma once

namespace network_messages {
	struct initial_game_state : public yojimbo::Message {
		std::string data;

		template <typename Stream>
		bool Serialize(Stream& stream) {
			int length = 0;
			if (Stream::IsWriting) {
				length = (int)data.size();
			}
			serialize_int(stream, length, 0, 1024);
			if (Stream::IsReading) {
				data.resize(length);
			}
			serialize_bytes(stream, (uint8_t*)data.data(), length);
			return true;
		}

		YOJIMBO_VIRTUAL_SERIALIZE_FUNCTIONS();
	};

	struct initial_game_state_end : public yojimbo::Message {
		size_t data_hash = 0;
		size_t tick = 0;
		int local_client_idx;

		template <typename Stream>
		bool Serialize(Stream& stream) {
			serialize_uint64(stream, data_hash);
			serialize_uint64(stream, tick);
			serialize_uint32(stream, local_client_idx);
			return true;
		}

		YOJIMBO_VIRTUAL_SERIALIZE_FUNCTIONS();
	};

	struct tick_input : public yojimbo::Message {
		game_input input;

		template <typename Stream>
		bool Serialize(Stream& stream) {
			return input.Serialize(stream);
		}

		YOJIMBO_VIRTUAL_SERIALIZE_FUNCTIONS();
	};

	struct inputs : public yojimbo::Message {
		std::vector<int> input_client_indices;
		std::vector<game_input> inputs;

		template <typename Stream>
		bool Serialize(Stream& stream) {
			int num = (int)input_client_indices.size();
			serialize_int(stream, num, 0, yojimbo::MaxClients);
			if (Stream::IsReading) {
				input_client_indices.resize(num);
				inputs.resize(num);
			}

			for (int i = 0; i < num; ++i) {
				serialize_int(stream, input_client_indices[i], 0, yojimbo::MaxClients);

				auto& input = inputs[i];
				if (!input.Serialize(stream))
					return false;
			}
			return true;
		}

		YOJIMBO_VIRTUAL_SERIALIZE_FUNCTIONS();
	};

	struct create_player : public yojimbo::Message {
		int client_idx;

		template <typename Stream>
		bool Serialize(Stream& stream) {
			serialize_int(stream, client_idx, 0, yojimbo::MaxClients);
			return true;
		}

		YOJIMBO_VIRTUAL_SERIALIZE_FUNCTIONS();
	};
}