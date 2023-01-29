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

#ifndef __SOURCE2_RESOURCE_EDIT_INFO_HPP__
#define __SOURCE2_RESOURCE_EDIT_INFO_HPP__

#include "util_source2.hpp"
#include "block.hpp"
#include <array>
#include <memory>
#include <mathutil/umath.h>

namespace source2::resource {
	class REDIBlock;
	class DLLUS2 ResourceEditInfo : public Block {
	  public:
		static std::shared_ptr<REDIBlock> ConstructStruct(REDIStruct id);

		REDIBlock &GetStruct(REDIStruct type);
		virtual BlockType GetType() const override;
		virtual void Read(const Resource &resource, ufile::IFile &f) override;
		virtual void DebugPrint(std::stringstream &ss, const std::string &t = "") const override;
	  private:
		std::array<std::shared_ptr<REDIBlock>, umath::to_integral(REDIStruct::Count)> m_structs = {};
	};

	class DLLUS2 ResourceExtRefList : public Block {
	  public:
		struct ResourceReferenceInfo {
			uint64_t id = 0;
			std::string name;
		};
		virtual BlockType GetType() const override;
		virtual void Read(const Resource &resource, ufile::IFile &f) override;
		virtual void DebugPrint(std::stringstream &ss, const std::string &t = "") const override;
		const std::vector<ResourceReferenceInfo> &GetResourceReferenceInfos() const;
	  private:
		std::vector<ResourceReferenceInfo> m_resourceReferenceInfos = {};
	};

	enum class DXGI_FORMAT : uint32_t {
		UNKNOWN = 0,
		R32G32B32A32_TYPELESS = 1,
		R32G32B32A32_FLOAT = 2,
		R32G32B32A32_UINT = 3,
		R32G32B32A32_SINT = 4,
		R32G32B32_TYPELESS = 5,
		R32G32B32_FLOAT = 6,
		R32G32B32_UINT = 7,
		R32G32B32_SINT = 8,
		R16G16B16A16_TYPELESS = 9,
		R16G16B16A16_FLOAT = 10,
		R16G16B16A16_UNORM = 11,
		R16G16B16A16_UINT = 12,
		R16G16B16A16_SNORM = 13,
		R16G16B16A16_SINT = 14,
		R32G32_TYPELESS = 15,
		R32G32_FLOAT = 16,
		R32G32_UINT = 17,
		R32G32_SINT = 18,
		R32G8X24_TYPELESS = 19,
		D32_FLOAT_S8X24_UINT = 20,
		R32_FLOAT_X8X24_TYPELESS = 21,
		X32_TYPELESS_G8X24_UINT = 22,
		R10G10B10A2_TYPELESS = 23,
		R10G10B10A2_UNORM = 24,
		R10G10B10A2_UINT = 25,
		R11G11B10_FLOAT = 26,
		R8G8B8A8_TYPELESS = 27,
		R8G8B8A8_UNORM = 28,
		R8G8B8A8_UNORM_SRGB = 29,
		R8G8B8A8_UINT = 30,
		R8G8B8A8_SNORM = 31,
		R8G8B8A8_SINT = 32,
		R16G16_TYPELESS = 33,
		R16G16_FLOAT = 34,
		R16G16_UNORM = 35,
		R16G16_UINT = 36,
		R16G16_SNORM = 37,
		R16G16_SINT = 38,
		R32_TYPELESS = 39,
		D32_FLOAT = 40,
		R32_FLOAT = 41,
		R32_UINT = 42,
		R32_SINT = 43,
		R24G8_TYPELESS = 44,
		D24_UNORM_S8_UINT = 45,
		R24_UNORM_X8_TYPELESS = 46,
		X24_TYPELESS_G8_UINT = 47,
		R8G8_TYPELESS = 48,
		R8G8_UNORM = 49,
		R8G8_UINT = 50,
		R8G8_SNORM = 51,
		R8G8_SINT = 52,
		R16_TYPELESS = 53,
		R16_FLOAT = 54,
		D16_UNORM = 55,
		R16_UNORM = 56,
		R16_UINT = 57,
		R16_SNORM = 58,
		R16_SINT = 59,
		R8_TYPELESS = 60,
		R8_UNORM = 61,
		R8_UINT = 62,
		R8_SNORM = 63,
		R8_SINT = 64,
		A8_UNORM = 65,
		R1_UNORM = 66,
		R9G9B9E5_SHAREDEXP = 67,
		R8G8_B8G8_UNORM = 68,
		G8R8_G8B8_UNORM = 69,
		BC1_TYPELESS = 70,
		BC1_UNORM = 71,
		BC1_UNORM_SRGB = 72,
		BC2_TYPELESS = 73,
		BC2_UNORM = 74,
		BC2_UNORM_SRGB = 75,
		BC3_TYPELESS = 76,
		BC3_UNORM = 77,
		BC3_UNORM_SRGB = 78,
		BC4_TYPELESS = 79,
		BC4_UNORM = 80,
		BC4_SNORM = 81,
		BC5_TYPELESS = 82,
		BC5_UNORM = 83,
		BC5_SNORM = 84,
		B5G6R5_UNORM = 85,
		B5G5R5A1_UNORM = 86,
		B8G8R8A8_UNORM = 87,
		B8G8R8X8_UNORM = 88,
		R10G10B10_XR_BIAS_A2_UNORM = 89,
		B8G8R8A8_TYPELESS = 90,
		B8G8R8A8_UNORM_SRGB = 91,
		B8G8R8X8_TYPELESS = 92,
		B8G8R8X8_UNORM_SRGB = 93,
		BC6H_TYPELESS = 94,
		BC6H_UF16 = 95,
		BC6H_SF16 = 96,
		BC7_TYPELESS = 97,
		BC7_UNORM = 98,
		BC7_UNORM_SRGB = 99,
		AYUV = 100,
		Y410 = 101,
		Y416 = 102,
		NV12 = 103,
		P010 = 104,
		P016 = 105,
		//420_OPAQUE = 106,
		YUY2 = 107,
		Y210 = 108,
		Y216 = 109,
		NV11 = 110,
		AI44 = 111,
		IA44 = 112,
		P8 = 113,
		A8P8 = 114,
		B4G4R4A4_UNORM = 115,
		P208 = 130,
		V208 = 131,
		V408 = 132,
	};
	DLLUS2 std::string to_string(DXGI_FORMAT format);

