// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-FileCopyrightText: (c) 2015 Steam Database
// SPDX-License-Identifier: MIT

module;

#include <algorithm>

#include <cstring>

#include <lz4.h>
#include <string>
#include <memory>
#include <stdexcept>
#include <cmath>
#include <vector>
#include <unordered_map>
#ifdef _WIN32
#include <Windows.h>
#endif

module source2;

import :impl;

using namespace source2;

#ifdef __linux__
using byte = uint8_t;
#endif

#undef GetObject
#undef max

std::string resource::to_string(KVType type)
{
	switch(type) {
	case KVType::Invalid:
		return "Invalid";
	case KVType::STRING_MULTI:
		return "STRING_MULTI";
	case KVType::Null:
		return "Null";
	case KVType::BOOLEAN:
		return "BOOLEAN";
	case KVType::INT64:
		return "INT64";
	case KVType::UINT64:
		return "UINT64";
	case KVType::DOUBLE:
		return "DOUBLE";
	case KVType::STRING:
		return "STRING";
	case KVType::BINARY_BLOB:
		return "BINARY_BLOB";
	case KVType::ARRAY:
		return "ARRAY";
	case KVType::OBJECT:
		return "OBJECT";
	case KVType::ARRAY_TYPED:
		return "ARRAY_TYPED";
	case KVType::INT32:
		return "INT32";
	case KVType::UINT32:
		return "UINT32";
	case KVType::BOOLEAN_TRUE:
		return "BOOLEAN_TRUE";
	case KVType::BOOLEAN_FALSE:
		return "BOOLEAN_FALSE";
	case KVType::INT64_ZERO:
		return "INT64_ZERO";
	case KVType::INT64_ONE:
		return "INT64_ONE";
	case KVType::DOUBLE_ZERO:
		return "DOUBLE_ZERO";
	case KVType::DOUBLE_ONE:
		return "DOUBLE_ONE";
	}
	return "Unknown";
}

///////////////

resource::IKeyValueCollection *resource::IKeyValueCollection::FindSubCollection(const std::string &key)
{
	auto oCollection = IKeyValueCollection::FindValue<IKeyValueCollection *>(*this, key);
	return oCollection.has_value() ? *oCollection : nullptr;
}
resource::BinaryBlob *resource::IKeyValueCollection::FindBinaryBlob(const std::string &key)
{
	if(typeid(*this) == typeid(NTROStruct))
		return static_cast<NTROStruct &>(*this).FindBinaryBlob(key);
	else if(typeid(*this) == typeid(KVObject))
		return static_cast<KVObject &>(*this).FindBinaryBlob(key);
	return {};
}

///////////////

BlockType resource::ResourceData::GetType() const { return BlockType::DATA; }
resource::IKeyValueCollection *resource::ResourceData::GetData()
{
	auto *ntro = dynamic_cast<NTRO *>(this);
	if(ntro)
		return ntro->GetOutput().get();
	auto *binaryKv3 = dynamic_cast<BinaryKV3 *>(this);
	if(binaryKv3)
		return binaryKv3->GetData().get();
	auto *keyValuesOrNTRO = dynamic_cast<KeyValuesOrNTRO *>(this);
	return keyValuesOrNTRO ? keyValuesOrNTRO->GetData().get() : nullptr;
}
void resource::ResourceData::Read(const Resource &resource, ufile::IFile &f) {}
void resource::ResourceData::DebugPrint(std::stringstream &ss, const std::string &t) const
{
	ss << t << "ResourceData = {\n";
	ss << t << "}\n";
}

///////////////

resource::NTROValue::NTROValue(DataType type, bool pointer) : type {type}, pointer {pointer} {}

///////////////

resource::NTROArray::NTROArray(DataType type, uint32_t count, bool pointer, bool isIndirection) : NTROValue {type, pointer}, m_bIsIndirection {isIndirection} { m_contents.resize(count); }
const std::vector<std::shared_ptr<resource::NTROValue>> &resource::NTROArray::GetContents() const { return const_cast<NTROArray *>(this)->GetContents(); }
std::vector<std::shared_ptr<resource::NTROValue>> &resource::NTROArray::GetContents() { return m_contents; }
bool resource::NTROArray::IsIndirection() const { return m_bIsIndirection; }
resource::BinaryBlob &resource::NTROArray::InitBinaryBlob()
{
	if(type != DataType::Byte || m_binaryBlob.empty() == false)
		return m_binaryBlob;
	auto &contents = GetContents();
	m_binaryBlob.reserve(contents.size());
	for(auto &v : contents) {
		uint8_t value = 0;
		if(v->type == DataType::Byte)
			value = static_cast<TNTROValue<unsigned char> &>(*v).value;
		m_binaryBlob.push_back(value);
	}
	return m_binaryBlob;
}
void resource::NTROArray::DebugPrint(std::stringstream &ss, const std::string &t) const
{
	ss << t << "NTROArray = {\n";
	ss << t << "\tIs indirection = " << m_bIsIndirection << "\n";
	ss << t << "\tContents:\n";
	for(auto i = decltype(m_contents.size()) {0u}; i < m_contents.size(); ++i) {
		auto &contents = m_contents.at(i);
		ss << t << "\t\t[" << std::to_string(i) << "] = {\n";
		contents->DebugPrint(ss, t + "\t\t\t");
		ss << t << "\t\t}\n";
	}
	ss << t << "}\n";
}

///////////////

resource::NTROStruct::NTROStruct(const std::string &name) : m_name {name} {}
resource::NTROStruct::NTROStruct(const std::vector<std::shared_ptr<NTROValue>> &values)
{
	m_contents.reserve(values.size());
	for(auto i = decltype(values.size()) {0u}; i < values.size(); ++i) {
		auto &v = values.at(i);
		Add(std::to_string(i), *v);
	}
}
const std::unordered_map<std::string, std::shared_ptr<resource::NTROValue>> &resource::NTROStruct::GetContents() const { return m_contents; }
resource::BinaryBlob *resource::NTROStruct::FindBinaryBlob(const std::string &key)
{
	auto *val = dynamic_cast<NTROArray *>(FindValue(key));
	if(val == nullptr || val->type != DataType::Byte)
		return nullptr;
	return &val->InitBinaryBlob();
}
resource::NTROValue *resource::NTROStruct::FindValue(const std::string &key)
{
	auto it = m_contents.find(key);
	return (it != m_contents.end()) ? it->second.get() : nullptr;
}
const resource::NTROValue *resource::NTROStruct::FindValue(const std::string &key) const { return const_cast<NTROStruct *>(this)->FindValue(key); }
resource::NTROArray *resource::NTROStruct::FindArray(const std::string &key)
{
	auto o = FindValue<std::shared_ptr<NTROArray>>(key);
	return o.has_value() ? o->get() : nullptr;
}
const resource::NTROArray *resource::NTROStruct::FindArray(const std::string &key) const { return const_cast<NTROStruct *>(this)->FindArray(key); }
void resource::NTROStruct::Add(const std::string &id, NTROValue &val) { m_contents.insert(std::make_pair(id, val.shared_from_this())); }
void resource::NTROStruct::DebugPrint(std::stringstream &ss, const std::string &t) const
{
	ss << t << "NTROStruct = {\n";
	ss << t << "\tName = " << m_name << "\n";
	ss << t << "\tContents:\n";
	for(auto &pair : m_contents) {
		auto &contents = pair.second;
		ss << t << "\t\t[" << pair.first << "] = {\n";
		contents->DebugPrint(ss, t + "\t\t\t");
		ss << t << "\t\t}\n";
	}
	ss << t << "}\n";
}

///////////////

void resource::NTRO::Read(const Resource &resource, ufile::IFile &f)
{
	auto *block = static_cast<const ResourceIntrospectionManifest *>(resource.FindBlock(source2::BlockType::NTRO));
	if(block == nullptr)
		return;
	auto &structs = block->GetReferencedStructs();
	if(m_structName.empty() == false) {
		auto it = std::find_if(structs.begin(), structs.end(), [this](const source2::resource::ResourceIntrospectionManifest::ResourceDiskStruct &strct) { return strct.name == m_structName; });
		if(it == structs.end())
			return;
		m_output = ReadStructure(resource, *it, GetOffset(), f);
		return;
	}
	for(auto &refStruct : structs) {
		m_output = ReadStructure(resource, refStruct, GetOffset(), f);
		break;
	}
}
void resource::NTRO::ReadFieldIntrospection(const Resource &resource, const ResourceIntrospectionManifest::ResourceDiskStruct::Field &field, NTROStruct &structEntry, ufile::IFile &f)
{
	auto count = std::max<uint16_t>(field.count, static_cast<uint16_t>(1));
	auto pointer = false;
	uint64_t prevOffset = 0;
	if(field.indirections.empty() == false) {
		if(field.indirections.size() > 1)
			throw std::runtime_error {"More than one indirection, not yet handled."};
		if(field.count > 0)
			throw std::runtime_error {"Indirection.Count > 0 && field.Count > 0"};
		auto indirection = field.indirections.at(0); // TODO: depth needs fixing?
		auto offset = f.Read<uint32_t>();
		if(indirection == 0x03) {
			pointer = true;
			if(offset == 0) {
				auto value = std::make_shared<TNTROValue<uint8_t>>(field.type, 0u, true);
				structEntry.Add(field.fieldName, *value); //being byte shouldn't matter
				return;
			}
			prevOffset = f.Tell();
			f.Seek(f.Tell() + offset - sizeof(uint32_t));
		}
		else if(indirection == 0x04) {
			count = f.Read<uint32_t>();

			prevOffset = f.Tell();

			if(count > 0)
				f.Seek(f.Tell() + offset - sizeof(uint32_t) * 2);
		}
		else
			throw std::runtime_error {"Unknown indirection. (" + std::to_string(indirection) + ")"};
	}

	//if (pointer)
	//{
	//    Writer.Write("{0} {1}* = (ptr) ->", ValveDataType(field.Type), field.FieldName);
	//}
	if(field.count > 0 || field.indirections.size() > 0) {
		auto ntroValues = std::make_shared<NTROArray>(field.type, (int)count, pointer, field.indirections.empty() == false);

		for(auto i = decltype(count) {0u}; i < count; ++i)
			ntroValues->GetContents().at(i) = ReadField(resource, field, pointer, f);

		structEntry.Add(field.fieldName, *ntroValues);
	}
	else {
		for(auto i = decltype(count) {0u}; i < count; ++i) {
			auto pfield = ReadField(resource, field, pointer, f);
			structEntry.Add(field.fieldName, *pfield);
		}
	}
	if(prevOffset > 0)
		f.Seek(prevOffset);
}

