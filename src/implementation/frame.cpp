// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <unordered_map>

#include <string>

module source2;

using namespace source2;

void resource::Frame::SetPosition(const std::string &bone, const Vector3 &pos)
{
	InsertIfUnknown(bone);
	m_bones[bone].position = pos;
}
void resource::Frame::SetRotation(const std::string &bone, const Quat &rot)
{
	InsertIfUnknown(bone);
	m_bones[bone].rotation = rot;
}
const std::unordered_map<std::string, resource::FrameBone> &resource::Frame::GetBones() const { return m_bones; }

void resource::Frame::InsertIfUnknown(const std::string &bone)
{
	auto it = m_bones.find(bone);
	if(it != m_bones.end())
		return;
	m_bones.insert(std::make_pair(bone, FrameBone {Vector3 {}, uquat::identity()}));
}
