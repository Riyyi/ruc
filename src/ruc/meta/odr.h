/*
 * Copyright (C) 2022 Riyyi
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

namespace ruc {

namespace detail {

// Avoid ODR (One Definition Rule) violations,
// variable templates are required to have external linkage
template<typename T>
constexpr T staticConst {};

} // namespace detail

} // namespace ruc
