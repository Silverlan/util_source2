// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

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
#include <cinttypes>

class DataStream;
class VFilePtrInternal;
namespace ufile {
	struct IFile;
};
namespace util {
	using GUID = std::array<uint8_t, 16>;
};

#endif
