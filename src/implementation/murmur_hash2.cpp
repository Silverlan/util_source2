// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

module source2;

import :murmurhash;

uint32_t source2::murmur2::hash(const std::string &data, uint32_t seed)
{
	if(data.empty())
		return hash(nullptr, 0, seed);
	return hash(reinterpret_cast<const uint8_t *>(data.data()), data.length(), seed);
}
uint32_t source2::murmur2::hash(const std::vector<uint8_t> &data, uint32_t seed) { return hash(data.data(), data.size(), seed); }
uint32_t source2::murmur2::hash(const uint8_t *key, size_t len, uint32_t seed)
{
	if(key == nullptr)
		return 0;

	// Source: https://github.com/explosion/murmurhash/blob/3745e935b97eaa00801b30d5a2e6789b3a31015d/murmurhash/MurmurHash2.cpp
	// 'm' and 'r' are mixing constants generated offline.
	// They're not really 'magic', they just happen to work well.

	const uint32_t m = 0x5bd1e995;
	const int r = 24;

	// Initialize the hash to a 'random' value

	uint32_t h = seed ^ len;

	// Mix 4 bytes at a time into the hash

	const unsigned char *data = (const unsigned char *)key;

	while(len >= 4) {
		uint32_t k = *(uint32_t *)data;

		k *= m;
		k ^= k >> r;
		k *= m;

		h *= m;
		h ^= k;

		data += 4;
		len -= 4;
	}

	// Handle the last few bytes of the input array

	switch(len) {
	case 3:
		h ^= data[2] << 16;
	case 2:
		h ^= data[1] << 8;
	case 1:
		h ^= data[0];
		h *= m;
	};

	// Do a few final mixes of the hash to ensure the last few
	// bytes are well-incorporated.

	h ^= h >> 13;
	h *= m;
	h ^= h >> 15;

	return h;
}
