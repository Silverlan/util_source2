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

module;

#include <sharedutils/util_ifile.hpp>
#include <fsys/filesystem.h>
#include <cstring>

module source2;

import :impl;

using namespace source2;

std::string resource::to_string(DXGI_FORMAT format)
{
	switch(format) {
	case DXGI_FORMAT::UNKNOWN:
		return "UNKNOWN";
	case DXGI_FORMAT::R32G32B32A32_TYPELESS:
		return "R32G32B32A32_TYPELESS";
	case DXGI_FORMAT::R32G32B32A32_FLOAT:
		return "R32G32B32A32_FLOAT";
	case DXGI_FORMAT::R32G32B32A32_UINT:
		return "R32G32B32A32_UINT";
	case DXGI_FORMAT::R32G32B32A32_SINT:
		return "R32G32B32A32_SINT";
	case DXGI_FORMAT::R32G32B32_TYPELESS:
		return "R32G32B32_TYPELESS";
	case DXGI_FORMAT::R32G32B32_FLOAT:
		return "R32G32B32_FLOAT";
	case DXGI_FORMAT::R32G32B32_UINT:
		return "R32G32B32_UINT";
	case DXGI_FORMAT::R32G32B32_SINT:
		return "R32G32B32_SINT";
	case DXGI_FORMAT::R16G16B16A16_TYPELESS:
		return "R16G16B16A16_TYPELESS";
	case DXGI_FORMAT::R16G16B16A16_FLOAT:
		return "R16G16B16A16_FLOAT";
	case DXGI_FORMAT::R16G16B16A16_UNORM:
		return "R16G16B16A16_UNORM";
	case DXGI_FORMAT::R16G16B16A16_UINT:
		return "R16G16B16A16_UINT";
	case DXGI_FORMAT::R16G16B16A16_SNORM:
		return "R16G16B16A16_SNORM";
	case DXGI_FORMAT::R16G16B16A16_SINT:
		return "R16G16B16A16_SINT";
	case DXGI_FORMAT::R32G32_TYPELESS:
		return "R32G32_TYPELESS";
	case DXGI_FORMAT::R32G32_FLOAT:
		return "R32G32_FLOAT";
	case DXGI_FORMAT::R32G32_UINT:
		return "R32G32_UINT";
	case DXGI_FORMAT::R32G32_SINT:
		return "R32G32_SINT";
	case DXGI_FORMAT::R32G8X24_TYPELESS:
		return "R32G8X24_TYPELESS";
	case DXGI_FORMAT::D32_FLOAT_S8X24_UINT:
		return "D32_FLOAT_S8X24_UINT";
	case DXGI_FORMAT::R32_FLOAT_X8X24_TYPELESS:
		return "R32_FLOAT_X8X24_TYPELESS";
	case DXGI_FORMAT::X32_TYPELESS_G8X24_UINT:
		return "X32_TYPELESS_G8X24_UINT";
	case DXGI_FORMAT::R10G10B10A2_TYPELESS:
		return "R10G10B10A2_TYPELESS";
	case DXGI_FORMAT::R10G10B10A2_UNORM:
		return "R10G10B10A2_UNORM";
	case DXGI_FORMAT::R10G10B10A2_UINT:
		return "R10G10B10A2_UINT";
	case DXGI_FORMAT::R11G11B10_FLOAT:
		return "R11G11B10_FLOAT";
	case DXGI_FORMAT::R8G8B8A8_TYPELESS:
		return "R8G8B8A8_TYPELESS";
	case DXGI_FORMAT::R8G8B8A8_UNORM:
		return "R8G8B8A8_UNORM";
	case DXGI_FORMAT::R8G8B8A8_UNORM_SRGB:
		return "R8G8B8A8_UNORM_SRGB";
	case DXGI_FORMAT::R8G8B8A8_UINT:
		return "R8G8B8A8_UINT";
	case DXGI_FORMAT::R8G8B8A8_SNORM:
		return "R8G8B8A8_SNORM";
	case DXGI_FORMAT::R8G8B8A8_SINT:
		return "R8G8B8A8_SINT";
	case DXGI_FORMAT::R16G16_TYPELESS:
		return "R16G16_TYPELESS";
	case DXGI_FORMAT::R16G16_FLOAT:
		return "R16G16_FLOAT";
	case DXGI_FORMAT::R16G16_UNORM:
		return "R16G16_UNORM";
	case DXGI_FORMAT::R16G16_UINT:
		return "R16G16_UINT";
	case DXGI_FORMAT::R16G16_SNORM:
		return "R16G16_SNORM";
	case DXGI_FORMAT::R16G16_SINT:
		return "R16G16_SINT";
	case DXGI_FORMAT::R32_TYPELESS:
		return "R32_TYPELESS";
	case DXGI_FORMAT::D32_FLOAT:
		return "D32_FLOAT";
	case DXGI_FORMAT::R32_FLOAT:
		return "R32_FLOAT";
	case DXGI_FORMAT::R32_UINT:
		return "R32_UINT";
	case DXGI_FORMAT::R32_SINT:
		return "R32_SINT";
	case DXGI_FORMAT::R24G8_TYPELESS:
		return "R24G8_TYPELESS";
	case DXGI_FORMAT::D24_UNORM_S8_UINT:
		return "D24_UNORM_S8_UINT";
	case DXGI_FORMAT::R24_UNORM_X8_TYPELESS:
		return "R24_UNORM_X8_TYPELESS";
	case DXGI_FORMAT::X24_TYPELESS_G8_UINT:
		return "X24_TYPELESS_G8_UINT";
	case DXGI_FORMAT::R8G8_TYPELESS:
		return "R8G8_TYPELESS";
	case DXGI_FORMAT::R8G8_UNORM:
		return "R8G8_UNORM";
	case DXGI_FORMAT::R8G8_UINT:
		return "R8G8_UINT";
	case DXGI_FORMAT::R8G8_SNORM:
		return "R8G8_SNORM";
	case DXGI_FORMAT::R8G8_SINT:
		return "R8G8_SINT";
	case DXGI_FORMAT::R16_TYPELESS:
		return "R16_TYPELESS";
	case DXGI_FORMAT::R16_FLOAT:
		return "R16_FLOAT";
	case DXGI_FORMAT::D16_UNORM:
		return "D16_UNORM";
	case DXGI_FORMAT::R16_UNORM:
		return "R16_UNORM";
	case DXGI_FORMAT::R16_UINT:
		return "R16_UINT";
	case DXGI_FORMAT::R16_SNORM:
		return "R16_SNORM";
	case DXGI_FORMAT::R16_SINT:
		return "R16_SINT";
	case DXGI_FORMAT::R8_TYPELESS:
		return "R8_TYPELESS";
	case DXGI_FORMAT::R8_UNORM:
		return "R8_UNORM";
	case DXGI_FORMAT::R8_UINT:
		return "R8_UINT";
	case DXGI_FORMAT::R8_SNORM:
		return "R8_SNORM";
	case DXGI_FORMAT::R8_SINT:
		return "R8_SINT";
	case DXGI_FORMAT::A8_UNORM:
		return "A8_UNORM";
	case DXGI_FORMAT::R1_UNORM:
		return "R1_UNORM";
	case DXGI_FORMAT::R9G9B9E5_SHAREDEXP:
		return "R9G9B9E5_SHAREDEXP";
	case DXGI_FORMAT::R8G8_B8G8_UNORM:
		return "R8G8_B8G8_UNORM";
	case DXGI_FORMAT::G8R8_G8B8_UNORM:
		return "G8R8_G8B8_UNORM";
	case DXGI_FORMAT::BC1_TYPELESS:
		return "BC1_TYPELESS";
	case DXGI_FORMAT::BC1_UNORM:
		return "BC1_UNORM";
	case DXGI_FORMAT::BC1_UNORM_SRGB:
		return "BC1_UNORM_SRGB";
	case DXGI_FORMAT::BC2_TYPELESS:
		return "BC2_TYPELESS";
	case DXGI_FORMAT::BC2_UNORM:
		return "BC2_UNORM";
	case DXGI_FORMAT::BC2_UNORM_SRGB:
		return "BC2_UNORM_SRGB";
	case DXGI_FORMAT::BC3_TYPELESS:
		return "BC3_TYPELESS";
	case DXGI_FORMAT::BC3_UNORM:
		return "BC3_UNORM";
	case DXGI_FORMAT::BC3_UNORM_SRGB:
		return "BC3_UNORM_SRGB";
	case DXGI_FORMAT::BC4_TYPELESS:
		return "BC4_TYPELESS";
	case DXGI_FORMAT::BC4_UNORM:
		return "BC4_UNORM";
	case DXGI_FORMAT::BC4_SNORM:
		return "BC4_SNORM";
	case DXGI_FORMAT::BC5_TYPELESS:
		return "BC5_TYPELESS";
	case DXGI_FORMAT::BC5_UNORM:
		return "BC5_UNORM";
	case DXGI_FORMAT::BC5_SNORM:
		return "BC5_SNORM";
	case DXGI_FORMAT::B5G6R5_UNORM:
		return "B5G6R5_UNORM";
	case DXGI_FORMAT::B5G5R5A1_UNORM:
		return "B5G5R5A1_UNORM";
	case DXGI_FORMAT::B8G8R8A8_UNORM:
		return "B8G8R8A8_UNORM";
	case DXGI_FORMAT::B8G8R8X8_UNORM:
		return "B8G8R8X8_UNORM";
	case DXGI_FORMAT::R10G10B10_XR_BIAS_A2_UNORM:
		return "R10G10B10_XR_BIAS_A2_UNORM";
	case DXGI_FORMAT::B8G8R8A8_TYPELESS:
		return "B8G8R8A8_TYPELESS";
	case DXGI_FORMAT::B8G8R8A8_UNORM_SRGB:
		return "B8G8R8A8_UNORM_SRGB";
	case DXGI_FORMAT::B8G8R8X8_TYPELESS:
		return "B8G8R8X8_TYPELESS";
	case DXGI_FORMAT::B8G8R8X8_UNORM_SRGB:
		return "B8G8R8X8_UNORM_SRGB";
	case DXGI_FORMAT::BC6H_TYPELESS:
		return "BC6H_TYPELESS";
	case DXGI_FORMAT::BC6H_UF16:
		return "BC6H_UF16";
	case DXGI_FORMAT::BC6H_SF16:
		return "BC6H_SF16";
	case DXGI_FORMAT::BC7_TYPELESS:
		return "BC7_TYPELESS";
	case DXGI_FORMAT::BC7_UNORM:
		return "BC7_UNORM";
	case DXGI_FORMAT::BC7_UNORM_SRGB:
		return "BC7_UNORM_SRGB";
	case DXGI_FORMAT::AYUV:
		return "AYUV";
	case DXGI_FORMAT::Y410:
		return "Y410";
	case DXGI_FORMAT::Y416:
		return "Y416";
	case DXGI_FORMAT::NV12:
		return "NV12";
	case DXGI_FORMAT::P010:
		return "P010";
	case DXGI_FORMAT::P016:
		return "P016";
	//case DXGI_FORMAT::420_OPAQUE:
	//	return "420_OPAQUE";
	case DXGI_FORMAT::YUY2:
		return "YUY2";
	case DXGI_FORMAT::Y210:
		return "Y210";
	case DXGI_FORMAT::Y216:
		return "Y216";
	case DXGI_FORMAT::NV11:
		return "NV11";
	case DXGI_FORMAT::AI44:
		return "AI44";
	case DXGI_FORMAT::IA44:
		return "IA44";
	case DXGI_FORMAT::P8:
		return "P8";
	case DXGI_FORMAT::A8P8:
		return "A8P8";
	case DXGI_FORMAT::B4G4R4A4_UNORM:
		return "B4G4R4A4_UNORM";
	case DXGI_FORMAT::P208:
		return "P208";
	case DXGI_FORMAT::V208:
		return "V208";
	case DXGI_FORMAT::V408:
		return "V408";
	}
	return "Invalid";
}

