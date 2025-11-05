// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-FileCopyrightText: (c) 2015 Steam Database
// SPDX-License-Identifier: MIT

module;

module source2;

import :impl;

using namespace source2;

BlockType resource::REDIBlock::GetType() const { return BlockType::REDI; }

void resource::InputDependencies::InputDependency::DebugPrint(std::stringstream &ss, const std::string &t) const
{
	ss << t << "Content relative filename: " << contentRelativeFilename << "\n";
	ss << t << "Content search path: " << contentSearchPath << "\n";
	ss << t << "File CRC: " << fileCRC << "\n";
	ss << t << "Flags: " << flags << "\n";
}

void resource::InputDependencies::Read(const Resource &resource, ufile::IFile &f)
{
	f.Seek(GetOffset());
	auto size = GetSize();
	m_inputDependencies.reserve(size);
	for(auto i = decltype(size) {0u}; i < size; ++i) {
		m_inputDependencies.push_back({});
		auto &dependency = m_inputDependencies.back();
		dependency.contentRelativeFilename = read_offset_string(f); // TODO: UTF8
		dependency.contentSearchPath = read_offset_string(f);       // TODO: UTF8
		dependency.fileCRC = f.Read<uint32_t>();
		dependency.flags = f.Read<uint32_t>();
	}
}

void resource::InputDependencies::DebugPrint(std::stringstream &ss, const std::string &t) const
{
	ss << t << "InputDependencies = {\n";
	for(auto i = decltype(m_inputDependencies.size()) {0u}; i < m_inputDependencies.size(); ++i) {
		auto &inputDependency = m_inputDependencies.at(i);
		ss << t << "\t{\n";
		inputDependency.DebugPrint(ss, t + "\t\t");
		ss << t << "\t},\n";
	}
	ss << t << "}\n";
}

void resource::ArgumentDependencies::ArgumentDependency::DebugPrint(std::stringstream &ss, const std::string &t) const
{
	ss << t << "Parameter name: " << parameterName << "\n";
	ss << t << "Parameter type: " << parameterType << "\n";
	ss << t << "Fingerprint: " << fingerprint << "\n";
	ss << t << "Fingerprint default: " << fingerprintDefault << "\n";
}

void resource::ArgumentDependencies::Read(const Resource &resource, ufile::IFile &f)
{
	f.Seek(GetOffset());
	auto size = GetSize();
	m_argumentDependencies.reserve(size);
	for(auto i = decltype(size) {0u}; i < size; ++i) {
		m_argumentDependencies.push_back({});
		auto &dependency = m_argumentDependencies.back();
		dependency.parameterName = read_offset_string(f); // TODO: UTF8
		dependency.parameterType = read_offset_string(f); // TODO: UTF8
		dependency.fingerprint = f.Read<uint32_t>();
		dependency.fingerprintDefault = f.Read<uint32_t>();
	}
}

void resource::ArgumentDependencies::DebugPrint(std::stringstream &ss, const std::string &t) const
{
	ss << t << "ArgumentDependencies = {\n";
	for(auto i = decltype(m_argumentDependencies.size()) {0u}; i < m_argumentDependencies.size(); ++i) {
		auto &argDependency = m_argumentDependencies.at(i);
		ss << t << "\t{\n";
		argDependency.DebugPrint(ss, t + "\t\t");
		ss << t << "\t},\n";
	}
	ss << t << "}\n";
}

