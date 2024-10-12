module;

#include <string>

module source2;

using namespace source2;

resource::AnimationGroup::AnimationGroup(ResourceData &animationData) : m_data {animationData.GetData()} {}
resource::AnimationGroup::AnimationGroup(Resource &resource) : AnimationGroup {*static_cast<ResourceData *>(resource.FindBlock(BlockType::DATA))} {}
resource::IKeyValueCollection *resource::AnimationGroup::GetDecodeKey() { return m_data->FindSubCollection("m_decodeKey"); }
std::vector<std::string> resource::AnimationGroup::GetAnimationArray() const { return m_data->FindArrayValues<std::string>("m_localHAnimArray"); }