std::string resource::to_string(DataType type)
{
	switch(type) {
	case DataType::Unknown:
		return "Unknown";
	case DataType::Struct:
		return "Struct";
	case DataType::Enum:
		return "Enum";
	case DataType::ExternalReference:
		return "ExternalReference";
	case DataType::String4:
		return "String4";
	case DataType::SByte:
		return "SByte";
	case DataType::Byte:
		return "Byte";
	case DataType::Int16:
		return "Int16";
	case DataType::UInt16:
		return "UInt16";
	case DataType::Int32:
		return "Int32";
	case DataType::UInt32:
		return "UInt32";
	case DataType::Int64:
		return "Int64";
	case DataType::UInt64:
		return "UInt64";
	case DataType::Float:
		return "Float";
	case DataType::Matrix2x4:
		return "Matrix2x4";
	case DataType::Vector:
		return "Vector";
	case DataType::Vector4D:
		return "Vector4D";
	case DataType::Quaternion:
		return "Quaternion";
	case DataType::Fltx4:
		return "Fltx4";
	case DataType::Color:
		return "Color";
	case DataType::Boolean:
		return "Boolean";
	case DataType::String:
		return "String";
	case DataType::Matrix3x4:
		return "Matrix3x4";
	case DataType::Matrix3x4a:
		return "Matrix3x4a";
	case DataType::CTransform:
		return "CTransform";
	case DataType::Vector4D_44:
		return "Vector4D_44";
	}
	return "Invalid";
}

