// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;


module source2;

using namespace source2;

resource::MeshSceneNode::MeshSceneNode(Scene &scene, std::shared_ptr<Resource> resource, Mesh &mesh) : SceneNode {scene, resource}, m_mesh {mesh.shared_from_this()} {}
const std::shared_ptr<resource::Mesh> &resource::MeshSceneNode::GetMesh() const { return m_mesh; }
