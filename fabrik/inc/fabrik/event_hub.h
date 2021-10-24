
#pragma once

namespace detail {
	template<typename T>
	class event_dispatch {
	public:
		static void emit(void* hub, const T& evt) {
			for (auto& existingEntry : _entries) {
				if (existingEntry.hub == hub) {
					for (auto& listener : existingEntry.listeners) {
						listener(evt);
					}
					break;
				}
			}
		}

		static void add_listener(void* hub, std::function<void(const T&)> f) {
			entry* entry = nullptr;
			for (auto& existingEntry : _entries) {
				if (existingEntry.hub == hub) {
					entry = &existingEntry;
					break;
				}
			}

			if (entry == nullptr) {
				_entries.push_back({ hub });
				entry = &_entries[_entries.size() - 1];
			}

			entry->listeners.push_back(f);
		}

	private:
		struct entry {
			void* hub;
			std::vector<std::function<void(const T&)>> listeners;
		};
		inline static std::vector<entry> _entries;
	};
}

class event_hub {
public:
	template<typename T, typename... ArgTs>
	void emit(ArgTs&&... args) {
		detail::event_dispatch<T>::emit(this, T{ std::forward<ArgTs>(args)... });
	}

	template<typename T>
	void add_listener(std::function<void(const T&)> f) {
		detail::event_dispatch<T>::add_listener(this, f);
	}

	/// Add listener class with this->on_event(...); being a valid call.
	template<typename T, typename C>
	void add_listener_instance(C* instance) {
		auto f = [instance](const T& evt) { instance->on_event(evt); };
		detail::event_dispatch<T>::add_listener(this, f);
	}
};