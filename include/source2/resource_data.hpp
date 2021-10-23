/*
MIT License

Copyright (c) 2021 Silverlan
Copyright (c) 2015 Steam Database

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef __SOURCE2_RESOURCE_DATA_HPP__
#define __SOURCE2_RESOURCE_DATA_HPP__

#include "block.hpp"
#include "resource_edit_info.hpp"
#include <string>
#include <vector>
#include <optional>
#include <unordered_map>
#include <mathutil/uvec.h>
#include <mathutil/umat.h>
#include <sharedutils/util_string.h>

#undef GetObject

class DataStream;
namespace util {using GUID = std::array<uint8_t,16>;};
namespace source2::resource
{
	class IKeyValueCollection;
	class NTROStruct;
	class KVObject;
	class ResourceData;
	template<typename T0,typename T1>
		std::optional<T1> cast_to_type(const T0 &v);

	class ResourceData
		: public Block
	{
	public:
		virtual BlockType GetType() const override;
		virtual void Read(const Resource &resource,std::shared_ptr<VFilePtrInternal> f) override;
		virtual void DebugPrint(std::stringstream &ss,const std::string &t="") const;
		IKeyValueCollection *GetData();
	};

	struct NTROValue
		: public std::enable_shared_from_this<NTROValue>
	{
		NTROValue(DataType type=DataType::Unknown,bool pointer=false);
		DataType type = DataType::Unknown;
		bool pointer = false;
		virtual void DebugPrint(std::stringstream &ss,const std::string &t="") const=0;
	};

	using BinaryBlob = std::vector<uint8_t>;
	struct NTROArray
		: public NTROValue
	{
	public:
		NTROArray(DataType type,uint32_t count,bool pointer=false,bool isIndirection=false);
		const std::vector<std::shared_ptr<NTROValue>> &GetContents() const;
		std::vector<std::shared_ptr<NTROValue>> &GetContents();
		bool IsIndirection() const;
		virtual void DebugPrint(std::stringstream &ss,const std::string &t) const override;

		BinaryBlob &InitBinaryBlob();
	private:
		std::vector<std::shared_ptr<NTROValue>> m_contents = {};
		BinaryBlob m_binaryBlob = {};
		bool m_bIsIndirection = false;
	};

	template<typename T>
		struct TNTROValue
			: public NTROValue
	{
		TNTROValue(DataType type,const T &value,bool pointer=false);
		virtual void DebugPrint(std::stringstream &ss,const std::string &t) const override;
		T value;
	};

	class IKeyValueCollection
		: public std::enable_shared_from_this<IKeyValueCollection>
	{
	public:
		IKeyValueCollection()=default;
		virtual ~IKeyValueCollection()=default;

		IKeyValueCollection *FindSubCollection(const std::string &key);
		BinaryBlob *FindBinaryBlob(const std::string &key);

		template<typename T>
			T FindValue(const std::string &key,const T &def);
		template<typename T>
			std::optional<T> FindValue(const std::string &key);
		template<typename T>
			std::optional<T> FindValue(const std::string &key) const;
		template<typename T>
			std::vector<T> FindArrayValues(const std::string &key);


		template<typename T>
			static std::optional<T> FindValue(IKeyValueCollection &collection,const std::string &key);
		template<typename T>
			static std::optional<T> FindValue(const IKeyValueCollection &collection,const std::string &key);
		template<typename T>
			static std::vector<T> FindArrayValues(IKeyValueCollection &collection,const std::string &key);
	};

	template<typename T>
		std::optional<T> cast_to_type(NTROValue &v0);

	class NTROStruct
		: public IKeyValueCollection
	{
	public:
		NTROStruct(const std::string &name);
		NTROStruct(const std::vector<std::shared_ptr<NTROValue>> &values);
		const std::unordered_map<std::string,std::shared_ptr<NTROValue>> &GetContents() const;
		BinaryBlob *FindBinaryBlob(const std::string &key);
		NTROValue *FindValue(const std::string &key);
		const NTROValue *FindValue(const std::string &key) const;
		NTROArray *FindArray(const std::string &key);
		const NTROArray *FindArray(const std::string &key) const;
		void Add(const std::string &id,NTROValue &val);
		void DebugPrint(std::stringstream &ss,const std::string &t="") const;

		template<typename T>
			std::vector<T> FindArrayValues(const std::string &key)
		{
			auto *array = FindArray(key);
			if(array == nullptr)
				return {};
			auto &contents = array->GetContents();
			auto n = contents.size();
			std::vector<T> arrayElements {};
			arrayElements.reserve(n);
			for(auto &val : contents)
			{
				auto o = cast_to_type<T>(*val);
				if(o.has_value() == false)
					continue;
				arrayElements.push_back(*o);
			}
			return arrayElements;
		}

		template<typename T>
			std::optional<T> FindValue(const std::string &key)
		{
			auto *val = FindValue(key);
			if(val == nullptr)
				return {};
			if constexpr(std::is_same_v<T,std::shared_ptr<NTROArray>>)
			{
				//if(val->type != DataType::Struct)
				//	return {};
				return std::static_pointer_cast<NTROArray>(static_cast<NTROArray*>(val)->shared_from_this());
			}
			else if constexpr(std::is_same_v<T,NTROStruct*>)
			{
				if(
					val->type != DataType::Vector && val->type != DataType::Quaternion && val->type != DataType::Color && val->type != DataType::Fltx4 &&
					val->type != DataType::Vector4D && val->type != DataType::Vector4D_44 && val->type != DataType::String4 && val->type != DataType::String &&
					val->type != DataType::Matrix2x4 && val->type != DataType::Matrix3x4 && val->type != DataType::Matrix3x4a && val->type != DataType::CTransform
				)
					return {};
				return std::optional<NTROStruct*>{&dynamic_cast<TNTROValue<NTROStruct>*>(val)->value};
			}
			else
			{
				// Only arithmetic types and string remaining
				return cast_to_type<T>(*val);
			}
			return {};
		}
	private:
		std::string m_name;
		std::unordered_map<std::string,std::shared_ptr<NTROValue>> m_contents;
	};

	class NTRO
		: public ResourceData
	{
	public:
		virtual void Read(const Resource &resource,std::shared_ptr<VFilePtrInternal> f) override;
		virtual void DebugPrint(std::stringstream &ss,const std::string &t="") const;
		const std::shared_ptr<NTROStruct> &GetOutput() const;
		void SetStructName(const std::string &structName);
	private:
		std::shared_ptr<NTROStruct> m_output = nullptr;
		std::string m_structName;
		std::shared_ptr<NTROStruct> ReadStructure(
			const Resource &resource,const ResourceIntrospectionManifest::ResourceDiskStruct &refStruct,int64_t startingOffset,std::shared_ptr<VFilePtrInternal> f
		);
		void ReadFieldIntrospection(
			const Resource &resource,const ResourceIntrospectionManifest::ResourceDiskStruct::Field &field,NTROStruct &structEntry,std::shared_ptr<VFilePtrInternal> f
		);
		std::shared_ptr<NTROValue> ReadField(
			const Resource &resource,const ResourceIntrospectionManifest::ResourceDiskStruct::Field &field,bool pointer,std::shared_ptr<VFilePtrInternal> f
		);
	};

	class Panorama
		: public ResourceData
	{
	public:
		struct NameEntry
		{
			std::string name;
			uint32_t unknown1;
			uint32_t unknown2;
		};
		const std::vector<NameEntry> &GetNames() const;
		virtual void Read(const Resource &resource,std::shared_ptr<VFilePtrInternal> f) override;
		virtual void DebugPrint(std::stringstream &ss,const std::string &t="") const override;
	private:
		std::vector<NameEntry> m_names;
		std::vector<uint8_t> m_data;
		uint32_t m_crc32;
	};

	class Sound
		: public ResourceData
	{
	public:
		enum class AudioFileType : uint32_t
		{
			AAC = 0,
			WAV = 1,
			MP3 = 2
		};

		AudioFileType GetSoundType() const;
		uint32_t GetSampleRate() const;
		uint32_t GetBits() const;
		uint32_t GetChannels() const;
		uint32_t GetAudioFormat() const;
		uint32_t GetSampleSize() const;
		uint32_t GetSampleCount() const;
		int32_t GetLoopStart() const;
		float GetDuration() const;
		uint32_t GetStreamingDataSize() const;
		virtual void Read(const Resource &resource,std::shared_ptr<VFilePtrInternal> f) override;
		virtual void DebugPrint(std::stringstream &ss,const std::string &t="") const override;
	private:
		void SetVersion4(std::shared_ptr<VFilePtrInternal> f);
		AudioFileType m_soundType = AudioFileType::AAC;
		uint32_t m_sampleRate = 0u;
		uint32_t m_bits = 0u;
		uint32_t m_channels = 0u;
		uint32_t m_audioFormat = 0u;
		uint32_t m_sampleSize = 0u;
		uint32_t m_sampleCount = 0u;
		int32_t m_loopStart = 0;
		float m_duration = 0.f;
		uint32_t m_streamingDataSize = 0u;
	};
	std::string to_string(Sound::AudioFileType type);

	class KeyValuesOrNTRO
		: public ResourceData
	{
	public:
		KeyValuesOrNTRO();
		KeyValuesOrNTRO(BlockType type,const std::string &introspectionStructName);
		virtual void Read(const Resource &resource,std::shared_ptr<VFilePtrInternal> f) override;
		virtual void DebugPrint(std::stringstream &ss,const std::string &t="") const override;
		virtual BlockType GetType() const override;

		const std::shared_ptr<resource::IKeyValueCollection> &GetData() const;
		const std::shared_ptr<resource::ResourceData> &GetBakingData() const;
	protected:
		std::shared_ptr<resource::IKeyValueCollection> m_data = nullptr;
		std::shared_ptr<resource::ResourceData> m_bakingData = nullptr;
		BlockType m_type = BlockType::None;
		std::string m_introspectionStructName;
	};

	class EntityLump;
	class Scene;
	class World
		: public KeyValuesOrNTRO
	{
	public:
		World(Resource &resource);
		std::vector<std::string> GetEntityLumpNames() const;
		std::vector<std::string> GetWorldNodeNames() const;
		std::shared_ptr<Scene> Load();
	private:
		Resource &m_resource;
		void LoadEntitiesFromLump(EntityLump &entityLump,const std::string &lumpName,Scene &scene);
	};

	class Scene;
	class WorldNode
		: public KeyValuesOrNTRO
	{
	public:
		void Load(Resource &resource,Scene &scene);
	private:
	};

	class SceneNode;
	class Entity;
	class Scene
	{
	public:
		void Add(SceneNode &node);
		void Add(Entity &ent);
		const std::vector<std::shared_ptr<SceneNode>> &GetSceneNodes() const;
		const std::vector<std::shared_ptr<Entity>> &GetEntities() const;
	private:
		std::vector<std::shared_ptr<SceneNode>> m_sceneNodes;
		std::vector<std::shared_ptr<Entity>> m_entities;
	};

	class SceneNode
		: public std::enable_shared_from_this<SceneNode>
	{
	public:
		enum class Type : uint8_t
		{
			None = 0,
			Mesh,
			Model
		};
		SceneNode(Scene &scene,std::shared_ptr<Resource> resource);
		void SetLayerName(const std::string &layerName);
		void SetTint(const Vector4 &tint);
		void SetLayerEnabled(bool enabled);
		const Mat4 &GetTransform() const;
		const std::shared_ptr<Resource> &GetResource() const;
		void SetTransform(const Mat4 &transform);
		virtual Type GetType() const=0;
	private:
		Mat4 m_transform = umat::identity();
		std::string m_layerName;
		bool m_layerEnabled = true;
		Vector4 m_tint = {1.f,1.f,1.f,1.f};
		Scene &m_scene;
		std::shared_ptr<Resource> m_resource = nullptr;
	};

	class Mesh;
	class MeshSceneNode
		: public SceneNode
	{
	public:
		MeshSceneNode(Scene &scene,std::shared_ptr<Resource> resource,Mesh &mesh);
		const std::shared_ptr<Mesh> &GetMesh() const;
		virtual Type GetType() const override {return Type::Mesh;}
	private:
		std::shared_ptr<Mesh> m_mesh = nullptr;
	};

	class Model;
	class Animation;
	class ModelSceneNode
		: public SceneNode
	{
	public:
		ModelSceneNode(Scene &scene,std::shared_ptr<Resource> resource,Model &mdl);
		virtual Type GetType() const override {return Type::Model;}
		const std::shared_ptr<Model> &GetModel() const;
	private:
		std::shared_ptr<Model> m_model;
		//std::vector<std::shared_ptr<RenderableMesh> m_renderableMeshes;
		std::vector<std::shared_ptr<Animation>> m_animations;
		std::unordered_map<std::string,std::string> m_skinMaterials;
	};

	struct EntityProperty
	{
		enum class Type : uint32_t
		{
			Invalid = 0,
			Bool = 0x06,
			Float = 0x01,
			Color255 = 0x09,
			NodeId = 0x05,
			Flags = 0x25,
			Integer = 0x1a,
			Vector = 0x03,
			Angle = 0x27,
			String = 0x1e
		};

		Type type = Type::Invalid;
		std::optional<std::string> name = {};
		std::shared_ptr<void> data = nullptr;
		std::optional<std::string> ToString() const;
		template<typename T>
			std::optional<T> GetData() const
		{
			switch(type)
			{
			case Type::Bool:
				return cast_to_type<bool,T>(*static_cast<bool*>(data.get()));
			case Type::Float:
				return cast_to_type<float,T>(*static_cast<float*>(data.get()));
			case Type::Color255:
			{
				if constexpr(std::is_same_v<T,std::array<uint8_t,4>>)
					return *static_cast<std::array<uint8_t,4>*>(data.get());
				break;
			}
			case Type::NodeId:
			case Type::Flags:
				return cast_to_type<uint32_t,T>(*static_cast<uint32_t*>(data.get()));
			case Type::Integer:
				return cast_to_type<uint64_t,T>(*static_cast<uint64_t*>(data.get()));
			case Type::Vector:
			{
				if constexpr(std::is_same_v<T,Vector3>)
					return *static_cast<Vector3*>(data.get());
				break;
			}
			case Type::Angle:
			{
				if constexpr(std::is_same_v<T,Vector3>)
					return *static_cast<Vector3*>(data.get());
				else if constexpr(std::is_same_v<T,EulerAngles>)
				{
					auto &v = *static_cast<Vector3*>(data.get());
					return EulerAngles{v.x,v.y,v.z};
				}
				break;
			}
			case Type::String:
				return cast_to_type<std::string,T>(*static_cast<std::string*>(data.get()));
			}
			return {};
		}
		template<typename T>
			T GetData(const T &def) const
		{
			auto val = GetData<T>();
			return val.has_value() ? *val : def;
		}
	};

	class Entity
		: public std::enable_shared_from_this<Entity>
	{
	public:
		static constexpr uint32_t MURMUR2_SEED = 0x31415926;
		static std::shared_ptr<Entity> Create(const std::unordered_map<uint32_t,EntityProperty> &properties);
		const std::unordered_map<uint32_t,EntityProperty> &GetProperties() const;
		std::unordered_map<std::string,std::string> GetKeyValues() const;
		EntityProperty *FindProperty(const std::string &key);
		template<typename T>
			std::optional<T> FindProperty(const std::string &key)
		{
			auto *prop = FindProperty(key);
			if(prop == nullptr)
				return {};
			return prop->GetData<T>();
		}
		template<typename T>
			T FindProperty(const std::string &key,const T &def)
		{
			auto prop = FindProperty<T>(key);
			if(prop.has_value() == false)
				return def;
			return *prop;
		}
	private:
		Entity(const std::unordered_map<uint32_t,EntityProperty> &properties);
		std::unordered_map<uint32_t,EntityProperty> m_properties = {};
	};

	class EntityLump
		: public KeyValuesOrNTRO
	{
	public:
		std::vector<std::string> GetChildEntityNames() const;
		std::vector<std::shared_ptr<Entity>> GetEntities() const;
	private:
		std::shared_ptr<Entity> ParseEntityProperties(const std::vector<uint8_t> &bytes) const;
		void ReadTypedValue(DataStream &ds,uint32_t keyHash,const std::optional<std::string> &keyName,std::unordered_map<uint32_t,EntityProperty> &properties) const;
	};

	class SoundStackScript
		: public ResourceData
	{

	};
	
	class ParticleSystem
		: public ResourceData
	{

	};

	struct FrameBone
	{
		Vector3 position = {};
		Quat rotation = uquat::identity();
	};

	class Frame
	{
	public:
		void SetPosition(const std::string &bone,const Vector3 &pos);
		void SetRotation(const std::string &bone,const Quat &rot);
		const std::unordered_map<std::string,FrameBone> &GetBones() const;
	private:
		void InsertIfUnknown(const std::string &bone);
		std::unordered_map<std::string,FrameBone> m_bones {};
	};

	enum class AnimDecoderType : uint32_t
	{
		Unknown,
		CCompressedReferenceFloat,
		CCompressedStaticFloat,
		CCompressedFullFloat,
		CCompressedReferenceVector3,
		CCompressedStaticVector3,
		CCompressedStaticFullVector3,
		CCompressedAnimVector3,
		CCompressedDeltaVector3,
		CCompressedFullVector3,
		CCompressedReferenceQuaternion,
		CCompressedStaticQuaternion,
		CCompressedAnimQuaternion,
		CCompressedReferenceInt,
		CCompressedStaticChar,
		CCompressedFullChar,
		CCompressedStaticShort,
		CCompressedFullShort,
		CCompressedStaticInt,
		CCompressedFullInt,
		CCompressedReferenceBool,
		CCompressedStaticBool,
		CCompressedFullBool,
		CCompressedReferenceColor32,
		CCompressedStaticColor32,
		CCompressedFullColor32,
		CCompressedReferenceVector2D,
		CCompressedStaticVector2D,
		CCompressedFullVector2D,
		CCompressedReferenceVector4D,
		CCompressedStaticVector4D,
		CCompressedFullVector4D
	};
	class Animation
		: public ResourceData
	{
	public:
		static std::vector<std::shared_ptr<Animation>> CreateAnimations(IKeyValueCollection &animationData,IKeyValueCollection &decodeKey);
		static std::shared_ptr<Animation> Create(
			IKeyValueCollection &animDesc,IKeyValueCollection &decodeKey,const std::vector<AnimDecoderType> &decoderArray,const std::vector<IKeyValueCollection*> &segmentArray
		);

		const std::string &GetName() const;
		float GetFPS() const;
		const std::vector<std::shared_ptr<Frame>> &GetFrames() const;
	private:
		Animation(
			IKeyValueCollection &animDesc,IKeyValueCollection &decodeKey,const std::vector<AnimDecoderType> &decoderArray,const std::vector<IKeyValueCollection*> &segmentArray
		);
		void ConstructFromDesc(
			IKeyValueCollection &animDesc,IKeyValueCollection &decodeKey,const std::vector<AnimDecoderType> &decoderArray,const std::vector<IKeyValueCollection*> &segmentArray
		);
		void ReadSegment(
			int64_t frame,IKeyValueCollection &segment, IKeyValueCollection &decodeKey, const std::vector<AnimDecoderType> &decoderArray, Frame &outFrame,uint32_t numFrames
		);
		static Quat ReadQuaternion(DataStream &ds);
		static std::vector<AnimDecoderType> MakeDecoderArray(const std::vector<IKeyValueCollection*> &decoderArray);
		std::string m_name;
		float m_fps = 0.f;
		std::vector<std::shared_ptr<Frame>> m_frames = {};
	};
	
	class AnimationGroup
	{
	public:
		AnimationGroup(ResourceData &animationData);
		AnimationGroup(Resource &resource);
		IKeyValueCollection *GetDecodeKey();
		std::vector<std::string> GetAnimationArray() const;
	private:
		IKeyValueCollection *m_data = nullptr;
	};

	class AnimDecoder
	{
	public:
		static int32_t GetSize(AnimDecoderType t);
		static AnimDecoderType FromString(const std::string &str);
	};

	class Bone
		: public std::enable_shared_from_this<Bone>
	{
	public:
		static std::shared_ptr<Bone> Create(const std::string &name,const Vector3 &position,const Quat &rotation);
		void AddChild(Bone &bone);
		void SetParent(Bone &parent);
		const std::string &GetName() const;
		Bone *GetParent() const;
		const std::vector<std::shared_ptr<Bone>> &GetChildren() const;
		const Vector3 &GetPosition() const;
		const Quat &GetRotation() const;
	private:
		Bone(const std::string &name,const Vector3 &position,const Quat &rotation);
		std::string m_name;
		std::weak_ptr<Bone> m_parent = {};
		std::vector<std::shared_ptr<Bone>> m_children = {};
		Vector3 m_position = {};
		Quat m_rotation = uquat::identity();
	};

	class Skeleton
		: public ResourceData
	{
	public:
		static std::shared_ptr<Skeleton> Create(IKeyValueCollection &modelData);
		static constexpr int32_t BoneUsedByVertexLod0 = 0x00000400;
		const std::vector<std::shared_ptr<Bone>> &GetRootBones() const;
		const std::vector<std::shared_ptr<Bone>> &GetBoneList() const;

		const std::vector<int32_t> &GetRemappingTable() const;
		const std::vector<int32_t> &GetRemappingTableStarts() const;
	private:
		Skeleton(IKeyValueCollection &skeletonData,const std::vector<int32_t> &remappingTable,const std::vector<int32_t> &remappingTableStarts);
		void ConstructFromNTRO(IKeyValueCollection &skeletonData);
		std::vector<std::shared_ptr<Bone>> m_rootBones = {};
		std::vector<std::shared_ptr<Bone>> m_boneList = {};
		std::vector<int32_t> m_remappingTable = {};
		std::vector<int32_t> m_remappingTableStarts = {};
	};

	class Mesh
		: public std::enable_shared_from_this<Mesh>
	{
	public:
		static std::shared_ptr<Mesh> Create(ResourceData &data,VBIB &vbib,int64_t meshIdx=-1);
		static std::shared_ptr<Mesh> Create(Resource &resource);

		const std::pair<Vector3,Vector3> &GetBounds() const;
		std::shared_ptr<VBIB> GetVBIB() const;
		std::shared_ptr<ResourceData> GetResourceData() const;
		int64_t GetMeshIndex() const;
	private:
		Mesh(ResourceData &data,VBIB &vbib,int64_t meshIdx=-1);
		void UpdateBounds();
		int64_t m_meshIdx = -1;
		std::shared_ptr<ResourceData> m_resourceData = nullptr;
		std::shared_ptr<VBIB> m_vbib = nullptr;
		std::pair<Vector3,Vector3> m_bounds = {};
		std::shared_ptr<Skeleton> m_skeleton = nullptr;

	};

	class Skin
	{
	public:
		Skin(const std::string &name,std::vector<std::string> &&materials);
		const std::string &GetName() const;
		const std::vector<std::string> &GetMaterials() const;
	private:
		std::string m_name;
		std::vector<std::string> m_materials;
	};

	class Model
		: public KeyValuesOrNTRO
	{
	public:
		Model(Resource &resource);
		std::vector<std::shared_ptr<Mesh>> GetEmbeddedMeshes() const;
		std::vector<Skin> GetSkins();
		std::vector<std::string> GetReferencedMeshNames();
		std::shared_ptr<Skeleton> GetSkeleton() const;
		std::string GetName() const;
		void GetReferencedAnimationGroupNames();
		std::vector<std::shared_ptr<Animation>> GetEmbeddedAnimations(Resource &resource);
		void GetMeshGroups();
		void GetDefaultMeshGroups();
		void GetActiveMeshMaskForGroup(const std::string &groupName);
	private:
		std::vector<Skin> m_skins;
		Resource &m_resource;
	};

	class Material
		: public KeyValuesOrNTRO
	{
	public:
		const std::string &GetName() const;
		const std::string &GetShaderName() const;
		virtual void Read(const Resource &resource,std::shared_ptr<VFilePtrInternal> f) override;
		virtual void DebugPrint(std::stringstream &ss,const std::string &t="") const override;

		const int64_t *FindIntParam(const std::string &key) const;
		const float *FindFloatParam(const std::string &key) const;
		const Vector4 *FindVectorParam(const std::string &key) const;
		const std::string *FindTextureParam(const std::string &key) const;

		const int64_t *FindIntAttr(const std::string &key) const;
		const float *FindFloatAttr(const std::string &key) const;
		const Vector4 *FindVectorAttr(const std::string &key) const;
		const std::string *FindTextureAttr(const std::string &key) const;
	private:
		std::string m_name;
		std::string m_shaderName;
		std::unordered_map<std::string,int64_t> m_intParams;
		std::unordered_map<std::string,float> m_floatParams;
		std::unordered_map<std::string,Vector4> m_vectorParams;
		std::unordered_map<std::string,std::string> m_textureParams;

		std::unordered_map<std::string,int64_t> m_intAttributes;
		std::unordered_map<std::string,float> m_floatAttributes;
		std::unordered_map<std::string,Vector4> m_vectorAttributes;
		std::unordered_map<std::string,std::string> m_stringAttributes;
	};

	class SoundEventScript
		: public NTRO
	{
	public:
		const std::unordered_map<std::string,std::string> &GetValues() const;
		virtual void Read(const Resource &resource,std::shared_ptr<VFilePtrInternal> f) override;
		virtual void DebugPrint(std::stringstream &ss,const std::string &t="") const override;
	private:
		std::unordered_map<std::string,std::string> m_values {};
	};

	class Texture
		: public ResourceData
	{
	public:
		static void UncompressBC7(uint32_t RowBytes, DataStream &ds, std::vector<uint8_t> &data, int w, int h, bool hemiOctRB, bool invert);
		uint16_t GetVersion() const;
		uint16_t GetWidth() const;
		uint16_t GetHeight() const;
		uint16_t GetDepth() const;
		Vector4 GetReflectivity() const;
		VTexFlags GetFlags() const;
		VTexFormat GetFormat() const;
		uint8_t GetMipMapCount() const;
		uint32_t GetPicmip0Res() const;
		const std::unordered_map<VTexExtraData,std::vector<uint8_t>> &GetExtraData() const;
		virtual void Read(const Resource &resource,std::shared_ptr<VFilePtrInternal> f) override;
		virtual void DebugPrint(std::stringstream &ss,const std::string &t="") const override;

		uint32_t GetBlockSize();
		uint64_t GetMipmapDataOffset(uint8_t mipmap);

		void ReadTextureData(uint8_t mipLevel,std::vector<uint8_t> &outData);
		std::vector<uint8_t> GetDecompressedTextureAtMipLevel(int mipLevel);

		uint32_t CalculateBufferSizeForMipLevel(uint8_t mipLevel);
	private:
		VTexFlags m_flags = VTexFlags::None;
		Vector4 m_reflectivity = {};
		uint16_t m_width = 0u;
		uint16_t m_height = 0u;
		uint16_t m_depth = 0u;
		VTexFormat m_format = VTexFormat::UNKNOWN;
		uint8_t m_mipMapCount = 0u;
		uint32_t m_picmip0Res = 0u;
		uint64_t m_dataOffset = 0ull;
		std::unordered_map<VTexExtraData,std::vector<uint8_t>> m_extraData {};
		uint32_t m_nonPow2Width = 0u;
		uint32_t m_nonPow2Height = 0u;
		std::shared_ptr<VFilePtrInternal> m_file = nullptr;

		bool m_isCompressed = false;
		std::vector<int32_t> m_compressedMips = {};
	};

	enum class KVType : uint8_t
	{
		Invalid = std::numeric_limits<uint8_t>::max(),
		STRING_MULTI = 0, // STRING_MULTI doesn't have an ID
		Null = 1,
		BOOLEAN = 2,
		INT64 = 3,
		UINT64 = 4,
		DOUBLE = 5,
		STRING = 6,
		BINARY_BLOB = 7,
		ARRAY = 8,
		OBJECT = 9,
		ARRAY_TYPED = 10,
		INT32 = 11,
		UINT32 = 12,
		BOOLEAN_TRUE = 13,
		BOOLEAN_FALSE = 14,
		INT64_ZERO = 15,
		INT64_ONE = 16,
		DOUBLE_ZERO = 17,
		DOUBLE_ONE = 18,
	};
	std::string to_string(KVType type);

	enum class KVFlag : uint8_t
	{
		None,
		Resource,
		DeferredResource
	};

	class KVValue
		: public std::enable_shared_from_this<KVValue>
	{
	public:
		KVValue(KVType type,std::shared_ptr<void> value,KVFlag flags=KVFlag::None);
		void *GetObject();
		const void *GetObject() const;
		KVFlag GetFlags() const;
		KVType GetType() const;
		void DebugPrint(std::stringstream &ss,const std::string &t="") const;
		template<typename T>
			std::optional<T> GetObjectValue()
		{
			if constexpr(std::is_same_v<T,nullptr_t>)
				return {};
			else if constexpr(std::is_same_v<T,const std::string*> || std::is_same_v<T,std::string*>)
			{
				if(GetType() != KVType::STRING)
					return {};
				return static_cast<std::string*>(GetObject());
			}
			else if constexpr(std::is_same_v<T,const BinaryBlob*> || std::is_same_v<T,BinaryBlob*>)
			{
				if(GetType() != KVType::BINARY_BLOB)
					return {};
				return static_cast<BinaryBlob*>(GetObject());
			}
			else if constexpr(
				std::is_same_v<T,const KVObject*> || std::is_same_v<T,KVObject*> ||
				std::is_same_v<T,const IKeyValueCollection*> || std::is_same_v<T,IKeyValueCollection*>
			)
			{
				if(GetType() != KVType::ARRAY && GetType() != KVType::ARRAY_TYPED && GetType() != KVType::OBJECT)
					return {};
				return static_cast<KVObject*>(GetObject());
			}
			else
			{
				// Only arithmetic types and string remaining
				return cast_to_type<T>(*this);
			}
			return {};
		}
	protected:
		std::shared_ptr<void> m_object = nullptr;
		KVFlag m_flags = KVFlag::None;
		KVType m_type = KVType::Invalid;
	};

	template<typename T>
		std::optional<T> cast_to_type(KVValue &v0)
	{
		switch(v0.GetType())
		{
		case KVType::Null:
			return {};
		case KVType::BOOLEAN:
			return cast_to_type<bool,T>(*static_cast<const bool*>(v0.GetObject()));
		case KVType::INT64:
			return cast_to_type<int64_t,T>(*static_cast<const int64_t*>(v0.GetObject()));
		case KVType::UINT64:
			return cast_to_type<uint64_t,T>(*static_cast<const uint64_t*>(v0.GetObject()));
		case KVType::DOUBLE:
			return cast_to_type<double,T>(*static_cast<const double*>(v0.GetObject()));
		case KVType::STRING:
			return cast_to_type<std::string,T>(*static_cast<const std::string*>(v0.GetObject()));
		case KVType::INT32:
			return cast_to_type<int32_t,T>(*static_cast<const int32_t*>(v0.GetObject()));
		case KVType::UINT32:
			return cast_to_type<uint32_t,T>(*static_cast<const uint32_t*>(v0.GetObject()));
		case KVType::OBJECT:
		case KVType::ARRAY:
		case KVType::ARRAY_TYPED:
			return cast_to_type<IKeyValueCollection,T>(*static_cast<const KVObject*>(v0.GetObject()));
		}
		return {};
	}

	class KVObject
		: public std::enable_shared_from_this<KVObject>,
		public IKeyValueCollection
	{
	public:
		KVObject(const std::string &name,bool isArray=false);
		void AddProperty(const std::string &name,KVValue &value);
		const std::unordered_map<std::string,std::shared_ptr<KVValue>> &GetValues() const;
		KVValue *FindValue(const std::string &key);
		const KVValue *FindValue(const std::string &key) const;
		KVObject *FindArray(const std::string &key);
		const KVObject *FindArray(const std::string &key) const;
		BinaryBlob *FindBinaryBlob(const std::string &key);
		template<typename T>
			std::vector<T> FindArrayValues(const std::string &key)
		{
			auto *array = FindArray(key);
			if(array == nullptr)
				return {};
			auto n = array->GetArrayCount();

			std::vector<T> arrayElements {};
			arrayElements.reserve(n);
			for(auto i=decltype(n){0u};i<n;++i)
			{
				auto v = array->GetArrayValue<T>(i);
				if(v.has_value() == false)
					continue;
				arrayElements.push_back(*v);
			}
			return arrayElements;
		}

		template<typename T>
			std::optional<T> FindValue(const std::string &key,std::optional<KVType> optTypeFilter={})
		{
			auto *val = FindValue(key);
			if(val == nullptr || (optTypeFilter.has_value() && val->GetType() != *optTypeFilter))
				return {};
			return val->GetObjectValue<T>();
		}

		bool IsArray() const;
		uint32_t GetArrayCount() const;

		KVValue *GetArrayValue(uint32_t idx,std::optional<KVType> confirmType={});
		const KVValue *GetArrayValue(uint32_t idx,std::optional<KVType> confirmType={}) const;
		template<typename T>
			std::optional<T> GetArrayValue(uint32_t idx,std::optional<KVType> confirmType={})
		{
			auto *val = GetArrayValue(idx,confirmType);
			return val ? val->GetObjectValue<T>() : std::optional<T>{};
		}
		void DebugPrint(std::stringstream &ss,const std::string &t="") const;
	private:
		std::string m_key;
		std::unordered_map<std::string,std::shared_ptr<KVValue>> m_values;
		bool m_isArray = false;
		uint32_t m_count = 0u;
	};

	class BinaryKV3
		: public ResourceData
	{
	public:
		static const util::GUID KV3_ENCODING_BINARY_BLOCK_COMPRESSED;
		static const util::GUID KV3_ENCODING_BINARY_UNCOMPRESSED;
		static const util::GUID KV3_ENCODING_BINARY_BLOCK_LZ4;
		static const util::GUID KV3_FORMAT_GENERIC;
		static constexpr int32_t MAGIC = 0x03564B56; // VKV3 (3 isn't ascii, its 0x03)
		static constexpr int32_t MAGIC2 = 0x4B563301; // KV3\x01
		static const std::array<uint8_t,16> ENCODING;
		static const std::array<uint8_t,16> FORMAT;
		static const std::array<uint8_t,4> SIG; // VKV3 (3 isn't ascii, its 0x03)

		const std::vector<std::string> &GetStringArray() const;
		const std::shared_ptr<KVObject> &GetData() const;
		std::shared_ptr<KVObject> &GetData();

		virtual void Read(const Resource &resource,std::shared_ptr<VFilePtrInternal> f) override;
		void DebugPrint(std::stringstream &ss,const std::string &t="") const;
		virtual BlockType GetType() const override;
		BinaryKV3()=default;
		BinaryKV3(BlockType type);
	private:
		//template<class TKVValue,typename T>
		//	static std::shared_ptr<KVValue> MakeValue(KVType type, T data, KVFlag flag);
		//template<class TKVValue,typename TPtr=void>
		//	static std::shared_ptr<KVValue> MakeValueFromPtr(KVType type, TPtr &data, KVFlag flag);
		//template<class TKVValue>
		//	static std::shared_ptr<KVValue> MakeValueFromPtr(KVType type, nullptr_t nptr, KVFlag flag);
		static KVType ConvertBinaryOnlyKVType(KVType type);
		static std::shared_ptr<KVValue> MakeValue(KVType type,std::shared_ptr<void> data,KVFlag flag);
		void ReadVersion2(std::shared_ptr<VFilePtrInternal> f,DataStream &outData);
		void BlockDecompress(std::shared_ptr<VFilePtrInternal> f,DataStream &outData);
		void DecompressLZ4(std::shared_ptr<VFilePtrInternal> f,DataStream &outData);
		std::pair<KVType,KVFlag> ReadType(DataStream &ds);
		std::shared_ptr<KVObject> ReadBinaryValue(const std::string &name, KVType datatype, KVFlag flagInfo, DataStream ds, std::shared_ptr<KVObject> optParent);
		std::shared_ptr<KVObject> ParseBinaryKV3(DataStream &ds,std::shared_ptr<KVObject> optParent,bool inArray=false);
		int64_t m_currentBinaryBytesOffset = -1;
		int64_t m_currentEightBytesOffset = -1;
		int64_t m_currentTypeIndex = 0;
		std::vector<std::string> m_stringArray = {};
		std::vector<uint8_t> m_typesArray = {};
		bool m_hasTypesArray = false;
		std::shared_ptr<KVObject> m_data = nullptr;
		BlockType m_blockType = BlockType::DATA;
	};
};

template<typename T>
	T source2::resource::IKeyValueCollection::FindValue(const std::string &key,const T &def)
{
	auto optVal = FindValue<T>(key);
	return optVal.has_value() ? *optVal : def;
}
template<typename T>
	std::optional<T> source2::resource::IKeyValueCollection::FindValue(const std::string &key) {return FindValue<T>(*this,key);}
template<typename T>
	std::optional<T> source2::resource::IKeyValueCollection::FindValue(const std::string &key) const {return FindValue<T>(*this,key);}
template<typename T>
	std::vector<T> source2::resource::IKeyValueCollection::FindArrayValues(const std::string &key) {return FindArrayValues<T>(*this,key);}


template<typename T>
	std::optional<T> source2::resource::IKeyValueCollection::FindValue(IKeyValueCollection &collection,const std::string &key)
{
	if(typeid(collection) == typeid(NTROStruct))
		return static_cast<NTROStruct&>(collection).FindValue<T>(key);
	else if(typeid(collection) == typeid(KVObject))
		return static_cast<KVObject&>(collection).FindValue<T>(key);
	return {};
}
template<typename T>
	std::optional<T> source2::resource::IKeyValueCollection::FindValue(const IKeyValueCollection &collection,const std::string &key)
{
	return FindValue<T>(const_cast<IKeyValueCollection&>(collection),key);
}
template<typename T>
	std::vector<T> source2::resource::IKeyValueCollection::FindArrayValues(IKeyValueCollection &collection,const std::string &key)
{
	if(typeid(collection) == typeid(NTROStruct))
		return static_cast<NTROStruct&>(collection).FindArrayValues<T>(key);
	else if(typeid(collection) == typeid(KVObject))
		return static_cast<KVObject&>(collection).FindArrayValues<T>(key);
	return {};
}

//////////////

template<typename T>
	source2::resource::TNTROValue<T>::TNTROValue(DataType type,const T &value,bool pointer)
	: NTROValue{type,pointer},value{value}
{}
template<typename T>
	void source2::resource::TNTROValue<T>::DebugPrint(std::stringstream &ss,const std::string &t) const
{
	ss<<t<<"TNTROValue<"<<typeid(T).name()<<"> = {\n";
	ss<<t<<"\tType = "<<to_string(type)<<"\n";
	ss<<t<<"\tPointer = "<<pointer<<"\n";
	if constexpr(std::is_same_v<T,NTROStruct>)
	{
		ss<<t<<"\tValue:\n";
		value.DebugPrint(ss,t +"\t\t");
	}
	else
		ss<<t<<"\tValue = "<<value<<"\n";
	ss<<t<<"}\n";
}

//////////////

template<typename T>
	std::optional<T> source2::resource::cast_to_type(NTROValue &v0)
{
	if constexpr(std::is_same_v<T,Mat4>)
	{
		auto *vAr = dynamic_cast<NTROArray*>(&v0);
		if(vAr == nullptr)
			return {};
		auto &contents = vAr->GetContents();
		if(contents.size() != 4)
			return {};
		Mat4 result {};
		for(uint8_t i=0;i<4;++i)
		{
			auto &val = contents.at(i);
			if(val->type != DataType::Vector4D)
				return {};
			auto v = cast_to_type<Vector4>(*val);
			if(v.has_value() == false)
				return {};
			result[i] = *v;
		}
		return result;
	}
	switch(v0.type)
	{
	//case DataType::Null:
	//	return {};
	case DataType::Byte:
		return cast_to_type<uint8_t,T>(static_cast<TNTROValue<uint8_t>&>(v0).value);
	case DataType::SByte:
		return cast_to_type<int8_t,T>(static_cast<TNTROValue<int8_t>&>(v0).value);
	case DataType::Boolean:
		return cast_to_type<bool,T>(static_cast<TNTROValue<bool>&>(v0).value);
	case DataType::Int16:
		return cast_to_type<int16_t,T>(static_cast<TNTROValue<int16_t>&>(v0).value);
	case DataType::UInt16:
		return cast_to_type<uint16_t,T>(static_cast<TNTROValue<uint16_t>&>(v0).value);
	case DataType::Int32:
		return cast_to_type<int32_t,T>(static_cast<TNTROValue<int32_t>&>(v0).value);
	case DataType::UInt32:
	case DataType::Enum:
		return cast_to_type<uint32_t,T>(static_cast<TNTROValue<uint32_t>&>(v0).value);
	case DataType::Float:
		return cast_to_type<float,T>(static_cast<TNTROValue<float>&>(v0).value);
	case DataType::Int64:
		return cast_to_type<int64_t,T>(static_cast<TNTROValue<int64_t>&>(v0).value);
	case DataType::UInt64:
		return cast_to_type<uint64_t,T>(static_cast<TNTROValue<uint64_t>&>(v0).value);
	case DataType::String:
	case DataType::ExternalReference:
		return cast_to_type<std::string,T>(static_cast<TNTROValue<std::string>&>(v0).value);
	case DataType::Struct:
		return cast_to_type<IKeyValueCollection*,T>(static_cast<TNTROValue<std::shared_ptr<NTROStruct>>&>(v0).value.get());
	default:
	{
		auto *vStrct = dynamic_cast<TNTROValue<NTROStruct>*>(&v0);
		if(vStrct == nullptr)
			return {};
		return cast_to_type<IKeyValueCollection,T>(vStrct->value);
	}
	}
	return {};
}

template<typename T0,typename T1>
	std::optional<T1> source2::resource::cast_to_type(const T0 &v)
{
	if constexpr(std::is_same_v<T0,T1>)
		return v;
	if constexpr(std::is_arithmetic_v<T0>)
	{
		if constexpr(std::is_arithmetic_v<T1>)
			return static_cast<T1>(v);
		else if constexpr(std::is_same_v<T1,std::string>)
			return std::to_string(v);
		else
			return {};
	}
	else if constexpr(std::is_same_v<T0,std::string>)
	{
		if constexpr(std::is_integral_v<T1>)
			return ustring::to_int(v);
		else if constexpr(std::is_arithmetic_v<T1>)
			return ustring::to_float(v);
		else if constexpr(std::is_same_v<T1,Vector3>)
			return uvec::create(v);
		else if constexpr(std::is_same_v<T1,EulerAngles>)
			return EulerAngles{v};
		else
			return {};
	}
	else if constexpr(std::is_same_v<T0,IKeyValueCollection>)
	{
		if constexpr(std::is_same_v<T1,Vector3>)
		{
			Vector3 result {};
			for(uint8_t i=0;i<3;++i)
			{
				auto vc = IKeyValueCollection::template FindValue<float>(v,std::to_string(i));
				if(vc.has_value() == false)
					continue;
				result[i] = *vc;
			}
			return result;
		}
		if constexpr(std::is_same_v<T1,Vector4>)
		{
			Vector4 result {};
			for(uint8_t i=0;i<4;++i)
			{
				auto vc = IKeyValueCollection::template FindValue<float>(v,std::to_string(i));
				if(vc.has_value() == false)
					continue;
				result[i] = *vc;
			}
			return result;
		}
		else if constexpr(std::is_same_v<T1,Quat>)
		{
			Quat result {};
			for(auto i : {3,0,1,2}) // Source has components in order xyzw, glm in wxyz
			{
				auto vc = IKeyValueCollection::template FindValue<float>(v,std::to_string(i));
				if(vc.has_value() == false)
					continue;
				result[i] = *vc;
			}
			return result;
		}
		else if constexpr(std::is_same_v<T1,Mat4>)
		{
			auto result = umat::identity();
			for(uint8_t i=0;i<4;++i)
			{
				auto vec = v.FindValue<Vector4>(std::to_string(i));
				if(vec.has_value() == false)
					continue;
				for(uint8_t j=0;j<4;++j)
					result[i][j] = (*vec)[j];
			}
			return result;
		}
	}
	return {};
}

#endif
