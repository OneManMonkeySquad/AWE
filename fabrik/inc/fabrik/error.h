#pragma once

class error {
public:
	error(const char* text) : _text{ text } {}
	error(std::string text) : _text{ std::move(text) } {}

	const char* what() const {
		return _text.c_str();
	}

private:
	std::string _text;
};