module;

#include <string>
#include <memory>

module source2;

using namespace source2;

resource::Skin::Skin(const std::string &name, std::vector<std::string> &&materials) : m_name {name}, m_materials {std::move(materials)} {}
const std::string &resource::Skin::GetName() const { return m_name; }
const std::vector<std::string> &resource::Skin::GetMaterials() const { return m_materials; }

///////////////

resource::Model::Model(Resource &resource) : m_resource {resource} {}
std::string resource::Model::GetName() const
{
	auto *data = GetData().get();
	if(data == nullptr)
		return "";
	auto val = data->FindValue<std::string>("m_name");
	return val.has_value() ? *val : "";
}
std::vector<resource::Skin> resource::Model::GetSkins()
{
	auto *data = GetData().get();

#if 0
	{
		auto *data2 = dynamic_cast<NTROStruct*>(GetData().get());
		if(data2)
		{
			auto val = data2->FindValue<NTROArray>("m_materialGroups");
			if(val.has_value())
			{
				for(auto &v : val->GetContents())
				{
					//v->type;	
				}
			}
			//data2->FindValue
		}
	}
#endif
	if(data == nullptr)
		return {};
	std::vector<resource::Skin> skins {};
	auto matGroups = data->FindArrayValues<IKeyValueCollection *>("m_materialGroups");
	for(auto *matGroup : matGroups) {
		auto name = IKeyValueCollection::FindValue<std::string>(*matGroup, "m_name");
		auto materials = IKeyValueCollection::FindArrayValues<std::string>(*matGroup, "m_materials");
		if(name.has_value() == false)
			continue;
		skins.push_back(Skin {*name, std::move(materials)});
	}
	return skins;
}
std::shared_ptr<resource::Skeleton> resource::Model::GetSkeleton() const { return Skeleton::Create(*const_cast<Model *>(this)->GetData()); }
std::vector<std::shared_ptr<resource::Mesh>> resource::Model::GetEmbeddedMeshes() const
{
	std::vector<std::shared_ptr<resource::Mesh>> meshes {};
	auto *blockCtrl = dynamic_cast<BinaryKV3 *>(m_resource.FindBlock(BlockType::CTRL));
	if(blockCtrl == nullptr)
		return meshes;
	auto embeddedMeshes = blockCtrl->GetData()->FindArrayValues<IKeyValueCollection *>("embedded_meshes");
	for(auto *embeddedMesh : embeddedMeshes) {
		auto dataBlockIndex = IKeyValueCollection::FindValue<int32_t>(*embeddedMesh, "data_block");
		auto vbibBlockIndex = IKeyValueCollection::FindValue<int32_t>(*embeddedMesh, "vbib_block");
		auto *dataBlock = dataBlockIndex.has_value() ? dynamic_cast<source2::resource::ResourceData *>(m_resource.GetBlock(*dataBlockIndex).get()) : nullptr;
		auto *vbibBlock = vbibBlockIndex.has_value() ? dynamic_cast<source2::resource::VBIB *>(m_resource.GetBlock(*vbibBlockIndex).get()) : nullptr;
		if(!dataBlock || !vbibBlock)
			continue;
		auto meshIndex = embeddedMesh->FindValue<int64_t>("mesh_index", -1);
		auto mesh = Mesh::Create(*dataBlock, *vbibBlock, meshIndex);
		meshes.push_back(mesh);
	}
	return meshes;
}
std::vector<std::string> resource::Model::GetReferencedMeshNames()
{
	auto *data = GetData().get();
	return data ? data->FindArrayValues<std::string>("m_refMeshes") : std::vector<std::string> {};
}
void resource::Model::GetReferencedAnimationGroupNames() {}
std::vector<std::shared_ptr<resource::Animation>> resource::Model::GetEmbeddedAnimations(Resource &resource)
{
	std::vector<std::shared_ptr<resource::Animation>> animations {};
	auto *blockCtrl = dynamic_cast<BinaryKV3 *>(resource.FindBlock(BlockType::CTRL));
	if(blockCtrl == nullptr)
		return {};
	auto *embeddedAnimations = blockCtrl->GetData()->FindSubCollection("embedded_animation");
	if(embeddedAnimations == nullptr)
		return {};
	auto groupDataBlockIndex = embeddedAnimations->FindValue<int32_t>("group_data_block");
	auto animDataBlockIndex = embeddedAnimations->FindValue<int32_t>("anim_data_block");
	if(groupDataBlockIndex.has_value() == false || animDataBlockIndex.has_value() == false)
		return {};
	auto *block = dynamic_cast<ResourceData *>(resource.GetBlock(*groupDataBlockIndex).get());
	if(block == nullptr)
		return {};
	block->GetData();
	auto animGroup = std::make_shared<AnimationGroup>(*block);
	auto decodeKey = animGroup->GetDecodeKey();

	auto &animDataBlock = *dynamic_cast<ResourceData *>(resource.GetBlock(*animDataBlockIndex).get());
	return Animation::CreateAnimations(*animDataBlock.GetData(), *decodeKey);
}
void resource::Model::GetMeshGroups() {}
void resource::Model::GetDefaultMeshGroups() {}
void resource::Model::GetActiveMeshMaskForGroup(const std::string &groupName) {}