static std::shared_ptr<resource::NTROValue> get_float_array_ntro_value(uint32_t numValues, resource::DataType type, bool pointer, ufile::IFile &f)
{
	std::vector<std::shared_ptr<resource::NTROValue>> values {};
	values.reserve(numValues);
	for(auto i = decltype(numValues) {0u}; i < numValues; ++i) {
		auto val = f.Read<float>();
		values.push_back(std::static_pointer_cast<resource::NTROValue>(std::make_shared<resource::TNTROValue<float>>(resource::DataType::Float, val, pointer)));
	}
	return std::static_pointer_cast<resource::NTROValue>(std::make_shared<resource::TNTROValue<resource::NTROStruct>>(type, resource::NTROStruct {values}, pointer));
}
std::shared_ptr<resource::NTROValue> resource::NTRO::ReadField(const Resource &resource, const ResourceIntrospectionManifest::ResourceDiskStruct::Field &field, bool pointer, ufile::IFile &f)
{
	switch(field.type) {
	case DataType::Struct:
		{
			auto *introspectionManifest = resource.GetIntrospectionManifest();
			auto &refStructs = introspectionManifest->GetReferencedStructs();
			auto it = std::find_if(refStructs.begin(), refStructs.end(), [&field](const ResourceIntrospectionManifest::ResourceDiskStruct &diskStruct) { return diskStruct.id == field.typeData; });
			auto &newStruct = *it;
			return std::static_pointer_cast<resource::NTROValue>(std::make_shared<TNTROValue<std::shared_ptr<NTROStruct>>>(field.type, ReadStructure(resource, newStruct, f.Tell(), f), pointer));
		}
	case DataType::Enum:
		// TODO: Lookup in ReferencedEnums
		return std::static_pointer_cast<resource::NTROValue>(std::make_shared<TNTROValue<uint32_t>>(field.type, f.Read<uint32_t>(), pointer));

	case DataType::SByte:
		return std::static_pointer_cast<resource::NTROValue>(std::make_shared<TNTROValue<int8_t>>(field.type, f.Read<int8_t>(), pointer));

	case DataType::Byte:
		return std::static_pointer_cast<resource::NTROValue>(std::make_shared<TNTROValue<uint8_t>>(field.type, f.Read<uint8_t>(), pointer));

	case DataType::Boolean:
		return std::static_pointer_cast<resource::NTROValue>(std::make_shared<TNTROValue<bool>>(field.type, f.Read<uint8_t>() == 1 ? true : false, pointer));

	case DataType::Int16:
		return std::static_pointer_cast<resource::NTROValue>(std::make_shared<TNTROValue<int16_t>>(field.type, f.Read<int16_t>(), pointer));

	case DataType::UInt16:
		return std::static_pointer_cast<resource::NTROValue>(std::make_shared<TNTROValue<uint16_t>>(field.type, f.Read<uint16_t>(), pointer));

	case DataType::Int32:
		return std::static_pointer_cast<resource::NTROValue>(std::make_shared<TNTROValue<int32_t>>(field.type, f.Read<int32_t>(), pointer));

	case DataType::UInt32:
		return std::static_pointer_cast<resource::NTROValue>(std::make_shared<TNTROValue<uint32_t>>(field.type, f.Read<uint32_t>(), pointer));

	case DataType::Float:
		return std::static_pointer_cast<resource::NTROValue>(std::make_shared<TNTROValue<float>>(field.type, f.Read<float>(), pointer));

	case DataType::Int64:
		return std::static_pointer_cast<resource::NTROValue>(std::make_shared<TNTROValue<int64_t>>(field.type, f.Read<int64_t>(), pointer));

	case DataType::ExternalReference:
		{
			auto id = f.Read<uint64_t>();
			auto *externalReferences = resource.GetExternalReferences();
			std::string value {};
			if(externalReferences) {
				auto &refInfos = externalReferences->GetResourceReferenceInfos();
				auto it = std::find_if(refInfos.begin(), refInfos.end(), [id](const ResourceExtRefList::ResourceReferenceInfo &refInfo) { return refInfo.id == id; });
				if(it != refInfos.end())
					value = it->name;
			}
			return std::static_pointer_cast<resource::NTROValue>(std::make_shared<TNTROValue<std::string>>(field.type, value, pointer));
		}
	case DataType::UInt64:
		return std::static_pointer_cast<resource::NTROValue>(std::make_shared<TNTROValue<uint64_t>>(field.type, f.Read<uint64_t>(), pointer));

	case DataType::Vector:
		return get_float_array_ntro_value(3, field.type, pointer, f);
	case DataType::Quaternion:
		return get_float_array_ntro_value(4, field.type, pointer, f);
	case DataType::Color:
	case DataType::Fltx4:
	case DataType::Vector4D:
	case DataType::Vector4D_44:
		return get_float_array_ntro_value(4, field.type, pointer, f);
	case DataType::String4:
	case DataType::String:
		return std::static_pointer_cast<resource::NTROValue>(std::make_shared<TNTROValue<std::string>>(field.type, read_offset_string(f), pointer));

	case DataType::Matrix2x4:
		return get_float_array_ntro_value(8, field.type, pointer, f);
	case DataType::Matrix3x4:
	case DataType::Matrix3x4a:
		return get_float_array_ntro_value(12, field.type, pointer, f);
	case DataType::CTransform:
		return get_float_array_ntro_value(8, field.type, pointer, f);
	default:
		throw std::runtime_error {"Unknown data type: " + to_string(field.type) + " (name: " + field.fieldName + ")"};
	}
	return nullptr;
}
std::shared_ptr<resource::NTROStruct> resource::NTRO::ReadStructure(const Resource &resource, const ResourceIntrospectionManifest::ResourceDiskStruct &refStruct, int64_t startingOffset, ufile::IFile &f)
{
	auto structEntry = std::make_shared<NTROStruct>(refStruct.name);
	for(auto &field : refStruct.fieldIntrospection) {
		f.Seek(startingOffset + field.diskOffset);
		ReadFieldIntrospection(resource, field, *structEntry, f);
	}

	// Some structs are padded, so all the field sizes do not add up to the size on disk
	f.Seek(startingOffset + refStruct.diskSize);

	if(refStruct.baseStructId != 0) {
		auto previousOffset = f.Tell();
		auto *introspectionManifest = resource.GetIntrospectionManifest();
		auto &refStructs = introspectionManifest->GetReferencedStructs();
		auto it = std::find_if(refStructs.begin(), refStructs.end(), [&refStruct](const ResourceIntrospectionManifest::ResourceDiskStruct &diskStruct) { return diskStruct.id == refStruct.baseStructId; });

		auto &newStruct = *it;
		// Valve doesn't print this struct's type, so we can't just call ReadStructure *sigh*
		for(auto &field : newStruct.fieldIntrospection) {
			f.Seek(startingOffset + field.diskOffset);
			ReadFieldIntrospection(resource, field, *structEntry, f);
		}
		f.Seek(previousOffset);
	}
	return structEntry;
}
void resource::NTRO::DebugPrint(std::stringstream &ss, const std::string &t) const
{
	ss << t << "NTRO = {\n";
	ss << t << "\tOutput:\n";
	if(m_output)
		m_output->DebugPrint(ss, t + "\t\t");
	ss << t << "}\n";
}
const std::shared_ptr<resource::NTROStruct> &resource::NTRO::GetOutput() const { return m_output; }
void resource::NTRO::SetStructName(const std::string &structName) { m_structName = structName; }

///////////////

const std::vector<resource::Panorama::NameEntry> &resource::Panorama::GetNames() const { return m_names; }
void resource::Panorama::Read(const Resource &resource, ufile::IFile &f)
{
	f.Seek(GetOffset());
	m_crc32 = f.Read<uint32_t>();
	auto size = f.Read<uint16_t>();
	m_names.reserve(size);
	for(auto i = decltype(size) {0u}; i < size; ++i) {
		m_names.push_back({});
		auto &nameEntry = m_names.back();
		nameEntry.name = f.ReadString(); // TODO: UTF8
		nameEntry.unknown1 = f.Read<uint32_t>();
		nameEntry.unknown2 = f.Read<uint32_t>();
	}

	auto headerSize = f.Tell() - GetOffset();
	auto numDataBytes = size - headerSize;
	m_data.resize(numDataBytes);
	f.Read(m_data.data(), m_data.size() * sizeof(m_data.front()));
	// TODO
	//if(m_crc32.Compute(data) != CRC32)
	{
		//throw new InvalidDataException("CRC32 mismatch for read data.");
	}
}
void resource::Panorama::DebugPrint(std::stringstream &ss, const std::string &t) const
{
	ss << t << "Panorama = {\n";
	ss << t << "\tNames:\n";
	for(auto i = decltype(m_names.size()) {0u}; i < m_names.size(); ++i) {
		auto &nameInfo = m_names.at(i);
		ss << t << "\t[" << i << "] = {\n";
		ss << t << "\t\tName = " << nameInfo.name << "\n";
		ss << t << "\t\tUnknown1 = " << nameInfo.unknown1 << "\n";
		ss << t << "\t\tUnknown2 = " << nameInfo.unknown2 << "\n";
		ss << t << "\t}\n";
	}
	ss << t << "}\n";
}

///////////////

