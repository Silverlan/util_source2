/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "source2/resource_edit_info.hpp"
#include "source2/redi/redi.hpp"
#include "source2/mesh_optimizer.hpp"
#include <fsys/filesystem.h>

using namespace source2;

#pragma optimize("",off)
std::shared_ptr<resource::REDIBlock> resource::ResourceEditInfo::ConstructStruct(REDIStruct id)
{
	switch (id)
	{
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

	throw new std::invalid_argument{"Unknown struct in REDI block."};
}

resource::REDIBlock &resource::ResourceEditInfo::GetStruct(REDIStruct type)
{
	return *m_structs.at(umath::to_integral(type));
}
BlockType resource::ResourceEditInfo::GetType() const {return BlockType::REDI;}
void resource::ResourceEditInfo::Read(std::shared_ptr<VFilePtrInternal> f)
{
	f->Seek(GetOffset());
	for(auto i=decltype(m_structs.size()){0u};i<m_structs.size();++i)
	{
		auto block = ConstructStruct(static_cast<REDIStruct>(i));
		auto offset = f->Tell();
		block->SetOffset(offset +f->Read<uint32_t>());
		block->SetSize(f->Read<uint32_t>());
		offset = f->Tell();
		block->Read(f);
		m_structs.at(i) = block;
		f->Seek(offset);
	}
}

///////////

BlockType resource::ResourceExtRefList::GetType() const {return BlockType::RERL;}
void resource::ResourceExtRefList::Read(std::shared_ptr<VFilePtrInternal> f)
{
	f->Seek(GetOffset());

	auto offset = f->Read<uint32_t>();
	auto size = f->Read<uint32_t>();

	if (size == 0)
		return;

	f->Seek(f->Tell() +offset -sizeof(uint32_t) *2);
	m_resourceReferenceInfos.resize(size);
	for(auto i=decltype(size){0u};i<size;++i)
	{
		auto &resInfo = m_resourceReferenceInfos.at(i);
		resInfo.id = f->Read<uint64_t>();

		auto previousPosition = f->Tell();

		// jump to string
		// offset is counted from current position,
		// so we will need to add 8 to position later
		f->Seek(previousPosition +f->Read<int64_t>());

		resInfo.name = f->ReadString(); // TODO: UTF8

		f->Seek(previousPosition +sizeof(int64_t)); // 8 is to account for string offset
	}
}

///////////

BlockType resource::VBIB::GetType() const {return BlockType::VBIB;}
void resource::VBIB::Read(std::shared_ptr<VFilePtrInternal> f)
{
	f->Seek(GetOffset());

	auto vertexBufferOffset = f->Read<uint32_t>();
	auto vertexBufferCount = f->Read<uint32_t>();
	auto indexBufferOffset = f->Read<uint32_t>();
	auto indexBufferCount = f->Read<uint32_t>();

	f->Seek(GetOffset() +vertexBufferOffset);
	m_vertexBuffers.reserve(vertexBufferCount);
	for(auto i=decltype(vertexBufferCount){0u};i<vertexBufferCount;++i)
	{
		m_vertexBuffers.push_back({});
		auto &vertexBuffer = m_vertexBuffers.back();

		vertexBuffer.count = f->Read<uint32_t>();            //0
		vertexBuffer.size = f->Read<uint32_t>();             //4
		auto decompressedSize = vertexBuffer.count * vertexBuffer.size;

		auto refA = f->Tell();
		auto attributeOffset = f->Read<uint32_t>();  //8
		auto attributeCount = f->Read<uint32_t>();   //12

													//TODO: Read attributes in the future
		auto refB = f->Tell();
		auto dataOffset = f->Read<uint32_t>();       //16
		auto totalSize = f->Read<uint32_t>();        //20

		f->Seek(refA +attributeOffset);
		vertexBuffer.attributes.reserve(attributeCount);
		for(auto j=decltype(attributeCount){0u};j<attributeCount;++j)
		{
			auto previousPosition = f->Tell();

			vertexBuffer.attributes.push_back({});
			auto &attribute = vertexBuffer.attributes.back();

			attribute.name = f->ReadString(); // TODO: UTF8

			// Offset is always 40 bytes from the start
			f->Seek(previousPosition +36);

			attribute.type = f->Read<DXGI_FORMAT>();
			attribute.offset = f->Read<uint32_t>();

			// There's unusual amount of padding in attributes
			f->Seek(previousPosition +56);
		}

		f->Seek(refB +dataOffset);

		if (totalSize == decompressedSize)
		{
			vertexBuffer.buffer.resize(totalSize);
			f->Read(vertexBuffer.buffer.data(),vertexBuffer.buffer.size() *sizeof(vertexBuffer.buffer.front()));
		}
		else
		{
			std::vector<uint8_t> vertexBufferBytes {};
			vertexBufferBytes.resize(totalSize);
			f->Read(vertexBufferBytes.data(),vertexBufferBytes.size() *sizeof(vertexBufferBytes.front()));
			vertexBuffer.buffer = MeshOptimizerVertexDecoder::DecodeVertexBuffer((int)vertexBuffer.count, (int)vertexBuffer.size, vertexBufferBytes);
		}
		f->Seek(refB +4 +4); //Go back to the vertex array to read the next iteration
	}

	f->Seek(GetOffset() +8 +indexBufferOffset); //8 to take into account vertexOffset / count
	m_indexBuffers.reserve(indexBufferCount);
	for(auto i=decltype(indexBufferCount){0u};i<indexBufferCount;++i)
	{
		m_indexBuffers.push_back({});
		auto &indexBuffer = m_indexBuffers.back();

		indexBuffer.count = f->Read<uint32_t>();        //0
		indexBuffer.size = f->Read<uint32_t>();         //4
		auto decompressedSize = indexBuffer.count * indexBuffer.size;

		auto unknown1 = f->Read<uint32_t>();     //8
		auto unknown2 = f->Read<uint32_t>();     //12

		auto refC = f->Tell();
		auto dataOffset = f->Read<uint32_t>();   //16
		auto dataSize = f->Read<uint32_t>();     //20

		f->Seek(refC +dataOffset);

		if (dataSize == decompressedSize)
		{
			indexBuffer.buffer.resize(dataSize);
			f->Read(indexBuffer.buffer.data(),indexBuffer.buffer.size() *sizeof(indexBuffer.buffer.front()));
		}
		else
		{
			std::vector<uint8_t> indexBufferBytes {};
			indexBufferBytes.resize(dataSize);
			f->Read(indexBufferBytes.data(),indexBufferBytes.size() *sizeof(indexBufferBytes.front()));
			indexBuffer.buffer = MeshOptimizerIndexDecoder::DecodeIndexBuffer((int)indexBuffer.count, (int)indexBuffer.size, indexBufferBytes);
		}

		f->Seek(refC +4 +4); //Go back to the index array to read the next iteration.
	}
}

const std::vector<resource::VBIB::VertexBuffer> &resource::VBIB::GetVertexBuffers() const {return m_vertexBuffers;}
const std::vector<resource::VBIB::IndexBuffer> &resource::VBIB::GetIndexBuffers() const {return m_indexBuffers;}

void resource::VBIB::ReadVertexAttribute(uint32_t offset, const VertexBuffer &vertexBuffer, VertexAttribute &attribute,std::vector<float> &outData)
{
	offset = offset *vertexBuffer.size +attribute.offset;

	switch (attribute.type)
	{
	case DXGI_FORMAT::R32G32B32_FLOAT:
		outData.resize(3);
		memcpy(outData.data(),vertexBuffer.buffer.data() +offset,outData.size() *sizeof(outData.front()));
		break;

	case DXGI_FORMAT::R16G16_FLOAT:
	{
		std::array<uint16_t,2> shorts {};
		memcpy(shorts.data(),vertexBuffer.buffer.data() +offset,shorts.size() *sizeof(shorts.front()));

		outData.reserve(2);
		outData.at(0) = umath::float16_to_float32_glm(shorts.at(0));
		outData.at(1) = -umath::float16_to_float32_glm(shorts.at(1));
		break;
	}
	case DXGI_FORMAT::R32G32_FLOAT:
		outData.resize(2);
		memcpy(outData.data(),vertexBuffer.buffer.data() +offset,outData.size() *sizeof(outData.front()));
		outData.at(1) *= -1.f; // Flip texcoord
		break;
	case DXGI_FORMAT::R8G8B8A8_UINT:
	case DXGI_FORMAT::R8G8B8A8_UNORM:
	{
		std::array<uint8_t,4> bytes {};
		memcpy(bytes.data(),vertexBuffer.buffer.data() +offset,bytes.size() *sizeof(bytes.front()));

		outData.resize(4);
		for(auto i=decltype(outData.size()){0u};i<outData.size();++i)
		{
			outData.at(i) = attribute.type == DXGI_FORMAT::R8G8B8A8_UNORM
				? bytes.at(i) / static_cast<float>(std::numeric_limits<uint8_t>::max())
				: bytes.at(i);
		}

		break;
	}
	default:
		throw std::runtime_error{"Unsupported \"" +attribute.name +"\" DXGI_FORMAT." +std::to_string(umath::to_integral(attribute.type))};
	}
}
#pragma optimize("",on)
