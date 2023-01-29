/*
MIT License

Copyright (c) 2021 Silverlan
Copyright (c) 2015 Steam Database

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef __SOURCE2_IMPL_HPP__
#define __SOURCE2_IMPL_HPP__

#include <string>
#include <memory>
#include <optional>
#include <unordered_map>
#include <cinttypes>

class VFilePtrInternal;
namespace ufile {
	struct IFile;
};
namespace source2::resource {
	std::string read_offset_string(ufile::IFile &f);
};

namespace source2::impl {
	const std::unordered_map<uint32_t, std::string> &get_known_keyvalues();
	std::optional<std::string> hash_to_keyvalue(uint32_t hash);
};

#endif
