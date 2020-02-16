/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "source2/resource_data.hpp"
#include <fsys/filesystem.h>
#include <sharedutils/util.h>
#include <sharedutils/datastream.h>
#include <lz4.h>

using namespace source2;

#pragma optimize("",off)
BlockType resource::ResourceData::GetType() const {return BlockType::DATA;}
void resource::ResourceData::Read(std::shared_ptr<VFilePtrInternal> f) {}

const std::vector<resource::Panorama::NameEntry> &resource::Panorama::GetNames() const {return m_names;}
void resource::Panorama::Read(std::shared_ptr<VFilePtrInternal> f)
{
	f->Seek(GetOffset());
	m_crc32 = f->Read<uint32_t>();
	auto size = f->Read<uint16_t>();
	m_names.reserve(size);
	for(auto i=decltype(size){0u};i<size;++i)
	{
		m_names.push_back({});
		auto &nameEntry = m_names.back();
		nameEntry.name = f->ReadString(); // TODO: UTF8
		nameEntry.unknown1 = f->Read<uint32_t>();
		nameEntry.unknown2 = f->Read<uint32_t>();
	}

	auto headerSize = f->Tell() -GetOffset();
	auto numDataBytes = size -headerSize;
	m_data.resize(numDataBytes);
	f->Read(m_data.data(),m_data.size() *sizeof(m_data.front()));
	// TODO
	//if(m_crc32.Compute(data) != CRC32)
	{
		//throw new InvalidDataException("CRC32 mismatch for read data.");
	}
}

uint16_t resource::Texture::GetWidth() const {return (m_nonPow2Width > 0u) ? m_nonPow2Width : m_width;}
uint16_t resource::Texture::GetHeight() const {return (m_nonPow2Height > 0u) ? m_nonPow2Height : m_height;}
uint16_t resource::Texture::GetDepth() const {return m_depth;}
Vector4 resource::Texture::GetReflectivity() const {return m_reflectivity;}
VTexFlags resource::Texture::GetFlags() const {return m_flags;}
VTexFormat resource::Texture::GetFormat() const {return m_format;}
uint8_t resource::Texture::GetMipMapCount() const {return m_mipMapCount;}
uint32_t resource::Texture::GetPicmip0Res() const {return m_picmip0Res;}
const std::unordered_map<VTexExtraData,std::vector<uint8_t>> &resource::Texture::GetExtraData() const {return m_extraData;}
void resource::Texture::Read(std::shared_ptr<VFilePtrInternal> f)
{
	m_file = f;
	f->Seek(GetOffset());
	auto version = f->Read<uint16_t>();
	if(version != 1)
		throw std::runtime_error{"Unknown vtex version. ({" +std::to_string(version) +"} != expected 1)"};
	m_flags = f->Read<VTexFlags>();
	m_reflectivity = f->Read<Vector4>();
	m_width = f->Read<uint16_t>();
	m_height = f->Read<uint16_t>();
	m_depth = f->Read<uint16_t>();
	m_format = f->Read<VTexFormat>();
	m_mipMapCount = f->Read<uint8_t>();
	m_picmip0Res = f->Read<uint32_t>();

	auto extraDataOffset = f->Read<uint32_t>();
	auto extraDataCount = f->Read<uint32_t>();
	if(extraDataCount > 0u)
	{
		f->Seek(f->Tell() +extraDataOffset -sizeof(uint32_t) *2);
		for(auto i=decltype(extraDataCount){0u};i<extraDataCount;++i)
		{
			auto type = f->Read<VTexExtraData>();
			auto offset = f->Read<uint32_t>() -sizeof(uint32_t) *2;
			auto size = f->Read<uint32_t>();
			auto prevOffset = f->Tell();
			f->Seek(f->Tell() +offset);

			if(type == VTexExtraData::FILL_TO_POWER_OF_TWO)
			{
				f->Seek(f->Tell() +sizeof(uint16_t));
				auto nw = f->Read<uint16_t>();
				auto nh = f->Read<uint16_t>();
				if (nw > 0 && nh > 0 && m_width >= nw && m_height >= nh)
				{
					m_nonPow2Width = nw;
					m_nonPow2Height = nh;
				}
				f->Seek(f->Tell() -6);
			}

			auto it = m_extraData.insert(std::make_pair(type,std::vector<uint8_t>{})).first;
			auto &data = it->second;
			data.resize(size);
			f->Read(data.data(),data.size() *sizeof(data.front()));

			if (type == VTexExtraData::COMPRESSED_MIP_SIZE)
			{
				f->Seek(f->Tell() -size);

				auto int1 = f->Read<uint32_t>(); // 1?
				auto int2 = f->Read<uint32_t>(); // 8?
				auto mips = f->Read<uint32_t>();

				if (int1 != 1 && int1 != 0)
					throw std::runtime_error("int1 got: {int1}");

				if (int2 != 8)
					throw std::runtime_error("int2 expected 8 but got: {int2}");

				m_isCompressed = (int1 == 1); // TODO: Verify whether this int is the one that actually controls compression

				m_compressedMips.resize(mips);
				for(auto mip=decltype(mips){0u};mip<mips;++mip)
					m_compressedMips.at(mip) = f->Read<int32_t>();
			}
			f->Seek(prevOffset);
		}
	}
	m_dataOffset = GetOffset() +GetSize();
}

