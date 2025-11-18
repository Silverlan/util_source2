// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-FileCopyrightText: (c) 2015 Steam Database
// SPDX-License-Identifier: MIT

module;

#include "definitions.hpp"

export module source2:impl;

export import pragma.filesystem;

export namespace source2::resource {
	std::string read_offset_string(ufile::IFile &f);
};

export namespace source2::impl {
	const std::unordered_map<uint32_t, std::string> &get_known_keyvalues();
	std::optional<std::string> hash_to_keyvalue(uint32_t hash);
};
