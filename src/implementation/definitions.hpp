/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __UTIL_SOURCE2_DEFINITIONS_HPP__
#define __UTIL_SOURCE2_DEFINITIONS_HPP__

#ifndef US2_SHARED
#define DLLUS2
#elif US2_EXPORT
#ifdef __linux__
#define DLLUS2 __attribute__((visibility("default")))
#else
#define DLLUS2 __declspec(dllexport)
#endif
#else
#ifdef __linux__
#define DLLUS2
#else
#define DLLUS2 __declspec(dllimport)
#endif
#endif

#include <array>

class DataStream;
class VFilePtrInternal;
namespace ufile {
	struct IFile;
};
namespace util {
	using GUID = std::array<uint8_t, 16>;
};

#endif
