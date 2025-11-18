// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-FileCopyrightText: (c) 2015 Steam Database
// SPDX-License-Identifier: MIT

module;

module source2;

using namespace source2;

static std::span<uint8_t> DecodeBytesGroup(const std::span<uint8_t> &data, std::span<uint8_t> destination, int bitslog2);
static std::span<uint8_t> DecodeBytes(std::span<uint8_t> data, std::span<uint8_t> destination);
static std::span<uint8_t> DecodeVertexBlock(std::span<uint8_t> data, std::span<uint8_t> vertexData, int vertexCount, int vertexSize, std::span<uint8_t> lastVertex);

static std::span<uint8_t> slice(const std::span<uint8_t> &data, uint32_t start, uint32_t len = std::numeric_limits<uint32_t>::max()) { return data.subspan(start, len); }

static void copy_to(const std::span<uint8_t> &data, std::span<uint8_t> dst) { memcpy(dst.data(), data.data(), data.size() * sizeof(data.front())); }

uint32_t resource::MeshOptimizerVertexDecoder::GetVertexBlockSize(uint32_t vertexSize)
{
	auto result = VertexBlockSizeBytes / vertexSize;
	result &= ~(ByteGroupSize - 1);

	return result < VertexBlockMaxSize ? result : VertexBlockMaxSize;
}

uint8_t resource::MeshOptimizerVertexDecoder::Unzigzag8(uint8_t v) { return static_cast<uint8_t>(-(v & 1) ^ (v >> 1)); }

std::span<uint8_t> DecodeBytesGroup(const std::span<uint8_t> &data, std::span<uint8_t> destination, int bitslog2)
{
	uint32_t dataOffset = 0u;
	int dataVar;
	uint8_t b;

	auto Next = [&b, &dataVar](int bits, uint8_t encv) -> uint8_t {
		auto enc = b >> (8 - bits);
		b <<= bits;

		auto isSame = enc == (1 << bits) - 1;

		dataVar += isSame ? 1 : 0;
		return isSame ? encv : (uint8_t)enc;
	};

	switch(bitslog2) {
	case 0:
		for(auto k = 0; k < resource::MeshOptimizerVertexDecoder::ByteGroupSize; k++) {
			destination[k] = 0;
		}

		return data;
	case 1:
		dataVar = 4;

		b = data[dataOffset++];
		destination[0] = Next(2, data[dataVar]);
		destination[1] = Next(2, data[dataVar]);
		destination[2] = Next(2, data[dataVar]);
		destination[3] = Next(2, data[dataVar]);

		b = data[dataOffset++];
		destination[4] = Next(2, data[dataVar]);
		destination[5] = Next(2, data[dataVar]);
		destination[6] = Next(2, data[dataVar]);
		destination[7] = Next(2, data[dataVar]);

		b = data[dataOffset++];
		destination[8] = Next(2, data[dataVar]);
		destination[9] = Next(2, data[dataVar]);
		destination[10] = Next(2, data[dataVar]);
		destination[11] = Next(2, data[dataVar]);

		b = data[dataOffset++];
		destination[12] = Next(2, data[dataVar]);
		destination[13] = Next(2, data[dataVar]);
		destination[14] = Next(2, data[dataVar]);
		destination[15] = Next(2, data[dataVar]);

		return slice(data, dataVar);
	case 2:
		dataVar = 8;

		b = data[dataOffset++];
		destination[0] = Next(4, data[dataVar]);
		destination[1] = Next(4, data[dataVar]);

		b = data[dataOffset++];
		destination[2] = Next(4, data[dataVar]);
		destination[3] = Next(4, data[dataVar]);

		b = data[dataOffset++];
		destination[4] = Next(4, data[dataVar]);
		destination[5] = Next(4, data[dataVar]);

		b = data[dataOffset++];
		destination[6] = Next(4, data[dataVar]);
		destination[7] = Next(4, data[dataVar]);

		b = data[dataOffset++];
		destination[8] = Next(4, data[dataVar]);
		destination[9] = Next(4, data[dataVar]);

		b = data[dataOffset++];
		destination[10] = Next(4, data[dataVar]);
		destination[11] = Next(4, data[dataVar]);

		b = data[dataOffset++];
		destination[12] = Next(4, data[dataVar]);
		destination[13] = Next(4, data[dataVar]);

		b = data[dataOffset++];
		destination[14] = Next(4, data[dataVar]);
		destination[15] = Next(4, data[dataVar]);

		return slice(data, dataVar);
	case 3:
		{
			copy_to(slice(data, 0, resource::MeshOptimizerVertexDecoder::ByteGroupSize), destination);

			return slice(data, resource::MeshOptimizerVertexDecoder::ByteGroupSize);
		}
	default:
		throw std::invalid_argument {"Unexpected bit length"};
	}
}