uint32_t resource::Texture::GetBlockSize()
{
	switch (m_format)
	{
	case VTexFormat::DXT1: return 8;
	case VTexFormat::DXT5: return 16;
	case VTexFormat::RGBA8888: return 4;
	case VTexFormat::R16: return 2;
	case VTexFormat::RG1616: return 4;
	case VTexFormat::RGBA16161616: return 8;
	case VTexFormat::R16F: return 2;
	case VTexFormat::RG1616F: return 4;
	case VTexFormat::RGBA16161616F: return 8;
	case VTexFormat::R32F: return 4;
	case VTexFormat::RG3232F: return 8;
	case VTexFormat::RGB323232F: return 12;
	case VTexFormat::RGBA32323232F: return 16;
	case VTexFormat::IA88: return 2;
	case VTexFormat::ETC2: return 8;
	case VTexFormat::ETC2_EAC: return 16;
	case VTexFormat::BGRA8888: return 4;
	case VTexFormat::ATI1N: return 8;
	}

	return 1;
}

uint64_t resource::Texture::GetMipmapDataOffset(uint8_t mipmap)
{
	auto offset = m_dataOffset;

	auto numMipmLevels = GetMipMapCount();
	if(numMipmLevels == 1)
		return offset;

	for (auto j = numMipmLevels - 1; j > mipmap; j--)
	{
		if (m_isCompressed)
		{
			offset += m_compressedMips[j];
		}
		else
		{
			offset += CalculateBufferSizeForMipLevel(j);
		}
	}
	return offset;
}

void resource::Texture::ReadTextureData(uint8_t mipLevel,std::vector<uint8_t> &outData)
{
	auto size = m_isCompressed ? m_compressedMips.at(mipLevel) : CalculateBufferSizeForMipLevel(mipLevel);

	outData.resize(size);
	m_file->Seek(GetMipmapDataOffset(mipLevel));
	m_file->Read(outData.data(),outData.size() *sizeof(outData.front()));
}

uint32_t resource::Texture::CalculateBufferSizeForMipLevel(uint8_t mipLevel)
{
	auto bytesPerPixel = GetBlockSize();
	auto width = m_width >> mipLevel;
	auto height = m_height >> mipLevel;
	auto depth = m_depth >> mipLevel;

	if (depth < 1)
		depth = 1;

	if (m_format == VTexFormat::DXT1
		|| m_format == VTexFormat::DXT5
		|| m_format == VTexFormat::ETC2
		|| m_format == VTexFormat::ETC2_EAC
		|| m_format == VTexFormat::ATI1N)
	{
		auto misalign = width % 4;

		if (misalign > 0)
		{
			width += 4 - misalign;
		}

		misalign = height % 4;

		if (misalign > 0)
		{
			height += 4 - misalign;
		}

		if (width < 4 && width > 0)
		{
			width = 4;
		}

		if (height < 4 && height > 0)
		{
			height = 4;
		}

		if (depth < 4 && depth > 1)
		{
			depth = 4;
		}

		auto numBlocks = (width * height) >> 4;
		numBlocks *= depth;
		return numBlocks * bytesPerPixel;
	}

	return width * height * depth * bytesPerPixel;
}

