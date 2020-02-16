/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "source2/redi/redi.hpp"
#include "source2/impl.hpp"
#include <sharedutils/util_string.h>
#include <fsys/filesystem.h>

using namespace source2;

BlockType resource::REDIBlock::GetType() const {return BlockType::REDI;}

void resource::InputDependencies::Read(std::shared_ptr<VFilePtrInternal> f)
{
	f->Seek(GetOffset());
	auto size = GetSize();
	m_inputDependencies.reserve(size);
	for(auto i=decltype(size){0u};i<size;++i)
	{
		m_inputDependencies.push_back({});
		auto &dependency = m_inputDependencies.back();
		dependency.contentRelativeFilename = read_offset_string(f); // TODO: UTF8
		dependency.contentSearchPath = read_offset_string(f); // TODO: UTF8
		dependency.fileCRC = f->Read<uint32_t>();
		dependency.flags = f->Read<uint32_t>();
	}
}

void resource::ArgumentDependencies::Read(std::shared_ptr<VFilePtrInternal> f)
{
	f->Seek(GetOffset());
	auto size = GetSize();
	m_argumentDependencies.reserve(size);
	for(auto i=decltype(size){0u};i<size;++i)
	{
		m_argumentDependencies.push_back({});
		auto &dependency = m_argumentDependencies.back();
		dependency.parameterName = read_offset_string(f); // TODO: UTF8
		dependency.parameterType = read_offset_string(f); // TODO: UTF8
		dependency.fingerprint = f->Read<uint32_t>();
		dependency.fingerprintDefault = f->Read<uint32_t>();
	}
}

ResourceType resource::SpecialDependencies::DetermineResourceTypeByCompilerIdentifier(const SpecialDependency &input)
{
	auto identifier = input.compilerIdentifier;
	if(ustring::compare(identifier.c_str(),"Compile",true,7))
		identifier = identifier.substr(7);

	// Special mappings and otherwise different identifiers
	if(identifier == "Psf")
		return ResourceType::ParticleSnapshot;
	if(identifier == "AnimGroup")
		return ResourceType::AnimationGroup;
	if(identifier == "VPhysXData")
		return ResourceType::PhysicsCollisionMesh;
	if(identifier == "Font")
		return ResourceType::BitmapFont;
	if(identifier == "RenderMesh")
		return ResourceType::Mesh;
	if(identifier == "Panorama")
	{
		if(input.string == "Panorama Style Compiler Version")
			return ResourceType::PanoramaStyle;
		if(input.string == "Panorama Script Compiler Version")
			return ResourceType::PanoramaScript;
		if(input.string == "Panorama Layout Compiler Version")
			return ResourceType::PanoramaLayout;
		if(input.string == "Panorama Dynamic Images Compiler Version")
			return ResourceType::PanoramaDynamicImages;
		return ResourceType::Panorama;
	}
	if(identifier == "VectorGraphic")
		return ResourceType::PanoramaVectorGraphic;

	if(identifier == "Animation")
		return ResourceType::Animation;
	if(identifier == "AnimationGroup")
		return ResourceType::AnimationGroup;
	if(identifier == "ActionList")
		return ResourceType::ActionList;
	if(identifier == "Sequence")
		return ResourceType::Sequence;
	if(identifier == "Particle")
		return ResourceType::Particle;
	if(identifier == "Material")
		return ResourceType::Material;
	if(identifier == "Sheet")
		return ResourceType::Sheet;
	if(identifier == "Mesh")
		return ResourceType::Mesh;
	if(identifier == "Texture")
		return ResourceType::Texture;
	if(identifier == "Model")
		return ResourceType::Model;
	if(identifier == "PhysicsCollisionMesh")
		return ResourceType::PhysicsCollisionMesh;
	if(identifier == "Sound")
		return ResourceType::Sound;
	if(identifier == "Morph")
		return ResourceType::Morph;
	if(identifier == "ResourceManifest")
		return ResourceType::ResourceManifest;
	if(identifier == "World")
		return ResourceType::World;
	if(identifier == "WorldNode")
		return ResourceType::WorldNode;
	if(identifier == "WorldVisibility")
		return ResourceType::WorldVisibility;
	if(identifier == "EntityLump")
		return ResourceType::EntityLump;
	if(identifier == "SurfaceProperties")
		return ResourceType::SurfaceProperties;
	if(identifier == "SoundEventScript")
		return ResourceType::SoundEventScript;
	if(identifier == "SoundStackScript")
		return ResourceType::SoundStackScript;
	if(identifier == "BitmapFont")
		return ResourceType::BitmapFont;
	if(identifier == "ResourceRemapTable")
		return ResourceType::ResourceRemapTable;
	if(identifier == "Panorama")
		return ResourceType::Panorama;
	if(identifier == "PanoramaStyle")
		return ResourceType::PanoramaStyle;
	if(identifier == "PanoramaLayout")
		return ResourceType::PanoramaLayout;
	if(identifier == "PanoramaDynamicImages")
		return ResourceType::PanoramaDynamicImages;
	if(identifier == "PanoramaScript")
		return ResourceType::PanoramaScript;
	if(identifier == "PanoramaVectorGraphic")
		return ResourceType::PanoramaVectorGraphic;
	if(identifier == "ParticleSnapshot")
		return ResourceType::ParticleSnapshot;
	if(identifier == "Map")
		return ResourceType::Map;
	return ResourceType::Unknown;
}
void resource::SpecialDependencies::Read(std::shared_ptr<VFilePtrInternal> f)
{
	f->Seek(GetOffset());
	auto size = GetSize();
	m_specialDependencies.reserve(size);
	for(auto i=decltype(size){0u};i<size;++i)
	{
		m_specialDependencies.push_back({});
		auto &dependency = m_specialDependencies.back();
		dependency.string = read_offset_string(f); // TODO: UTF8
		dependency.compilerIdentifier = read_offset_string(f); // TODO: UTF8
		dependency.fingerprint = f->Read<uint32_t>();
		dependency.userData = f->Read<uint32_t>();
	}
}
const std::vector<resource::SpecialDependencies::SpecialDependency> &resource::SpecialDependencies::GetSpecialDependencies() const {return m_specialDependencies;}