resource::Sound::AudioFileType resource::Sound::GetSoundType() const { return m_soundType; }
uint32_t resource::Sound::GetSampleRate() const { return m_sampleRate; }
uint32_t resource::Sound::GetBits() const { return m_bits; }
uint32_t resource::Sound::GetChannels() const { return m_channels; }
uint32_t resource::Sound::GetAudioFormat() const { return m_audioFormat; }
uint32_t resource::Sound::GetSampleSize() const { return m_sampleSize; }
uint32_t resource::Sound::GetSampleCount() const { return m_sampleCount; }
int32_t resource::Sound::GetLoopStart() const { return m_loopStart; }
float resource::Sound::GetDuration() const { return m_duration; }
uint32_t resource::Sound::GetStreamingDataSize() const { return m_streamingDataSize; }
void resource::Sound::SetVersion4(ufile::IFile &f)
{
	auto type = f.Read<uint16_t>();

	// We don't know if it's actually calculated, or if its a lookup
	switch(type) {
	case 0x0101:
		m_soundType = AudioFileType::WAV;
		m_bits = 8;
		break;
	case 0x0100:
		m_soundType = AudioFileType::WAV;
		m_bits = 16;
		break;
	case 0x0200:
		m_soundType = AudioFileType::WAV;
		m_bits = 32;
		break;
	case 0x0102:
		m_soundType = AudioFileType::MP3;
		m_bits = 16;
		break;
	case 0x0202:
		m_soundType = AudioFileType::MP3;
		m_bits = 32;
		break;
	}
}
void resource::Sound::Read(const Resource &resource, ufile::IFile &f)
{
	f.Seek(GetOffset());
	if(resource.GetVersion() > 4)
		throw std::invalid_argument {"Invalid vsnd version '" + std::to_string(resource.GetVersion()) + "'"};
	if(resource.GetVersion() == 4) {
		m_sampleRate = f.Read<uint16_t>();
		SetVersion4(f);
		m_sampleSize = m_bits / 8;
		m_channels = 1;
		m_audioFormat = 1;
	}
	else {
		auto fExtractSub = [](uint32_t l, uint8_t offset, uint8_t nrBits) -> uint32_t {
			auto rightShifted = l >> offset;
			auto mask = (1 << nrBits) - 1;
			return static_cast<uint32_t>(rightShifted & mask);
		};
		auto bitpackedSoundInfo = f.Read<uint32_t>();
		auto type = fExtractSub(bitpackedSoundInfo, 0, 2);

		if(type > 2)
			throw std::runtime_error {"Unknown sound type in old vsnd version:" + std::to_string(type)};

		m_soundType = static_cast<AudioFileType>(type);
		m_bits = fExtractSub(bitpackedSoundInfo, 2, 5);
		m_channels = fExtractSub(bitpackedSoundInfo, 7, 2);
		m_sampleSize = fExtractSub(bitpackedSoundInfo, 9, 3);
		m_audioFormat = fExtractSub(bitpackedSoundInfo, 12, 2);
		m_sampleRate = fExtractSub(bitpackedSoundInfo, 14, 17);
	}
	m_loopStart = f.Read<int32_t>();
	m_sampleCount = f.Read<uint32_t>();
	m_duration = f.Read<float>();
	f.Seek(f.Tell() + sizeof(uint32_t) * 3);
	m_streamingDataSize = f.Read<uint32_t>();
}
void resource::Sound::DebugPrint(std::stringstream &ss, const std::string &t) const
{
	ss << t << "Sound = {\n";
	ss << t << "\tSound type = " << to_string(m_soundType) << "\n";
	ss << t << "\tSample rate = " << m_sampleRate << "\n";
	ss << t << "\tBits: " << m_bits << "\n";
	ss << t << "\tChannels: " << m_channels << "\n";
	ss << t << "\tAudio format: " << m_audioFormat << "\n";
	ss << t << "\tSample size: " << m_sampleSize << "\n";
	ss << t << "\tSample count: " << m_sampleCount << "\n";
	ss << t << "\tLoop start: " << m_loopStart << "\n";
	ss << t << "\tDuration: " << m_duration << "\n";
	ss << t << "\tStreaming data size: " << m_streamingDataSize << "\n";
	ss << t << "}\n";
}

std::string resource::to_string(Sound::AudioFileType type)
{
	switch(type) {
	case Sound::AudioFileType::AAC:
		return "AAC";
	case Sound::AudioFileType::WAV:
		return "WAV";
	case Sound::AudioFileType::MP3:
		return "MP3";
	}
	return "Invalid";
}

///////////////

resource::KeyValuesOrNTRO::KeyValuesOrNTRO(BlockType type, const std::string &introspectionStructName) : ResourceData {}, m_type {type}, m_introspectionStructName {introspectionStructName} {}
resource::KeyValuesOrNTRO::KeyValuesOrNTRO() : KeyValuesOrNTRO {BlockType::DATA, ""} {}
void resource::KeyValuesOrNTRO::Read(const Resource &resource, ufile::IFile &f)
{
	if(resource.FindBlock(source2::BlockType::NTRO) == nullptr) {
		auto kv3 = std::make_shared<BinaryKV3>();
		kv3->SetOffset(GetOffset());
		kv3->SetSize(GetSize());
		kv3->Read(resource, f);

		m_data = std::static_pointer_cast<IKeyValueCollection>(kv3->GetData());
		m_bakingData = std::static_pointer_cast<ResourceData>(kv3);
	}
	else {
		auto ntro = std::make_shared<NTRO>();
		ntro->SetStructName(m_introspectionStructName);
		ntro->SetOffset(GetOffset());
		ntro->SetSize(GetSize());
		ntro->Read(resource, f);

		m_data = std::static_pointer_cast<IKeyValueCollection>(ntro->GetOutput());
		m_bakingData = std::static_pointer_cast<ResourceData>(ntro);
	}
}
void resource::KeyValuesOrNTRO::DebugPrint(std::stringstream &ss, const std::string &t) const {}
BlockType resource::KeyValuesOrNTRO::GetType() const { return m_type; }
const std::shared_ptr<resource::IKeyValueCollection> &resource::KeyValuesOrNTRO::GetData() const { return m_data; }
const std::shared_ptr<resource::ResourceData> &resource::KeyValuesOrNTRO::GetBakingData() const { return m_bakingData; }

///////////////

const std::string &resource::Material::GetName() const { return m_name; }
const std::string &resource::Material::GetShaderName() const { return m_shaderName; }

template<typename T>
static void collect_material_parameters(resource::IKeyValueCollection &data, const std::string &identifier, const std::string &key, std::unordered_map<std::string, T> &outData)
{
	auto params = resource::IKeyValueCollection::FindArrayValues<resource::IKeyValueCollection *>(data, identifier);
	for(auto &param : params) {
		auto name = param->FindValue<std::string>("m_name", "");
		auto val = param->FindValue<T>(key, T {});
		outData.insert(std::make_pair(name, val));
	}
}

void resource::Material::Read(const Resource &resource, ufile::IFile &f)
{
	KeyValuesOrNTRO::Read(resource, f);

	auto oName = IKeyValueCollection::FindValue<std::string>(*GetData(), "m_materialName");
	m_name = oName.has_value() ? *oName : "";

	auto oShaderName = IKeyValueCollection::FindValue<std::string>(*GetData(), "m_shaderName");
	m_shaderName = oShaderName.has_value() ? *oShaderName : "";

	collect_material_parameters(*GetData(), "m_intParams", "m_nValue", m_intParams);
	collect_material_parameters(*GetData(), "m_floatParams", "m_flValue", m_floatParams);
	collect_material_parameters(*GetData(), "m_vectorParams", "m_value", m_vectorParams);
	collect_material_parameters(*GetData(), "m_textureParams", "m_pValue", m_textureParams);

	collect_material_parameters(*GetData(), "m_intAttributes", "m_nValue", m_intAttributes);
	collect_material_parameters(*GetData(), "m_floatAttributes", "m_flValue", m_floatAttributes);
	collect_material_parameters(*GetData(), "m_vectorAttributes", "m_value", m_vectorAttributes);
	collect_material_parameters(*GetData(), "m_stringAttributes", "m_pValue", m_stringAttributes);
}
const int64_t *resource::Material::FindIntParam(const std::string &key) const
{
	auto it = m_intParams.find(key);
	if(it == m_intParams.end())
		return nullptr;
	return &it->second;
}
const float *resource::Material::FindFloatParam(const std::string &key) const
{
	auto it = m_floatParams.find(key);
	if(it == m_floatParams.end())
		return nullptr;
	return &it->second;
}
const Vector4 *resource::Material::FindVectorParam(const std::string &key) const
{
	auto it = m_vectorParams.find(key);
	if(it == m_vectorParams.end())
		return nullptr;
	return &it->second;
}
const std::string *resource::Material::FindTextureParam(const std::string &key) const
{
	auto it = m_textureParams.find(key);
	if(it == m_textureParams.end())
		return nullptr;
	return &it->second;
}

const int64_t *resource::Material::FindIntAttr(const std::string &key) const
{
	auto it = m_intAttributes.find(key);
	if(it == m_intAttributes.end())
		return nullptr;
	return &it->second;
}
const float *resource::Material::FindFloatAttr(const std::string &key) const
{
	auto it = m_floatAttributes.find(key);
	if(it == m_floatAttributes.end())
		return nullptr;
	return &it->second;
}
const Vector4 *resource::Material::FindVectorAttr(const std::string &key) const
{
	auto it = m_vectorAttributes.find(key);
	if(it == m_vectorAttributes.end())
		return nullptr;
	return &it->second;
}
const std::string *resource::Material::FindTextureAttr(const std::string &key) const
{
	auto it = m_stringAttributes.find(key);
	if(it == m_stringAttributes.end())
		return nullptr;
	return &it->second;
}
void resource::Material::DebugPrint(std::stringstream &ss, const std::string &t) const
{
	ss << t << "Material = {\n";
	ss << t << "\tName = " << GetName() << "\n";
	ss << t << "\tShader name = " << GetShaderName() << "\n";
	ss << t << "\tNames:\n";
	auto fPrintParams = [&ss, t](const std::string &identifier, const auto &params) {
		if(params.empty())
			return;
		ss << t << "\t" << identifier << ":\n";
		for(auto &pair : params)
			ss << t << "\t[" << pair.first << "] = " << pair.second << "\n";
	};
	fPrintParams("Int parameters", m_intParams);
	fPrintParams("Float parameters", m_floatParams);
	fPrintParams("Vector parameters", m_vectorParams);
	fPrintParams("Texture parameters", m_textureParams);

	fPrintParams("Int attributes", m_intAttributes);
	fPrintParams("Float attributes", m_floatAttributes);
	fPrintParams("Vector attributes", m_vectorAttributes);
	fPrintParams("String attributes", m_stringAttributes);
	ss << t << "}\n";
}

///////////////

void resource::SoundEventScript::Read(const Resource &resource, ufile::IFile &f) { NTRO::Read(resource, f); }
void resource::SoundEventScript::DebugPrint(std::stringstream &ss, const std::string &t) const {}
const std::unordered_map<std::string, std::string> &resource::SoundEventScript::GetValues() const { return m_values; }

///////////////

uint16_t resource::Texture::GetWidth() const { return (m_nonPow2Width > 0u) ? m_nonPow2Width : m_width; }
uint16_t resource::Texture::GetHeight() const { return (m_nonPow2Height > 0u) ? m_nonPow2Height : m_height; }
uint16_t resource::Texture::GetDepth() const { return m_depth; }
Vector4 resource::Texture::GetReflectivity() const { return m_reflectivity; }
VTexFlags resource::Texture::GetFlags() const { return m_flags; }
VTexFormat resource::Texture::GetFormat() const { return m_format; }
uint8_t resource::Texture::GetMipMapCount() const { return m_mipMapCount; }
uint32_t resource::Texture::GetPicmip0Res() const { return m_picmip0Res; }
const std::unordered_map<VTexExtraData, std::vector<uint8_t>> &resource::Texture::GetExtraData() const { return m_extraData; }