////////////////

resource::KVValue::KVValue(KVType type,const std::shared_ptr<void> &value,KVFlag flags)
	: m_object{value},m_flags{flags}
{}
void *resource::KVValue::GetObject() {return m_object.get();}
const void *resource::KVValue::GetObject() const {return const_cast<KVValue*>(this)->GetObject();}
resource::KVFlag resource::KVValue::GetFlags() const {return m_flags;}

////////////////

nullptr_t resource::KVValueNull::GetValue() const {return nullptr;}
resource::KVType resource::KVValueNull::GetType() const {return KVType::Null;}

bool resource::KVValueBool::GetValue() const {return *static_cast<bool*>(m_object.get());}
resource::KVType resource::KVValueBool::GetType() const {return KVType::BOOLEAN;}

int64_t resource::KVValueInt64::GetValue() const {return *static_cast<int64_t*>(m_object.get());}
resource::KVType resource::KVValueInt64::GetType() const {return KVType::INT64;}

uint64_t resource::KVValueUInt64::GetValue() const {return *static_cast<uint64_t*>(m_object.get());}
resource::KVType resource::KVValueUInt64::GetType() const {return KVType::UINT64;}

int32_t resource::KVValueInt32::GetValue() const {return *static_cast<int32_t*>(m_object.get());}
resource::KVType resource::KVValueInt32::GetType() const {return KVType::INT32;}

uint32_t resource::KVValueUInt32::GetValue() const {return *static_cast<uint32_t*>(m_object.get());}
resource::KVType resource::KVValueUInt32::GetType() const {return KVType::UINT32;}

double resource::KVValueDouble::GetValue() const {return *static_cast<double*>(m_object.get());}
resource::KVType resource::KVValueDouble::GetType() const {return KVType::DOUBLE;}

const std::string &resource::KVValueString::GetValue() const {return *static_cast<std::string*>(m_object.get());}
resource::KVType resource::KVValueString::GetType() const {return KVType::STRING;}

const resource::BinaryBlob &resource::KVValueBinaryBlob::GetValue() const {return *static_cast<BinaryBlob*>(m_object.get());}
resource::KVType resource::KVValueBinaryBlob::GetType() const {return KVType::BINARY_BLOB;}

const resource::KVObject &resource::KVValueArray::GetValue() const {return *static_cast<KVObject*>(m_object.get());}
resource::KVType resource::KVValueArray::GetType() const {return KVType::ARRAY;}

const resource::KVObject &resource::KVValueArrayTyped::GetValue() const {return *static_cast<KVObject*>(m_object.get());}
resource::KVType resource::KVValueArrayTyped::GetType() const {return KVType::ARRAY_TYPED;}

const resource::KVObject &resource::KVValueObject::GetValue() const {return *static_cast<KVObject*>(m_object.get());}
resource::KVType resource::KVValueObject::GetType() const {return KVType::OBJECT;}

////////////////

resource::KVObject::KVObject(const std::string &name,bool isArray)
	: m_key{name},m_isArray{isArray}
{}
void resource::KVObject::AddProperty(const std::string &name,const std::shared_ptr<KVValue> &value)
{
	if(m_isArray)
		m_values.insert(std::make_pair(std::to_string(m_count),value));
	else
		m_values.insert(std::make_pair(name,value));
	++m_count;
}
const std::unordered_map<std::string,std::shared_ptr<resource::KVValue>> &resource::KVObject::GetValues() const {return m_values;}

