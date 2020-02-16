/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "source2/resource.hpp"
#include "source2/resource_data.hpp"
#include "source2/resource_edit_info.hpp"
#include "source2/redi/redi.hpp"
#include <algorithm>
#include <array>
#include <sharedutils/util_string.h>
#include <fsys/filesystem.h>

using namespace source2;

resource::Block *resource::Resource::FindBlock(BlockType type)
{
	auto it = std::find_if(m_blocks.begin(),m_blocks.end(),[type](const std::shared_ptr<Block> &block) {
		return block->GetType() == type;
		});
	return (it != m_blocks.end()) ? it->get() : nullptr;
}
const std::vector<std::shared_ptr<resource::Block>> &resource::Resource::GetBlocks() const {return m_blocks;}
void resource::Resource::Read(std::shared_ptr<VFilePtrInternal> f)
{
	auto fileSize = f->Read<uint32_t>();
	if (fileSize == 0x55AA1234)
		throw std::runtime_error{"Use ValvePak library to parse VPK files."};
	if (fileSize == 0x32736376) // "vcs2"
		throw std::runtime_error{"Use CompiledShader() class to parse compiled shader files."};

	auto headerVersion = f->Read<uint16_t>();
	constexpr uint32_t knownHeaderVersion = 12u;
	if (headerVersion != knownHeaderVersion)
		throw std::runtime_error{"(Bad header version. (" +std::to_string(headerVersion) +" != expected " +std::to_string(knownHeaderVersion) +")"};

	m_version = f->Read<uint16_t>();
	auto startOffset = f->Tell();
	auto blockOffset = f->Read<uint32_t>();
	auto blockCount = f->Read<uint32_t>();
	m_blocks.reserve(blockCount);
	for(auto i=decltype(blockCount){0u};i<blockCount;++i)
	{
		auto blockType = f->Read<std::array<char,4>>(); // TODO: UTF8?
		auto position = f->Tell();
		auto offset = position +f->Read<uint32_t>();
		auto size = f->Read<uint32_t>();
		std::shared_ptr<source2::resource::Block> block = nullptr;
		if(ustring::compare(blockType.data(),"DATA",true,blockType.size()))
		{
			f->Seek(offset);
			auto magic = f->Read<uint32_t>();
			if (magic == source2::resource::BinaryKV3::MAGIC || magic == source2::resource::BinaryKV3::MAGIC2)
				block = std::make_shared<source2::resource::BinaryKV3>();
			f->Seek(position);
		}
		if(block == nullptr)
			block = ConstructFromType(std::string{blockType.data(),blockType.size()});

		block->SetOffset(offset);
		block->SetSize(size);
		block->Read(f);

		m_blocks.push_back(block);

		switch(block->GetType())
		{
		case BlockType::REDI:
			// Try to determine resource type by looking at first compiler indentifier
			if(m_resourceType == ResourceType::Unknown)
			{
				auto &specialDeps = static_cast<SpecialDependencies&>(static_cast<ResourceEditInfo&>(*block).GetStruct(REDIStruct::SpecialDependencies));
				auto &list = specialDeps.GetSpecialDependencies();
				if(list.empty() == false)
					m_resourceType = SpecialDependencies::DetermineResourceTypeByCompilerIdentifier(list.front());
			}
			break;
		case BlockType::NTRO:
			throw std::runtime_error{"NTRO blocks not yet implemented!"};
			/*if(m_resourceType == ResourceType::Unknown && IntrospectionManifest.ReferencedStructs.Count > 0)
			{
			switch (IntrospectionManifest.ReferencedStructs[0].Name)
			{
			case "VSoundEventScript_t":
			ResourceType = ResourceType.SoundEventScript;
			break;

			case "CWorldVisibility":
			ResourceType = ResourceType.WorldVisibility;
			break;
			}
			}*/
			break;
		}
		// TODO: Determine resource type

		f->Seek(position +sizeof(uint32_t) *2);
	}
}
std::shared_ptr<source2::resource::Block> resource::Resource::ConstructFromType(std::string input)
{
	if(input == "DATA")
		return ConstructResourceType();
	else if(input == "REDI")
		return std::make_shared<source2::resource::ResourceEditInfo>();
	else if(input == "RERL")
		return std::make_shared<source2::resource::ResourceExtRefList>();
	else if(input == "VBIB")
		return std::make_shared<source2::resource::VBIB>();
	//else if(input == "NTRO")
	//	return std::make_shared<source2::resource::NTRO>();
	// TODO: Implement other types
	throw std::runtime_error{"Support for type " +input +" not yet implemented!"};
}
std::shared_ptr<source2::resource::ResourceData> resource::Resource::ConstructResourceType()
{
	switch (m_resourceType)
	{
	case ResourceType::Panorama:
	case ResourceType::PanoramaStyle:
	case ResourceType::PanoramaScript:
	case ResourceType::PanoramaLayout:
	case ResourceType::PanoramaDynamicImages:
	case ResourceType::PanoramaVectorGraphic:
		return std::make_shared<source2::resource::Panorama>();

	case ResourceType::Sound:
		return nullptr; // new Sound(); // TODO

	case ResourceType::Texture:
		return std::make_shared<source2::resource::Texture>();

		//case ResourceType::Material:
		//    return new Material(); // TODO

	case ResourceType::SoundEventScript:
		return nullptr; // new SoundEventScript(); // TODO

	case ResourceType::Particle:
		return nullptr; // new BinaryKV3(); // TODO

	case ResourceType::Mesh:
		if (m_version == 0)
		{
			break;
		}

		return nullptr; // new BinaryKV3(); // TODO
	}

	if(FindBlock(BlockType::NTRO))
		return nullptr; // new NTRO(); // TODO
	return std::make_shared<source2::resource::ResourceData>();
}
