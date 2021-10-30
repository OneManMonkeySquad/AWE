
#include "pch.h"
#include "engine.h"
#include "utils.h"
#include "allegro_input.h"
#include "renderer.h"
#include "yojimbo_network.h"
#include "app_state_manager.h"
#include "event_hub.h"
#include "scene_manager.h"
#include "resource_manager.h"

namespace {
	int _printf_impl(const char* fmt, ...) {
		char dbgmsg[200];
		va_list arglist;

		va_start(arglist, fmt);
		auto written = _vsnprintf_s(dbgmsg, sizeof(dbgmsg), fmt, arglist);
		va_end(arglist);

		print(std::string_view{ dbgmsg, (size_t)written });
		return written;
	}

	void _assert_impl(const char* condition, const char* function, const char* file, int line) {
		panic(std::format("{}\n\n{} {}:{}", condition, function, file, line));
	}
}

engine::engine(std::unique_ptr<resource_manager> resource_manager,
			   std::unique_ptr<renderer> renderer,
			   std::unique_ptr<input> input,
			   std::unique_ptr<yojimbo_network> network) {
	_event_hub = std::make_unique<event_hub>();

	_resource_manager = std::move(resource_manager);

	_renderer = std::move(renderer);
	_renderer->initialize(this);

	_input = std::move(input);
	_input->initialize();

	_network = std::move(network);

	_app_state_manager = std::make_unique<app_state_manager>(*this);

	_scene_manager = std::make_unique<scene_manager>();
}

engine::~engine() {}

void engine::begin_frame() {
	_input->begin_frame();

	// Aus irgendeinem Grund muss ImGui::NewFrame NACH dem verarbeiten der Allegro Events aufgerufen werden,
	// ansonsten funktioniert das Mausrad nicht...
	_renderer->begin_frame();

	_network->begin_frame();

	_app_state_manager->begin_frame();

	_resource_manager->begin_frame();
}

void engine::tick() {
	_app_state_manager->tick();

	_renderer->tick();
}

renderer& engine::get_renderer() const {
	return *_renderer;
}

input& engine::get_input() const {
	return *_input;
}

yojimbo_network& engine::get_network() const {
	return *_network;
}

app_state_manager& engine::get_app_state_manager() const {
	return *_app_state_manager;
}

event_hub& engine::get_event_hub() const {
	return *_event_hub;
}

scene_manager& engine::get_scene_manager() const {
	return *_scene_manager;
}

resource_manager& engine::get_resource_manager() const {
	return *_resource_manager;
}

bool awe_init() {
	if (!al_install_system(ALLEGRO_VERSION_INT, nullptr) ||
		!al_install_mouse() ||
		!al_install_keyboard() ||
		!al_install_joystick())
		return false;

	if (!InitializeYojimbo())
		panic("InitializeYojimbo");

	yojimbo_set_printf_function(_printf_impl);
	yojimbo_set_assert_function(_assert_impl);

	yojimbo_log_level(YOJIMBO_LOG_LEVEL_INFO);

	return true;
}

void awe_shutdown() {
	ShutdownYojimbo();

	al_uninstall_joystick();
	al_uninstall_keyboard();
	al_uninstall_mouse();
	al_shutdown_ttf_addon();
	al_shutdown_image_addon();
	al_shutdown_primitives_addon();
	//al_uninstall_system();
}