std::span<uint8_t> DecodeBytes(std::span<uint8_t> data, std::span<uint8_t> destination)
{
	if(destination.size() % resource::MeshOptimizerVertexDecoder::ByteGroupSize != 0)
		throw std::invalid_argument {"Expected data length to be a multiple of ByteGroupSize."};

	auto headerSize = ((destination.size() / resource::MeshOptimizerVertexDecoder::ByteGroupSize) + 3) / 4;
	auto header = slice(data, 0);

	data = slice(data, headerSize);

	for(auto i = 0; i < destination.size(); i += resource::MeshOptimizerVertexDecoder::ByteGroupSize) {
		if(data.size() < resource::MeshOptimizerVertexDecoder::TailMaxSize)
			throw std::runtime_error {"Cannot decode"};

		auto headerOffset = i / resource::MeshOptimizerVertexDecoder::ByteGroupSize;

		auto bitslog2 = (header[headerOffset / 4] >> ((headerOffset % 4) * 2)) & 3;

		data = DecodeBytesGroup(data, slice(destination, i), bitslog2);
	}

	return data;
}

std::span<uint8_t> DecodeVertexBlock(std::span<uint8_t> data, std::span<uint8_t> vertexData, int vertexCount, int vertexSize, std::span<uint8_t> lastVertex)
{
	if(vertexCount <= 0 || vertexCount > resource::MeshOptimizerVertexDecoder::VertexBlockMaxSize)
		throw new std::invalid_argument {"Expected vertexCount to be between 0 and VertexMaxBlockSize"};

	std::vector<uint8_t> buffer {};
	buffer.resize(resource::MeshOptimizerVertexDecoder::VertexBlockMaxSize);
	std::vector<uint8_t> transposed {};
	transposed.resize(resource::MeshOptimizerVertexDecoder::VertexBlockSizeBytes);

	auto vertexCountAligned = (vertexCount + resource::MeshOptimizerVertexDecoder::ByteGroupSize - 1) & ~(resource::MeshOptimizerVertexDecoder::ByteGroupSize - 1);

	for(auto k = 0; k < vertexSize; ++k) {
		data = DecodeBytes(data, slice(buffer, 0, vertexCountAligned));

		auto vertexOffset = k;

		auto p = lastVertex[k];

		for(auto i = 0; i < vertexCount; ++i) {
			auto v = (uint8_t)(resource::MeshOptimizerVertexDecoder::Unzigzag8(buffer[i]) + p);

			transposed[vertexOffset] = v;
			p = v;

			vertexOffset += vertexSize;
		}
	}

	copy_to(slice(transposed, 0, vertexCount * vertexSize), vertexData);

	copy_to(slice(transposed, vertexSize * (vertexCount - 1), vertexSize), lastVertex);

	return data;
}

std::vector<uint8_t> resource::MeshOptimizerVertexDecoder::DecodeVertexBuffer(int vertexCount, int vertexSize, const std::vector<uint8_t> &vertexBuffer)
{
	if(vertexSize <= 0 || vertexSize > 256)
		throw std::invalid_argument {"Vertex size is expected to be between 1 and 256"};

	if(vertexSize % 4 != 0) {
		throw std::invalid_argument {"Vertex size is expected to be a multiple of 4."};
	}

	if(vertexBuffer.size() < 1 + vertexSize) {
		throw std::invalid_argument {"Vertex buffer is too short."};
	}

	auto vertexSpan = std::span<uint8_t> {const_cast<std::vector<uint8_t> &>(vertexBuffer)};

	auto header = vertexSpan[0];
	vertexSpan = slice(vertexSpan, 1);
	if(header != VertexHeader) {
		throw std::invalid_argument {"Invalid vertex buffer header, expected " + std::to_string(VertexHeader) + " but got " + std::to_string(header) + "."};
	}

	std::vector<uint8_t> lastVertex {};
	lastVertex.resize(vertexSize);

	copy_to(slice(vertexSpan, vertexBuffer.size() - 1 - vertexSize, vertexSize), lastVertex);

	auto vertexBlockSize = GetVertexBlockSize(vertexSize);

	auto vertexOffset = 0;

	std::vector<uint8_t> result {};
	result.resize(vertexCount * vertexSize);

	while(vertexOffset < vertexCount) {
		auto blockSize = vertexOffset + vertexBlockSize < vertexCount ? vertexBlockSize : vertexCount - vertexOffset;

		vertexSpan = DecodeVertexBlock(vertexSpan, slice(result, vertexOffset * vertexSize), blockSize, vertexSize, lastVertex);

		vertexOffset += blockSize;
	}

	return result;
}