static std::array<std::array<uint8_t, 16>, 64> BC7PartitionTable3 {
  //Partition table for 3-subset BPTC, with the 4×4 block of values for each partition number
  std::array<uint8_t, 16> {0, 0, 1, 1, 0, 0, 1, 1, 0, 2, 2, 1, 2, 2, 2, 2},
  {0, 0, 0, 1, 0, 0, 1, 1, 2, 2, 1, 1, 2, 2, 2, 1},
  {0, 0, 0, 0, 2, 0, 0, 1, 2, 2, 1, 1, 2, 2, 1, 1},
  {0, 2, 2, 2, 0, 0, 2, 2, 0, 0, 1, 1, 0, 1, 1, 1},
  {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 2, 1, 1, 2, 2},
  {0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 2, 2, 0, 0, 2, 2},
  {0, 0, 2, 2, 0, 0, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1},
  {0, 0, 1, 1, 0, 0, 1, 1, 2, 2, 1, 1, 2, 2, 1, 1},
  {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2},
  {0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2},
  {0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2},
  {0, 0, 1, 2, 0, 0, 1, 2, 0, 0, 1, 2, 0, 0, 1, 2},
  {0, 1, 1, 2, 0, 1, 1, 2, 0, 1, 1, 2, 0, 1, 1, 2},
  {0, 1, 2, 2, 0, 1, 2, 2, 0, 1, 2, 2, 0, 1, 2, 2},
  {0, 0, 1, 1, 0, 1, 1, 2, 1, 1, 2, 2, 1, 2, 2, 2},
  {0, 0, 1, 1, 2, 0, 0, 1, 2, 2, 0, 0, 2, 2, 2, 0},
  {0, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 2, 1, 1, 2, 2},
  {0, 1, 1, 1, 0, 0, 1, 1, 2, 0, 0, 1, 2, 2, 0, 0},
  {0, 0, 0, 0, 1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2},
  {0, 0, 2, 2, 0, 0, 2, 2, 0, 0, 2, 2, 1, 1, 1, 1},
  {0, 1, 1, 1, 0, 1, 1, 1, 0, 2, 2, 2, 0, 2, 2, 2},
  {0, 0, 0, 1, 0, 0, 0, 1, 2, 2, 2, 1, 2, 2, 2, 1},
  {0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 2, 2, 0, 1, 2, 2},
  {0, 0, 0, 0, 1, 1, 0, 0, 2, 2, 1, 0, 2, 2, 1, 0},
  {0, 1, 2, 2, 0, 1, 2, 2, 0, 0, 1, 1, 0, 0, 0, 0},
  {0, 0, 1, 2, 0, 0, 1, 2, 1, 1, 2, 2, 2, 2, 2, 2},
  {0, 1, 1, 0, 1, 2, 2, 1, 1, 2, 2, 1, 0, 1, 1, 0},
  {0, 0, 0, 0, 0, 1, 1, 0, 1, 2, 2, 1, 1, 2, 2, 1},
  {0, 0, 2, 2, 1, 1, 0, 2, 1, 1, 0, 2, 0, 0, 2, 2},
  {0, 1, 1, 0, 0, 1, 1, 0, 2, 0, 0, 2, 2, 2, 2, 2},
  {0, 0, 1, 1, 0, 1, 2, 2, 0, 1, 2, 2, 0, 0, 1, 1},
  {0, 0, 0, 0, 2, 0, 0, 0, 2, 2, 1, 1, 2, 2, 2, 1},
  {0, 0, 0, 0, 0, 0, 0, 2, 1, 1, 2, 2, 1, 2, 2, 2},
  {0, 2, 2, 2, 0, 0, 2, 2, 0, 0, 1, 2, 0, 0, 1, 1},
  {0, 0, 1, 1, 0, 0, 1, 2, 0, 0, 2, 2, 0, 2, 2, 2},
  {0, 1, 2, 0, 0, 1, 2, 0, 0, 1, 2, 0, 0, 1, 2, 0},
  {0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 0, 0, 0, 0},
  {0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0},
  {0, 1, 2, 0, 2, 0, 1, 2, 1, 2, 0, 1, 0, 1, 2, 0},
  {0, 0, 1, 1, 2, 2, 0, 0, 1, 1, 2, 2, 0, 0, 1, 1},
  {0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 0, 0, 0, 0, 1, 1},
  {0, 1, 0, 1, 0, 1, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2},
  {0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 2, 1, 2, 1, 2, 1},
  {0, 0, 2, 2, 1, 1, 2, 2, 0, 0, 2, 2, 1, 1, 2, 2},
  {0, 0, 2, 2, 0, 0, 1, 1, 0, 0, 2, 2, 0, 0, 1, 1},
  {0, 2, 2, 0, 1, 2, 2, 1, 0, 2, 2, 0, 1, 2, 2, 1},
  {0, 1, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 0, 1, 0, 1},
  {0, 0, 0, 0, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1},
  {0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 2, 2, 2, 2},
  {0, 2, 2, 2, 0, 1, 1, 1, 0, 2, 2, 2, 0, 1, 1, 1},
  {0, 0, 0, 2, 1, 1, 1, 2, 0, 0, 0, 2, 1, 1, 1, 2},
  {0, 0, 0, 0, 2, 1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 2},
  {0, 2, 2, 2, 0, 1, 1, 1, 0, 1, 1, 1, 0, 2, 2, 2},
  {0, 0, 0, 2, 1, 1, 1, 2, 1, 1, 1, 2, 0, 0, 0, 2},
  {0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 2, 2, 2, 2},
  {0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 1, 2, 2, 1, 1, 2},
  {0, 1, 1, 0, 0, 1, 1, 0, 2, 2, 2, 2, 2, 2, 2, 2},
  {0, 0, 2, 2, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 2, 2},
  {0, 0, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2, 0, 0, 2, 2},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 1, 2},
  {0, 0, 0, 2, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 1},
  {0, 2, 2, 2, 1, 2, 2, 2, 0, 2, 2, 2, 1, 2, 2, 2},
  {0, 1, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
  {0, 1, 1, 1, 2, 0, 1, 1, 2, 2, 0, 1, 2, 2, 2, 0},
};

static std::array<uint8_t, 64> BC7AnchorIndices32 {//BPTC anchor index values for the second subset of three-subset partitioning, by partition number
  3, 3, 15, 15, 8, 3, 15, 15, 8, 8, 6, 6, 6, 5, 3, 3, 3, 3, 8, 15, 3, 3, 6, 10, 5, 8, 8, 6, 8, 5, 15, 15, 8, 15, 3, 5, 6, 10, 8, 15, 15, 3, 15, 5, 15, 15, 15, 15, 3, 15, 5, 5, 5, 8, 5, 10, 5, 10, 8, 13, 15, 12, 3, 3};
static std::array<uint8_t, 64> BC7AnchorIndices33 {//BPTC anchor index values for the third subset of three-subset partitioning, by partition number
  15, 8, 8, 3, 15, 15, 3, 8, 15, 15, 15, 15, 15, 15, 15, 8, 15, 8, 15, 3, 15, 8, 15, 8, 3, 15, 6, 10, 15, 15, 10, 8, 15, 3, 15, 10, 10, 8, 9, 10, 6, 15, 8, 15, 3, 6, 6, 8, 15, 3, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 3, 15, 15, 8};
static std::array<uint8_t, 8> BC7IndLength = {3, 3, 2, 2, 2, 2, 4, 2};

static std::array<std::array<uint8_t, 16>, 64> BPTCPartitionTable2 {
  //Partition table for 2-subset BPTC, with the 4×4 block of values for each partition number
  std::array<uint8_t, 16> {0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1},
  {0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1},
  {0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1},
  {0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 1},
  {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1},
  {0, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1},
  {0, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1},
  {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 1},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1},
  {0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  {0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1},
  {0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1},
  {0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1},
  {0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1},
  {0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0},
  {0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0},
  {0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0},
  {0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1},
  {0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0},
  {0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0},
  {0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0},
  {0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 0},
  {0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0},
  {0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0},
  {0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 0},
  {0, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0},
  {0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1},
  {0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1},
  {0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0},
  {0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0},
  {0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0},
  {0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0},
  {0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1},
  {0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1},
  {0, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 0},
  {0, 0, 0, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0},
  {0, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 0, 0},
  {0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 0, 0},
  {0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0},
  {0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1},
  {0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1},
  {0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0},
  {0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0},
  {0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0},
  {0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0},
  {0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1},
  {0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1},
  {0, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0},
  {0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0},
  {0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 0, 0, 1},
  {0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1},
  {0, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1},
  {0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1},
  {0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1},
  {0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0},
  {0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0},
  {0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1},
};
static std::array<uint8_t, 64> BPTCAnchorIndices2 {// BPTC anchor index values for the second subset of two-subset partitioning, by partition number
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 2, 8, 2, 2, 8, 8, 15, 2, 8, 2, 2, 8, 8, 2, 2, 15, 15, 6, 8, 2, 8, 15, 15, 2, 8, 2, 2, 2, 15, 15, 6, 6, 2, 6, 8, 15, 15, 2, 2, 15, 15, 15, 15, 15, 2, 2, 15};
static std::array<uint8_t, 4> BPTCWeights2 = {0, 21, 43, 64};
static std::array<uint8_t, 8> BPTCWeights3 = {0, 9, 19, 27, 47, 46, 55, 64};
static std::array<uint8_t, 16> BPTCWeights4 = {0, 4, 9, 13, 17, 21, 26, 30, 34, 38, 43, 47, 51, 55, 60, 64};

static uint16_t BPTCInterpolateFactor(int weight, int e0, int e1) { return (uint16_t)((((64 - weight) * e0) + (weight * e1) + 32) >> 6); }

