
#pragma once

#include "error.h"
#include "utils.h"

template<typename T>
class expected {
public:
	expected(error err) : _ok(false), _err(std::move(err)) {}
	expected(T value) : _ok(true), _val(std::move(value)) {}

	expected(expected<T>&& other) {
		_ok = other._ok;
		if (_ok) {
			_val = std::move(other._val);
		}
		else {
			_err = std::move(other._err);
		}
	}

	~expected() {
		if (_ok) {
			_val.~T();
		}
		else {
			_err.~error();
		}
	}

	T operator*() {
		fabrik_assert(_ok);
		return _val;
	}

private:
	bool _ok;
	union {
		error _err;
		T _val;
	};
};