std::shared_ptr<resource::REDIBlock> resource::ResourceEditInfo::ConstructStruct(REDIStruct id)
{
	switch(id) {
	case REDIStruct::InputDependencies:
		return std::make_shared<InputDependencies>();
	case REDIStruct::AdditionalInputDependencies:
		return std::make_shared<AdditionalInputDependencies>();
	case REDIStruct::ArgumentDependencies:
		return std::make_shared<ArgumentDependencies>();
	case REDIStruct::SpecialDependencies:
		return std::make_shared<SpecialDependencies>();
	case REDIStruct::CustomDependencies:
		return std::make_shared<CustomDependencies>();
	case REDIStruct::AdditionalRelatedFiles:
		return std::make_shared<AdditionalRelatedFiles>();
	case REDIStruct::ChildResourceList:
		return std::make_shared<ChildResourceList>();
	case REDIStruct::ExtraIntData:
		return std::make_shared<ExtraIntData>();
	case REDIStruct::ExtraFloatData:
		return std::make_shared<ExtraFloatData>();
	case REDIStruct::ExtraStringData:
		return std::make_shared<ExtraStringData>();
	}

	throw new std::invalid_argument {"Unknown struct in REDI block."};
}

resource::REDIBlock &resource::ResourceEditInfo::GetStruct(REDIStruct type) { return *m_structs.at(umath::to_integral(type)); }
BlockType resource::ResourceEditInfo::GetType() const { return BlockType::REDI; }
void resource::ResourceEditInfo::Read(const Resource &resource, ufile::IFile &f)
{
	f.Seek(GetOffset());
	for(auto i = decltype(m_structs.size()) {0u}; i < m_structs.size(); ++i) {
		auto block = ConstructStruct(static_cast<REDIStruct>(i));
		auto offset = f.Tell();
		block->SetOffset(offset + f.Read<uint32_t>());
		block->SetSize(f.Read<uint32_t>());
		offset = f.Tell();
		block->Read(resource, f);
		m_structs.at(i) = block;
		f.Seek(offset);
	}
}