void resource::Texture::UncompressBC7(uint32_t RowBytes, util::DataStream &ds, std::vector<uint8_t> &data, int w, int h, bool hemiOctRB, bool invert)
{
	auto blockCountX = (w + 3) / 4;
	auto blockCountY = (h + 3) / 4;

	for(auto j = decltype(blockCountY) {0u}; j < blockCountY; ++j) {
		for(auto i = decltype(blockCountX) {0u}; i < blockCountX; ++i) {
			auto block0 = ds->Read<uint64_t>();
			auto block64 = ds->Read<uint64_t>();
			int m = 0;
			for(; m < 8; m++) {
				if((block0 >> m & 1) == 1) {
					break;
				}
			}

			byte pb = 0;
			byte rb = 0;
			byte isb = 0;
			std::array<std::array<uint8_t, 4>, 6> endpoints {};
			byte epbits = 0;
			byte spbits = 0;
			uint64_t ib = 0;
			uint64_t ib2 = 0;

			if(m == 0) {
				pb = (byte)(block0 >> 1 & 0xF); //4bit
			}
			else if(m == 1 || m == 2 || m == 3 || m == 7) {
				pb = (byte)((block0 >> (m + 1)) & 0x3F); //6bit
			}

			auto ReadEndpoints = [&](int start, int ns2, int cb, int astart, int ab) {
				auto GetVal = [&](int p, byte vm) -> uint8_t {
					byte res = 0;
					if(p < 64) {
						res = (byte)(block0 >> p & vm);
						if(p + cb > 64) {
							res |= (byte)(block64 << (64 - p) & vm);
						}
					}
					else {
						res = (byte)(block64 >> (p - 64) & vm);
					}

					return res;
				};

				auto mask = (byte)((0x1 << cb) - 1);
				for(int c = 0; c < 3; c++) {
					for(int s = 0; s < ns2; s++) {
						int ofs = start + (cb * ((c * ns2) + s));
						endpoints[s][c] = GetVal(ofs, mask);
						if(m == 1) {
							endpoints[s][c] = (byte)(endpoints[s][c] << 2 | ((spbits >> (s >> 1) & 1) << 1) | (endpoints[s][c] >> 5));
						}
						else if(m == 0 || m == 3 || m == 6 || m == 7) {
							endpoints[s][c] = (byte)(endpoints[s][c] << (8 - cb) | ((epbits >> s & 1) << (7 - cb)) | (endpoints[s][c] >> ((cb * 2) - 7)));
						}
						else {
							endpoints[s][c] = (byte)(endpoints[s][c] << (8 - cb) | (endpoints[s][c] >> ((cb * 2) - 8)));
						}
					}
				}

				if(ab != 0) {
					mask = (byte)((0x1 << ab) - 1);
					for(int s = 0; s < ns2; s++) {
						int ofs = astart + (ab * s);
						endpoints[s][3] = GetVal(ofs, mask);
						if(m == 6 || m == 7) {
							endpoints[s][3] = (byte)((endpoints[s][3] << (8 - ab)) | ((epbits >> s & 1) << (7 - ab)) | (endpoints[s][3] >> ((ab * 2) - 7)));
						}
						else {
							endpoints[s][3] = (byte)((endpoints[s][3] << (8 - ab)) | (endpoints[s][3] >> ((ab * 2) - 8)));
						}
					}
				}
			};

			if(m == 0) {
				epbits = (byte)(block64 >> 13 & 0x3F);
				ReadEndpoints(5, 6, 4, 0, 0);
				ib = block64 >> 19;
			}
			else if(m == 1) {
				spbits = (byte)((block64 >> 16 & 1) | ((block64 >> 17 & 1) << 1));
				ReadEndpoints(8, 4, 6, 0, 0);
				ib = block64 >> 18;
			}
			else if(m == 2) {
				ReadEndpoints(9, 6, 5, 0, 0);
				ib = block64 >> 35;
			}
			else if(m == 3) {
				epbits = (byte)(block64 >> 30 & 0xF);
				ReadEndpoints(10, 4, 7, 0, 0);
				ib = block64 >> 34;
			}
			else if(m == 4) {
				rb = (byte)(block0 >> 5 & 0x3);
				isb = (byte)(block0 >> 7 & 0x1);
				ReadEndpoints(8, 2, 5, 38, 6);
				ib = (block0 >> 50) | (block64 << 14);
				ib2 = block64 >> 17;
			}
			else if(m == 5) {
				rb = (byte)((block0 >> 6) & 0x3);
				ReadEndpoints(8, 2, 7, 50, 8);
				ib = block64 >> 2;
				ib2 = block64 >> 33;
			}
			else if(m == 6) {
				epbits = (byte)((block0 >> 63) | ((block64 & 1) << 1));
				ReadEndpoints(7, 2, 7, 49, 7);
				ib = block64 >> 1;
			}
			else if(m == 7) {
				epbits = (byte)(block64 >> 30 & 0xF);
				ReadEndpoints(14, 4, 5, 74, 5);
				ib = block64 >> 34;
			}

			int ib2l = (m == 4) ? 3 : 2;
			for(int by = 0; by < 4; by++) {
				for(int bx = 0; bx < 4; bx++) {
					int io = (by * 4) + bx;
					auto pixelIndex = (((j * 4) + by) * RowBytes) + (((i * 4) + bx) * 4);

					byte cweight = 0;
					byte aweight = 0;
					byte subset = 0;

					int isAnchor = 0;
					if(m == 0 || m == 2) { //3 subsets
						isAnchor = (io == 0 || io == BC7AnchorIndices32[pb] || io == BC7AnchorIndices33[pb]) ? 1 : 0;
						subset = (byte)(BC7PartitionTable3[pb][io] * 2);
					}
					else if(m == 1 || m == 3 || m == 7) { //2 subsets
						subset = (byte)(BPTCPartitionTable2[pb][io] * 2);
						isAnchor = (io == 0 || io == BPTCAnchorIndices2[pb]) ? 1 : 0;
					}
					else if(m == 4 || m == 5 || m == 6) { //1 subset
						isAnchor = (io == 0) ? 1 : 0;
					}

					if(m == 0 || m == 1) { //3 bit
						cweight = BPTCWeights3[ib & (0x7u >> isAnchor)];
					}
					else if(m == 6) { //4 bit
						cweight = BPTCWeights4[ib & (0xFu >> isAnchor)];
					}
					else { //2 bit
						cweight = BPTCWeights2[ib & (0x3u >> isAnchor)];
					}

					ib >>= BC7IndLength[m] - isAnchor;

					if(m == 4) {
						aweight = BPTCWeights3[ib2 & (0x7u >> isAnchor)];
						ib2 >>= ib2l - isAnchor;

						if(isb == 1) {
							byte t = cweight;
							cweight = aweight;
							aweight = t;
						}
					}
					else if(m == 5) {
						aweight = BPTCWeights2[ib2 & (0x3u >> isAnchor)];
						ib2 >>= ib2l - isAnchor;
					}
					else if(m > 5) {
						aweight = cweight;
					}

					data[pixelIndex] = (byte)BPTCInterpolateFactor(cweight, endpoints[subset][2], endpoints[subset + 1][2]);
					data[pixelIndex + 1] = (byte)BPTCInterpolateFactor(cweight, endpoints[subset][1], endpoints[subset + 1][1]);
					data[pixelIndex + 2] = (byte)BPTCInterpolateFactor(cweight, endpoints[subset][0], endpoints[subset + 1][0]);

					if(m < 4) {
						data[pixelIndex + 3] = std::numeric_limits<uint8_t>::max();
					}
					else {
						data[pixelIndex + 3] = (byte)BPTCInterpolateFactor(aweight, endpoints[subset][3], endpoints[subset + 1][3]);

						if((m == 4 || m == 5) && rb != 0) {
							byte t = data[pixelIndex + 3];
							data[pixelIndex + 3] = data[pixelIndex + 3 - rb];
							data[pixelIndex + 3 - rb] = t;
						}
					}

					if(hemiOctRB) {
						float nx = ((data[pixelIndex + 2] + data[pixelIndex + 1]) / 255.0f) - 1.003922f;
						float ny = (data[pixelIndex + 2] - data[pixelIndex + 1]) / 255.0f;
						float nz = 1 - fabsf(nx) - fabsf(ny);

						float l = (float)sqrtf((nx * nx) + (ny * ny) + (nz * nz));
						data[pixelIndex + 3] = data[pixelIndex + 0]; //b to alpha
						data[pixelIndex + 2] = (byte)(((nx / l * 0.5f) + 0.5f) * 255);
						data[pixelIndex + 1] = (byte)(((ny / l * 0.5f) + 0.5f) * 255);
						data[pixelIndex + 0] = (byte)(((nz / l * 0.5f) + 0.5f) * 255);
					}

					if(invert) {
						data[pixelIndex + 1] = (byte)(~data[pixelIndex + 1]); // LegacySource1InvertNormals
					}
				}
			}
		}
	}
}
void resource::Texture::Read(const Resource &resource, ufile::IFile &f)
{
	m_file = &f;
	f.Seek(GetOffset());
	auto version = f.Read<uint16_t>();
	if(version != 1)
		throw std::runtime_error {"Unknown vtex version. ({" + std::to_string(version) + "} != expected 1)"};
	m_flags = f.Read<VTexFlags>();
	m_reflectivity = f.Read<Vector4>();
	m_width = f.Read<uint16_t>();
	m_height = f.Read<uint16_t>();
	m_depth = f.Read<uint16_t>();
	m_format = f.Read<VTexFormat>();
	m_mipMapCount = f.Read<uint8_t>();
	m_picmip0Res = f.Read<uint32_t>();

	auto extraDataOffset = f.Read<uint32_t>();
	auto extraDataCount = f.Read<uint32_t>();
	if(extraDataCount > 0u) {
		f.Seek(f.Tell() + extraDataOffset - sizeof(uint32_t) * 2);
		for(auto i = decltype(extraDataCount) {0u}; i < extraDataCount; ++i) {
			auto type = f.Read<VTexExtraData>();
			auto offset = f.Read<uint32_t>() - sizeof(uint32_t) * 2;
			auto size = f.Read<uint32_t>();
			auto prevOffset = f.Tell();
			f.Seek(f.Tell() + offset);

			if(type == VTexExtraData::FILL_TO_POWER_OF_TWO) {
				f.Seek(f.Tell() + sizeof(uint16_t));
				auto nw = f.Read<uint16_t>();
				auto nh = f.Read<uint16_t>();
				if(nw > 0 && nh > 0 && m_width >= nw && m_height >= nh) {
					m_nonPow2Width = nw;
					m_nonPow2Height = nh;
				}
				f.Seek(f.Tell() - 6);
			}

			auto it = m_extraData.insert(std::make_pair(type, std::vector<uint8_t> {})).first;
			auto &data = it->second;
			data.resize(size);
			f.Read(data.data(), data.size() * sizeof(data.front()));

			if(type == VTexExtraData::COMPRESSED_MIP_SIZE) {
				f.Seek(f.Tell() - size);

				auto int1 = f.Read<uint32_t>(); // 1?
				auto int2 = f.Read<uint32_t>(); // 8?
				auto mips = f.Read<uint32_t>();

				if(int1 != 1 && int1 != 0)
					throw std::runtime_error("int1 got: {int1}");

				if(int2 != 8)
					throw std::runtime_error("int2 expected 8 but got: {int2}");

				m_isCompressed = (int1 == 1); // TODO: Verify whether this int is the one that actually controls compression

				m_compressedMips.resize(mips);
				for(auto mip = decltype(mips) {0u}; mip < mips; ++mip)
					m_compressedMips.at(mip) = f.Read<int32_t>();
			}
			f.Seek(prevOffset);
		}
	}
	m_dataOffset = GetOffset() + GetSize();
}
void resource::Texture::DebugPrint(std::stringstream &ss, const std::string &t) const
{
	ss << t << "Texture = {\n";
	ss << t << "\tFlags: " << to_string(m_flags) << "\n";
	ss << t << "\tReflectivity: " << m_reflectivity << "\n";
	ss << t << "\tWidth: " << m_width << "\n";
	ss << t << "\tHeight: " << m_height << "\n";
	ss << t << "\tDepth: " << m_depth << "\n";
	ss << t << "\tFormat: " << to_string(m_format) << "\n";
	ss << t << "\tMipmap count: " << m_mipMapCount << "\n";
	ss << t << "\tPicmip0 res: " << m_picmip0Res << "\n";
	ss << t << "\tData offset: " << m_dataOffset << "\n";
	ss << t << "\tNon pow2 width: " << m_nonPow2Width << "\n";
	ss << t << "\tNon pow2 height: " << m_nonPow2Height << "\n";
	ss << t << "\tCompressed: " << m_isCompressed << "\n";
	ss << t << "\tExtra data: " << m_extraData.size() << "\n";
	ss << t << "}\n";
}