////////////////

void resource::MeshOptimizerIndexDecoder::PushEdgeFifo(std::array<std::pair<uint32_t, uint32_t>, 16> &fifo, int &offset, uint32_t a, uint32_t b)
{
	fifo[offset] = {a, b};
	offset = (offset + 1) & 15;
}

void resource::MeshOptimizerIndexDecoder::PushVertexFifo(std::array<uint32_t, 16> &fifo, int &offset, uint32_t v, bool cond)
{
	fifo[offset] = v;
	offset = (offset + (cond ? 1 : 0)) & 15;
}

uint32_t resource::MeshOptimizerIndexDecoder::DecodeVByte(util::DataStream &data)
{
	auto lead = static_cast<uint32_t>(data->Read<uint8_t>());

	if(lead < 128) {
		return lead;
	}

	auto result = lead & 127;
	auto shift = 7;

	for(auto i = 0; i < 4; i++) {
		auto group = static_cast<uint32_t>(data->Read<uint8_t>());
		result |= (group & 127) << shift;
		shift += 7;

		if(group < 128) {
			break;
		}
	}

	return result;
}

uint32_t resource::MeshOptimizerIndexDecoder::DecodeIndex(util::DataStream &data, uint32_t next, uint32_t last)
{
	auto v = DecodeVByte(data);
	auto d = (uint32_t)((v >> 1) ^ -(v & 1));

	return last + d;
}
void resource::MeshOptimizerIndexDecoder::WriteTriangle(std::vector<uint8_t> &destination, int offset, int indexSize, uint32_t a, uint32_t b, uint32_t c)
{
	offset *= indexSize;
	if(indexSize == 2) {
		std::array<uint16_t, 3> values = {static_cast<uint16_t>(a), static_cast<uint16_t>(b), static_cast<uint16_t>(c)};
		memcpy(destination.data() + offset, values.data(), values.size() * sizeof(values.front()));
	}
	else {
		std::array<uint32_t, 3> values = {a, b, c};
		memcpy(destination.data() + offset, values.data(), values.size() * sizeof(values.front()));
	}
}

