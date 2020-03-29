#include "source2/resource_data.hpp"
#include "source2/resource.hpp"

using namespace source2;

resource::ModelSceneNode::ModelSceneNode(Scene &scene,std::shared_ptr<Resource> resource,Model &mdl)
	: SceneNode{scene,resource},m_model{std::static_pointer_cast<Model>(mdl.shared_from_this())}
{}
const std::shared_ptr<resource::Model> &resource::ModelSceneNode::GetModel() const {return m_model;}
