// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

module source2;

using namespace source2;

int32_t resource::AnimDecoder::GetSize(AnimDecoderType t)
{
	switch(t) {
	case AnimDecoderType::CCompressedFullVector3:
		return 12;
	case AnimDecoderType::CCompressedStaticVector3:
	case AnimDecoderType::CCompressedAnimVector3:
	case AnimDecoderType::CCompressedAnimQuaternion:
		return 6;
	default:
		break;
	}

	return 0;
}
static std::unordered_map<std::string, resource::AnimDecoderType> g_animDecoderStringToType = {
  {"CCompressedReferenceFloat", resource::AnimDecoderType::CCompressedReferenceFloat},
  {"CCompressedStaticFloat", resource::AnimDecoderType::CCompressedStaticFloat},
  {"CCompressedFullFloat", resource::AnimDecoderType::CCompressedFullFloat},
  {"CCompressedReferenceVector3", resource::AnimDecoderType::CCompressedReferenceVector3},
  {"CCompressedStaticVector3", resource::AnimDecoderType::CCompressedStaticVector3},
  {"CCompressedStaticFullVector3", resource::AnimDecoderType::CCompressedStaticFullVector3},
  {"CCompressedAnimVector3", resource::AnimDecoderType::CCompressedAnimVector3},
  {"CCompressedDeltaVector3", resource::AnimDecoderType::CCompressedDeltaVector3},
  {"CCompressedFullVector3", resource::AnimDecoderType::CCompressedFullVector3},
  {"CCompressedReferenceQuaternion", resource::AnimDecoderType::CCompressedReferenceQuaternion},
  {"CCompressedStaticQuaternion", resource::AnimDecoderType::CCompressedStaticQuaternion},
  {"CCompressedAnimQuaternion", resource::AnimDecoderType::CCompressedAnimQuaternion},
  {"CCompressedReferenceInt", resource::AnimDecoderType::CCompressedReferenceInt},
  {"CCompressedStaticChar", resource::AnimDecoderType::CCompressedStaticChar},
  {"CCompressedFullChar", resource::AnimDecoderType::CCompressedFullChar},
  {"CCompressedStaticShort", resource::AnimDecoderType::CCompressedStaticShort},
  {"CCompressedFullShort", resource::AnimDecoderType::CCompressedFullShort},
  {"CCompressedStaticInt", resource::AnimDecoderType::CCompressedStaticInt},
  {"CCompressedFullInt", resource::AnimDecoderType::CCompressedFullInt},
  {"CCompressedReferenceBool", resource::AnimDecoderType::CCompressedReferenceBool},
  {"CCompressedStaticBool", resource::AnimDecoderType::CCompressedStaticBool},
  {"CCompressedFullBool", resource::AnimDecoderType::CCompressedFullBool},
  {"CCompressedReferenceColor32", resource::AnimDecoderType::CCompressedReferenceColor32},
  {"CCompressedStaticColor32", resource::AnimDecoderType::CCompressedStaticColor32},
  {"CCompressedFullColor32", resource::AnimDecoderType::CCompressedFullColor32},
  {"CCompressedReferenceVector2D", resource::AnimDecoderType::CCompressedReferenceVector2D},
  {"CCompressedStaticVector2D", resource::AnimDecoderType::CCompressedStaticVector2D},
  {"CCompressedFullVector2D", resource::AnimDecoderType::CCompressedFullVector2D},
  {"CCompressedReferenceVector4D", resource::AnimDecoderType::CCompressedReferenceVector4D},
  {"CCompressedStaticVector4D", resource::AnimDecoderType::CCompressedStaticVector4D},
  {"CCompressedFullVector4D", resource::AnimDecoderType::CCompressedFullVector4D},
};
resource::AnimDecoderType resource::AnimDecoder::FromString(const std::string &str)
{
	auto it = g_animDecoderStringToType.find(str);
	if(it == g_animDecoderStringToType.end())
		return AnimDecoderType::Unknown;
	return it->second;
}

/////////////

