#include "source2/resource_data.hpp"
#include "source2/resource.hpp"

using namespace source2;

#pragma optimize("",off)
std::shared_ptr<resource::Bone> resource::Bone::Create(const std::string &name,const std::vector<int32_t> &skinIndices,const Vector3 &position,const Quat &rotation)
{
	return std::shared_ptr<Bone>{new Bone{name,skinIndices,position,rotation}};
}
resource::Bone::Bone(const std::string &name,const std::vector<int32_t> &skinIndices,const Vector3 &position,const Quat &rotation)
	: m_name{name},m_skinIndices{skinIndices},m_position{position},m_rotation{rotation}
{}
void resource::Bone::AddChild(Bone &bone)
{
	m_children.push_back(bone.shared_from_this());
	bone.m_parent = shared_from_this();
}
void resource::Bone::SetParent(Bone &parent)
{
	if(m_parent.expired() == false)
	{
		auto curParent = m_parent.lock();
		auto it = std::find_if(curParent->m_children.begin(),curParent->m_children.end(),[this](const std::shared_ptr<Bone> &boneOther) {
			return boneOther.get() == this;
		});
		if(it != curParent->m_children.end())
			curParent->m_children.erase(it);
	}
	parent.AddChild(*this);
}
const std::string &resource::Bone::GetName() const {return m_name;}
resource::Bone *resource::Bone::GetParent() const {return m_parent.lock().get();}
const std::vector<std::shared_ptr<resource::Bone>> &resource::Bone::GetChildren() const {return m_children;}
const std::vector<int32_t> &resource::Bone::GetSkinIndices() const {return m_skinIndices;}
const Vector3 &resource::Bone::GetPosition() const {return m_position;}
const Quat &resource::Bone::GetRotation() const {return m_rotation;}

////////////

std::shared_ptr<resource::Skeleton> resource::Skeleton::Create(IKeyValueCollection &modelData)
{
	auto *data = dynamic_cast<KVObject*>(&modelData);
	auto dataSkeleton = data->FindValue<const KVObject*>("m_modelSkeleton");
	if(dataSkeleton.has_value() == false)
		return nullptr;
	// Get the remap table and invert it for our construction method
	auto remapTable = IKeyValueCollection::FindArrayValues<int32_t>(modelData,"m_remappingTable");
	int32_t start = 0;
	auto end = remapTable.size();

	auto remapTableStarts = IKeyValueCollection::FindArrayValues<int32_t>(modelData,"m_remappingTableStarts");

	// we only use lod 1
	if(remapTableStarts.size() > 1)
	{
		start = remapTableStarts.at(0);
		end = remapTableStarts.at(1);
	}

	std::unordered_map<int32_t,std::vector<int32_t>> invMapTable {};
	invMapTable.reserve(end -start);
	uint32_t idx = 0u;
	for(auto i=start;i<end;++i)
	{
		auto idxRemap = remapTable.at(i);
		auto it = invMapTable.find(idxRemap);
		if(it == invMapTable.end())
			it = invMapTable.insert(std::make_pair(idxRemap,std::vector<int32_t>{})).first;
		it->second.push_back(idx++);
	}
	return std::shared_ptr<Skeleton>{new Skeleton{*const_cast<KVObject*>(*dataSkeleton),invMapTable}};
}
resource::Skeleton::Skeleton(IKeyValueCollection &skeletonData,const std::unordered_map<int32_t,std::vector<int32_t>> &invMapTable)
{
	std::cout<<"TODO!"<<std::endl;
	//auto *data = dynamic_cast<KVObject*>(&modelData);
	//auto skeleton = data->FindArrayValues<KVValueObject*>("m_modelSkeleton");
	//var remapTable = modelData.GetIntegerArray("m_remappingTable");
	//ConstructFromNTRO(modelData.GetSubCollection("m_modelSkeleton"), invMapTable);
	//std::cout<<"SKELETON: "<<&skeleton<<std::endl;
	//IKeyValueCollection::FindValue("m_modelSkeleton");
	ConstructFromNTRO(skeletonData,invMapTable);
}
void resource::Skeleton::ConstructFromNTRO(IKeyValueCollection &skeletonData,const std::unordered_map<int32_t,std::vector<int32_t>> &invMapTable)
{
	auto boneNames = skeletonData.FindArrayValues<std::string>(skeletonData,"m_boneName");
	auto boneParents = skeletonData.FindArrayValues<int32_t>(skeletonData,"m_nParent");
	auto boneFlags = skeletonData.FindArrayValues<int32_t>(skeletonData,"m_nFlag");
	auto bonePositions = skeletonData.FindArrayValues<Vector3>(skeletonData,"m_bonePosParent");
	auto boneRotations = skeletonData.FindArrayValues<Quat>(skeletonData,"m_boneRotParent");


	std::cout<<"Bone names: "<<boneNames.size()<<std::endl;
	auto &bones = m_boneList;
	bones.reserve(boneNames.size());
	for(auto i=decltype(boneNames.size()){0u};i<boneNames.size();++i)
	{
		if((boneFlags.at(i) & BoneUsedByVertexLod0) != BoneUsedByVertexLod0)
			continue;
		auto &boneName = boneNames.at(i);
		auto &pos = bonePositions.at(i);
		auto &rot = boneRotations.at(i);
		
		auto it = invMapTable.find(i);
		bones.push_back(Bone::Create(boneName,(it != invMapTable.end()) ? it->second : std::vector<int32_t>{},pos,rot));
	}

	for(auto i=decltype(bones.size()){0u};i<bones.size();++i)
	{
		auto &bone = bones.at(i);
		auto parentIdx = boneParents.at(i);
		if(parentIdx == -1)
		{
			m_rootBones.push_back(bone);
			continue;
		}
		bone->SetParent(*bones.at(parentIdx));
	}
}
const std::vector<std::shared_ptr<resource::Bone>> &resource::Skeleton::GetRootBones() const {return m_rootBones;}
const std::vector<std::shared_ptr<resource::Bone>> &resource::Skeleton::GetBoneList() const {return m_boneList;}
#pragma optimize("",on)
