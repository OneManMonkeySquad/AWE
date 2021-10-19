#pragma once

struct ALLEGRO_EVENT_QUEUE;
class renderer;
class input;

class engine {
public:
	engine(std::string data_path);
	~engine();

	bool run_frame();

	renderer& get_renderer() const;
	input& get_input() const;

	ALLEGRO_EVENT_QUEUE* get_event_queue();

private:
	ALLEGRO_EVENT_QUEUE* _event_queue = nullptr;

	std::unique_ptr<renderer> _renderer;
	std::unique_ptr<input> _input;
};
