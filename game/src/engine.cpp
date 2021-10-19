
#include "pch.h"
#include "engine.h"
#include "utils.h"
#include "input.h"
#include "renderer.h"

engine::engine(std::string data_path) {
	if (!al_install_system(ALLEGRO_VERSION_INT, nullptr) ||
		!al_install_mouse() ||
		!al_install_keyboard())
		panic("Allegro Init Failed");

	_renderer = std::make_unique<renderer>("AWE", data_path);
	_input = std::make_unique<input>();

	_event_queue = al_create_event_queue();
	al_register_event_source(_event_queue, al_get_display_event_source(_renderer->get_display()));
	al_register_event_source(_event_queue, al_get_keyboard_event_source());
	al_register_event_source(_event_queue, al_get_mouse_event_source());
}

engine::~engine() {
	al_destroy_event_queue(_event_queue);

	al_uninstall_keyboard();
	al_uninstall_mouse();
	al_shutdown_ttf_addon();
	al_shutdown_image_addon();
	al_shutdown_primitives_addon();
	//al_uninstall_system();
}

bool engine::run_frame() {
	_input->begin_frame();

	ALLEGRO_EVENT ev;
	while (al_get_next_event(_event_queue, &ev)) {
		_input->on_event(ev);

		if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
			return true;
	}

	// Aus irgendeinem Grund muss ImGui::NewFrame NACH dem verarbeiten der Allegro Events aufgerufen werden,
	// ansonsten funktioniert das Mausrad nicht...
	_renderer->begin_frame();

	return false;
}

renderer& engine::get_renderer() const {
	return *_renderer;
}

input& engine::get_input() const {
	return *_input;
}

ALLEGRO_EVENT_QUEUE* engine::get_event_queue() {
	return _event_queue;
}