resource::KVValue *resource::KVObject::FindValue(const std::string &key)
{
	auto it = m_values.find(key);
	return (it != m_values.end()) ? it->second.get() : nullptr;
}
const resource::KVValue *resource::KVObject::FindValue(const std::string &key) const {return const_cast<KVObject*>(this)->FindValue(key);}

bool resource::KVObject::IsArray() const {return m_isArray;}
uint32_t resource::KVObject::GetArrayCount() const {return m_count;}
resource::KVValue *resource::KVObject::GetArrayValue(uint32_t idx,std::optional<KVType> confirmType)
{
	if(IsArray() == false)
		return nullptr;
	auto *val = FindValue(std::to_string(idx));
	return (val && (confirmType.has_value() == false || val->GetType() == *confirmType)) ? val : nullptr;
}
const resource::KVValue *resource::KVObject::GetArrayValue(uint32_t idx,std::optional<KVType> confirmType) const {return const_cast<KVObject*>(this)->GetArrayValue(idx,confirmType);}

////////////////

const util::GUID source2::resource::BinaryKV3::KV3_ENCODING_BINARY_BLOCK_COMPRESSED = { 0x46, 0x1A, 0x79, 0x95, 0xBC, 0x95, 0x6C, 0x4F, 0xA7, 0x0B, 0x05, 0xBC, 0xA1, 0xB7, 0xDF, 0xD2 };
const util::GUID source2::resource::BinaryKV3::KV3_ENCODING_BINARY_UNCOMPRESSED = { 0x00, 0x05, 0x86, 0x1B, 0xD8, 0xF7, 0xC1, 0x40, 0xAD, 0x82, 0x75, 0xA4, 0x82, 0x67, 0xE7, 0x14 };
const util::GUID source2::resource::BinaryKV3::KV3_ENCODING_BINARY_BLOCK_LZ4 = { 0x8A, 0x34, 0x47, 0x68, 0xA1, 0x63, 0x5C, 0x4F, 0xA1, 0x97, 0x53, 0x80, 0x6F, 0xD9, 0xB1, 0x19 };
const util::GUID source2::resource::BinaryKV3::KV3_FORMAT_GENERIC = { 0x7C, 0x16, 0x12, 0x74, 0xE9, 0x06, 0x98, 0x46, 0xAF, 0xF2, 0xE6, 0x3E, 0xB5, 0x90, 0x37, 0xE7 };
const std::array<uint8_t,16> source2::resource::BinaryKV3::ENCODING = { 0x46, 0x1A, 0x79, 0x95, 0xBC, 0x95, 0x6C, 0x4F, 0xA7, 0x0B, 0x05, 0xBC, 0xA1, 0xB7, 0xDF, 0xD2 };
const std::array<uint8_t,16> source2::resource::BinaryKV3::FORMAT = { 0x7C, 0x16, 0x12, 0x74, 0xE9, 0x06, 0x98, 0x46, 0xAF, 0xF2, 0xE6, 0x3E, 0xB5, 0x90, 0x37, 0xE7 };
const std::array<uint8_t,4> source2::resource::BinaryKV3::SIG = { 0x56, 0x4B, 0x56, 0x03 }; // VKV3 (3 isn't ascii, its 0x03)
const std::vector<std::string> &resource::BinaryKV3::GetStringArray() const {return m_stringArray;}
const std::shared_ptr<resource::KVObject> &resource::BinaryKV3::GetData() const {return m_data;}
void resource::BinaryKV3::Read(std::shared_ptr<VFilePtrInternal> f)
{
	f->Seek(GetOffset());

	DataStream ds {};
	auto magic = f->Read<uint32_t>();
	if(magic == MAGIC2)
	{
		ReadVersion2(f,ds);
		return;
	}

	if (magic != MAGIC)
		throw std::runtime_error{"Invalid KV3 signature " +std::to_string(magic)};

	auto encoding = f->Read<util::GUID>();
	auto format = f->Read<util::GUID>();

	// Valve's implementation lives in LoadKV3Binary()
	// KV3_ENCODING_BINARY_BLOCK_COMPRESSED calls CBlockCompress::FastDecompress()
	// and then it proceeds to call LoadKV3BinaryUncompressed, which should be the same routine for KV3_ENCODING_BINARY_UNCOMPRESSED
	// Old binary with debug symbols for ref: https://users.alliedmods.net/~asherkin/public/bins/dota_symbols/bin/osx64/libmeshsystem.dylib

	if(util::compare_guid(encoding,KV3_ENCODING_BINARY_BLOCK_COMPRESSED))
		BlockDecompress(f,ds);
	else if (util::compare_guid(encoding,KV3_ENCODING_BINARY_BLOCK_LZ4))
		DecompressLZ4(f,ds);
	else if (util::compare_guid(encoding,KV3_ENCODING_BINARY_UNCOMPRESSED))
	{
		auto szCpy = f->GetSize() -f->Tell();
		ds->Resize(szCpy);
		f->Read(ds->GetData(),szCpy);
		ds->SetOffset(0);
	}
	else
		throw new std::runtime_error{"Unrecognised KV3 Encoding: " +util::guid_to_string(encoding)};

	auto numStrings = ds->Read<uint32_t>();
	m_stringArray.reserve(numStrings);
	for(auto i=decltype(numStrings){0u};i<numStrings;++i)
		m_stringArray.push_back(ds->ReadString()); // TODO: UTF8
	m_data = ParseBinaryKV3(ds,nullptr,true);
}
void resource::BinaryKV3::ReadVersion2(std::shared_ptr<VFilePtrInternal> f,DataStream &outData)
{
	auto format = f->Read<util::GUID>();

	auto compressionMethod = f->Read<int32_t>();
	auto countOfBinaryBytes = f->Read<int32_t>(); // how many bytes (binary blobs)
	auto countOfIntegers = f->Read<int32_t>(); // how many 4 byte values (ints)
	auto countOfEightByteValues = f->Read<int32_t>(); // how many 8 byte values (doubles)

	if (compressionMethod == 0)
	{
		auto length = f->Read<int32_t>();

		outData->Resize(length);
		f->Read(outData->GetData(),length);
	}
	else if (compressionMethod == 1)
		DecompressLZ4(f,outData);
	else
		throw std::runtime_error{"Unknown KV3 compression method: " +std::to_string(compressionMethod)};

	m_currentBinaryBytesOffset = 0;
	outData->SetOffset(countOfBinaryBytes);

	if (outData->GetOffset() % 4 != 0)
	{
		// Align to % 4 after binary blobs
		outData->SetOffset(outData->GetOffset() +4 - (outData->GetOffset() % 4));
	}

	auto countOfStrings = outData->Read<int32_t>();
	auto kvDataOffset = outData->GetOffset();

	// Subtract one integer since we already read it (countOfStrings)
	outData->SetOffset(outData->GetOffset() +(countOfIntegers - 1) * 4);

	if (outData->GetOffset() % 8 != 0)
	{
		// Align to % 8 for the start of doubles
		outData->SetOffset(outData->GetOffset() +8 - (outData->GetOffset() % 8));
	}

	m_currentEightBytesOffset = outData->GetOffset();

	outData->SetOffset(outData->GetOffset() +countOfEightByteValues * 8);

	m_stringArray.resize(countOfStrings);

	for(auto i=decltype(countOfStrings){0u};i<countOfStrings;++i)
		m_stringArray.at(i) = outData->ReadString(); // UTF8

	// bytes after the string table is kv types, minus 4 static bytes at the end
	auto typesLength = outData->GetInternalSize() - 4 - outData->GetOffset();
	m_typesArray.resize(typesLength);
	m_hasTypesArray = true;
	outData->Read(m_typesArray.data(),m_typesArray.size());

	// Move back to the start of the KV data for reading.
	outData->SetOffset(kvDataOffset);

	m_data = ParseBinaryKV3(outData, nullptr, true);
}
void resource::BinaryKV3::BlockDecompress(std::shared_ptr<VFilePtrInternal> f,DataStream &outData)
{
	// It is flags, right?
	auto flags = f->Read<std::array<uint8_t,4>>();

	// outWrite.Write(flags);
	if ((flags[3] & 0x80) > 0)
	{
		auto size = f->GetSize() -f->Tell();
		outData->Resize(size);
		f->Read(outData->GetData(),size);
	}
	else
	{
		outData->Reserve(f->GetSize() -f->Tell());
		auto running = true;
		while (f->Tell() != f->GetSize() && running)
		{
			//try
			//{
				auto blockMask = f->Read<uint16_t>();
				for(auto i=decltype(blockMask){0u};i<(sizeof(blockMask) *8);++i)
				{
					// is the ith bit 1
					if ((blockMask & (1 << i)) > 0)
					{
						auto offsetSize = f->Read<uint16_t>();
						auto offset = ((offsetSize & 0xFFF0) >> 4) + 1;
						auto size = (offsetSize & 0x000F) + 3;

						auto lookupSize = (offset < size) ? offset : size; // If the offset is larger or equal to the size, use the size instead.

						// Kill me now
						auto p = outData->GetOffset();
						outData->SetOffset(p -offset);
						std::vector<uint8_t> data {};
						data.resize(lookupSize);
						outData->Read(data.data(),data.size() *sizeof(data.front()));
						outData->SetOffset(p);

						while (size > 0)
						{
							outData->Write(data.data(),(lookupSize < size) ? lookupSize : size);
							size -= lookupSize;
						}
					}
					else
					{
						auto data = f->Read<uint8_t>();
						outData->Write(data);
					}

					//TODO: is there a better way of making an unsigned 12bit number?
					// TODO
					if (outData->GetInternalSize() == (flags[2] << 16) + (flags[1] << 8) + flags[0])
					{
						running = false;
						break;
					}
				}
			//}
			//catch (EndOfStreamException)
			//{
			//	break;
			//}
		}
	}
	outData->SetOffset(0);
}

