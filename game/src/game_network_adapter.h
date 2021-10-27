
#pragma once

#include "network_messages.h"

enum TestMessageType {
	INITIAL_GAME_STATE_MESSAGE,
	INITIAL_GAME_STATE_END_MESSAGE,
	TICK_INPUT_MESSAGE,
	INPUTS_MESSAGE,
	CREATE_PLAYER_MESSAGE,

	NUM_TEST_MESSAGE_TYPES
};

YOJIMBO_MESSAGE_FACTORY_START(TestMessageFactory, NUM_TEST_MESSAGE_TYPES);
YOJIMBO_DECLARE_MESSAGE_TYPE(INITIAL_GAME_STATE_MESSAGE, network_messages::initial_game_state);
YOJIMBO_DECLARE_MESSAGE_TYPE(INITIAL_GAME_STATE_END_MESSAGE, network_messages::initial_game_state_end);
YOJIMBO_DECLARE_MESSAGE_TYPE(TICK_INPUT_MESSAGE, network_messages::tick_input);
YOJIMBO_DECLARE_MESSAGE_TYPE(INPUTS_MESSAGE, network_messages::inputs);
YOJIMBO_DECLARE_MESSAGE_TYPE(CREATE_PLAYER_MESSAGE, network_messages::create_player);
YOJIMBO_MESSAGE_FACTORY_FINISH();

class game_network_adapter : public network_adapter {
public:
	game_network_adapter(engine& engine)
		: network_adapter{ engine } {}

	yojimbo::MessageFactory* CreateMessageFactory(yojimbo::Allocator& allocator) {
		return YOJIMBO_NEW(allocator, TestMessageFactory, allocator);
	}
};