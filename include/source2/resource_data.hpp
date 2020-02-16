/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __SOURCE2_RESOURCE_DATA_HPP__
#define __SOURCE2_RESOURCE_DATA_HPP__

#include "block.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <mathutil/uvec.h>

class DataStream;
namespace util {using GUID = std::array<uint8_t,16>;};
namespace source2::resource
{
	class ResourceData
		: public Block
	{
	public:
		virtual BlockType GetType() const override;
		virtual void Read(std::shared_ptr<VFilePtrInternal> f) override;
	};

	class Panorama
		: public ResourceData
	{
	public:
		struct NameEntry
		{
			std::string name;
			uint32_t unknown1;
			uint32_t unknown2;
		};
		const std::vector<NameEntry> &GetNames() const;
		virtual void Read(std::shared_ptr<VFilePtrInternal> f) override;
	private:
		std::vector<NameEntry> m_names;
		std::vector<uint8_t> m_data;
		uint32_t m_crc32;
	};

	class Texture
		: public ResourceData
	{
	public:
		uint16_t GetVersion() const;
		uint16_t GetWidth() const;
		uint16_t GetHeight() const;
		uint16_t GetDepth() const;
		Vector4 GetReflectivity() const;
		VTexFlags GetFlags() const;
		VTexFormat GetFormat() const;
		uint8_t GetMipMapCount() const;
		uint32_t GetPicmip0Res() const;
		const std::unordered_map<VTexExtraData,std::vector<uint8_t>> &GetExtraData() const;
		virtual void Read(std::shared_ptr<VFilePtrInternal> f) override;

		uint32_t GetBlockSize();
		uint64_t GetMipmapDataOffset(uint8_t mipmap);

		void ReadTextureData(uint8_t mipLevel,std::vector<uint8_t> &outData);

		uint32_t CalculateBufferSizeForMipLevel(uint8_t mipLevel);
	private:
		VTexFlags m_flags = VTexFlags::None;
		Vector4 m_reflectivity = {};
		uint16_t m_width = 0u;
		uint16_t m_height = 0u;
		uint16_t m_depth = 0u;
		VTexFormat m_format = VTexFormat::UNKNOWN;
		uint8_t m_mipMapCount = 0u;
		uint32_t m_picmip0Res = 0u;
		uint64_t m_dataOffset = 0ull;
		std::unordered_map<VTexExtraData,std::vector<uint8_t>> m_extraData {};
		uint32_t m_nonPow2Width = 0u;
		uint32_t m_nonPow2Height = 0u;
		std::shared_ptr<VFilePtrInternal> m_file = nullptr;

		bool m_isCompressed = false;
		std::vector<int32_t> m_compressedMips = {};
	};

	enum class KVType : uint8_t
	{
		STRING_MULTI = 0, // STRING_MULTI doesn't have an ID
		Null = 1,
		BOOLEAN = 2,
		INT64 = 3,
		UINT64 = 4,
		DOUBLE = 5,
		STRING = 6,
		BINARY_BLOB = 7,
		ARRAY = 8,
		OBJECT = 9,
		ARRAY_TYPED = 10,
		INT32 = 11,
		UINT32 = 12,
		BOOLEAN_TRUE = 13,
		BOOLEAN_FALSE = 14,
		INT64_ZERO = 15,
		INT64_ONE = 16,
		DOUBLE_ZERO = 17,
		DOUBLE_ONE = 18,
	};

	enum class KVFlag : uint8_t
	{
		None,
		Resource,
		DeferredResource
	};

	class KVValue
	{
	public:
		KVValue(KVType type,const std::shared_ptr<void> &value)
			: m_type{type},m_object{value}
		{}
	private:
		KVType m_type = KVType::STRING_MULTI;
		std::shared_ptr<void> m_object = nullptr;
	};

	class KVFlaggedValue
		: public KVValue
	{
	public:
		KVFlaggedValue(KVType type,const std::shared_ptr<void> &value)
			: KVValue{type,value}
		{}
		KVFlaggedValue(KVType type,KVFlag flag,const std::shared_ptr<void> &value)
			: KVValue{type,value},m_flag{flag}
		{}
		KVFlag GetFlag() const {return m_flag;}
	private:
		KVFlag m_flag = KVFlag::None;
	};

	using BinaryBlob = std::vector<uint8_t>;
	class KVObject
		: public std::enable_shared_from_this<KVObject>
	{
	public:
		KVObject(const std::string &name,bool isArray=false);
		void AddProperty(const std::string &name,const std::shared_ptr<KVValue> &value);
		const std::unordered_map<std::string,std::shared_ptr<KVValue>> &GetValues() const;
	private:
		std::string m_key;
		std::unordered_map<std::string,std::shared_ptr<KVValue>> m_values;
		bool m_isArray = false;
		uint32_t m_count = 0u;
	};

	class BinaryKV3
		: public ResourceData
	{
	public:
		static const util::GUID KV3_ENCODING_BINARY_BLOCK_COMPRESSED;
		static const util::GUID KV3_ENCODING_BINARY_UNCOMPRESSED;
		static const util::GUID KV3_ENCODING_BINARY_BLOCK_LZ4;
		static const util::GUID KV3_FORMAT_GENERIC;
		static constexpr int32_t MAGIC = 0x03564B56; // VKV3 (3 isn't ascii, its 0x03)
		static constexpr int32_t MAGIC2 = 0x4B563301; // KV3\x01
		static const std::array<uint8_t,16> ENCODING;
		static const std::array<uint8_t,16> FORMAT;
		static const std::array<uint8_t,4> SIG; // VKV3 (3 isn't ascii, its 0x03)

		virtual void Read(std::shared_ptr<VFilePtrInternal> f) override;
	private:
		template<typename T>
			static std::shared_ptr<KVValue> MakeValue(KVType type, T data, KVFlag flag);
		static std::shared_ptr<KVValue> MakeValueFromPtr(KVType type, const std::shared_ptr<void> &data, KVFlag flag);
		static KVType ConvertBinaryOnlyKVType(KVType type);
		void ReadVersion2(std::shared_ptr<VFilePtrInternal> f,DataStream &outData);
		void BlockDecompress(std::shared_ptr<VFilePtrInternal> f,DataStream &outData);
		void DecompressLZ4(std::shared_ptr<VFilePtrInternal> f,DataStream &outData);
		std::pair<KVType,KVFlag> ReadType(DataStream &ds);
		std::shared_ptr<KVObject> ReadBinaryValue(const std::string &name, KVType datatype, KVFlag flagInfo, DataStream ds, KVObject *optParent);
		std::shared_ptr<KVObject> ParseBinaryKV3(DataStream &ds,KVObject *optParent,bool inArray=false);
		int64_t m_currentBinaryBytesOffset = -1;
		int64_t m_currentEightBytesOffset = -1;
		int64_t m_currentTypeIndex = 0;
		std::vector<std::string> m_stringArray = {};
		std::vector<uint8_t> m_typesArray = {};
		bool m_hasTypesArray = false;
		std::shared_ptr<KVObject> m_data = nullptr;
	};
};

#endif