void resource::BinaryKV3::DecompressLZ4(std::shared_ptr<VFilePtrInternal> f,DataStream &outData)
{
	auto uncompressedSize = f->Read<uint32_t>();
	auto compressedSize = GetSize() - (f->Tell() - GetOffset());

	std::vector<uint8_t> input {};
	input.resize(compressedSize);
	f->Read(input.data(),input.size() *sizeof(input.front()));

	outData->Resize(uncompressedSize);
	auto result = LZ4_decompress_safe(reinterpret_cast<char*>(input.data()),reinterpret_cast<char*>(outData->GetData()),input.size(),uncompressedSize);
	if(result < 0)
		throw std::runtime_error{"Unable to decompress LZ4 data: " +std::to_string(result)};

	outData->SetOffset(0);
}

std::pair<resource::KVType,resource::KVFlag> resource::BinaryKV3::ReadType(DataStream &ds)
{
	uint8_t databyte;
	if(m_hasTypesArray)
		databyte = m_typesArray.at(m_currentTypeIndex++);
	else
		databyte = ds->Read<uint8_t>();

	auto flagInfo = KVFlag::None;
	if ((databyte & 0x80) > 0)
	{
		databyte &= 0x7F; // Remove the flag bit

		if (m_hasTypesArray)
			flagInfo = static_cast<KVFlag>(m_typesArray.at(m_currentTypeIndex++));
		else
			flagInfo = ds->Read<KVFlag>();
	}
	return {static_cast<resource::KVType>(databyte),flagInfo};
}