std::vector<uint8_t> resource::MeshOptimizerIndexDecoder::DecodeIndexBuffer(int indexCount, int indexSize, std::vector<uint8_t> &buffer)
{
	if(indexCount % 3 != 0)
		throw std::invalid_argument {"Expected indexCount to be a multiple of 3."};

	if(indexSize != 2 && indexSize != 4)
		throw std::invalid_argument {"Expected indexSize to be either 2 or 4"};

	auto dataOffset = 1 + (indexCount / 3);

	// the minimum valid encoding is header, 1 byte per triangle and a 16-byte codeaux table
	if(buffer.size() < dataOffset + 16)
		throw std::invalid_argument {"Index buffer is too short."};

	if(buffer[0] != IndexHeader)
		throw std::invalid_argument {"Incorrect index buffer header."};

	std::array<uint32_t, 16> vertexFifo {};
	std::array<std::pair<uint32_t, uint32_t>, 16> edgeFifo {};
	auto edgeFifoOffset = 0;
	auto vertexFifoOffset = 0;

	auto next = 0u;
	auto last = 0u;

	auto bufferIndex = 1;
	auto data = slice(buffer, dataOffset, buffer.size() - 16 - dataOffset);

	auto codeauxTable = slice(buffer, buffer.size() - 16);

	std::vector<uint8_t> destination {};
	destination.resize(indexCount * indexSize);

	util::DataStream ds {data.data(), static_cast<uint32_t>(data.size() * sizeof(data.front()))};
	ds->SetOffset(0);
	for(auto i = 0; i < indexCount; i += 3) {
		auto codetri = buffer[bufferIndex++];

		if(codetri < 0xf0) {
			auto fe = codetri >> 4;

			auto &ab = edgeFifo[(edgeFifoOffset - 1 - fe) & 15];

			auto fec = codetri & 15;

			if(fec != 15) {
				auto c = fec == 0 ? next : vertexFifo[(vertexFifoOffset - 1 - fec) & 15];

				auto fec0 = fec == 0;
				next += fec0 ? 1u : 0u;

				WriteTriangle(destination, i, indexSize, ab.first, ab.second, c);

				PushVertexFifo(vertexFifo, vertexFifoOffset, c, fec0);

				PushEdgeFifo(edgeFifo, edgeFifoOffset, c, ab.second);
				PushEdgeFifo(edgeFifo, edgeFifoOffset, ab.first, c);
			}
			else {
				auto c = last = DecodeIndex(ds, next, last);

				WriteTriangle(destination, i, indexSize, ab.first, ab.second, c);

				PushVertexFifo(vertexFifo, vertexFifoOffset, c);

				PushEdgeFifo(edgeFifo, edgeFifoOffset, c, ab.second);
				PushEdgeFifo(edgeFifo, edgeFifoOffset, ab.first, c);
			}
		}
		else {
			if(codetri < 0xfe) {
				auto codeaux = codeauxTable[codetri & 15];

				auto feb = codeaux >> 4;
				auto fec = codeaux & 15;

				auto a = next++;

				auto b = (feb == 0) ? next : vertexFifo[(vertexFifoOffset - feb) & 15];

				auto feb0 = feb == 0 ? 1u : 0u;
				next += feb0;

				auto c = (fec == 0) ? next : vertexFifo[(vertexFifoOffset - fec) & 15];

				auto fec0 = fec == 0 ? 1u : 0u;
				next += fec0;

				WriteTriangle(destination, i, indexSize, a, b, c);

				PushVertexFifo(vertexFifo, vertexFifoOffset, a);
				PushVertexFifo(vertexFifo, vertexFifoOffset, b, feb0 == 1u);
				PushVertexFifo(vertexFifo, vertexFifoOffset, c, fec0 == 1u);

				PushEdgeFifo(edgeFifo, edgeFifoOffset, b, a);
				PushEdgeFifo(edgeFifo, edgeFifoOffset, c, b);
				PushEdgeFifo(edgeFifo, edgeFifoOffset, a, c);
			}
			else {
				auto codeaux = static_cast<uint32_t>(ds->Read<uint8_t>());

				auto fea = codetri == 0xfe ? 0 : 15;
				auto feb = codeaux >> 4;
				auto fec = codeaux & 15;

				auto a = (fea == 0) ? next++ : 0;
				auto b = (feb == 0) ? next++ : vertexFifo[(vertexFifoOffset - feb) & 15];
				auto c = (fec == 0) ? next++ : vertexFifo[(vertexFifoOffset - fec) & 15];

				if(fea == 15) {
					last = a = DecodeIndex(ds, next, last);
				}

				if(feb == 15) {
					last = b = DecodeIndex(ds, next, last);
				}

				if(fec == 15) {
					last = c = DecodeIndex(ds, next, last);
				}

				WriteTriangle(destination, i, indexSize, a, b, c);

				PushVertexFifo(vertexFifo, vertexFifoOffset, a);
				PushVertexFifo(vertexFifo, vertexFifoOffset, b, (feb == 0) || (feb == 15));
				PushVertexFifo(vertexFifo, vertexFifoOffset, c, (fec == 0) || (fec == 15));

				PushEdgeFifo(edgeFifo, edgeFifoOffset, b, a);
				PushEdgeFifo(edgeFifo, edgeFifoOffset, c, b);
				PushEdgeFifo(edgeFifo, edgeFifoOffset, a, c);
			}
		}
	}

	if(ds->GetOffset() != ds->GetInternalSize())
		throw std::runtime_error {"we didn't read all data bytes and stopped before the boundary between data and codeaux table"};

	return destination;
}
