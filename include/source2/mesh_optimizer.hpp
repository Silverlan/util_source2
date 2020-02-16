/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __SOURCE2_MESH_OPTIMIZER_HPP__
#define __SOURCE2_MESH_OPTIMIZER_HPP__

#include <cinttypes>
#include <vector>
#include <string>
#include <array>
#include <cinttypes>
#include <stdexcept>

class DataStream;
namespace source2::resource
{
	class MeshOptimizerVertexDecoder
	{
	public:
		static constexpr uint8_t VertexHeader = 0xa0;

		static constexpr int32_t VertexBlockSizeBytes = 8192;
		static constexpr int32_t VertexBlockMaxSize = 256;
		static constexpr int32_t ByteGroupSize = 16;
		static constexpr int32_t TailMaxSize = 32;

		static uint32_t GetVertexBlockSize(uint32_t vertexSize);
		static uint8_t Unzigzag8(uint8_t v);
		static std::vector<uint8_t> DecodeBytesGroup(const std::vector<uint8_t> &data, std::vector<uint8_t> &destination, int bitslog2);
		static std::vector<uint8_t> DecodeBytes(std::vector<uint8_t> data, std::vector<uint8_t> &destination);
		static std::vector<uint8_t> DecodeVertexBlock(std::vector<uint8_t> data, std::vector<uint8_t> &vertexData, int vertexCount, int vertexSize, std::vector<uint8_t> &lastVertex);
		static std::vector<uint8_t> DecodeVertexBuffer(int vertexCount, int vertexSize, std::vector<uint8_t> &vertexBuffer);
	};

	class MeshOptimizerIndexDecoder
	{
	public:
		static constexpr uint8_t IndexHeader = 0xe0;

		static void PushEdgeFifo(std::array<std::pair<uint32_t, uint32_t>,16> &fifo, int &offset, uint32_t a, uint32_t b);
		static void PushVertexFifo(std::array<uint32_t,16> &fifo, int &offset, uint32_t v, bool cond = true);
		static uint32_t DecodeVByte(DataStream &data);
		static uint32_t DecodeIndex(DataStream &data, uint32_t next, uint32_t last);
		static void WriteTriangle(std::vector<uint8_t> &destination, int offset, int indexSize, uint32_t a, uint32_t b, uint32_t c);
		static std::vector<uint8_t> DecodeIndexBuffer(int indexCount, int indexSize, std::vector<uint8_t> &buffer);
	};
};

#endif
