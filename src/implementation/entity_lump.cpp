// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <unordered_map>

#include <vector>

#include <optional>
#include <string>
#include <memory>
#include <stdexcept>

module source2;

import :impl;
import :murmurhash;

using namespace source2;

std::optional<std::string> resource::EntityProperty::ToString() const
{
	switch(type) {
	case Type::Bool:
		return GetData<bool>(false) ? "1" : "0";
	case Type::Float:
		return std::to_string(GetData<float>(0.f));
	case Type::Color255:
		{
			auto col = GetData<std::array<uint8_t, 4>>({});
			return std::to_string(col.at(0)) + ' ' + std::to_string(col.at(1)) + ' ' + std::to_string(col.at(2)) + ' ' + std::to_string(col.at(3));
		}
	case Type::NodeId:
	case Type::Flags:
		return std::to_string(GetData<uint32_t>(0));
	case Type::Integer:
		return std::to_string(GetData<uint64_t>(0));
	case Type::Vector:
	case Type::Angle:
		{
			auto v = GetData<Vector3>({});
			return std::to_string(v.x) + ' ' + std::to_string(v.y) + ' ' + std::to_string(v.z);
		}
	case Type::String:
		return GetData<std::string>("");
	default:
		break;
	}
	return {};
}

///////////

std::shared_ptr<resource::Entity> resource::Entity::Create(const std::unordered_map<uint32_t, EntityProperty> &properties) { return std::shared_ptr<Entity> {new Entity {properties}}; }
resource::Entity::Entity(const std::unordered_map<uint32_t, EntityProperty> &properties) : m_properties {properties} {}
const std::unordered_map<uint32_t, resource::EntityProperty> &resource::Entity::GetProperties() const { return m_properties; }
std::unordered_map<std::string, std::string> resource::Entity::GetKeyValues() const
{
	std::unordered_map<std::string, std::string> keyValues {};
	for(auto &pair : m_properties) {
		auto key = impl::hash_to_keyvalue(pair.first);
		auto val = pair.second.ToString();
		if(key.has_value() == false || val.has_value() == false)
			continue; // Unknown keyvalue
		keyValues[*key] = *val;
	}
	return keyValues;
}
resource::EntityProperty *resource::Entity::FindProperty(const std::string &key)
{
	auto hash = source2::murmur2::hash(key, MURMUR2_SEED);
	auto it = m_properties.find(hash);
	return (it != m_properties.end()) ? &it->second : nullptr;
}

///////////

std::vector<std::string> resource::EntityLump::GetChildEntityNames() const
{
	auto *data = GetData().get();
	if(data == nullptr)
		return {};
	return data->FindArrayValues<std::string>("m_childLumps");
}
std::vector<std::shared_ptr<resource::Entity>> resource::EntityLump::GetEntities() const
{
	auto *data = GetData().get();
	if(data == nullptr)
		return {};
	std::vector<std::shared_ptr<resource::Entity>> ents {};
	auto entityKeyValues = data->FindArrayValues<IKeyValueCollection *>("m_entityKeyValues");
	ents.reserve(entityKeyValues.size());
	for(auto &kv : entityKeyValues) {
		auto *kvData = kv->FindBinaryBlob("m_keyValuesData");
		auto ent = ParseEntityProperties(*kvData);
		// TODO: m_connections
		if(ent == nullptr)
			continue;
		ents.push_back(ent);
	}
	return ents;
}
std::shared_ptr<resource::Entity> resource::EntityLump::ParseEntityProperties(const std::vector<uint8_t> &bytes) const
{
	util::DataStream ds {const_cast<uint8_t *>(bytes.data()), static_cast<uint32_t>(bytes.size())};
	ds->SetOffset(0);
	auto a = ds->Read<uint32_t>();
	if(a != 1)
		throw std::runtime_error {"First field in entity lump is not 1"};
	auto hashedFieldsCount = ds->Read<uint32_t>();
	auto stringFieldsCount = ds->Read<uint32_t>();
	std::unordered_map<uint32_t, EntityProperty> properties {};

	for(auto i = decltype(hashedFieldsCount) {0u}; i < hashedFieldsCount; ++i) {
		// murmur2 hashed field name (see EntityLumpKeyLookup)
		auto keyHash = ds->Read<uint32_t>();
		ReadTypedValue(ds, keyHash, {}, properties);
	}
	for(auto i = decltype(stringFieldsCount) {0u}; i < stringFieldsCount; ++i) {
		auto keyHash = ds->Read<uint32_t>();
		auto keyName = ds->ReadString();
		ReadTypedValue(ds, keyHash, keyName, properties);
	}
	return Entity::Create(properties);
}
void resource::EntityLump::ReadTypedValue(util::DataStream &ds, uint32_t keyHash, const std::optional<std::string> &keyName, std::unordered_map<uint32_t, EntityProperty> &properties) const
{
	auto type = ds->Read<EntityProperty::Type>();
	EntityProperty property {};
	property.type = type;
	property.name = keyName;
	switch(type) {
	case EntityProperty::Type::Bool:                              // boolean
		property.data = std::make_shared<bool>(ds->Read<bool>()); // 1
		break;
	case EntityProperty::Type::Float:                               // float
		property.data = std::make_shared<float>(ds->Read<float>()); // 4
		break;
	case EntityProperty::Type::Color255:                                                              // color255
		property.data = std::make_shared<std::array<uint8_t, 4>>(ds->Read<std::array<uint8_t, 4>>()); // 4
		break;
	case EntityProperty::Type::NodeId:                                    // node_id
	case EntityProperty::Type::Flags:                                     // flags
		property.data = std::make_shared<uint32_t>(ds->Read<uint32_t>()); // 4
		break;
	case EntityProperty::Type::Integer:                                   // integer
		property.data = std::make_shared<uint64_t>(ds->Read<uint64_t>()); // 8
		break;
	case EntityProperty::Type::Vector:                                  // vector
	case EntityProperty::Type::Angle:                                   // angle
		property.data = std::make_shared<Vector3>(ds->Read<Vector3>()); // 12
		break;
	case EntityProperty::Type::String:                                   // string
		property.data = std::make_shared<std::string>(ds->ReadString()); // null term variable
		break;
	default:
		throw std::runtime_error {"Unknown type " + std::to_string(umath::to_integral(type))};
	}
	properties.insert(std::make_pair(keyHash, property));
}
