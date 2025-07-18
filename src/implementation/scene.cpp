// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <vector>
#include <memory>

module source2;

using namespace source2;

void resource::Scene::Add(SceneNode &node)
{
	if(m_sceneNodes.size() == m_sceneNodes.capacity())
		m_sceneNodes.reserve(m_sceneNodes.size() * 1.5f + 100);
	m_sceneNodes.push_back(node.shared_from_this());
}
void resource::Scene::Add(Entity &ent)
{
	if(m_entities.size() == m_entities.capacity())
		m_entities.reserve(m_entities.size() * 1.5f + 100);
	m_entities.push_back(ent.shared_from_this());
}
const std::vector<std::shared_ptr<resource::SceneNode>> &resource::Scene::GetSceneNodes() const { return m_sceneNodes; }
const std::vector<std::shared_ptr<resource::Entity>> &resource::Scene::GetEntities() const { return m_entities; }
