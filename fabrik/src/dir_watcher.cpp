
#include "pch.h"
#include "dir_watcher.h"
#include "defer.h"
#include "utils.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

struct background_watcher_data {
	std::thread thread;

	std::string directory;
	std::atomic<bool> exit;
	std::atomic<bool> directoryDirty;
	std::function<void()> callback;
};

namespace {
	void background_thread(background_watcher_data& data) {
		debug::set_current_thread_name("dir_watcher");

		print("Watching for changes in {}", data.directory);
		auto hDir = FindFirstChangeNotificationA(data.directory.c_str(),
			TRUE,
			FILE_NOTIFY_CHANGE_FILE_NAME |
			FILE_NOTIFY_CHANGE_DIR_NAME |
			FILE_NOTIFY_CHANGE_SIZE |
			FILE_NOTIFY_CHANGE_LAST_WRITE |
			FILE_NOTIFY_CHANGE_ATTRIBUTES |
			FILE_NOTIFY_CHANGE_CREATION);
		if (hDir == INVALID_HANDLE_VALUE)
			panic("FindFirstChangeNotification failed");

		defer{ FindCloseChangeNotification(hDir); };

		while (!data.exit) {
			if (WaitForSingleObject(hDir, 200) != WAIT_OBJECT_0)
				continue;

			data.directoryDirty = true;

			if (!FindNextChangeNotification(hDir))
				panic("FindNextChangeNotification failed");
		}
	}
}

dir_watcher::dir_watcher(std::filesystem::path directory) {
	_data = std::make_unique<background_watcher_data>();
	_data->directory = directory.string();
	_data->exit = false;
	_data->directoryDirty = false;

	_data->thread = std::thread{ background_thread, std::ref(*_data) };
}

dir_watcher::~dir_watcher() {
	_data->exit = true;
	_data->thread.join();
}

bool dir_watcher::is_directory_dirty() const {
	return _data->directoryDirty;
}

void dir_watcher::reset_directory_dirty() {
	_data->directoryDirty = false;
}