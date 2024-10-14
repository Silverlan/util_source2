module;

#include <string>
#include <memory>
#include <algorithm>
#include <mathutil/uvec.h>

module source2;

using namespace source2;

std::shared_ptr<resource::Bone> resource::Bone::Create(const std::string &name, const Vector3 &position, const Quat &rotation) { return std::shared_ptr<Bone> {new Bone {name, position, rotation}}; }
resource::Bone::Bone(const std::string &name, const Vector3 &position, const Quat &rotation) : m_name {name}, m_position {position}, m_rotation {rotation} {}
void resource::Bone::AddChild(Bone &bone)
{
	m_children.push_back(bone.shared_from_this());
	bone.m_parent = shared_from_this();
}
void resource::Bone::SetParent(Bone &parent)
{
	if(m_parent.expired() == false) {
		auto curParent = m_parent.lock();
		auto it = std::find_if(curParent->m_children.begin(), curParent->m_children.end(), [this](const std::shared_ptr<Bone> &boneOther) { return boneOther.get() == this; });
		if(it != curParent->m_children.end())
			curParent->m_children.erase(it);
	}
	parent.AddChild(*this);
}
const std::string &resource::Bone::GetName() const { return m_name; }
resource::Bone *resource::Bone::GetParent() const { return m_parent.lock().get(); }
const std::vector<std::shared_ptr<resource::Bone>> &resource::Bone::GetChildren() const { return m_children; }
const Vector3 &resource::Bone::GetPosition() const { return m_position; }
const Quat &resource::Bone::GetRotation() const { return m_rotation; }

////////////

std::shared_ptr<resource::Skeleton> resource::Skeleton::Create(IKeyValueCollection &modelData)
{
	auto *data = dynamic_cast<IKeyValueCollection *>(&modelData);
	auto dataSkeleton = data->FindValue<IKeyValueCollection *>("m_modelSkeleton");
	if(dataSkeleton.has_value() == false)
		return nullptr;
	// Get the remap table and invert it for our construction method
	auto remapTable = IKeyValueCollection::FindArrayValues<int32_t>(modelData, "m_remappingTable");
	auto remapTableStarts = IKeyValueCollection::FindArrayValues<int32_t>(modelData, "m_remappingTableStarts");

	return std::shared_ptr<Skeleton> {new Skeleton {*const_cast<IKeyValueCollection *>(*dataSkeleton), remapTable, remapTableStarts}};
}
resource::Skeleton::Skeleton(IKeyValueCollection &skeletonData, const std::vector<int32_t> &remappingTable, const std::vector<int32_t> &remappingTableStarts) : m_remappingTable {remappingTable}, m_remappingTableStarts {remappingTableStarts}
{
	// std::cout<<"TODO!"<<std::endl;
	//auto *data = dynamic_cast<KVObject*>(&modelData);
	//auto skeleton = data->FindArrayValues<KVValueObject*>("m_modelSkeleton");
	//var remapTable = modelData.GetIntegerArray("m_remappingTable");
	//ConstructFromNTRO(modelData.GetSubCollection("m_modelSkeleton"), invMapTable);
	//std::cout<<"SKELETON: "<<&skeleton<<std::endl;
	//IKeyValueCollection::FindValue("m_modelSkeleton");
	ConstructFromNTRO(skeletonData);
}
void resource::Skeleton::ConstructFromNTRO(IKeyValueCollection &skeletonData)
{
	auto boneNames = skeletonData.FindArrayValues<std::string>(skeletonData, "m_boneName");
	auto boneParents = skeletonData.FindArrayValues<int32_t>(skeletonData, "m_nParent");
	auto boneFlags = skeletonData.FindArrayValues<int32_t>(skeletonData, "m_nFlag");
	auto bonePositions = skeletonData.FindArrayValues<Vector3>(skeletonData, "m_bonePosParent");
	auto boneRotations = skeletonData.FindArrayValues<Quat>(skeletonData, "m_boneRotParent");

	//std::cout<<"Bone names: "<<boneNames.size()<<std::endl;
	auto &bones = m_boneList;
	bones.reserve(boneNames.size());
	for(auto i = decltype(boneNames.size()) {0u}; i < boneNames.size(); ++i) {
		//if((boneFlags.at(i) & BoneUsedByVertexLod0) != BoneUsedByVertexLod0)
		//	continue;
		auto &boneName = boneNames.at(i);
		auto &pos = bonePositions.at(i);
		auto &rot = boneRotations.at(i);
		bones.push_back(Bone::Create(boneName, pos, rot));
	}

	for(auto i = decltype(bones.size()) {0u}; i < bones.size(); ++i) {
		auto &bone = bones.at(i);
		auto parentIdx = boneParents.at(i);
		if(parentIdx == -1) {
			m_rootBones.push_back(bone);
			continue;
		}
		bone->SetParent(*bones.at(parentIdx));
	}
}
const std::vector<std::shared_ptr<resource::Bone>> &resource::Skeleton::GetRootBones() const { return m_rootBones; }
const std::vector<std::shared_ptr<resource::Bone>> &resource::Skeleton::GetBoneList() const { return m_boneList; }

const std::vector<int32_t> &resource::Skeleton::GetRemappingTable() const { return m_remappingTable; }
const std::vector<int32_t> &resource::Skeleton::GetRemappingTableStarts() const { return m_remappingTableStarts; }
