/*
 * Copyright (C) 2022 Riyyi
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <cstdlib> // free, malloc

#include "meta/assert.h"

namespace ruc {

template<typename T>
class Singleton {
public:
	static inline T& the()
	{
		if (s_instance == nullptr) {
			// Allocate memory for this instance
			s_instance = static_cast<T*>(malloc(sizeof(T)));
			// Call the constructor using "placement new"
			new (s_instance) T { s {} };
		}

		return *s_instance;
	}

	static inline void destroy()
	{
		if (!s_instance) {
			return;
		}

		// Call the destructor
		s_instance->~T();
		// Deallocate memory
		free(static_cast<void*>(s_instance));

		s_instance = nullptr;
	}

	// Remove copy constructor and copy assignment operator
	Singleton(const Singleton&) = delete;
	Singleton& operator=(const Singleton&) = delete;
	Singleton(Singleton&&) = delete;
	Singleton& operator=(Singleton&&) = delete;

protected:
	Singleton() {}

	// Constructor token
	struct s {};

private:
	static T* s_instance;
};

template<typename T>
T* Singleton<T>::s_instance = nullptr;

} // namespace ruc
