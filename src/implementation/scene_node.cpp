module;

#include <string>
#include <memory>
#include <mathutil/uvec.h>

module source2;

using namespace source2;

resource::SceneNode::SceneNode(Scene &scene, std::shared_ptr<Resource> resource) : m_scene {scene}, m_resource {resource} {}
void resource::SceneNode::SetLayerName(const std::string &layerName) { m_layerName = layerName; }
void resource::SceneNode::SetTint(const Vector4 &tint) { m_tint = tint; }
void resource::SceneNode::SetLayerEnabled(bool enabled) { m_layerEnabled = enabled; }
const Mat4 &resource::SceneNode::GetTransform() const { return m_transform; }
void resource::SceneNode::SetTransform(const Mat4 &transform) { m_transform = transform; }
const std::shared_ptr<resource::Resource> &resource::SceneNode::GetResource() const { return m_resource; }
