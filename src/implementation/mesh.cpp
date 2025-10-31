// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;


module source2;

using namespace source2;

std::shared_ptr<resource::Mesh> resource::Mesh::Create(ResourceData &data, VBIB &vbib, int64_t meshIdx) { return std::shared_ptr<Mesh> {new Mesh {data, vbib, meshIdx}}; }

std::shared_ptr<resource::Mesh> resource::Mesh::Create(Resource &resource)
{
	auto *data = dynamic_cast<ResourceData *>(resource.FindBlock(BlockType::DATA));
	auto *vbib = dynamic_cast<VBIB *>(resource.FindBlock(BlockType::VBIB));
	if(data == nullptr || vbib == nullptr)
		return nullptr;
	return Create(*data, *vbib);
}

resource::Mesh::Mesh(ResourceData &data, VBIB &vbib, int64_t meshIdx) : m_resourceData {std::static_pointer_cast<ResourceData>(data.shared_from_this())}, m_vbib {std::static_pointer_cast<VBIB>(vbib.shared_from_this())}, m_meshIdx {meshIdx} {}
int64_t resource::Mesh::GetMeshIndex() const { return m_meshIdx; }
const std::pair<Vector3, Vector3> &resource::Mesh::GetBounds() const { return m_bounds; }
std::shared_ptr<resource::VBIB> resource::Mesh::GetVBIB() const { return m_vbib; }
std::shared_ptr<resource::ResourceData> resource::Mesh::GetResourceData() const { return m_resourceData; }
void resource::Mesh::UpdateBounds()
{
#if 0
	auto *sceneObjects = GetData()->FindValue("m_sceneObjects");


	var sceneObjects = GetData().GetArray("m_sceneObjects");
	if (sceneObjects.Length == 0)
	{
		MinBounds = MaxBounds = new Vector3(0, 0, 0);
		return;
	}

	var minBounds = sceneObjects[0].GetSubCollection("m_vMinBounds").ToVector3();
	var maxBounds = sceneObjects[0].GetSubCollection("m_vMaxBounds").ToVector3();

	for (int i = 1; i < sceneObjects.Length; ++i)
	{
		var localMin = sceneObjects[i].GetSubCollection("m_vMinBounds").ToVector3();
		var localMax = sceneObjects[i].GetSubCollection("m_vMaxBounds").ToVector3();

		minBounds.X = System.Math.Min(minBounds.X, localMin.X);
		minBounds.Y = System.Math.Min(minBounds.Y, localMin.Y);
		minBounds.Z = System.Math.Min(minBounds.Z, localMin.Z);
		maxBounds.X = System.Math.Max(maxBounds.X, localMax.X);
		maxBounds.Y = System.Math.Max(maxBounds.Y, localMax.Y);
		maxBounds.Z = System.Math.Max(maxBounds.Z, localMax.Z);
	}

	MinBounds = minBounds;
	MaxBounds = maxBounds;
}











	std::vector<std::shared_ptr<resource::Mesh>> meshes {};
	auto *blockCtrl = dynamic_cast<BinaryKV3*>(m_resource.FindBlock(BlockType::CTRL));
	if(blockCtrl == nullptr)
		return meshes;
	auto *embeddedMeshes = blockCtrl->GetData()->FindValue("embedded_meshes");

	return meshes;
#endif
	/*var meshes = new List<Mesh>();

	if (Resource.ContainsBlockType(BlockType.CTRL))
	{
	var ctrl = Resource.GetBlockByType(BlockType.CTRL) as BinaryKV3;
	var embeddedMeshes = ctrl.Data.GetArray("embedded_meshes");

	foreach (var embeddedMesh in embeddedMeshes)
	{
	var dataBlockIndex = (int)embeddedMesh.GetIntegerProperty("data_block");
	var vbibBlockIndex = (int)embeddedMesh.GetIntegerProperty("vbib_block");

	meshes.Add(new Mesh(Resource.GetBlockByIndex(dataBlockIndex) as ResourceData, Resource.GetBlockByIndex(vbibBlockIndex) as VBIB));
	}
	}

	return meshes;*/
}
