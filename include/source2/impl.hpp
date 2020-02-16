/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __SOURCE2_IMPL_HPP__
#define __SOURCE2_IMPL_HPP__

#include <string>
#include <memory>
#include <cinttypes>

class VFilePtrInternal;
namespace source2::resource
{
	std::string read_offset_string(std::shared_ptr<VFilePtrInternal> f);
};

#endif