uint32_t resource::Texture::GetBlockSize()
{
	switch(m_format) {
	case VTexFormat::DXT1:
		return 8;
	case VTexFormat::DXT5:
		return 16;
	case VTexFormat::RGBA8888:
		return 4;
	case VTexFormat::R16:
		return 2;
	case VTexFormat::RG1616:
		return 4;
	case VTexFormat::RGBA16161616:
		return 8;
	case VTexFormat::R16F:
		return 2;
	case VTexFormat::RG1616F:
		return 4;
	case VTexFormat::RGBA16161616F:
		return 8;
	case VTexFormat::R32F:
		return 4;
	case VTexFormat::RG3232F:
		return 8;
	case VTexFormat::RGB323232F:
		return 12;
	case VTexFormat::RGBA32323232F:
		return 16;
	case VTexFormat::BC6H:
		return 16;
	case VTexFormat::BC7:
		return 16;
	case VTexFormat::IA88:
		return 2;
	case VTexFormat::ETC2:
		return 8;
	case VTexFormat::ETC2_EAC:
		return 16;
	case VTexFormat::BGRA8888:
		return 4;
	case VTexFormat::ATI1N:
		return 8;
	case VTexFormat::ATI2N:
		return 16;
	}

	return 1;
}

uint64_t resource::Texture::GetMipmapDataOffset(uint8_t mipmap)
{
	auto offset = m_dataOffset;

	auto numMipmLevels = GetMipMapCount();
	if(numMipmLevels == 1)
		return offset;

	for(auto j = numMipmLevels - 1; j > mipmap; j--) {
		if(m_isCompressed) {
			offset += m_compressedMips[j];
		}
		else {
			offset += CalculateBufferSizeForMipLevel(j);
		}
	}
	return offset;
}

void resource::Texture::ReadTextureData(uint8_t mipLevel, std::vector<uint8_t> &outData)
{
	m_file->Seek(GetMipmapDataOffset(mipLevel));
	auto uncompressedSize = CalculateBufferSizeForMipLevel(mipLevel);
	auto compressedSize = m_isCompressed ? m_compressedMips.at(mipLevel) : uncompressedSize;
	outData.resize(uncompressedSize);
	if(compressedSize >= uncompressedSize) {
		m_file->Read(outData.data(), outData.size() * sizeof(outData.front()));
		return;
	}
	std::vector<uint8_t> compressedData {};
	compressedData.resize(compressedSize);
	m_file->Read(compressedData.data(), compressedData.size() * sizeof(compressedData.front()));

	auto result = LZ4_decompress_safe(reinterpret_cast<char *>(compressedData.data()), reinterpret_cast<char *>(outData.data()), compressedData.size() * sizeof(compressedData.front()), outData.size() * sizeof(outData.front()));
	if(result < 0)
		throw std::runtime_error {"Unable to decompress LZ4 data: " + std::to_string(result)};
}

uint32_t resource::Texture::CalculateBufferSizeForMipLevel(uint8_t mipLevel)
{
	auto bytesPerPixel = GetBlockSize();
	auto width = m_width >> mipLevel;
	auto height = m_height >> mipLevel;
	auto depth = m_depth >> mipLevel;

	if(depth < 1)
		depth = 1;

	if(m_format == VTexFormat::DXT1 || m_format == VTexFormat::DXT5 || m_format == VTexFormat::BC6H || m_format == VTexFormat::BC7 || m_format == VTexFormat::ETC2 || m_format == VTexFormat::ETC2_EAC || m_format == VTexFormat::ATI1N || m_format == VTexFormat::ATI2N) {
		auto misalign = width % 4;

		if(misalign > 0) {
			width += 4 - misalign;
		}

		misalign = height % 4;

		if(misalign > 0) {
			height += 4 - misalign;
		}

		if(width < 4 && width > 0) {
			width = 4;
		}

		if(height < 4 && height > 0) {
			height = 4;
		}

		if(depth < 4 && depth > 1) {
			depth = 4;
		}

		auto numBlocks = (width * height) >> 4;
		numBlocks *= depth;
		return numBlocks * bytesPerPixel;
	}

	return width * height * depth * bytesPerPixel;
}

////////////////

resource::KVValue::KVValue(KVType type, std::shared_ptr<void> value, KVFlag flags) : m_object {value}, m_flags {flags}, m_type {type} {}
void *resource::KVValue::GetObject() { return m_object.get(); }
const void *resource::KVValue::GetObject() const { return const_cast<KVValue *>(this)->GetObject(); }
resource::KVFlag resource::KVValue::GetFlags() const { return m_flags; }
resource::KVType resource::KVValue::GetType() const { return m_type; }
void resource::KVValue::DebugPrint(std::stringstream &ss, const std::string &t) const
{
	ss << t << "KVValue = {\n";
	ss << t << "\tType = " << to_string(m_type) << "\n";
	ss << t << "\tValue = ";
	switch(m_type) {
	case KVType::Invalid:
		ss << "";
		break;
	case KVType::Null:
		ss << "null";
		break;
	case KVType::BOOLEAN:
		ss << *static_cast<const bool *>(GetObject());
		break;
	case KVType::INT64:
		ss << *static_cast<const int64_t *>(GetObject());
		break;
	case KVType::UINT64:
		ss << *static_cast<const uint64_t *>(GetObject());
		break;
	case KVType::DOUBLE:
		ss << *static_cast<const double *>(GetObject());
		break;
	case KVType::STRING:
	case KVType::STRING_MULTI:
		ss << *static_cast<const std::string *>(GetObject());
		break;
	case KVType::BINARY_BLOB:
		ss << "[BINARY_BLOB]";
		break;
	case KVType::ARRAY:
		ss << "[ARRAY]";
		break;
	case KVType::OBJECT:
		ss << "[OBJECT]";
		break;
	case KVType::ARRAY_TYPED:
		ss << "[ARRAY_TYPED]";
		break;
	case KVType::INT32:
		ss << *static_cast<const int32_t *>(GetObject());
		break;
	case KVType::UINT32:
		ss << *static_cast<const uint32_t *>(GetObject());
		break;
	}
	ss << "\n";
	ss << t << "}\n";
}

////////////////

resource::KVObject::KVObject(const std::string &name, bool isArray) : m_key {name}, m_isArray {isArray} {}

const std::unordered_map<std::string, std::shared_ptr<resource::KVValue>> &resource::KVObject::GetValues() const { return m_values; }

resource::KVValue *resource::KVObject::FindValue(const std::string &key)
{
	auto it = m_values.find(key);
	return (it != m_values.end()) ? it->second.get() : nullptr;
}
const resource::KVValue *resource::KVObject::FindValue(const std::string &key) const { return const_cast<KVObject *>(this)->FindValue(key); }
resource::KVObject *resource::KVObject::FindArray(const std::string &key)
{
	auto oval = FindValue<KVObject *>(key, KVType::ARRAY_TYPED);
	if(oval.has_value() == false)
		oval = FindValue<KVObject *>(key, KVType::ARRAY);
	if(oval.has_value() == false || *oval == nullptr)
		return nullptr;
	return *oval;
}
const resource::KVObject *resource::KVObject::FindArray(const std::string &key) const { return const_cast<KVObject *>(this)->FindArray(key); }
resource::BinaryBlob *resource::KVObject::FindBinaryBlob(const std::string &key)
{
	auto *val = FindValue(key);
	if(val == nullptr || val->GetType() != KVType::BINARY_BLOB)
		return {};
	auto oBlob = val->GetObjectValue<BinaryBlob *>();
	if(oBlob.has_value() == false || *oBlob == nullptr)
		return {};
	return *oBlob;
}

void resource::KVObject::AddProperty(const std::string &name, KVValue &value)
{
	auto pvalue = value.shared_from_this();
	if(m_isArray)
		m_values.insert(std::make_pair(std::to_string(m_count), pvalue));
	else
		m_values.insert(std::make_pair(name, pvalue));
	++m_count;
}

bool resource::KVObject::IsArray() const { return m_isArray; }
uint32_t resource::KVObject::GetArrayCount() const { return m_count; }
resource::KVValue *resource::KVObject::GetArrayValue(uint32_t idx, std::optional<KVType> confirmType)
{
	if(IsArray() == false)
		return nullptr;
	auto *val = FindValue(std::to_string(idx));
	return (val && (confirmType.has_value() == false || val->GetType() == *confirmType)) ? val : nullptr;
}
const resource::KVValue *resource::KVObject::GetArrayValue(uint32_t idx, std::optional<KVType> confirmType) const { return const_cast<KVObject *>(this)->GetArrayValue(idx, confirmType); }
void resource::KVObject::DebugPrint(std::stringstream &ss, const std::string &t) const
{
	ss << t << "KVObject:\n";
	ss << t << "\tKey: " << m_key << "\n";
	ss << t << "\tIs array: " << m_isArray << "\n";
	ss << t << "\tCount: " << m_count << "\n";
	ss << t << "\tValues:\n";
	for(auto &pair : m_values) {
		auto &v = pair.second;
		ss << t << "\t\t[" << pair.first << "] = {\n";
		v->DebugPrint(ss, t + "\t\t\t");
		ss << t << "\t\t}\n";
	}
}

////////////////

