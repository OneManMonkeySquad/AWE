
#pragma once

class engine;

class app_state {
public:
	app_state(engine& engine) : _engine{ engine } {}
	virtual ~app_state() = default;

	virtual void enter() = 0;
	virtual void exit() = 0;
	virtual void begin_frame() = 0;
	virtual void tick() = 0;

protected:
	engine& _engine;
};

class app_state_manager {
public:
	app_state_manager(engine& engine) : _engine{ engine } {}

	template<typename T, typename... ArgTs>
	void switch_to(ArgTs&&... args) {
		auto type = entt::resolve<T>();
		print("Switching app_state to {}", type.info().name());


		_new_state_next_frame = std::make_unique<T>(_engine, std::forward<ArgTs>(args)...);
	}

	void begin_frame() {
		if (_new_state_next_frame != nullptr) {
			if (_current_state != nullptr) {
				_current_state->exit();
			}
			_current_state = std::move(_new_state_next_frame);
			_current_state->enter();
		}

		if (_current_state != nullptr) {
			_current_state->begin_frame();
		}
	}

	void tick() {
		if (_current_state != nullptr) {
			_current_state->tick();
		}
	}

private:
	std::unique_ptr<app_state> _current_state;
	std::unique_ptr<app_state> _new_state_next_frame;
	engine& _engine;
};