std::vector<std::shared_ptr<resource::Animation>> resource::Animation::CreateAnimations(IKeyValueCollection &animationData, IKeyValueCollection &decodeKey)
{
	auto animArray = IKeyValueCollection::FindArrayValues<IKeyValueCollection *>(animationData, "m_animArray");
	if(animArray.empty())
		return {};
	auto decoderArray = MakeDecoderArray(animationData.FindArrayValues<IKeyValueCollection *>("m_decoderArray"));
	auto segmentArray = animationData.FindArrayValues<IKeyValueCollection *>("m_segmentArray");
	std::vector<std::shared_ptr<Animation>> anims {};
	anims.reserve(segmentArray.size());
	for(auto *anim : animArray) {
		auto animStrct = Animation::Create(*anim, decodeKey, decoderArray, segmentArray);
		if(animStrct == nullptr)
			continue;
		anims.push_back(animStrct);
	}
	return anims;
}
std::shared_ptr<resource::Animation> resource::Animation::Create(IKeyValueCollection &animDesc, IKeyValueCollection &decodeKey, const std::vector<AnimDecoderType> &decoderArray, const std::vector<IKeyValueCollection *> &segmentArray)
{
	return std::shared_ptr<Animation> {new Animation {animDesc, decodeKey, decoderArray, segmentArray}};
}
resource::Animation::Animation(IKeyValueCollection &animDesc, IKeyValueCollection &decodeKey, const std::vector<AnimDecoderType> &decoderArray, const std::vector<IKeyValueCollection *> &segmentArray) { ConstructFromDesc(animDesc, decodeKey, decoderArray, segmentArray); }
const std::string &resource::Animation::GetName() const { return m_name; }
float resource::Animation::GetFPS() const { return m_fps; }
const std::vector<std::shared_ptr<resource::Frame>> &resource::Animation::GetFrames() const { return m_frames; }
Quat resource::Animation::ReadQuaternion(pragma::util::DataStream &ds)
{
	auto bytes = ds->Read<std::array<uint8_t, 6>>();

	// Values
	auto i1 = bytes[0] + ((bytes[1] & 63) << 8);
	auto i2 = bytes[2] + ((bytes[3] & 63) << 8);
	auto i3 = bytes[4] + ((bytes[5] & 63) << 8);

	// Signs
	auto s1 = bytes[1] & 128;
	auto s2 = bytes[3] & 128;
	auto s3 = bytes[5] & 128;

	auto c = static_cast<float>(pragma::math::sin(pragma::math::pi / 4.0f)) / 16384.0f;
	auto t1 = static_cast<float>(pragma::math::sin(pragma::math::pi / 4.0f));
	auto x = (bytes[1] & 64) == 0 ? c * (i1 - 16384) : c * i1;
	auto y = (bytes[3] & 64) == 0 ? c * (i2 - 16384) : c * i2;
	auto z = (bytes[5] & 64) == 0 ? c * (i3 - 16384) : c * i3;

	auto w = static_cast<float>(pragma::math::sqrt(1 - (x * x) - (y * y) - (z * z)));

	// Apply sign 3
	if(s3 == 128)
		w *= -1;

	// Apply sign 1 and 2
	if(s1 == 128)
		return s2 == 128 ? Quat {x, y, z, w} : Quat {y, z, w, x};

	return s2 == 128 ? Quat {z, w, x, y} : Quat {w, x, y, z};
}
std::vector<resource::AnimDecoderType> resource::Animation::MakeDecoderArray(const std::vector<IKeyValueCollection *> &decoderArray)
{
	std::vector<AnimDecoderType> array;
	array.reserve(decoderArray.size());
	for(auto *decoder : decoderArray) {
		auto name = IKeyValueCollection::FindValue<std::string>(*decoder, "m_szName");
		array.push_back(AnimDecoder::FromString(name.has_value() ? *name : ""));
	}
	return array;
}
void resource::Animation::ReadSegment(int64_t frame, IKeyValueCollection &segment, IKeyValueCollection &decodeKey, const std::vector<AnimDecoderType> &decoderArray, Frame &outFrame, uint32_t numFrames)
{
	// Clamp the frame number to be between 0 and the maximum frame
	frame = frame < 0 ? 0 : frame;
	frame = frame >= numFrames ? numFrames - 1 : frame;

	auto localChannel = segment.FindValue<int32_t>("m_nLocalChannel", 0);
	auto *dataChannel = decodeKey.FindArrayValues<IKeyValueCollection *>("m_dataChannelArray").at(localChannel);
	auto boneNames = dataChannel->FindArrayValues<std::string>("m_szElementNameArray");

	auto channelAttribute = dataChannel->FindValue<std::string>("m_szVariableName", "");

	// Read container
	auto *container = segment.FindBinaryBlob("m_container");
	if(container == nullptr)
		return;
	pragma::util::DataStream ds {container->data(), static_cast<uint32_t>(container->size())};
	ds->SetOffset(0);
	auto elementIndexArray = dataChannel->FindArrayValues<int32_t>("m_nElementIndexArray");
	auto numChannelElements = decodeKey.FindValue<int32_t>("m_nChannelElements", 0);
	std::vector<int32_t> elementBones {};
	elementBones.resize(numChannelElements);
	for(auto i = decltype(elementIndexArray.size()) {0u}; i < elementIndexArray.size(); ++i)
		elementBones.at(elementIndexArray.at(i)) = i;

	// Read header
	auto decoderIdx = ds->Read<int16_t>();
	auto cardinality = ds->Read<int16_t>();
	auto numBones = ds->Read<int16_t>();
	auto totalLength = ds->Read<int16_t>();
	auto decoder = decoderArray.at(decoderIdx);

	// Read bone list
	std::vector<int32_t> elements {};
	elements.reserve(numBones);
	for(auto i = decltype(numBones) {0u}; i < numBones; ++i) {
		auto el = ds->Read<int16_t>();
		elements.push_back(el);
	}

	// Skip data to find the data for the current frame.
	// Structure is just | Bone 0 - Frame 0 | Bone 1 - Frame 0 | Bone 0 - Frame 1 | Bone 1 - Frame 1|
	if(ds->GetOffset() + (AnimDecoder::GetSize(decoder) * frame * numBones) < ds->GetSize())
		ds->SetOffset(ds->GetOffset() + AnimDecoder::GetSize(decoder) * frame * numBones);

	// Read animation data for all bones
	for(auto element = decltype(numBones) {0u}; element < numBones; ++element) {
		// Get the bone we are reading for
		auto &bone = elementBones.at(elements.at(element));

		// Look at the decoder to see what to read
		switch(decoder) {
		case AnimDecoderType::CCompressedStaticFullVector3:
		case AnimDecoderType::CCompressedFullVector3:
		case AnimDecoderType::CCompressedDeltaVector3:
			{
				if(channelAttribute == "Position") {
					auto pos = ds->Read<Vector3>();
					outFrame.SetPosition(boneNames.at(bone), pos);
				}
				break;
			}
		case AnimDecoderType::CCompressedAnimVector3:
		case AnimDecoderType::CCompressedStaticVector3:
			{
				if(channelAttribute == "Position") {
					auto x = ds->Read<uint16_t>();
					auto y = ds->Read<uint16_t>();
					auto z = ds->Read<uint16_t>();
					Vector3 pos {pragma::math::float16_to_float32_glm(x), pragma::math::float16_to_float32_glm(y), pragma::math::float16_to_float32_glm(z)};
					outFrame.SetPosition(boneNames.at(bone), pos);
				}
				break;
			}
		case AnimDecoderType::CCompressedAnimQuaternion:
			if(channelAttribute == "Angle")
				outFrame.SetRotation(boneNames.at(bone), ReadQuaternion(ds));
			break;
		default:
			break;
		}
	}
}
void resource::Animation::ConstructFromDesc(IKeyValueCollection &animDesc, IKeyValueCollection &decodeKey, const std::vector<AnimDecoderType> &decoderArray, const std::vector<IKeyValueCollection *> &segmentArray)
{
	// Get animation properties
	m_name = animDesc.FindValue<std::string>("m_name", "");
	m_fps = animDesc.FindValue<float>("fps", 0.f);

	IKeyValueCollection *pData = nullptr;
	auto aData = animDesc.FindArrayValues<IKeyValueCollection *>("m_pData");
	if(aData.empty())
		pData = animDesc.FindSubCollection("m_pData");
	else
		pData = aData.front();
	if(pData == nullptr)
		return;
	auto frameBlockArray = pData->FindArrayValues<IKeyValueCollection *>("m_frameblockArray");

	auto numFrames = pData->FindValue<int32_t>("m_nFrames", 0);
	std::vector<std::shared_ptr<Frame>> frames {};
	frames.reserve(numFrames);

	// Figure out each frame
	for(auto frameIndex = decltype(numFrames) {0u}; frameIndex < numFrames; ++frameIndex) {
		// Create new frame object
		m_frames.push_back(std::make_shared<Frame>());
		auto &frame = m_frames.back();

		// Read all frame blocks
		for(auto &frameBlock : frameBlockArray) {
			auto startFrame = frameBlock->FindValue<int32_t>("m_nStartFrame", 0);
			auto endFrame = frameBlock->FindValue<int32_t>("m_nEndFrame", 0);

			// Only consider blocks that actual contain info for this frame
			if(frameIndex >= startFrame && frameIndex <= endFrame) {
				auto segmentIndexArray = frameBlock->FindArrayValues<int32_t>("m_segmentIndexArray");

				for(auto segmentIndex : segmentIndexArray) {
					auto segment = segmentArray.at(segmentIndex);
					ReadSegment(frameIndex - startFrame, *segment, decodeKey, decoderArray, *frame, numFrames);
				}
			}
		}
	}
}
