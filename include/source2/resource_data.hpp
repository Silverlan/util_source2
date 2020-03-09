/*
MIT License

Copyright (c) 2020 Silverlan
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

#undef GetObject

class DataStream;
namespace util {using GUID = std::array<uint8_t,16>;};
namespace source2::resource
{
	class ResourceData
		: public Block
	{
	public:
		virtual BlockType GetType() const override;
		virtual void Read(const Resource &resource,std::shared_ptr<VFilePtrInternal> f) override;
		virtual void DebugPrint(std::stringstream &ss,const std::string &t="") const;
	};

	struct NTROValue
		: public std::enable_shared_from_this<NTROValue>
	{
		NTROValue(DataType type=DataType::Unknown,bool pointer=false);
		DataType type = DataType::Unknown;
		bool pointer = false;
		virtual void DebugPrint(std::stringstream &ss,const std::string &t="") const=0;
	};

	struct NTROArray
		: public NTROValue
	{
	public:
		NTROArray(DataType type,uint32_t count,bool pointer=false,bool isIndirection=false);
		const std::vector<std::shared_ptr<NTROValue>> &GetContents() const;
		std::vector<std::shared_ptr<NTROValue>> &GetContents();
		bool IsIndirection() const;
		virtual void DebugPrint(std::stringstream &ss,const std::string &t) const override;
	private:
		std::vector<std::shared_ptr<NTROValue>> m_contents = {};
		bool m_bIsIndirection = false;
	};

	template<typename T>
		struct TNTROValue
			: public NTROValue
	{
		TNTROValue(DataType type,const T &value,bool pointer=false)
			: NTROValue{type,pointer},value{value}
		{}
		virtual void DebugPrint(std::stringstream &ss,const std::string &t) const override
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
		T value;
	};

	class IKeyValueCollection
	{
	public:
		IKeyValueCollection()=default;
		virtual ~IKeyValueCollection()=default;
		template<typename T>
			static std::optional<T> FindValue(IKeyValueCollection &collection,const std::string &key)
		{
			if(typeid(collection) == typeid(NTROStruct))
				return static_cast<NTROStruct&>(collection).FindValue<T>(key);
			else if(typeid(collection) == typeid(KVObject))
				return static_cast<KVObject&>(collection).FindValue<T>(key);
			return {};
		}
		template<typename T>
			static std::optional<T> FindValue(const IKeyValueCollection &collection,const std::string &key)
			{
				return FindValue(const_cast<IKeyValueCollection&>(collection),key);
			}
	};
	class NTROStruct
		: public IKeyValueCollection
	{
	public:
		NTROStruct(const std::string &name);
		NTROStruct(const std::vector<std::shared_ptr<NTROValue>> &values);
		const std::unordered_map<std::string,std::shared_ptr<NTROValue>> &GetContents() const;
		NTROValue *FindValue(const std::string &key);
		const NTROValue *FindValue(const std::string &key) const;
		void Add(const std::string &id,const std::shared_ptr<NTROValue> &val);
		void DebugPrint(std::stringstream &ss,const std::string &t="") const;

		template<typename T>
			std::optional<T> FindValue(const std::string &key)
		{
			auto *val = FindValue(key);
			if(val == nullptr)
				return {};
			if constexpr(std::is_same_v<T,uint8_t>)
			{
				if(val->type != DataType::Byte)
					return {};
				return static_cast<TNTROValue<uint8_t>*>(val)->value;
			}
			else if constexpr(std::is_same_v<T,int8_t>)
			{
				if(val->type != DataType::SByte)
					return {};
				return static_cast<TNTROValue<int8_t>*>(val)->value;
			}
			else if constexpr(std::is_same_v<T,bool>)
			{
				if(val->type != DataType::Boolean)
					return {};
				return static_cast<TNTROValue<bool>*>(val)->value;
			}
			else if constexpr(std::is_same_v<T,int16_t>)
			{
				if(val->type != DataType::Int16)
					return {};
				return static_cast<TNTROValue<int16_t>*>(val)->value;
			}
			else if constexpr(std::is_same_v<T,uint16_t>)
			{
				if(val->type != DataType::UInt16)
					return {};
				return static_cast<TNTROValue<uint16_t>*>(val)->value;
			}
			else if constexpr(std::is_same_v<T,int32_t>)
			{
				if(val->type != DataType::Int32)
					return {};
				return static_cast<TNTROValue<int32_t>*>(val)->value;
			}
			else if constexpr(std::is_same_v<T,uint32_t>)
			{
				if(val->type != DataType::UInt32 && val->type != DataType::Enum)
					return {};
				return static_cast<TNTROValue<uint32_t>*>(val)->value;
			}
			else if constexpr(std::is_same_v<T,float>)
			{
				if(val->type != DataType::Float)
					return {};
				return static_cast<TNTROValue<float>*>(val)->value;
			}
			else if constexpr(std::is_same_v<T,int64_t>)
			{
				if(val->type != DataType::Int64)
					return {};
				return static_cast<TNTROValue<int64_t>*>(val)->value;
			}
			else if constexpr(std::is_same_v<T,uint64_t>)
			{
				if(val->type != DataType::UInt64)
					return {};
				return static_cast<TNTROValue<uint64_t>*>(val)->value;
			}
			else if constexpr(std::is_same_v<T,std::string>)
			{
				if(val->type != DataType::ExternalReference && val->type != DataType::String)
					return {};
				return static_cast<TNTROValue<std::string>*>(val)->value;
			}
			else if constexpr(std::is_same_v<T,std::shared_ptr<NTROArray>>)
			{
				if(val->type != DataType::Struct)
					return {};
				return std::static_pointer_cast<NTROArray>(static_cast<NTROArray*>(val)->shared_from_this());//TNTROValue<std::shared_ptr<NTROStruct>>*>(val)->value;
			}
			else if constexpr(std::is_same_v<T,std::shared_ptr<resource::NTROValue>>)
			{
				if(
					val->type != DataType::Vector && val->type != DataType::Quaternion && val->type != DataType::Color && val->type != DataType::Fltx4 &&
					val->type != DataType::Vector4D && val->type != DataType::Vector4D_44 && val->type != DataType::String4 && val->type != DataType::String &&
					val->type != DataType::Matrix2x4 && val->type != DataType::Matrix3x4 && val->type != DataType::Matrix3x4a && val->type != DataType::CTransform
				)
					return {};
				return static_cast<TNTROValue<std::shared_ptr<resource::NTROValue>>*>(val)->value;
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
	private:
		std::shared_ptr<NTROStruct> m_output = nullptr;
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

	class KVObject;
	class ResourceData;
	class KeyValuesOrNTRO
		: public ResourceData
	{
	public:
		virtual void Read(const Resource &resource,std::shared_ptr<VFilePtrInternal> f) override;
		virtual void DebugPrint(std::stringstream &ss,const std::string &t="") const override;

		const std::shared_ptr<resource::IKeyValueCollection> &GetData() const;
		const std::shared_ptr<resource::ResourceData> &GetBakingData() const;
	private:
		std::shared_ptr<resource::IKeyValueCollection> m_data = nullptr;
		std::shared_ptr<resource::ResourceData> m_bakingData = nullptr;
	};

	class Material
		: public KeyValuesOrNTRO
	{
	public:
		const std::string &GetName() const;
		const std::string &GetShaderName() const;
		virtual void Read(const Resource &resource,std::shared_ptr<VFilePtrInternal> f) override;
		virtual void DebugPrint(std::stringstream &ss,const std::string &t="") const override;
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

	enum class KVFlag : uint8_t
	{
		None,
		Resource,
		DeferredResource
	};

	using BinaryBlob = std::vector<uint8_t>;
	class KVValue
	{
	public:
		KVValue(KVType type,const std::shared_ptr<void> &value,KVFlag flags=KVFlag::None);
		void *GetObject();
		const void *GetObject() const;
		KVFlag GetFlags() const;
		virtual KVType GetType() const=0;
		virtual void DebugPrint(std::stringstream &ss,const std::string &t="") const=0;
	protected:
		std::shared_ptr<void> m_object = nullptr;
		KVFlag m_flags = KVFlag::None;
	};

	class KVValueNull
		: public KVValue
	{
	public:
		using KVValue::KVValue;
		nullptr_t GetValue() const;
		virtual KVType GetType() const;
		virtual void DebugPrint(std::stringstream &ss,const std::string &t="") const override;
	};

	class KVValueBool
		: public KVValue
	{
	public:
		using KVValue::KVValue;
		bool GetValue() const;
		virtual KVType GetType() const override;
		virtual void DebugPrint(std::stringstream &ss,const std::string &t="") const override;
	};

	class KVValueInt64
		: public KVValue
	{
	public:
		using KVValue::KVValue;
		int64_t GetValue() const;
		virtual KVType GetType() const override;
		virtual void DebugPrint(std::stringstream &ss,const std::string &t="") const override;
	};

	class KVValueUInt64
		: public KVValue
	{
	public:
		using KVValue::KVValue;
		uint64_t GetValue() const;
		virtual KVType GetType() const override;
		virtual void DebugPrint(std::stringstream &ss,const std::string &t="") const override;
	};

	class KVValueInt32
		: public KVValue
	{
	public:
		using KVValue::KVValue;
		int32_t GetValue() const;
		virtual KVType GetType() const override;
		virtual void DebugPrint(std::stringstream &ss,const std::string &t="") const override;
	};

	class KVValueUInt32
		: public KVValue
	{
	public:
		using KVValue::KVValue;
		uint32_t GetValue() const;
		virtual KVType GetType() const override;
		virtual void DebugPrint(std::stringstream &ss,const std::string &t="") const override;
	};

	class KVValueDouble
		: public KVValue
	{
	public:
		using KVValue::KVValue;
		double GetValue() const;
		virtual KVType GetType() const override;
		virtual void DebugPrint(std::stringstream &ss,const std::string &t="") const override;
	};

	class KVValueString
		: public KVValue
	{
	public:
		using KVValue::KVValue;
		const std::string &GetValue() const;
		virtual KVType GetType() const override;
		virtual void DebugPrint(std::stringstream &ss,const std::string &t="") const override;
	};

	class KVValueBinaryBlob
		: public KVValue
	{
	public:
		using KVValue::KVValue;
		const BinaryBlob &GetValue() const;
		virtual KVType GetType() const override;
		virtual void DebugPrint(std::stringstream &ss,const std::string &t="") const override;
	};

	class KVObject;
	class KVValueArray
		: public KVValue
	{
	public:
		using KVValue::KVValue;
		const KVObject &GetValue() const;
		virtual KVType GetType() const override;
		virtual void DebugPrint(std::stringstream &ss,const std::string &t="") const override;
	};

	class KVValueArrayTyped
		: public KVValue
	{
	public:
		using KVValue::KVValue;
		const KVObject &GetValue() const;
		virtual KVType GetType() const override;
		virtual void DebugPrint(std::stringstream &ss,const std::string &t="") const override;
	};

	class KVValueObject
		: public KVValue
	{
	public:
		using KVValue::KVValue;
		const KVObject &GetValue() const;
		virtual KVType GetType() const override;
		virtual void DebugPrint(std::stringstream &ss,const std::string &t="") const override;
	};

	class KVObject
		: public std::enable_shared_from_this<KVObject>,
		public IKeyValueCollection
	{
	public:
		KVObject(const std::string &name,bool isArray=false);
		void AddProperty(const std::string &name,const std::shared_ptr<KVValue> &value);
		const std::unordered_map<std::string,std::shared_ptr<KVValue>> &GetValues() const;
		KVValue *FindValue(const std::string &key);
		const KVValue *FindValue(const std::string &key) const;

		template<typename T>
			std::optional<T> FindValue(const std::string &key)
		{
			auto *val = FindValue(key);
			if(val == nullptr)
				return {};
			if constexpr(std::is_same_v<T,nullptr_t>)
				return {};
			else if constexpr(std::is_same_v<T,bool>)
			{
				if(val->GetType() != KVType::BOOLEAN)
					return {};
				return static_cast<KVValueBool*>(val)->GetValue();
			}
			else if constexpr(std::is_same_v<T,int64_t>)
			{
				if(val->GetType() != KVType::INT64)
					return {};
				return static_cast<KVValueInt64*>(val)->GetValue();
			}
			else if constexpr(std::is_same_v<T,uint64_t>)
			{
				if(val->GetType() != KVType::UINT64)
					return {};
				return static_cast<KVValueUInt64*>(val)->GetValue();
			}
			else if constexpr(std::is_same_v<T,int32_t>)
			{
				if(val->GetType() != KVType::INT32)
					return {};
				return static_cast<KVValueInt32*>(val)->GetValue();
			}
			else if constexpr(std::is_same_v<T,uint32_t>)
			{
				if(val->GetType() != KVType::UINT32)
					return {};
				return static_cast<KVValueUInt32*>(val)->GetValue();
			}
			else if constexpr(std::is_same_v<T,double>)
			{
				if(val->GetType() != KVType::DOUBLE)
					return {};
				return static_cast<KVValueDouble*>(val)->GetValue();
			}
			else if constexpr(std::is_same_v<T,std::string>)
			{
				if(val->GetType() != KVType::STRING)
					return {};
				return static_cast<KVValueString*>(val)->GetValue();
			}
			else if constexpr(std::is_same_v<T,const std::string*>)
			{
				if(val->GetType() != KVType::STRING)
					return {};
				return &static_cast<KVValueString*>(val)->GetValue();
			}
			else if constexpr(std::is_same_v<T,const BinaryBlob*>)
			{
				if(val->GetType() != KVType::BINARY_BLOB)
					return {};
				return &static_cast<KVValueBinaryBlob*>(val)->GetValue();
			}
			else if constexpr(std::is_same_v<T,const KVObject*>)
			{
				if(val->GetType() == KVType::ARRAY)
					return &static_cast<KVValueArray*>(val)->GetValue();
				else if(val->GetType() == KVType::ARRAY_TYPED)
					return &static_cast<KVValueArrayTyped*>(val)->GetValue();
				else if(val->GetType() == KVType::OBJECT)
					return &static_cast<KVValueObject*>(val)->GetValue();
				return {};
			}
			return {};
		}

		bool IsArray() const;
		uint32_t GetArrayCount() const;

		template<class TKVValue>
			TKVValue *GetArrayValue(uint32_t idx);
		template<class TKVValue>
			const TKVValue *GetArrayValue(uint32_t idx) const;

		KVValue *GetArrayValue(uint32_t idx,std::optional<KVType> confirmType={});
		const KVValue *GetArrayValue(uint32_t idx,std::optional<KVType> confirmType={}) const;
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

		virtual void Read(const Resource &resource,std::shared_ptr<VFilePtrInternal> f) override;
		void DebugPrint(std::stringstream &ss,const std::string &t="") const;
		virtual BlockType GetType() const override;
		BinaryKV3()=default;
		BinaryKV3(BlockType type);
	private:
		template<class TKVValue,typename T>
			static std::shared_ptr<KVValue> MakeValue(KVType type, T data, KVFlag flag);
		template<class TKVValue>
			static std::shared_ptr<KVValue> MakeValueFromPtr(KVType type, const std::shared_ptr<void> &data, KVFlag flag);
		static KVType ConvertBinaryOnlyKVType(KVType type);
		void ReadVersion2(std::shared_ptr<VFilePtrInternal> f,DataStream &outData);
		void BlockDecompress(std::shared_ptr<VFilePtrInternal> f,DataStream &outData);
		void DecompressLZ4(std::shared_ptr<VFilePtrInternal> f,DataStream &outData);
		std::pair<KVType,KVFlag> ReadType(DataStream &ds);
		std::shared_ptr<KVObject> ReadBinaryValue(const std::string &name, KVType datatype, KVFlag flagInfo, DataStream ds, KVObject *optParent);
		std::shared_ptr<KVObject> ParseBinaryKV3(DataStream &ds,KVObject *optParent,bool inArray=false);
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

template<class TKVValue>
	TKVValue *source2::resource::KVObject::GetArrayValue(uint32_t idx)
{
	if(IsArray() == false)
		return nullptr;
	return dynamic_cast<TKVValue*>(FindValue(std::to_string(idx)));
}
template<class TKVValue>
	const TKVValue *source2::resource::KVObject::GetArrayValue(uint32_t idx) const {return const_cast<KVObject*>(this)->GetArrayValue<TKVValue>(idx);}


#endif