ResourceType resource::SpecialDependencies::DetermineResourceTypeByCompilerIdentifier(const SpecialDependency &input)
{
	auto identifier = input.compilerIdentifier;
	if(ustring::compare(identifier.c_str(), "Compile", true, 7))
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
	if(identifier == "Panorama") {
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
void resource::SpecialDependencies::SpecialDependency::DebugPrint(std::stringstream &ss, const std::string &t) const
{
	ss << t << "String: " << string << "\n";
	ss << t << "Compiler identifier: " << compilerIdentifier << "\n";
	ss << t << "Fingerprint: " << fingerprint << "\n";
	ss << t << "User data: " << userData << "\n";
}
void resource::SpecialDependencies::Read(const Resource &resource, ufile::IFile &f)
{
	f.Seek(GetOffset());
	auto size = GetSize();
	m_specialDependencies.reserve(size);
	for(auto i = decltype(size) {0u}; i < size; ++i) {
		m_specialDependencies.push_back({});
		auto &dependency = m_specialDependencies.back();
		dependency.string = read_offset_string(f);             // TODO: UTF8
		dependency.compilerIdentifier = read_offset_string(f); // TODO: UTF8
		dependency.fingerprint = f.Read<uint32_t>();
		dependency.userData = f.Read<uint32_t>();
	}
}
void resource::SpecialDependencies::DebugPrint(std::stringstream &ss, const std::string &t) const
{
	ss << t << "SpecialDependencies = {\n";
	for(auto i = decltype(m_specialDependencies.size()) {0u}; i < m_specialDependencies.size(); ++i) {
		auto &specialDep = m_specialDependencies.at(i);
		ss << t << "\t{\n";
		specialDep.DebugPrint(ss, t + "\t\t");
		ss << t << "\t},\n";
	}
	ss << t << "}\n";
}
const std::vector<resource::SpecialDependencies::SpecialDependency> &resource::SpecialDependencies::GetSpecialDependencies() const { return m_specialDependencies; }

void resource::CustomDependencies::Read(const Resource &resource, ufile::IFile &f)
{
	f.Seek(GetOffset());
	if(GetSize() > 0)
		throw new std::runtime_error {"CustomDependencies block is not handled."};
}

void resource::CustomDependencies::DebugPrint(std::stringstream &ss, const std::string &t) const
{
	ss << t << "CustomDependencies = {\n";
	ss << t << "}\n";
}

void resource::AdditionalRelatedFiles::Read(const Resource &resource, ufile::IFile &f)
{
	f.Seek(GetOffset());
	auto size = GetSize();
	m_additionalRelatedFiles.reserve(size);
	for(auto i = decltype(size) {0u}; i < size; ++i) {
		m_additionalRelatedFiles.push_back({});
		auto &dependency = m_additionalRelatedFiles.back();
		dependency.contentRelativeFilename = read_offset_string(f); // TODO: UTF8
		dependency.contentSearchPath = read_offset_string(f);       // TODO: UTF8
	}
}

void resource::AdditionalRelatedFiles::AdditionalRelatedFile::DebugPrint(std::stringstream &ss, const std::string &t) const
{
	ss << t << "Content relative filename: " << contentRelativeFilename << "\n";
	ss << t << "Content search path: " << contentSearchPath << "\n";
}

void resource::AdditionalRelatedFiles::DebugPrint(std::stringstream &ss, const std::string &t) const
{
	ss << t << "AdditionalRelatedFiles = {\n";
	for(auto i = decltype(m_additionalRelatedFiles.size()) {0u}; i < m_additionalRelatedFiles.size(); ++i) {
		auto &additionalRelatedFile = m_additionalRelatedFiles.at(i);
		ss << t << "\t{\n";
		additionalRelatedFile.DebugPrint(ss, t + "\t\t");
		ss << t << "\t},\n";
	}
	ss << t << "}\n";
}

void resource::ChildResourceList::Read(const Resource &resource, ufile::IFile &f)
{
	f.Seek(GetOffset());
	auto size = GetSize();
	m_references.reserve(size);
	for(auto i = decltype(size) {0u}; i < size; ++i) {
		m_references.push_back({});
		auto &dependency = m_references.back();
		dependency.id = f.Read<uint64_t>();
		dependency.resourceName = read_offset_string(f); // TODO: UTF8

		f.Seek(f.Tell() + 4); // ????
	}
}

void resource::ChildResourceList::ReferenceInfo::DebugPrint(std::stringstream &ss, const std::string &t) const
{
	ss << t << "Id: " << id << "\n";
	ss << t << "Resource name: " << resourceName << "\n";
}

void resource::ChildResourceList::DebugPrint(std::stringstream &ss, const std::string &t) const
{
	ss << t << "ChildResourceList = {\n";
	for(auto i = decltype(m_references.size()) {0u}; i < m_references.size(); ++i) {
		auto &refInfo = m_references.at(i);
		ss << t << "\t{\n";
		refInfo.DebugPrint(ss, t + "\t\t");
		ss << t << "\t},\n";
	}
	ss << t << "}\n";
}

void resource::ExtraIntData::Read(const Resource &resource, ufile::IFile &f)
{
	f.Seek(GetOffset());
	auto size = GetSize();
	m_editIntData.reserve(size);
	for(auto i = decltype(size) {0u}; i < size; ++i) {
		m_editIntData.push_back({});
		auto &dependency = m_editIntData.back();
		dependency.name = read_offset_string(f); // TODO: UTF8
		dependency.value = f.Read<int32_t>();
	}
}

void resource::ExtraIntData::EditIntData::DebugPrint(std::stringstream &ss, const std::string &t) const
{
	ss << t << "Name: " << name << "\n";
	ss << t << "Value: " << value << "\n";
}

void resource::ExtraIntData::DebugPrint(std::stringstream &ss, const std::string &t) const
{
	ss << t << "ExtraIntData = {\n";
	for(auto i = decltype(m_editIntData.size()) {0u}; i < m_editIntData.size(); ++i) {
		auto &editIntData = m_editIntData.at(i);
		ss << t << "\t{\n";
		editIntData.DebugPrint(ss, t + "\t\t");
		ss << t << "\t},\n";
	}
	ss << t << "}\n";
}

void resource::ExtraFloatData::Read(const Resource &resource, ufile::IFile &f)
{
	f.Seek(GetOffset());
	auto size = GetSize();
	m_editFloatData.reserve(size);
	for(auto i = decltype(size) {0u}; i < size; ++i) {
		m_editFloatData.push_back({});
		auto &dependency = m_editFloatData.back();
		dependency.name = read_offset_string(f); // TODO: UTF8
		dependency.value = f.Read<float>();
	}
}

void resource::ExtraFloatData::EditFloatData::DebugPrint(std::stringstream &ss, const std::string &t) const
{
	ss << t << "Name: " << name << "\n";
	ss << t << "Value: " << value << "\n";
}

void resource::ExtraFloatData::DebugPrint(std::stringstream &ss, const std::string &t) const
{
	ss << t << "ExtraFloatData = {\n";
	for(auto i = decltype(m_editFloatData.size()) {0u}; i < m_editFloatData.size(); ++i) {
		auto &editFloatData = m_editFloatData.at(i);
		ss << t << "\t{\n";
		editFloatData.DebugPrint(ss, t + "\t\t");
		ss << t << "\t},\n";
	}
	ss << t << "}\n";
}

void resource::ExtraStringData::Read(const Resource &resource, ufile::IFile &f)
{
	f.Seek(GetOffset());
	auto size = GetSize();
	m_editStringData.reserve(size);
	for(auto i = decltype(size) {0u}; i < size; ++i) {
		m_editStringData.push_back({});
		auto &dependency = m_editStringData.back();
		dependency.name = read_offset_string(f);  // TODO: UTF8
		dependency.value = read_offset_string(f); // TODO: UTF8
	}
}

void resource::ExtraStringData::EditStringData::DebugPrint(std::stringstream &ss, const std::string &t) const
{
	ss << t << "Name: " << name << "\n";
	ss << t << "Value: " << value << "\n";
}

void resource::ExtraStringData::DebugPrint(std::stringstream &ss, const std::string &t) const
{
	ss << t << "ExtraFloatData = {\n";
	for(auto i = decltype(m_editStringData.size()) {0u}; i < m_editStringData.size(); ++i) {
		auto &editStringData = m_editStringData.at(i);
		ss << t << "\t{\n";
		editStringData.DebugPrint(ss, t + "\t\t");
		ss << t << "\t},\n";
	}
	ss << t << "}\n";
}
