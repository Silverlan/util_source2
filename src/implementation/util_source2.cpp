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

#include <fsys/filesystem.h>
#include <sharedutils/util_string.h>
#include <mathutil/uvec.h>

module source2;

std::string source2::to_string(REDIStruct blockType)
{
	switch(blockType) {
	case REDIStruct::InputDependencies:
		return "InputDependencies";
	case REDIStruct::AdditionalInputDependencies:
		return "AdditionalInputDependencies";
	case REDIStruct::ArgumentDependencies:
		return "ArgumentDependencies";
	case REDIStruct::SpecialDependencies:
		return "SpecialDependencies";
	case REDIStruct::CustomDependencies:
		return "CustomDependencies";
	case REDIStruct::AdditionalRelatedFiles:
		return "AdditionalRelatedFiles";
	case REDIStruct::ChildResourceList:
		return "ChildResourceList";
	case REDIStruct::ExtraIntData:
		return "ExtraIntData";
	case REDIStruct::ExtraFloatData:
		return "ExtraFloatData";
	case REDIStruct::ExtraStringData:
		return "ExtraStringData";
	}
	return "Invalid";
}
std::string source2::to_string(ResourceType blockType)
{
	switch(blockType) {
	case ResourceType::Unknown:
		return "Unknown";
	case ResourceType::Animation:
		return "Animation";
	case ResourceType::AnimationGroup:
		return "AnimationGroup";
	case ResourceType::ActionList:
		return "ActionList";
	case ResourceType::Sequence:
		return "Sequence";
	case ResourceType::Particle:
		return "Particle";
	case ResourceType::Material:
		return "Material";
	case ResourceType::Sheet:
		return "Sheet";
	case ResourceType::Mesh:
		return "Mesh";
	case ResourceType::Texture:
		return "Texture";
	case ResourceType::Model:
		return "Model";
	case ResourceType::PhysicsCollisionMesh:
		return "PhysicsCollisionMesh";
	case ResourceType::Sound:
		return "Sound";
	case ResourceType::Morph:
		return "Morph";
	case ResourceType::ResourceManifest:
		return "ResourceManifest";
	case ResourceType::World:
		return "World";
	case ResourceType::WorldNode:
		return "WorldNode";
	case ResourceType::WorldVisibility:
		return "WorldVisibility";
	case ResourceType::EntityLump:
		return "EntityLump";
	case ResourceType::SurfaceProperties:
		return "SurfaceProperties";
	case ResourceType::SoundEventScript:
		return "SoundEventScript";
	case ResourceType::SoundStackScript:
		return "SoundStackScript";
	case ResourceType::BitmapFont:
		return "BitmapFont";
	case ResourceType::ResourceRemapTable:
		return "ResourceRemapTable";
	case ResourceType::Panorama:
		return "Panorama";
	case ResourceType::PanoramaStyle:
		return "PanoramaStyle";
	case ResourceType::PanoramaLayout:
		return "PanoramaLayout";
	case ResourceType::PanoramaDynamicImages:
		return "PanoramaDynamicImages";
	case ResourceType::PanoramaScript:
		return "PanoramaScript";
	case ResourceType::PanoramaVectorGraphic:
		return "PanoramaVectorGraphic";
	case ResourceType::ParticleSnapshot:
		return "ParticleSnapshot";
	case ResourceType::Map:
		return "Map";
	}
	return "Invalid";
}
std::string source2::to_string(VTexExtraData blockType)
{
	switch(blockType) {
	case VTexExtraData::UNKNOWN:
		return "UNKNOWN";
	case VTexExtraData::FALLBACK_BITS:
		return "FALLBACK_BITS";
	case VTexExtraData::SHEET:
		return "SHEET";
	case VTexExtraData::FILL_TO_POWER_OF_TWO:
		return "FILL_TO_POWER_OF_TWO";
	case VTexExtraData::COMPRESSED_MIP_SIZE:
		return "COMPRESSED_MIP_SIZE";
	}
	return "Invalid";
}
std::string source2::to_string(VTexFlags blockType)
{
	switch(blockType) {
	case VTexFlags::None:
		return "None";
	case VTexFlags::SUGGEST_CLAMPS:
		return "SUGGEST_CLAMPS";
	case VTexFlags::SUGGEST_CLAMPT:
		return "SUGGEST_CLAMPT";
	case VTexFlags::SUGGEST_CLAMPU:
		return "SUGGEST_CLAMPU";
	case VTexFlags::NO_LOD:
		return "NO_LOD";
	case VTexFlags::CUBE_TEXTURE:
		return "CUBE_TEXTURE";
	case VTexFlags::VOLUME_TEXTURE:
		return "VOLUME_TEXTURE";
	case VTexFlags::TEXTURE_ARRAY:
		return "TEXTURE_ARRAY";
	}
	return "Invalid";
}
std::string source2::to_string(VTexFormat blockType)
{
	switch(blockType) {
	case VTexFormat::UNKNOWN:
		return "UNKNOWN";
	case VTexFormat::DXT1:
		return "DXT1";
	case VTexFormat::DXT5:
		return "DXT5";
	case VTexFormat::I8:
		return "I8";
	case VTexFormat::RGBA8888:
		return "RGBA8888";
	case VTexFormat::R16:
		return "R16";
	case VTexFormat::RG1616:
		return "RG1616";
	case VTexFormat::RGBA16161616:
		return "RGBA16161616";
	case VTexFormat::R16F:
		return "R16F";
	case VTexFormat::RG1616F:
		return "RG1616F";
	case VTexFormat::RGBA16161616F:
		return "RGBA16161616F";
	case VTexFormat::R32F:
		return "R32F";
	case VTexFormat::RG3232F:
		return "RG3232F";
	case VTexFormat::RGB323232F:
		return "RGB323232F";
	case VTexFormat::RGBA32323232F:
		return "RGBA32323232F";
	case VTexFormat::JPEG_RGBA8888:
		return "JPEG_RGBA8888";
	case VTexFormat::PNG_RGBA8888:
		return "PNG_RGBA8888";
	case VTexFormat::JPEG_DXT5:
		return "JPEG_DXT5";
	case VTexFormat::PNG_DXT5:
		return "PNG_DXT5";
	case VTexFormat::BC6H:
		return "BC6H";
	case VTexFormat::BC7:
		return "BC7";
	case VTexFormat::ATI2N:
		return "ATI2N";
	case VTexFormat::IA88:
		return "IA88";
	case VTexFormat::ETC2:
		return "ETC2";
	case VTexFormat::ETC2_EAC:
		return "ETC2_EAC";
	case VTexFormat::R11_EAC:
		return "R11_EAC";
	case VTexFormat::RG11_EAC:
		return "RG11_EAC";
	case VTexFormat::ATI1N:
		return "ATI1N";
	case VTexFormat::BGRA8888:
		return "BGRA8888";
	}
	return "Invalid";
}
std::string source2::to_string(BlockType blockType)
{
	switch(blockType) {
	case BlockType::None:
		return "None";
	case BlockType::RERL:
		return "RERL";
	case BlockType::REDI:
		return "REDI";
	case BlockType::NTRO:
		return "NTRO";
	case BlockType::DATA:
		return "DATA";
	case BlockType::VBIB:
		return "VBIB";
	case BlockType::VXVS:
		return "VXVS";
	case BlockType::SNAP:
		return "SNAP";
	case BlockType::CTRL:
		return "CTRL";
	case BlockType::MDAT:
		return "MDAT";
	case BlockType::MRPH:
		return "MRPH";
	case BlockType::MBUF:
		return "MBUF";
	case BlockType::ANIM:
		return "ANIM";
	case BlockType::ASEQ:
		return "ASEQ";
	case BlockType::AGRP:
		return "AGRP";
	case BlockType::PHYS:
		return "PHYS";
	}
	return "Invalid";
}

std::shared_ptr<source2::resource::Resource> source2::load_resource(ufile::IFile &file, const std::function<std::unique_ptr<ufile::IFile>(const std::string &)> &fAssetLoader)
{
	auto resource = std::make_shared<resource::Resource>(fAssetLoader);
	if(resource->Read(file) == false)
		resource = nullptr;
	return resource;
}

void source2::debug_print(resource::Resource &resource, std::stringstream &ss)
{
	for(auto &block : resource.GetBlocks()) {
		ss << "[" << to_string(block->GetType()) << "]\n";
		block->DebugPrint(ss, "\t");
	}
}
