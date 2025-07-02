// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-FileCopyrightText: (c) 2015 Steam Database
// SPDX-License-Identifier: MIT

module;

#include <fsys/filesystem.h>
#include <sharedutils/util_ifile.hpp>

module source2;

import :impl;

using namespace source2;

std::string resource::read_offset_string(ufile::IFile &f)
{
	auto currentOffset = f.Tell();
	auto offset = f.Read<uint32_t>();
	if(offset == 0)
		return {};
	f.Seek(currentOffset + offset);
	auto str = f.ReadString(); // TODO: Encoding
	f.Seek(currentOffset + sizeof(uint32_t));
	return str;
}
