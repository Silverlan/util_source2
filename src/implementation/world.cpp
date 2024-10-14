module;

#include <string>
#include <vector>

module source2;

import :resource_data;

using namespace source2;

resource::World::World(Resource &resource) : m_resource {resource} {}
void resource::World::LoadEntitiesFromLump(EntityLump &entityLump, const std::string &lumpName, Scene &scene)
{
	auto childEntities = entityLump.GetChildEntityNames();
	for(auto &childEntityName : childEntities) {
		auto lumpEntRes = m_resource.LoadResource(childEntityName + "_c");
		auto *entityLump = lumpEntRes ? dynamic_cast<EntityLump *>(lumpEntRes->FindBlock(BlockType::DATA)) : nullptr;
		if(entityLump == nullptr)
			continue;
		auto childName = entityLump->GetData()->FindValue<std::string>("m_name", "");
		LoadEntitiesFromLump(*entityLump, childName, scene);
	}

	for(auto &ent : entityLump.GetEntities())
		scene.Add(*ent);
}
std::vector<std::string> resource::World::GetEntityLumpNames() const
{
	auto *data = GetData().get();
	if(data == nullptr)
		return {};
	return data->FindArrayValues<std::string>("m_entityLumps");
}
std::vector<std::string> resource::World::GetWorldNodeNames() const
{
	auto *data = GetData().get();
	if(data == nullptr)
		return {};
	auto worldNodes = data->FindArrayValues<IKeyValueCollection *>("m_worldNodes");
	std::vector<std::string> worldNodeNames {};
	worldNodeNames.reserve(worldNodes.size());
	for(auto &node : worldNodes) {
		auto worldNodePrefix = IKeyValueCollection::FindValue<std::string>(*node, "m_worldNodePrefix");
		if(worldNodePrefix.has_value() == false)
			continue;
		worldNodeNames.push_back(*worldNodePrefix);
	}
	return worldNodeNames;
}