void resource::ResourceEditInfo::DebugPrint(std::stringstream &ss, const std::string &t) const
{
	ss << t << "ResourceEditInfo = {\n";
	for(auto i = decltype(m_structs.size()) {0u}; i < m_structs.size(); ++i) {
		auto &strct = m_structs.at(i);
		ss << t << "\t[" << to_string(strct->GetType()) << "] = {\n";
		m_structs.at(i)->DebugPrint(ss, t + "\t\t");
		ss << t << "\t}\n";
	}
	ss << t << "}\n";
}

///////////

BlockType resource::ResourceExtRefList::GetType() const { return BlockType::RERL; }
const std::vector<resource::ResourceExtRefList::ResourceReferenceInfo> &resource::ResourceExtRefList::GetResourceReferenceInfos() const { return m_resourceReferenceInfos; }
void resource::ResourceExtRefList::Read(const Resource &resource, ufile::IFile &f)
{
	f.Seek(GetOffset());

	auto offset = f.Read<uint32_t>();
	auto size = f.Read<uint32_t>();

	if(size == 0)
		return;

	f.Seek(f.Tell() + offset - sizeof(uint32_t) * 2);
	m_resourceReferenceInfos.resize(size);
	for(auto i = decltype(size) {0u}; i < size; ++i) {
		auto &resInfo = m_resourceReferenceInfos.at(i);
		resInfo.id = f.Read<uint64_t>();

		auto previousPosition = f.Tell();

		// jump to string
		// offset is counted from current position,
		// so we will need to add 8 to position later
		f.Seek(previousPosition + f.Read<int64_t>());

		resInfo.name = f.ReadString(); // TODO: UTF8

		f.Seek(previousPosition + sizeof(int64_t)); // 8 is to account for string offset
	}
}
void resource::ResourceExtRefList::DebugPrint(std::stringstream &ss, const std::string &t) const
{
	ss << t << "ResourceExtRefList = {\n";
	for(auto i = decltype(m_resourceReferenceInfos.size()) {0u}; i < m_resourceReferenceInfos.size(); ++i) {
		auto &refInfo = m_resourceReferenceInfos.at(i);
		ss << t << "\t[" << i << "] = {\n";
		ss << t << "\t\tId = " << refInfo.id << "\n";
		ss << t << "\t\tName = " << refInfo.name << "\n";
		ss << t << "\t}\n";
	}
	ss << t << "}\n";
}

///////////

