#pragma once

struct ALLEGRO_EVENT_QUEUE;
class renderer;
class input;
class yojimbo_network;
class app_state_manager;
class event_hub;
class scene_manager;
class resource_manager;

class engine {
public:
	engine(std::string data_path,
		   std::unique_ptr<renderer> renderer,
		   std::unique_ptr<input> input,
		   std::unique_ptr<yojimbo_network> network);
	~engine();

	void begin_frame();
	void tick();

	renderer& get_renderer() const;
	input& get_input() const;
	yojimbo_network& get_network() const;
	app_state_manager& get_app_state_manager() const;
	event_hub& get_event_hub() const;
	scene_manager& get_scene_manager() const;
	resource_manager& get_resource_manager() const;

private:
	std::unique_ptr<renderer> _renderer;
	std::unique_ptr<input> _input;
	std::unique_ptr<yojimbo_network> _network;
	std::unique_ptr<app_state_manager> _app_state_manager;
	std::unique_ptr<event_hub> _event_hub;
	std::unique_ptr<scene_manager> _scene_manager;
	std::unique_ptr<resource_manager> _resource_manager;
};

bool awe_init();
void awe_shutdown();