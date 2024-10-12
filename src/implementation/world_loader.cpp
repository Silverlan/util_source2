module;

#include <string>
#include <memory>
#include <mathutil/umat.h>

module source2;

using namespace source2;

std::shared_ptr<resource::Scene> resource::World::Load()
{
	auto scene = std::make_shared<Scene>();
	for(auto &worldNodeName : GetWorldNodeNames()) {
		auto worldNodeRes = m_resource.LoadResource(worldNodeName + ".vwnod_c");
		auto *worldNode = worldNodeRes ? dynamic_cast<WorldNode *>(worldNodeRes->FindBlock(BlockType::DATA)) : nullptr;
		if(worldNode == nullptr)
			continue;
		worldNode->Load(*worldNodeRes, *scene);
	}

	for(auto &lumpName : GetEntityLumpNames()) {
		auto lumpNodeRes = m_resource.LoadResource(lumpName + "_c");
		auto *entityLump = lumpNodeRes ? dynamic_cast<EntityLump *>(lumpNodeRes->FindBlock(BlockType::DATA)) : nullptr;
		if(entityLump == nullptr)
			continue;
		LoadEntitiesFromLump(*entityLump, "world_layer_base", *scene);
	}
	return scene;
}

void resource::WorldNode::WorldNode::Load(Resource &resource, Scene &scene)
{
	auto data = GetData();
	if(data == nullptr)
		return;
	auto worldLayers = data->FindArrayValues<std::string>("m_layerNames");
	auto sceneObjectLayerIndices = data->FindArrayValues<int32_t>("m_sceneObjectLayerIndices");
	auto sceneObjects = data->FindArrayValues<resource::IKeyValueCollection *>("m_sceneObjects");
	uint32_t i = 0;
	for(auto *sceneObject : sceneObjects) {
		int32_t layerIndex = (i < sceneObjectLayerIndices.size()) ? sceneObjectLayerIndices.at(i) : -1;
		++i;

		// sceneObject is SceneObject_t
		auto renderableModel = sceneObject->FindValue<std::string>("m_renderableModel");
		auto matrix = sceneObject->FindValue<Mat4>("m_vTransform", Mat4 {1.f}); // TODO

		auto tintColor = sceneObject->FindValue<Vector4>("m_vTintColor", Vector4 {1.f, 1.f, 1.f, 1.f});
		if(tintColor.w == 0) {
			// Ignoring tintColor, it will fuck things up.
			tintColor = {1.f, 1.f, 1.f, 1.f};
		}

		if(renderableModel.has_value()) {
			auto newResource = resource.LoadResource(*renderableModel + "_c");
			auto *model = newResource ? dynamic_cast<resource::Model *>(newResource->FindBlock(BlockType::DATA)) : nullptr;
			if(model == nullptr)
				continue;

			// TODO
			auto modelNode = std::make_shared<resource::ModelSceneNode>(scene, newResource, *model);
			modelNode->SetTransform(matrix);
			modelNode->SetLayerName(worldLayers.at(layerIndex));
			modelNode->SetTint(tintColor);

			scene.Add(*modelNode);
		}

		auto renderable = sceneObject->FindValue<std::string>("m_renderable");
		if(renderable.has_value()) {
			auto newResource = resource.LoadResource(*renderable);

			if(newResource == nullptr)
				continue;

			auto mesh = resource::Mesh::Create(*newResource);
			auto meshSceneNode = std::make_shared<resource::MeshSceneNode>(scene, newResource, *mesh);
			meshSceneNode->SetTransform(matrix);
			meshSceneNode->SetTint(tintColor);
			meshSceneNode->SetLayerName(worldLayers.at(layerIndex));

			scene.Add(*meshSceneNode);
		}
	}
}