const util::GUID source2::resource::BinaryKV3::KV3_ENCODING_BINARY_BLOCK_COMPRESSED = {0x46, 0x1A, 0x79, 0x95, 0xBC, 0x95, 0x6C, 0x4F, 0xA7, 0x0B, 0x05, 0xBC, 0xA1, 0xB7, 0xDF, 0xD2};
const util::GUID source2::resource::BinaryKV3::KV3_ENCODING_BINARY_UNCOMPRESSED = {0x00, 0x05, 0x86, 0x1B, 0xD8, 0xF7, 0xC1, 0x40, 0xAD, 0x82, 0x75, 0xA4, 0x82, 0x67, 0xE7, 0x14};
const util::GUID source2::resource::BinaryKV3::KV3_ENCODING_BINARY_BLOCK_LZ4 = {0x8A, 0x34, 0x47, 0x68, 0xA1, 0x63, 0x5C, 0x4F, 0xA1, 0x97, 0x53, 0x80, 0x6F, 0xD9, 0xB1, 0x19};
const util::GUID source2::resource::BinaryKV3::KV3_FORMAT_GENERIC = {0x7C, 0x16, 0x12, 0x74, 0xE9, 0x06, 0x98, 0x46, 0xAF, 0xF2, 0xE6, 0x3E, 0xB5, 0x90, 0x37, 0xE7};
const std::array<uint8_t, 16> source2::resource::BinaryKV3::ENCODING = {0x46, 0x1A, 0x79, 0x95, 0xBC, 0x95, 0x6C, 0x4F, 0xA7, 0x0B, 0x05, 0xBC, 0xA1, 0xB7, 0xDF, 0xD2};
const std::array<uint8_t, 16> source2::resource::BinaryKV3::FORMAT = {0x7C, 0x16, 0x12, 0x74, 0xE9, 0x06, 0x98, 0x46, 0xAF, 0xF2, 0xE6, 0x3E, 0xB5, 0x90, 0x37, 0xE7};
const std::array<uint8_t, 4> source2::resource::BinaryKV3::SIG = {0x56, 0x4B, 0x56, 0x03}; // VKV3 (3 isn't ascii, its 0x03)
const std::vector<std::string> &resource::BinaryKV3::GetStringArray() const { return m_stringArray; }
const std::shared_ptr<resource::KVObject> &resource::BinaryKV3::GetData() const { return const_cast<BinaryKV3 *>(this)->GetData(); }
std::shared_ptr<resource::KVObject> &resource::BinaryKV3::GetData() { return m_data; }
resource::BinaryKV3::BinaryKV3(BlockType type) : m_blockType {type} {}
void resource::BinaryKV3::Read(const Resource &resource, ufile::IFile &f)
{
	f.Seek(GetOffset());

	util::DataStream ds {};
	auto magic = f.Read<uint32_t>();
	if(magic == MAGIC2) {
		ReadVersion2(f, ds);
		return;
	}

	if(magic != MAGIC)
		throw std::runtime_error {"Invalid KV3 signature " + std::to_string(magic)};

	auto encoding = f.Read<util::GUID>();
	auto format = f.Read<util::GUID>();

	// Valve's implementation lives in LoadKV3Binary()
	// KV3_ENCODING_BINARY_BLOCK_COMPRESSED calls CBlockCompress::FastDecompress()
	// and then it proceeds to call LoadKV3BinaryUncompressed, which should be the same routine for KV3_ENCODING_BINARY_UNCOMPRESSED
	// Old binary with debug symbols for ref: https://users.alliedmods.net/~asherkin/public/bins/dota_symbols/bin/osx64/libmeshsystem.dylib

	if(util::compare_guid(encoding, KV3_ENCODING_BINARY_BLOCK_COMPRESSED))
		BlockDecompress(f, ds);
	else if(util::compare_guid(encoding, KV3_ENCODING_BINARY_BLOCK_LZ4))
		DecompressLZ4(f, ds);
	else if(util::compare_guid(encoding, KV3_ENCODING_BINARY_UNCOMPRESSED)) {
		auto szCpy = f.GetSize() - f.Tell();
		ds->Resize(szCpy);
		f.Read(ds->GetData(), szCpy);
		ds->SetOffset(0);
	}
	else
		throw new std::runtime_error {"Unrecognised KV3 Encoding: " + util::guid_to_string(encoding)};

	auto numStrings = ds->Read<uint32_t>();
	m_stringArray.reserve(numStrings);
	for(auto i = decltype(numStrings) {0u}; i < numStrings; ++i)
		m_stringArray.push_back(ds->ReadString()); // TODO: UTF8
	m_data = ParseBinaryKV3(ds, nullptr, true);
}
void resource::BinaryKV3::DebugPrint(std::stringstream &ss, const std::string &t) const
{
	ss << t << "Texture = {\n";
	ss << t << "\tCurrent binary bytes offset: " << m_currentBinaryBytesOffset << "\n";
	ss << t << "\tCurrent eight bytes offset: " << m_currentEightBytesOffset << "\n";
	ss << t << "\tCurrent type index: " << m_currentTypeIndex << "\n";
	ss << t << "\tHas types array: " << m_hasTypesArray << "\n";
	ss << t << "\tBlock type: " << to_string(m_blockType) << "\n";
	ss << t << "\tData:\n";
	ss << t << "\t{\n";
	m_data->DebugPrint(ss, t + "\t\t");
	ss << t << "\t}\n";
	ss << t << "\tStringArray:\n";
	for(auto i = decltype(m_stringArray.size()) {0u}; i < m_stringArray.size(); ++i) {
		auto &str = m_stringArray.at(i);
		ss << t << "\t\t[" << i << "] = {\n";
		ss << t << "\t\t\t" << str << "\n";
		ss << t << "\t\t}\n";
	}
	ss << t << "\tTypesArray:\n";
	for(auto i = decltype(m_typesArray.size()) {0u}; i < m_typesArray.size(); ++i) {
		auto &type = m_typesArray.at(i);
		ss << t << "\t\t[" << i << "] = {\n";
		ss << t << "\t\t\t" << type << "\n";
		ss << t << "\t\t}\n";
	}
	ss << t << "}\n";
}
BlockType resource::BinaryKV3::GetType() const { return m_blockType; }
void resource::BinaryKV3::ReadVersion2(ufile::IFile &f, util::DataStream &outData)
{
	auto format = f.Read<util::GUID>();

	auto compressionMethod = f.Read<int32_t>();
	auto countOfBinaryBytes = f.Read<int32_t>();     // how many bytes (binary blobs)
	auto countOfIntegers = f.Read<int32_t>();        // how many 4 byte values (ints)
	auto countOfEightByteValues = f.Read<int32_t>(); // how many 8 byte values (doubles)

	if(compressionMethod == 0) {
		auto length = f.Read<int32_t>();

		outData->Resize(length);
		f.Read(outData->GetData(), length);
	}
	else if(compressionMethod == 1)
		DecompressLZ4(f, outData);
	else
		throw std::runtime_error {"Unknown KV3 compression method: " + std::to_string(compressionMethod)};

	m_currentBinaryBytesOffset = 0;
	outData->SetOffset(countOfBinaryBytes);

	if(outData->GetOffset() % 4 != 0) {
		// Align to % 4 after binary blobs
		outData->SetOffset(outData->GetOffset() + 4 - (outData->GetOffset() % 4));
	}

	auto countOfStrings = outData->Read<int32_t>();
	auto kvDataOffset = outData->GetOffset();

	// Subtract one integer since we already read it (countOfStrings)
	outData->SetOffset(outData->GetOffset() + (countOfIntegers - 1) * 4);

	if(outData->GetOffset() % 8 != 0) {
		// Align to % 8 for the start of doubles
		outData->SetOffset(outData->GetOffset() + 8 - (outData->GetOffset() % 8));
	}

	m_currentEightBytesOffset = outData->GetOffset();

	outData->SetOffset(outData->GetOffset() + countOfEightByteValues * 8);

	m_stringArray.resize(countOfStrings);

	for(auto i = decltype(countOfStrings) {0u}; i < countOfStrings; ++i)
		m_stringArray.at(i) = outData->ReadString(); // UTF8

	// bytes after the string table is kv types, minus 4 static bytes at the end
	auto typesLength = outData->GetInternalSize() - 4 - outData->GetOffset();
	m_typesArray.resize(typesLength);
	m_hasTypesArray = true;
	outData->Read(m_typesArray.data(), m_typesArray.size());

	// Move back to the start of the KV data for reading.
	outData->SetOffset(kvDataOffset);

	m_data = ParseBinaryKV3(outData, nullptr, true);
}
void resource::BinaryKV3::BlockDecompress(ufile::IFile &f, util::DataStream &outData)
{
	// It is flags, right?
	auto flags = f.Read<std::array<uint8_t, 4>>();

	// outWrite.Write(flags);
	if((flags[3] & 0x80) > 0) {
		auto size = f.GetSize() - f.Tell();
		outData->Resize(size);
		f.Read(outData->GetData(), size);
	}
	else {
		outData->Reserve(f.GetSize() - f.Tell());
		std::vector<uint8_t> vOutData {};
		vOutData.reserve(f.GetSize() - f.Tell());
		auto running = true;
		uint64_t dataReadOffset = 0ull;
		uint64_t dataWriteOffset = 0ull;
		auto fRead = [&vOutData, &dataReadOffset](void *outData, uint64_t size) {
			memcpy(outData, vOutData.data() + dataReadOffset, size);
			dataReadOffset += size;
		};
		auto fWrite = [&vOutData, &dataWriteOffset, &dataReadOffset](const void *inData, uint64_t size) {
			if(dataWriteOffset + size >= vOutData.size())
				vOutData.resize(dataWriteOffset + size);
			memcpy(vOutData.data() + dataWriteOffset, inData, size);
			dataWriteOffset += size;
			dataReadOffset += size;
		};
		auto fWriteFront = [&vOutData, &dataWriteOffset, &dataReadOffset](const void *inData, uint64_t size) {
			if(dataWriteOffset + size >= vOutData.capacity())
				vOutData.reserve(dataWriteOffset + size);
			for(uint8_t i = 0; i < size; ++i)
				vOutData.insert(vOutData.begin(), 0);
			memcpy(vOutData.data(), inData, size);

			dataWriteOffset += size;
			dataReadOffset += size;
		};
		while(f.Tell() != f.GetSize() && running) {
			//try
			//{
			auto blockMask = f.Read<uint16_t>();
			for(auto i = decltype(blockMask) {0u}; i < (sizeof(blockMask) * 8); ++i) {
				// is the ith bit 1
				if((blockMask & (1 << i)) > 0) {
					auto offsetSize = f.Read<uint16_t>();
					auto offset = ((offsetSize & 0xFFF0) >> 4) + 1;
					auto size = (offsetSize & 0x000F) + 3;

					auto lookupSize = (offset < size) ? offset : size; // If the offset is larger or equal to the size, use the size instead.

					auto p = dataReadOffset;
					dataReadOffset = p - offset;
					std::vector<uint8_t> data {};
					data.resize(lookupSize);
					fRead(data.data(), data.size() * sizeof(data.front()));
					dataWriteOffset = p;
					dataReadOffset = p;

					while(size > 0) {
						fWrite(data.data(), (lookupSize < size) ? lookupSize : size);
						size -= lookupSize;
					}
				}
				else {
					auto data = f.Read<uint8_t>();
					fWrite(&data, sizeof(data));
				}

				//TODO: is there a better way of making an unsigned 12bit number?
				// TODO
				if(vOutData.size() == (flags[2] << 16) + (flags[1] << 8) + flags[0]) {
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
		outData->Write(vOutData.data(), vOutData.size() * sizeof(vOutData.front()));
	}
	outData->SetOffset(0);
}

void resource::BinaryKV3::DecompressLZ4(ufile::IFile &f, util::DataStream &outData)
{
	auto uncompressedSize = f.Read<uint32_t>();
	auto compressedSize = GetSize() - (f.Tell() - GetOffset());

	std::vector<uint8_t> input {};
	input.resize(compressedSize);
	f.Read(input.data(), input.size() * sizeof(input.front()));

	outData->Resize(uncompressedSize);
	auto result = LZ4_decompress_safe(reinterpret_cast<char *>(input.data()), reinterpret_cast<char *>(outData->GetData()), input.size(), uncompressedSize);
	if(result < 0)
		throw std::runtime_error {"Unable to decompress LZ4 data: " + std::to_string(result)};

	outData->SetOffset(0);
}

std::pair<resource::KVType, resource::KVFlag> resource::BinaryKV3::ReadType(util::DataStream &ds)
{
	uint8_t databyte;
	if(m_hasTypesArray)
		databyte = m_typesArray.at(m_currentTypeIndex++);
	else
		databyte = ds->Read<uint8_t>();

	auto flagInfo = KVFlag::None;
	if((databyte & 0x80) > 0) {
		databyte &= 0x7F; // Remove the flag bit

		if(m_hasTypesArray)
			flagInfo = static_cast<KVFlag>(m_typesArray.at(m_currentTypeIndex++));
		else
			flagInfo = ds->Read<KVFlag>();
	}
	return {static_cast<resource::KVType>(databyte), flagInfo};
}

std::shared_ptr<resource::KVValue> resource::BinaryKV3::MakeValue(KVType type, std::shared_ptr<void> data, KVFlag flag)
{
	auto realType = ConvertBinaryOnlyKVType(type);
	if(flag != KVFlag::None)
		return std::make_shared<KVValue>(realType, data, flag);

	return std::make_shared<KVValue>(realType, data);
}

resource::KVType resource::BinaryKV3::ConvertBinaryOnlyKVType(KVType type)
{
	switch(type) {
	case KVType::BOOLEAN:
	case KVType::BOOLEAN_TRUE:
	case KVType::BOOLEAN_FALSE:
		return KVType::BOOLEAN;
	case KVType::INT64:
	case KVType::INT64_ZERO:
	case KVType::INT64_ONE:
		return KVType::INT64;
	case KVType::INT32:
		return KVType::INT32;
	case KVType::UINT64:
		return KVType::UINT64;
	case KVType::UINT32:
		return KVType::UINT32;
	case KVType::DOUBLE:
	case KVType::DOUBLE_ZERO:
	case KVType::DOUBLE_ONE:
		return KVType::DOUBLE;
	case KVType::ARRAY_TYPED:
		return KVType::ARRAY;
	}

	return type;
}

#if 0
template<class TKVValue,typename T>
	std::shared_ptr<resource::KVValue> resource::BinaryKV3::MakeValue(KVType type, T data, KVFlag flag)
{
	auto v = std::make_shared<T>(data);
	return std::static_pointer_cast<resource::KVValue>(MakeValueFromPtr<TKVValue>(type,*v,flag));
}

template<class TKVValue,typename TPtr>
	std::shared_ptr<resource::KVValue> resource::BinaryKV3::MakeValueFromPtr(KVType type, TPtr &data, KVFlag flag)
{
	auto pdata = data.shared_from_this();
	auto realType = ConvertBinaryOnlyKVType(type);

	if (flag != KVFlag::None)
	{
		auto p = std::make_shared<TKVValue>(realType, pdata, flag);
		return std::static_pointer_cast<resource::KVValue>(p);
	}
	auto p = std::make_shared<TKVValue>(realType, pdata);
	return std::static_pointer_cast<resource::KVValue>(p);
}

template<class TKVValue>
	std::shared_ptr<resource::KVValue> resource::BinaryKV3::MakeValueFromPtr(KVType type, nullptr_t nptr, KVFlag flag)
{
	auto realType = ConvertBinaryOnlyKVType(type);

	if (flag != KVFlag::None)
	{
		auto p = std::make_shared<TKVValue>(realType, nptr, flag);
		return std::static_pointer_cast<resource::KVValue>(p);
	}
	auto p = std::make_shared<TKVValue>(realType, nptr);
	return std::static_pointer_cast<resource::KVValue>(p);
}
#endif
std::shared_ptr<resource::KVObject> resource::BinaryKV3::ReadBinaryValue(const std::string &name, KVType datatype, KVFlag flagInfo, util::DataStream ds, std::shared_ptr<KVObject> parent)
{
	auto currentOffset = ds->GetOffset();

	std::shared_ptr<void> value = nullptr; // Only needed so the object doesn't get destroyed in the switch-case scope
	switch(datatype) {
	case KVType::Null:
		{
			auto v = MakeValue(datatype, nullptr, flagInfo);
			parent->AddProperty(name, *v);
			break;
		}
	case KVType::BOOLEAN:
		{
			if(m_currentBinaryBytesOffset > -1)
				ds->SetOffset(m_currentBinaryBytesOffset);

			auto v = MakeValue(datatype, std::make_shared<bool>(ds->Read<bool>()), flagInfo);
			parent->AddProperty(name, *v);

			if(m_currentBinaryBytesOffset > -1) {
				m_currentBinaryBytesOffset++;
				ds->SetOffset(currentOffset);
			}

			break;
		}
	case KVType::BOOLEAN_TRUE:
		{
			auto v = MakeValue(datatype, std::make_shared<bool>(true), flagInfo);
			parent->AddProperty(name, *v);
			break;
		}
	case KVType::BOOLEAN_FALSE:
		{
			auto v = MakeValue(datatype, std::make_shared<bool>(false), flagInfo);
			parent->AddProperty(name, *v);
			break;
		}
	case KVType::INT64_ZERO:
		{
			auto v = MakeValue(datatype, std::make_shared<int64_t>(0ll), flagInfo);
			parent->AddProperty(name, *v);
			break;
		}
	case KVType::INT64_ONE:
		{
			auto v = MakeValue(datatype, std::make_shared<int64_t>(1ull), flagInfo);
			parent->AddProperty(name, *v);
			break;
		}
	case KVType::INT64:
		{
			if(m_currentEightBytesOffset > 0)
				ds->SetOffset(m_currentEightBytesOffset);

			auto v = MakeValue(datatype, std::make_shared<int64_t>(ds->Read<int64_t>()), flagInfo);
			parent->AddProperty(name, *v);

			if(m_currentEightBytesOffset > 0) {
				m_currentEightBytesOffset = ds->GetOffset();
				ds->SetOffset(currentOffset);
			}

			break;
		}
	case KVType::UINT64:
		{
			if(m_currentEightBytesOffset > 0)
				ds->SetOffset(m_currentEightBytesOffset);

			auto v = MakeValue(datatype, std::make_shared<uint64_t>(ds->Read<uint64_t>()), flagInfo);
			parent->AddProperty(name, *v);

			if(m_currentEightBytesOffset > 0) {
				m_currentEightBytesOffset = ds->GetOffset();
				ds->SetOffset(currentOffset);
			}

			break;
		}
	case KVType::INT32:
		{
			auto v = MakeValue(datatype, std::make_shared<int32_t>(ds->Read<int32_t>()), flagInfo);
			parent->AddProperty(name, *v);
			break;
		}
	case KVType::UINT32:
		{
			auto v = MakeValue(datatype, std::make_shared<uint32_t>(ds->Read<uint32_t>()), flagInfo);
			parent->AddProperty(name, *v);
			break;
		}
	case KVType::DOUBLE:
		{
			if(m_currentEightBytesOffset > 0)
				ds->SetOffset(m_currentEightBytesOffset);

			auto v = MakeValue(datatype, std::make_shared<double>(ds->Read<double>()), flagInfo);
			parent->AddProperty(name, *v);

			if(m_currentEightBytesOffset > 0) {
				m_currentEightBytesOffset = ds->GetOffset();
				ds->SetOffset(currentOffset);
			}

			break;
		}
	case KVType::DOUBLE_ZERO:
		{
			auto v = MakeValue(datatype, std::make_shared<double>(0.0), flagInfo);
			parent->AddProperty(name, *v);
			break;
		}
	case KVType::DOUBLE_ONE:
		{
			auto v = MakeValue(datatype, std::make_shared<double>(1.0), flagInfo);
			parent->AddProperty(name, *v);
			break;
		}
	case KVType::STRING:
		{
			auto id = ds->Read<int32_t>();
			auto v = MakeValue(datatype, std::make_shared<std::string>(id == -1 ? std::string {} : m_stringArray.at(id)), flagInfo);
			parent->AddProperty(name, *v);
			break;
		}
	case KVType::BINARY_BLOB:
		{
			auto length = ds->Read<int32_t>();

			if(m_currentBinaryBytesOffset > -1)
				ds->SetOffset(m_currentBinaryBytesOffset);

			auto data = std::make_shared<BinaryBlob>();
			data->resize(length);
			ds->Read(data->data(), length);
			value = data;
			auto v = MakeValue(datatype, data, flagInfo);
			parent->AddProperty(name, *v);

			if(m_currentBinaryBytesOffset > -1) {
				m_currentBinaryBytesOffset = ds->GetOffset();
				ds->SetOffset(currentOffset + 4);
			}

			break;
		}
	case KVType::ARRAY:
		{
			auto arrayLength = ds->Read<int32_t>();
			auto array = std::make_shared<KVObject>(name, true);
			for(auto i = decltype(arrayLength) {0u}; i < arrayLength; ++i)
				ParseBinaryKV3(ds, array, true);

			value = array;
			auto v = MakeValue(datatype, array, flagInfo);
			parent->AddProperty(name, *v);
			break;
		}
	case KVType::ARRAY_TYPED:
		{
			auto typeArrayLength = ds->Read<int32_t>();
			auto type = ReadType(ds);
			auto typedArray = std::make_shared<KVObject>(name, true);

			for(auto i = decltype(typeArrayLength) {0u}; i < typeArrayLength; ++i)
				ReadBinaryValue(name, type.first, type.second, ds, typedArray);

			value = typedArray;
			auto v = MakeValue(datatype, typedArray, flagInfo);
			parent->AddProperty(name, *v);
			break;
		}
	case KVType::OBJECT:
		{
			auto objectLength = ds->Read<int32_t>();
			auto newObject = std::make_shared<KVObject>(name, false);
			for(auto i = decltype(objectLength) {0u}; i < objectLength; ++i)
				ParseBinaryKV3(ds, newObject, false);

			value = newObject;
			if(parent == nullptr)
				parent = newObject;
			else {
				auto v = MakeValue(datatype, newObject, flagInfo);
				parent->AddProperty(name, *v);
			}

			break;
		}
	default:
		throw new std::runtime_error {"Unknown KVType " + std::to_string(umath::to_integral(datatype)) + " for field '" + name + "' on byte " + std::to_string(ds->GetOffset() - 1)};
	}

	return parent ? parent : nullptr;
}

std::shared_ptr<resource::KVObject> resource::BinaryKV3::ParseBinaryKV3(util::DataStream &ds, std::shared_ptr<KVObject> parent, bool inArray)
{
	std::string name {};
	if(!inArray) {
		auto stringID = ds->Read<int32_t>();
		name = (stringID == -1) ? "" : m_stringArray.at(stringID);
	}

	auto type = ReadType(ds);
	return ReadBinaryValue(name, type.first, type.second, ds, parent);
}