BlockType resource::VBIB::GetType() const { return BlockType::VBIB; }
void resource::VBIB::Read(const Resource &resource, ufile::IFile &f)
{
	f.Seek(GetOffset());

	auto vertexBufferOffset = f.Read<uint32_t>();
	auto vertexBufferCount = f.Read<uint32_t>();
	auto indexBufferOffset = f.Read<uint32_t>();
	auto indexBufferCount = f.Read<uint32_t>();

	f.Seek(GetOffset() + vertexBufferOffset);
	m_vertexBuffers.reserve(vertexBufferCount);
	for(auto i = decltype(vertexBufferCount) {0u}; i < vertexBufferCount; ++i) {
		m_vertexBuffers.push_back({});
		auto &vertexBuffer = m_vertexBuffers.back();

		vertexBuffer.count = f.Read<uint32_t>(); //0
		vertexBuffer.size = f.Read<uint32_t>();  //4
		auto decompressedSize = vertexBuffer.count * vertexBuffer.size;

		auto refA = f.Tell();
		auto attributeOffset = f.Read<uint32_t>(); //8
		auto attributeCount = f.Read<uint32_t>();  //12

		//TODO: Read attributes in the future
		auto refB = f.Tell();
		auto dataOffset = f.Read<uint32_t>(); //16
		auto totalSize = f.Read<uint32_t>();  //20

		f.Seek(refA + attributeOffset);
		vertexBuffer.attributes.reserve(attributeCount);
		for(auto j = decltype(attributeCount) {0u}; j < attributeCount; ++j) {
			auto previousPosition = f.Tell();

			vertexBuffer.attributes.push_back({});
			auto &attribute = vertexBuffer.attributes.back();

			attribute.name = f.ReadString(); // TODO: UTF8

			// Offset is always 40 bytes from the start
			f.Seek(previousPosition + 36);

			attribute.type = f.Read<DXGI_FORMAT>();
			attribute.offset = f.Read<uint32_t>();

			// There's unusual amount of padding in attributes
			f.Seek(previousPosition + 56);
		}

		f.Seek(refB + dataOffset);

		if(totalSize == decompressedSize) {
			vertexBuffer.buffer.resize(totalSize);
			f.Read(vertexBuffer.buffer.data(), vertexBuffer.buffer.size() * sizeof(vertexBuffer.buffer.front()));
		}
		else {
			std::vector<uint8_t> vertexBufferBytes {};
			vertexBufferBytes.resize(totalSize);
			f.Read(vertexBufferBytes.data(), vertexBufferBytes.size() * sizeof(vertexBufferBytes.front()));
			vertexBuffer.buffer = MeshOptimizerVertexDecoder::DecodeVertexBuffer((int)vertexBuffer.count, (int)vertexBuffer.size, vertexBufferBytes);
		}
		f.Seek(refB + 4 + 4); //Go back to the vertex array to read the next iteration
	}

	f.Seek(GetOffset() + 8 + indexBufferOffset); //8 to take into account vertexOffset / count
	m_indexBuffers.reserve(indexBufferCount);
	for(auto i = decltype(indexBufferCount) {0u}; i < indexBufferCount; ++i) {
		m_indexBuffers.push_back({});
		auto &indexBuffer = m_indexBuffers.back();

		indexBuffer.count = f.Read<uint32_t>(); //0
		indexBuffer.size = f.Read<uint32_t>();  //4
		auto decompressedSize = indexBuffer.count * indexBuffer.size;

		auto unknown1 = f.Read<uint32_t>(); //8
		auto unknown2 = f.Read<uint32_t>(); //12

		auto refC = f.Tell();
		auto dataOffset = f.Read<uint32_t>(); //16
		auto dataSize = f.Read<uint32_t>();   //20

		f.Seek(refC + dataOffset);

		if(dataSize == decompressedSize) {
			indexBuffer.buffer.resize(dataSize);
			f.Read(indexBuffer.buffer.data(), indexBuffer.buffer.size() * sizeof(indexBuffer.buffer.front()));
		}
		else {
			std::vector<uint8_t> indexBufferBytes {};
			indexBufferBytes.resize(dataSize);
			f.Read(indexBufferBytes.data(), indexBufferBytes.size() * sizeof(indexBufferBytes.front()));
			indexBuffer.buffer = MeshOptimizerIndexDecoder::DecodeIndexBuffer((int)indexBuffer.count, (int)indexBuffer.size, indexBufferBytes);
		}

		f.Seek(refC + 4 + 4); //Go back to the index array to read the next iteration.
	}
}
void resource::VBIB::DebugPrint(std::stringstream &ss, const std::string &t) const
{
	ss << t << "VBIB = {\n";
	ss << t << "\tVertex buffers:\n";
	for(auto i = decltype(m_vertexBuffers.size()) {0u}; i < m_vertexBuffers.size(); ++i) {
		auto &vbuf = m_vertexBuffers.at(i);
		ss << t << "\t\t[" << i << "] = {\n";
		ss << t << "\t\t\tVertex count = " << vbuf.count << "\n";
		ss << t << "\t\t\tSize per vertex = " << vbuf.size << "\n";
		ss << t << "\t\t\tAttributes:\n";
		for(auto j = decltype(vbuf.attributes.size()) {0u}; j < vbuf.attributes.size(); ++j) {
			auto &attr = vbuf.attributes.at(j);
			ss << t << "\t\t\t[" << j << "] = {\n";
			ss << t << "\t\t\t\tName = " << attr.name << "\n";
			ss << t << "\t\t\t\tOffset = " << attr.offset << "\n";
			ss << t << "\t\t\t\tType = " << to_string(attr.type) << "\n";
			ss << t << "\t\t\t}\n";
		}
		ss << t << "\t\t}\n";
	}
	ss << t << "\tIndex buffers:\n";
	for(auto i = decltype(m_indexBuffers.size()) {0u}; i < m_indexBuffers.size(); ++i) {
		auto &ibuf = m_indexBuffers.at(i);
		ss << t << "\t\t[" << i << "] = {\n";
		ss << t << "\t\t\tIndex count = " << ibuf.count << "\n";
		ss << t << "\t\t\tSize per index = " << ibuf.size << "\n";
		ss << t << "\t\t}\n";
	}
	ss << t << "}\n";
}

