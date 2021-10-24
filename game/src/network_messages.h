
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
			serialize_int(stream, length, 0, 1073741824);
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

		template <typename Stream>
		bool Serialize(Stream& stream) {
			serialize_uint64(stream, data_hash);
			return true;
		}

		YOJIMBO_VIRTUAL_SERIALIZE_FUNCTIONS();
	};
}