resource::KVType resource::BinaryKV3::ConvertBinaryOnlyKVType(KVType type)
{
	switch (type)
	{
	case KVType::BOOLEAN:
	case KVType::BOOLEAN_TRUE:
	case KVType::BOOLEAN_FALSE:
		return KVType::BOOLEAN;
	case KVType::INT64:
	case KVType::INT32:
	case KVType::INT64_ZERO:
	case KVType::INT64_ONE:
		return KVType::INT64;
	case KVType::UINT64:
	case KVType::UINT32:
		return KVType::UINT64;
	case KVType::DOUBLE:
	case KVType::DOUBLE_ZERO:
	case KVType::DOUBLE_ONE:
		return KVType::DOUBLE;
	case KVType::ARRAY_TYPED:
		return KVType::ARRAY;
	}

	return type;
}

template<class TKVValue,typename T>
	std::shared_ptr<resource::KVValue> resource::BinaryKV3::MakeValue(KVType type, T data, KVFlag flag)
{
	return MakeValueFromPtr<TKVValue>(type,std::make_shared<T>(data),flag);
}

template<class TKVValue>
	std::shared_ptr<resource::KVValue> resource::BinaryKV3::MakeValueFromPtr(KVType type, const std::shared_ptr<void> &data, KVFlag flag)
{
	auto realType = ConvertBinaryOnlyKVType(type);

	if (flag != KVFlag::None)
		return std::make_shared<TKVValue>(realType, data, flag);

	return std::make_shared<TKVValue>(realType, data);
}

