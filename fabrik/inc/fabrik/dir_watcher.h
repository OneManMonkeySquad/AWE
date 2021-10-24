#pragma once

struct background_watcher_data;

class dir_watcher {
public:
	dir_watcher(std::filesystem::path directory);
	~dir_watcher();

	bool is_directory_dirty() const;
	void reset_directory_dirty();

private:
	std::unique_ptr<background_watcher_data> _data;
};