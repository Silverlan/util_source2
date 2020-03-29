#include "source2/resource_data.hpp"

using namespace source2;

#pragma optimize("",off)
resource::MeshSceneNode::MeshSceneNode(Scene &scene,std::shared_ptr<Resource> resource,Mesh &mesh)
	: SceneNode{scene,resource},m_mesh{mesh.shared_from_this()}
{}
const std::shared_ptr<resource::Mesh> &resource::MeshSceneNode::GetMesh() const {return m_mesh;}
#pragma optimize("",on)
