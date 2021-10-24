
#pragma once

#include "network_messages.h"

enum TestMessageType {
	INITIAL_GAME_STATE_MESSAGE,
	INITIAL_GAME_STATE_END_MESSAGE,
	NUM_TEST_MESSAGE_TYPES
};

YOJIMBO_MESSAGE_FACTORY_START(TestMessageFactory, NUM_TEST_MESSAGE_TYPES);
YOJIMBO_DECLARE_MESSAGE_TYPE(INITIAL_GAME_STATE_MESSAGE, network_messages::initial_game_state);
YOJIMBO_DECLARE_MESSAGE_TYPE(INITIAL_GAME_STATE_END_MESSAGE, network_messages::initial_game_state_end);
YOJIMBO_MESSAGE_FACTORY_FINISH();

class game_network_adapter : public network_adapter {
public:
	game_network_adapter(engine& engine)
		: network_adapter{ engine } {}

	yojimbo::MessageFactory* CreateMessageFactory(yojimbo::Allocator& allocator) {
		return YOJIMBO_NEW(allocator, TestMessageFactory, allocator);
	}
};