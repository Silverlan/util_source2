#include "source2/resource_data.hpp"
#include "source2/resource.hpp"

using namespace source2;

std::shared_ptr<resource::Bone> resource::Bone::Create(const std::string &name,const std::vector<std::vector<int32_t>> &skinIndicesPerMesh,const Vector3 &position,const Quat &rotation)
{
	return std::shared_ptr<Bone>{new Bone{name,skinIndicesPerMesh,position,rotation}};
}
resource::Bone::Bone(const std::string &name,const std::vector<std::vector<int32_t>> &skinIndicesPerMesh,const Vector3 &position,const Quat &rotation)
	: m_name{name},m_skinIndicesPerMesh{skinIndicesPerMesh},m_position{position},m_rotation{rotation}
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
const std::vector<std::vector<int32_t>> &resource::Bone::GetSkinIndicesPerMesh() const {return m_skinIndicesPerMesh;}
const std::vector<int32_t> *resource::Bone::GetSkinIndices(uint32_t meshIdx) const
{
	return (meshIdx < m_skinIndicesPerMesh.size()) ? &m_skinIndicesPerMesh.at(meshIdx) : nullptr;
}
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
	auto remapTableStarts = IKeyValueCollection::FindArrayValues<int32_t>(modelData,"m_remappingTableStarts");

	std::vector<std::unordered_map<int32_t,std::vector<int32_t>>> remapTablePerMesh {};
	remapTablePerMesh.reserve(remapTableStarts.size());
	for(auto i=decltype(remapTableStarts.size()){0u};i<remapTableStarts.size();++i)
	{
		auto start = remapTableStarts.at(i);
		auto end = (i < remapTableStarts.size() -1) ? remapTableStarts.at(i +1) : remapTable.size();

		remapTablePerMesh.push_back({});
		auto &invMapTable = remapTablePerMesh.back();

		uint32_t idx = 0u;
		invMapTable.reserve(end -start);
		for(auto i=start;i<end;++i)
		{
			auto idxRemap = remapTable.at(i);
			auto it = invMapTable.find(idxRemap);
			if(it == invMapTable.end())
				it = invMapTable.insert(std::make_pair(idxRemap,std::vector<int32_t>{})).first;
			it->second.push_back(idx++);
		}
	}

	return std::shared_ptr<Skeleton>{new Skeleton{*const_cast<KVObject*>(*dataSkeleton),remapTablePerMesh}};
}
resource::Skeleton::Skeleton(IKeyValueCollection &skeletonData,const std::vector<std::unordered_map<int32_t,std::vector<int32_t>>> &invMapTablePerMesh)
{
	// std::cout<<"TODO!"<<std::endl;
	//auto *data = dynamic_cast<KVObject*>(&modelData);
	//auto skeleton = data->FindArrayValues<KVValueObject*>("m_modelSkeleton");
	//var remapTable = modelData.GetIntegerArray("m_remappingTable");
	//ConstructFromNTRO(modelData.GetSubCollection("m_modelSkeleton"), invMapTable);
	//std::cout<<"SKELETON: "<<&skeleton<<std::endl;
	//IKeyValueCollection::FindValue("m_modelSkeleton");
	ConstructFromNTRO(skeletonData,invMapTablePerMesh);
}
void resource::Skeleton::ConstructFromNTRO(IKeyValueCollection &skeletonData,const std::vector<std::unordered_map<int32_t,std::vector<int32_t>>> &invMapTablePerMesh)
{
	auto boneNames = skeletonData.FindArrayValues<std::string>(skeletonData,"m_boneName");
	auto boneParents = skeletonData.FindArrayValues<int32_t>(skeletonData,"m_nParent");
	auto boneFlags = skeletonData.FindArrayValues<int32_t>(skeletonData,"m_nFlag");
	auto bonePositions = skeletonData.FindArrayValues<Vector3>(skeletonData,"m_bonePosParent");
	auto boneRotations = skeletonData.FindArrayValues<Quat>(skeletonData,"m_boneRotParent");


	//std::cout<<"Bone names: "<<boneNames.size()<<std::endl;
	auto &bones = m_boneList;
	bones.reserve(boneNames.size());
	for(auto i=decltype(boneNames.size()){0u};i<boneNames.size();++i)
	{
		if((boneFlags.at(i) & BoneUsedByVertexLod0) != BoneUsedByVertexLod0)
			continue;
		auto &boneName = boneNames.at(i);
		auto &pos = bonePositions.at(i);
		auto &rot = boneRotations.at(i);
		
		std::vector<std::vector<int32_t>> skinIndicesPerMesh {};
		skinIndicesPerMesh.reserve(invMapTablePerMesh.size());
		for(auto &invMapTable : invMapTablePerMesh)
		{
			auto it = invMapTable.find(i);
			skinIndicesPerMesh.push_back((it != invMapTable.end()) ? it->second : std::vector<int32_t>{});
		}
		bones.push_back(Bone::Create(boneName,skinIndicesPerMesh,pos,rot));
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