void resource::CustomDependencies::Read(std::shared_ptr<VFilePtrInternal> f)
{
	f->Seek(GetOffset());
	if(GetSize() > 0)
		throw new std::runtime_error{"CustomDependencies block is not handled."};
}

void resource::AdditionalRelatedFiles::Read(std::shared_ptr<VFilePtrInternal> f)
{
	f->Seek(GetOffset());
	auto size = GetSize();
	m_additionalRelatedFiles.reserve(size);
	for(auto i=decltype(size){0u};i<size;++i)
	{
		m_additionalRelatedFiles.push_back({});
		auto &dependency = m_additionalRelatedFiles.back();
		dependency.contentRelativeFilename = read_offset_string(f); // TODO: UTF8
		dependency.contentSearchPath = read_offset_string(f); // TODO: UTF8
	}
}

void resource::ChildResourceList::Read(std::shared_ptr<VFilePtrInternal> f)
{
	f->Seek(GetOffset());
	auto size = GetSize();
	m_references.reserve(size);
	for(auto i=decltype(size){0u};i<size;++i)
	{
		m_references.push_back({});
		auto &dependency = m_references.back();
		dependency.id = f->Read<uint64_t>();
		dependency.resourceName = read_offset_string(f); // TODO: UTF8

		f->Seek(f->Tell() +4); // ????
	}
}

void resource::ExtraIntData::Read(std::shared_ptr<VFilePtrInternal> f)
{
	f->Seek(GetOffset());
	auto size = GetSize();
	m_editIntData.reserve(size);
	for(auto i=decltype(size){0u};i<size;++i)
	{
		m_editIntData.push_back({});
		auto &dependency = m_editIntData.back();
		dependency.name = read_offset_string(f); // TODO: UTF8
		dependency.value = f->Read<int32_t>();
	}
}

void resource::ExtraFloatData::Read(std::shared_ptr<VFilePtrInternal> f)
{
	f->Seek(GetOffset());
	auto size = GetSize();
	m_editFloatData.reserve(size);
	for(auto i=decltype(size){0u};i<size;++i)
	{
		m_editFloatData.push_back({});
		auto &dependency = m_editFloatData.back();
		dependency.name = read_offset_string(f); // TODO: UTF8
		dependency.value = f->Read<float>();
	}
}

void resource::ExtraStringData::Read(std::shared_ptr<VFilePtrInternal> f)
{
	f->Seek(GetOffset());
	auto size = GetSize();
	m_editStringData.reserve(size);
	for(auto i=decltype(size){0u};i<size;++i)
	{
		m_editStringData.push_back({});
		auto &dependency = m_editStringData.back();
		dependency.name = read_offset_string(f); // TODO: UTF8
		dependency.value = read_offset_string(f); // TODO: UTF8
	}
}