const std::vector<resource::VBIB::VertexBuffer> &resource::VBIB::GetVertexBuffers() const { return m_vertexBuffers; }
const std::vector<resource::VBIB::IndexBuffer> &resource::VBIB::GetIndexBuffers() const { return m_indexBuffers; }

void resource::VBIB::VertexBuffer::ReadVertexAttribute(uint32_t offset, const VertexAttribute &attribute, std::vector<float> &outData) const
{
	offset = offset * size + attribute.offset;

	switch(attribute.type) {
	case DXGI_FORMAT::R32G32B32_FLOAT:
		outData.resize(3);
		std::memcpy(outData.data(), buffer.data() + offset, outData.size() * sizeof(outData.front()));
		break;

	case DXGI_FORMAT::R16G16_FLOAT:
		{
			std::array<uint16_t, 2> shorts {};
			std::memcpy(shorts.data(), buffer.data() + offset, shorts.size() * sizeof(shorts.front()));

			outData.resize(2);
			outData.at(0) = umath::float16_to_float32_glm(shorts.at(0));
			outData.at(1) = umath::float16_to_float32_glm(shorts.at(1));
			break;
		}
	case DXGI_FORMAT::R16G16_UNORM:
		{
			std::array<uint16_t, 2> shorts {};
			std::memcpy(shorts.data(), buffer.data() + offset, shorts.size() * sizeof(shorts.front()));

			outData.resize(2);
			outData.at(0) = shorts.at(0) / static_cast<float>(std::numeric_limits<uint16_t>::max());
			outData.at(1) = shorts.at(1) / static_cast<float>(std::numeric_limits<uint16_t>::max());
			break;
		}
	case DXGI_FORMAT::R32G32_FLOAT:
		outData.resize(2);
		std::memcpy(outData.data(), buffer.data() + offset, outData.size() * sizeof(outData.front()));
		break;
	case DXGI_FORMAT::R32_FLOAT:
		outData.resize(1);
		std::memcpy(outData.data(), buffer.data() + offset, outData.size() * sizeof(outData.front()));
		break;
	case DXGI_FORMAT::R8G8B8A8_UINT:
	case DXGI_FORMAT::R8G8B8A8_UNORM:
		{
			std::array<uint8_t, 4> bytes {};
			std::memcpy(bytes.data(), buffer.data() + offset, bytes.size() * sizeof(bytes.front()));

			outData.resize(4);
			for(auto i = decltype(outData.size()) {0u}; i < outData.size(); ++i) {
				outData.at(i) = attribute.type == DXGI_FORMAT::R8G8B8A8_UNORM ? bytes.at(i) / static_cast<float>(std::numeric_limits<uint8_t>::max()) : bytes.at(i);
			}

			break;
		}
	default:
		throw std::runtime_error {"Unsupported \"" + attribute.name + "\" DXGI_FORMAT." + std::to_string(umath::to_integral(attribute.type))};
	}
}
void resource::VBIB::ReadVertexAttribute(uint32_t offset, const VertexBuffer &vertexBuffer, const VertexAttribute &attribute, std::vector<float> &outData) { vertexBuffer.ReadVertexAttribute(offset, attribute, outData); }

///////////

BlockType resource::MBUF::GetType() const { return BlockType::MBUF; }

///////////

BlockType resource::VXVS::GetType() const { return BlockType::VXVS; }

