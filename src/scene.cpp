#include "source2/resource_data.hpp"
#include "source2/resource.hpp"

using namespace source2;

#pragma optimize("",off)
void resource::Scene::Add(SceneNode &node)
{
	if(m_sceneNodes.size() == m_sceneNodes.capacity())
		m_sceneNodes.reserve(m_sceneNodes.size() *1.5f +100);
	m_sceneNodes.push_back(node.shared_from_this());
}
void resource::Scene::Add(Entity &ent)
{
	if(m_entities.size() == m_entities.capacity())
		m_entities.reserve(m_entities.size() *1.5f +100);
	m_entities.push_back(ent.shared_from_this());
}
const std::vector<std::shared_ptr<resource::SceneNode>> &resource::Scene::GetSceneNodes() const {return m_sceneNodes;}
const std::vector<std::shared_ptr<resource::Entity>> &resource::Scene::GetEntities() const {return m_entities;}
#pragma optimize("",on)