std::shared_ptr<resource::KVObject> resource::BinaryKV3::ReadBinaryValue(const std::string &name, KVType datatype, KVFlag flagInfo, DataStream ds, KVObject *parent)
{
	auto currentOffset = ds->GetOffset();

	std::shared_ptr<void> value = nullptr; // Only needed so the object doesn't get destroyed in the switch-case scope
	switch (datatype)
	{
	case KVType::Null:
		parent->AddProperty(name, MakeValueFromPtr<KVValueNull>(datatype, nullptr, flagInfo));
		break;
	case KVType::BOOLEAN:
		if (m_currentBinaryBytesOffset > -1)
			ds->SetOffset(m_currentBinaryBytesOffset);

		parent->AddProperty(name, MakeValue<KVValueBool,bool>(datatype, ds->Read<bool>(), flagInfo));

		if (m_currentBinaryBytesOffset > -1)
		{
			m_currentBinaryBytesOffset++;
			ds->SetOffset(currentOffset);
		}

		break;
	case KVType::BOOLEAN_TRUE:
		parent->AddProperty(name, MakeValue<KVValueBool,bool>(datatype, true, flagInfo));
		break;
	case KVType::BOOLEAN_FALSE:
		parent->AddProperty(name, MakeValue<KVValueBool,bool>(datatype, false, flagInfo));
		break;
	case KVType::INT64_ZERO:
		parent->AddProperty(name, MakeValue<KVValueInt64,int64_t>(datatype, 0ll, flagInfo));
		break;
	case KVType::INT64_ONE:
		parent->AddProperty(name, MakeValue<KVValueInt64,int64_t>(datatype, 1ull, flagInfo));
		break;
	case KVType::INT64:
		if (m_currentEightBytesOffset > 0)
			ds->SetOffset(m_currentEightBytesOffset);

		parent->AddProperty(name, MakeValue<KVValueInt64,int64_t>(datatype, ds->Read<int64_t>(), flagInfo));

		if (m_currentEightBytesOffset > 0)
		{
			m_currentEightBytesOffset = ds->GetOffset();
			ds->SetOffset(currentOffset);
		}

		break;
	case KVType::UINT64:
		if (m_currentEightBytesOffset > 0)
			ds->SetOffset(m_currentEightBytesOffset);

		parent->AddProperty(name, MakeValue<KVValueUInt64,uint64_t>(datatype, ds->Read<uint64_t>(), flagInfo));

		if (m_currentEightBytesOffset > 0)
		{
			m_currentEightBytesOffset = ds->GetOffset();
			ds->SetOffset(currentOffset);
		}

		break;
	case KVType::INT32:
		parent->AddProperty(name, MakeValue<KVValueInt32,int32_t>(datatype, ds->Read<int32_t>(), flagInfo));
		break;
	case KVType::UINT32:
		parent->AddProperty(name, MakeValue<KVValueUInt32,uint32_t>(datatype, ds->Read<uint32_t>(), flagInfo));
		break;
	case KVType::DOUBLE:
		if (m_currentEightBytesOffset > 0)
			ds->SetOffset(m_currentEightBytesOffset);

		parent->AddProperty(name, MakeValue<KVValueDouble,double>(datatype, ds->Read<double>(), flagInfo));

		if (m_currentEightBytesOffset > 0)
		{
			m_currentEightBytesOffset = ds->GetOffset();
			ds->SetOffset(currentOffset);
		}

		break;
	case KVType::DOUBLE_ZERO:
		parent->AddProperty(name, MakeValue<KVValueDouble,double>(datatype, 0.0, flagInfo));
		break;
	case KVType::DOUBLE_ONE:
		parent->AddProperty(name, MakeValue<KVValueDouble,double>(datatype, 1.0, flagInfo));
		break;
	case KVType::STRING:
	{
		auto id = ds->Read<int32_t>();
		parent->AddProperty(name, MakeValue<KVValueString,std::string>(datatype, id == -1 ? std::string{} : m_stringArray.at(id), flagInfo));
		break;
	}
	case KVType::BINARY_BLOB:
	{
		auto length = ds->Read<int32_t>();

		if (m_currentBinaryBytesOffset > -1)
			ds->SetOffset(m_currentBinaryBytesOffset);

		auto data = std::make_shared<BinaryBlob>();
		data->resize(length);
		ds->Read(data->data(),length);
		value = data;
		parent->AddProperty(name, MakeValueFromPtr<KVValueBinaryBlob>(datatype, data, flagInfo));

		if (m_currentBinaryBytesOffset > -1)
		{
			m_currentBinaryBytesOffset = ds->GetOffset();
			ds->SetOffset(currentOffset +4);
		}

		break;
	}
	case KVType::ARRAY:
	{
		auto arrayLength = ds->Read<int32_t>();
		auto array = std::make_shared<KVObject>(name, true);
		for(auto i=decltype(arrayLength){0u};i<arrayLength;++i)
			ParseBinaryKV3(ds, array.get(), true);

		value = array;
		parent->AddProperty(name, MakeValueFromPtr<KVValueArray>(datatype, array, flagInfo));
		break;
	}
	case KVType::ARRAY_TYPED:
	{
		auto typeArrayLength = ds->Read<int32_t>();
		auto type = ReadType(ds);
		auto typedArray = std::make_shared<KVObject>(name, true);

		for(auto i=decltype(typeArrayLength){0u};i<typeArrayLength;++i)
			ReadBinaryValue(name, type.first, type.second, ds, typedArray.get());

		value = typedArray;
		parent->AddProperty(name, MakeValueFromPtr<KVValueArrayTyped>(datatype, typedArray, flagInfo));
		break;
	}
	case KVType::OBJECT:
	{
		auto objectLength = ds->Read<int32_t>();
		auto newObject = std::make_shared<KVObject>(name, false);
		for(auto i=decltype(objectLength){0u};i<objectLength;++i)
			ParseBinaryKV3(ds, newObject.get(), false);

		value = newObject;
		if (parent == nullptr)
			parent = newObject.get();
		else
			parent->AddProperty(name, MakeValueFromPtr<KVValueObject>(datatype, newObject, flagInfo));

		break;
	}
	default:
		throw new std::runtime_error{"Unknown KVType " +std::to_string(umath::to_integral(datatype)) +" for field '" +name +"' on byte " +std::to_string(ds->GetOffset() - 1)};
	}

	return parent ? parent->shared_from_this() : nullptr;
}

std::shared_ptr<resource::KVObject> resource::BinaryKV3::ParseBinaryKV3(DataStream &ds,KVObject *parent,bool inArray)
{
	std::string name {};
	if (!inArray)
	{
		auto stringID = ds->Read<int32_t>();
		name = (stringID == -1) ? "" : m_stringArray.at(stringID);
	}

	auto type = ReadType(ds);
	return ReadBinaryValue(name, type.first, type.second, ds, parent);
}
#pragma optimize("",on)
