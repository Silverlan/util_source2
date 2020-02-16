/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "source2/impl.hpp"
#include <fsys/filesystem.h>

using namespace source2;

std::string resource::read_offset_string(std::shared_ptr<VFilePtrInternal> f)
{
	auto currentOffset = f->Tell();
	auto offset = f->Read<uint32_t>();
	if(offset == 0)
		return {};
	f->Seek(currentOffset +offset);
	auto str = f->ReadString(); // TODO: Encoding
	f->Seek(currentOffset +sizeof(uint32_t));
	return str;
}
