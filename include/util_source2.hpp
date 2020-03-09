/*
MIT License

Copyright (c) 2020 Silverlan
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

#ifndef __UTIL_SOURCE2_HPP__
#define __UTIL_SOURCE2_HPP__

#include <memory>
#include <sstream>

class VFilePtrInternal;
namespace source2
{
	enum class BlockType : uint8_t
	{
		None = 0,
		RERL = 1,
		REDI,
		NTRO,
		DATA,
		VBIB,
		VXVS,
		SNAP,
		CTRL,
		MDAT,
		MRPH,
		MBUF,
		ANIM,
		ASEQ,
		AGRP,
		PHYS
	};
	std::string to_string(BlockType blockType);

	enum VTexFormat : uint8_t
	{
		UNKNOWN = 0,
		DXT1 = 1,
		DXT5 = 2,
		I8 = 3,
		RGBA8888 = 4,
		R16 = 5,
		RG1616 = 6,
		RGBA16161616 = 7,
		R16F = 8,
		RG1616F = 9,
		RGBA16161616F = 10,
		R32F = 11,
		RG3232F = 12,
		RGB323232F = 13,
		RGBA32323232F = 14,
		JPEG_RGBA8888 = 15,
		PNG_RGBA8888 = 16,
		JPEG_DXT5 = 17,
		PNG_DXT5 = 18,
		BC6H = 19,
		BC7 = 20,
		ATI2N = 21,
		IA88 = 22,
		ETC2 = 23,
		ETC2_EAC = 24,
		R11_EAC = 25,
		RG11_EAC = 26,
		ATI1N = 27,
		BGRA8888 = 28
	};
	std::string to_string(VTexFormat blockType);

	enum class VTexFlags : uint16_t
	{
		None = 0u,
		SUGGEST_CLAMPS = 0x00000001,
		SUGGEST_CLAMPT = 0x00000002,
		SUGGEST_CLAMPU = 0x00000004,
		NO_LOD = 0x00000008,
		CUBE_TEXTURE = 0x00000010,
		VOLUME_TEXTURE = 0x00000020,
		TEXTURE_ARRAY = 0x00000040
	};
	std::string to_string(VTexFlags blockType);

	enum class VTexExtraData : uint32_t
	{
		UNKNOWN = 0,
		FALLBACK_BITS = 1,
		SHEET = 2,
		FILL_TO_POWER_OF_TWO = 3,
		COMPRESSED_MIP_SIZE = 4
	};
	std::string to_string(VTexExtraData blockType);

	enum class ResourceType : uint16_t
	{
		Unknown = 0u,
		Animation,
		AnimationGroup,
		ActionList,
		Sequence,
		Particle,
		Material,
		Sheet,
		Mesh,
		Texture,
		Model,
		PhysicsCollisionMesh,
		Sound,
		Morph,
		ResourceManifest,
		World,
		WorldNode,
		WorldVisibility,
		EntityLump,
		SurfaceProperties,
		SoundEventScript,
		SoundStackScript,
		BitmapFont,
		ResourceRemapTable,
		Panorama,
		PanoramaStyle,
		PanoramaLayout,
		PanoramaDynamicImages,
		PanoramaScript,
		PanoramaVectorGraphic,
		ParticleSnapshot,
		Map
	};
	std::string to_string(ResourceType blockType);

	enum class REDIStruct : uint8_t
	{
		InputDependencies,
		AdditionalInputDependencies,
		ArgumentDependencies,
		SpecialDependencies,
		CustomDependencies,
		AdditionalRelatedFiles,
		ChildResourceList,
		ExtraIntData,
		ExtraFloatData,
		ExtraStringData,

		Count
	};
	std::string to_string(REDIStruct blockType);

	namespace resource {class Resource;};
	std::shared_ptr<resource::Resource> load_resource(std::shared_ptr<VFilePtrInternal> file);
	void debug_print(resource::Resource &resource,std::stringstream &ss);
};

#endif
