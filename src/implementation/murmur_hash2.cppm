// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-FileCopyrightText: (c) 2015 Steam Database
// SPDX-License-Identifier: MIT

module;


export module source2:murmurhash;

export import std.compat;

export namespace source2 {
	namespace murmur2 {
		uint32_t hash(const std::string &data, uint32_t seed);
		uint32_t hash(const std::vector<uint8_t> &data, uint32_t seed);
		uint32_t hash(const uint8_t *data, size_t len, uint32_t seed);
	};
};
