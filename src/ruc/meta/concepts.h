/*
 * Copyright (C) 2022 Riyyi
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <type_traits> // is_integral_v, floating_point_v

namespace ruc::concepts {

template<class T>
concept Integral = std::is_integral_v<T>;

template<typename T>
concept FloatingPoint = std::is_floating_point_v<T>;

} // namespace ruc::concepts

using ruc::concepts::FloatingPoint;
using ruc::concepts::Integral;