void resource::VXVS::Read(const Resource &resource, ufile::IFile &f) { throw std::invalid_argument {"VXVS block type not yet implemented!"}; }
void resource::VXVS::DebugPrint(std::stringstream &ss, const std::string &t) const { ss << t << "VXVS = UNKNOWN\n"; }

///////////

BlockType resource::SNAP::GetType() const { return BlockType::SNAP; }

void resource::SNAP::Read(const Resource &resource, ufile::IFile &f) { throw std::invalid_argument {"SNAP block type not yet implemented!"}; }
void resource::SNAP::DebugPrint(std::stringstream &ss, const std::string &t) const { ss << t << "VXVS = UNKNOWN\n"; }

///////////

BlockType resource::ResourceIntrospectionManifest::GetType() const { return BlockType::NTRO; }

void resource::ResourceIntrospectionManifest::Read(const Resource &resource, ufile::IFile &f)
{
	f.Seek(GetOffset());
	m_introspectionVersion = f.Read<uint32_t>();
	ReadStructs(f);

	f.Seek(GetOffset() + sizeof(int32_t) * 3);
	ReadEnums(f);
}
uint32_t resource::ResourceIntrospectionManifest::GetIntrospectionVersion() const { return m_introspectionVersion; }
const std::vector<resource::ResourceIntrospectionManifest::ResourceDiskStruct> &resource::ResourceIntrospectionManifest::GetReferencedStructs() const { return m_referencedStructs; }
const std::vector<resource::ResourceIntrospectionManifest::ResourceDiskEnum> &resource::ResourceIntrospectionManifest::GetReferencedEnums() const { return m_referencedEnums; }
void resource::ResourceIntrospectionManifest::DebugPrint(std::stringstream &ss, const std::string &t) const
{
	ss << t << "ResourceIntrospectionManifest = {\n";
	ss << t << "\tIntrospection version: " << m_introspectionVersion << "\n";
	ss << t << "\tStructs:\n";
	for(auto i = decltype(m_referencedStructs.size()) {0u}; i < m_referencedStructs.size(); ++i) {
		auto &strct = m_referencedStructs.at(i);
		ss << t << "\t[" << i << "] = {\n";
		ss << t << "\t\tIntrospection version = " << strct.introspectionVersion << "\n";
		ss << t << "\t\tId = " << strct.id << "\n";
		ss << t << "\t\tName = " << strct.name << "\n";
		ss << t << "\t\tDisk CRC = " << strct.diskCrc << "\n";
		ss << t << "\t\tUser version = " << strct.userVersion << "\n";
		ss << t << "\t\tDisk size = " << strct.diskSize << "\n";
		ss << t << "\t\tAlignment = " << strct.alignment << "\n";
		ss << t << "\t\tBase struct id = " << strct.baseStructId << "\n";
		ss << t << "\t\tStruct flags = " << strct.structFlags << "\n";
		ss << t << "\t\tfield Introspection:\n";
		for(auto j = decltype(strct.fieldIntrospection.size()) {0u}; j < strct.fieldIntrospection.size(); ++j) {
			auto &introSpec = strct.fieldIntrospection.at(j);
			ss << t << "\t\t\t[" << j << "] = {\n";
			ss << t << "\t\t\t\tField name = " << introSpec.fieldName << "\n";
			ss << t << "\t\t\t\tCount = " << introSpec.count << "\n";
			ss << t << "\t\t\t\tDisk offset = " << introSpec.diskOffset << "\n";
			ss << t << "\t\t\t\tIndirections = " << introSpec.indirections.size() << "\n";
			ss << t << "\t\t\t\tType data = " << introSpec.typeData << "\n";
			ss << t << "\t\t\t\tType = " << to_string(introSpec.type) << "\n";
			ss << t << "\t\t\t}\n";
		}
		ss << t << "\t}\n";
	}
	ss << t << "\tEnums:\n";
	for(auto i = decltype(m_referencedEnums.size()) {0u}; i < m_referencedEnums.size(); ++i) {
		auto &en = m_referencedEnums.at(i);
		ss << t << "\t[" << i << "] = {\n";
		ss << t << "\t\tIntrospection version = " << en.introspectionVersion << "\n";
		ss << t << "\t\tId = " << en.id << "\n";
		ss << t << "\t\tName = " << en.name << "\n";
		ss << t << "\t\tDisk CRC = " << en.diskCrc << "\n";
		ss << t << "\t\tUser version = " << en.userVersion << "\n";
		ss << t << "\t\tEnum value introspection:\n";
		for(auto j = decltype(en.enumValueIntrospection.size()) {0u}; j < en.enumValueIntrospection.size(); ++j) {
			auto &introSpec = en.enumValueIntrospection.at(j);
			ss << t << "\t\t\t[" << j << "] = {\n";
			ss << t << "\t\t\t\tEnum value name = " << introSpec.enumValueName << "\n";
			ss << t << "\t\t\t\tEnum value = " << introSpec.enumValue << "\n";
			ss << t << "\t\t\t}\n";
		}
		ss << t << "\t}\n";
	}
	ss << t << "}\n";
}
void resource::ResourceIntrospectionManifest::ReadStructs(ufile::IFile &f)
{
	auto offset = f.Tell();
	auto entriesOffset = f.Read<uint32_t>();
	auto entriesCount = f.Read<uint32_t>();
	if(entriesCount == 0)
		return;
	f.Seek(offset + entriesOffset);
	m_referencedStructs.reserve(entriesCount);
	for(auto i = decltype(entriesCount) {0u}; i < entriesCount; ++i) {
		m_referencedStructs.push_back({});
		auto &diskStruct = m_referencedStructs.back();
		diskStruct.introspectionVersion = f.Read<uint32_t>();
		diskStruct.id = f.Read<uint32_t>();
		diskStruct.name = read_offset_string(f);
		diskStruct.diskCrc = f.Read<uint32_t>();
		diskStruct.userVersion = f.Read<int32_t>();
		diskStruct.diskSize = f.Read<uint16_t>();
		diskStruct.alignment = f.Read<uint16_t>();
		diskStruct.baseStructId = f.Read<uint32_t>();
		auto fieldsOffset = f.Read<uint32_t>();
		auto fieldsSize = f.Read<uint32_t>();
		if(fieldsSize > 0) {
			auto prev = f.Tell();
			f.Seek(f.Tell() + fieldsOffset - sizeof(uint32_t) * 2);
			diskStruct.fieldIntrospection.reserve(fieldsSize);
			for(auto j = decltype(fieldsSize) {0u}; j < fieldsSize; ++j) {
				diskStruct.fieldIntrospection.push_back({});
				auto &field = diskStruct.fieldIntrospection.back();
				field.fieldName = read_offset_string(f);
				field.count = f.Read<int16_t>();
				field.diskOffset = f.Read<int16_t>();

				auto indirectionOffset = f.Read<uint32_t>();
				auto indirectionSize = f.Read<uint32_t>();
				if(indirectionSize > 0) {
					auto prev2 = f.Tell();
					f.Seek(f.Tell() + indirectionOffset - sizeof(uint32_t) * 2);
					field.indirections.resize(indirectionSize);
					f.Read(field.indirections.data(), field.indirections.size() * sizeof(field.indirections.front()));
					f.Seek(prev2);
				}

				field.typeData = f.Read<uint32_t>();
				field.type = f.Read<DataType>();
				f.Seek(f.Tell() + 2); // ??
			}
			f.Seek(prev);
		}
		diskStruct.structFlags = f.Read<uint8_t>();
		f.Seek(f.Tell() + 3); // ??
	}
}
void resource::ResourceIntrospectionManifest::ReadEnums(ufile::IFile &f)
{
	auto offset = f.Tell();
	auto entriesOffset = f.Read<uint32_t>();
	auto entriesCount = f.Read<uint32_t>();
	if(entriesCount == 0)
		return;
	f.Seek(offset + entriesOffset);
	m_referencedEnums.reserve(entriesCount);
	for(auto i = decltype(entriesCount) {0u}; i < entriesCount; ++i) {
		m_referencedEnums.push_back({});
		auto &diskEnum = m_referencedEnums.back();
		diskEnum.introspectionVersion = f.Read<uint32_t>();
		diskEnum.id = f.Read<uint32_t>();
		diskEnum.name = read_offset_string(f);
		diskEnum.diskCrc = f.Read<uint32_t>();
		diskEnum.userVersion = f.Read<int32_t>();

		auto fieldsOffset = f.Read<uint32_t>();
		auto fieldsSize = f.Read<uint32_t>();
		if(fieldsSize > 0) {
			auto prev = f.Tell();
			f.Seek(f.Tell() + fieldsOffset - sizeof(uint32_t) * 2);
			diskEnum.enumValueIntrospection.reserve(fieldsSize);
			for(auto j = decltype(fieldsSize) {0u}; j < fieldsSize; ++j) {
				diskEnum.enumValueIntrospection.push_back({});
				auto &field = diskEnum.enumValueIntrospection.back();
				field.enumValueName = read_offset_string(f);
				field.enumValue = f.Read<int32_t>();
			}
			f.Seek(prev);
		}
	}
}