	enum class DataType : int16_t {
		Unknown = 0,
		Struct = 1,
		Enum = 2, // TODO: not verified with resourceinfo
		ExternalReference = 3,
		String4 = 4, // TODO: not verified with resourceinfo
		SByte = 10,
		Byte = 11,
		Int16 = 12,
		UInt16 = 13,
		Int32 = 14,
		UInt32 = 15,
		Int64 = 16, // TODO: not verified with resourceinfo
		UInt64 = 17,
		Float = 18,
		Matrix2x4 = 21, // TODO: FourVectors2D
		Vector = 22,
		Vector4D = 23,
		Quaternion = 25,
		Fltx4 = 27,
		Color = 28, // TODO: not verified with resourceinfo
		Boolean = 30,
		String = 31,
		Matrix3x4 = 33,
		Matrix3x4a = 36,
		CTransform = 40,
		Vector4D_44 = 44
	};

	class DLLUS2 VBIB : public Block {
	  public:
		struct VertexAttribute {
			std::string name;
			DXGI_FORMAT type = DXGI_FORMAT::UNKNOWN;
			uint32_t offset = 0;
		};

		struct VertexBuffer {
			uint32_t count = 0;
			uint32_t size = 0;
			std::vector<VertexAttribute> attributes;
			std::vector<uint8_t> buffer;

			void ReadVertexAttribute(uint32_t offset, const VertexAttribute &attribute, std::vector<float> &outData) const;
		};

		struct IndexBuffer {
			uint32_t count = 0;
			uint32_t size = 0;
			std::vector<uint8_t> buffer;
		};

		virtual BlockType GetType() const override;
		virtual void Read(const Resource &resource, ufile::IFile &f) override;
		virtual void DebugPrint(std::stringstream &ss, const std::string &t = "") const override;
		const std::vector<VertexBuffer> &GetVertexBuffers() const;
		const std::vector<IndexBuffer> &GetIndexBuffers() const;
	  private:
		void ReadVertexAttribute(uint32_t offset, const VertexBuffer &vertexBuffer, const VertexAttribute &attribute, std::vector<float> &outData);
		std::vector<VertexBuffer> m_vertexBuffers;
		std::vector<IndexBuffer> m_indexBuffers;
	};

	class DLLUS2 MBUF : public VBIB {
	  public:
		virtual BlockType GetType() const override;
	};

	class DLLUS2 VXVS : public Block {
	  public:
		virtual BlockType GetType() const override;
		virtual void Read(const Resource &resource, ufile::IFile &f) override;
		virtual void DebugPrint(std::stringstream &ss, const std::string &t = "") const override;
	};

	class DLLUS2 SNAP : public Block {
	  public:
		virtual BlockType GetType() const override;
		virtual void Read(const Resource &resource, ufile::IFile &f) override;
		virtual void DebugPrint(std::stringstream &ss, const std::string &t = "") const override;
	};

	class DLLUS2 ResourceIntrospectionManifest : public Block {
	  public:
		struct ResourceDiskStruct {
			struct Field {
				std::string fieldName;
				uint16_t count = 0;
				uint16_t diskOffset = 0;
				std::vector<uint8_t> indirections {};
				uint32_t typeData = 0;
				DataType type = DataType::Unknown;
			};

			uint32_t introspectionVersion = 0;
			uint32_t id = 0;
			std::string name = "";
			uint32_t diskCrc = 0;
			int32_t userVersion = 0;
			uint16_t diskSize = 0;
			uint16_t alignment = 0;
			uint32_t baseStructId = 0;
			uint8_t structFlags = 0;
			std::vector<Field> fieldIntrospection;
		};

		struct ResourceDiskEnum {
			struct Value {
				std::string enumValueName;
				int32_t enumValue = 0;
			};
			uint32_t introspectionVersion = 0;
			uint32_t id = 0;
			std::string name;
			uint32_t diskCrc = 0;
			int32_t userVersion = 0;
			std::vector<Value> enumValueIntrospection;
		};

		virtual BlockType GetType() const override;
		virtual void Read(const Resource &resource, ufile::IFile &f) override;
		virtual void DebugPrint(std::stringstream &ss, const std::string &t = "") const override;

		uint32_t GetIntrospectionVersion() const;
		const std::vector<ResourceDiskStruct> &GetReferencedStructs() const;
		const std::vector<ResourceDiskEnum> &GetReferencedEnums() const;
	  private:
		void ReadStructs(ufile::IFile &f);
		void ReadEnums(ufile::IFile &f);
		uint32_t m_introspectionVersion = 0;
		std::vector<ResourceDiskStruct> m_referencedStructs {};
		std::vector<ResourceDiskEnum> m_referencedEnums {};
	};
	DLLUS2 std::string to_string(DataType type);
};

#endif
