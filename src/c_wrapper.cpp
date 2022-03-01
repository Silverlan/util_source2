/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*
* Copyright (c) 2022 Silverlan
*/

#include "util_source2.hpp"
#include "source2/resource.hpp"
#include "source2/resource_data.hpp"
#include <sharedutils/util.h>
#include <sharedutils/util_file.h>
#include <type_traits>
#include <fsys/ifile.hpp>

#ifdef __linux__
	#define DLLS2 __attribute__((visibility("default")))
#else
	#define DLLS2  __declspec(dllexport)   // export DLL information
#endif

struct ResourceWrapper
{
	std::unique_ptr<fsys::File> file = nullptr;
	std::shared_ptr<source2::resource::Resource> resource = nullptr;
	std::string basePath;
};
static std::unique_ptr<ResourceWrapper> load_resource(const std::string &fileName,const std::optional<std::string> &path)
{
	auto fp = filemanager::open_system_file(fileName,filemanager::FileMode::Read | filemanager::FileMode::Binary);
	if(fp == nullptr)
	{
		if(path.has_value())
		{
			// Try to find resource relative to model location
			auto relFileName = ufile::get_file_from_filename(fileName);
			return load_resource(*path +relFileName,{});
		}
		return nullptr;
	}
	auto wrapper = std::make_unique<ResourceWrapper>();
	wrapper->file = std::make_unique<fsys::File>(fp);
	wrapper->basePath = ufile::get_path_from_filename(fileName);
	wrapper->resource = source2::load_resource(*wrapper->file,[](const std::string &path) -> std::unique_ptr<ufile::IFile> {
		auto fp = filemanager::open_system_file(path,filemanager::FileMode::Read | filemanager::FileMode::Binary);
		if(!fp)
			return nullptr;
		return std::make_unique<fsys::File>(fp);
	});
	return wrapper->resource ? std::move(wrapper) : nullptr;
}

using FrameBoneIterator = util::base_type<std::invoke_result_t<decltype(&source2::resource::Frame::GetBones),source2::resource::Frame>>::const_iterator;
using S2Anim = std::shared_ptr<source2::resource::Animation>;
extern "C" {
	DLLS2 ResourceWrapper *us2_open_resource(const char *fileName)
	{
		auto res = load_resource(fileName,{});
		if(!res)
			return nullptr;
		return res.release();
	}
	DLLS2 void us2_close_resource(ResourceWrapper *res) {delete res;}
	DLLS2 S2Anim **us2_animation_group_load(ResourceWrapper *res,uint32_t *outNumAnims)
	{
		auto *dataBlock = dynamic_cast<source2::resource::ResourceData*>(res->resource->FindBlock(source2::BlockType::DATA));
		auto data = dataBlock ? dataBlock->GetData() : nullptr;
		if(data == nullptr)
			return nullptr;
		auto animArray = data->FindArrayValues<std::string>("m_localHAnimArray");
		auto *decodeKey = data->FindSubCollection("m_decodeKey");
		if(decodeKey == nullptr)
			return nullptr;
		std::vector<std::shared_ptr<source2::resource::Animation>> animations;
		for(auto &animFile : animArray)
		{
			auto fileName = animFile +"_c";
			auto resource = ::load_resource(fileName,res->basePath);
			if(resource == nullptr)
				return nullptr;
			auto *dataBlock = dynamic_cast<source2::resource::ResourceData*>(resource->resource->FindBlock(source2::BlockType::DATA));
			auto data = dataBlock ? dataBlock->GetData() : nullptr;
			if(data == nullptr)
				continue;
			auto anims = source2::resource::Animation::CreateAnimations(*data,*decodeKey);
			animations.reserve(animations.size() +anims.size());
			for(auto &anim : anims)
				animations.push_back(anim);
		}

		*outNumAnims = animations.size();
		auto *r = new S2Anim*[animations.size()];
		for(uint32_t idx = 0; auto &anim : animations)
			r[idx++] = new S2Anim{anim};
		return r;
	}
	DLLS2 void us2_animation_group_destroy(S2Anim **anims,uint32_t numAnims)
	{
		for(auto i=decltype(numAnims){0u};i<numAnims;++i)
			delete anims[i];
		delete anims;
	}
	DLLS2 const char *us2_animation_get_name(source2::resource::Animation *animation)
	{
		return animation->GetName().c_str();
	}
	DLLS2 float us2_animation_get_fps(source2::resource::Animation *animation)
	{
		return animation->GetFPS();
	}
	DLLS2 uint32_t us2_animation_get_frame_count(source2::resource::Animation *animation)
	{
		return animation->GetFrames().size();
	}
	DLLS2 source2::resource::Frame *us2_animation_get_frame(source2::resource::Animation *animation,uint32_t idx)
	{
		auto &frames = animation->GetFrames();
		if(idx >= frames.size())
			return nullptr;
		return frames[idx].get();
	}
	
	DLLS2 uint32_t us2_animation_frame_get_bone_count(source2::resource::Frame *frame)
	{
		return frame->GetBones().size();
	}
	DLLS2 FrameBoneIterator *us2_animation_frame_bone_iterator_create(source2::resource::Frame *frame)
	{
		auto &bones = frame->GetBones();
		if(bones.empty())
			return nullptr;
		auto it = bones.begin();
		return new FrameBoneIterator{it};
	}
	DLLS2 void us2_animation_frame_bone_iterator_destroy(FrameBoneIterator *it) {delete it;}
	DLLS2 bool us2_animation_frame_bone_iterator_next(source2::resource::Frame *frame,FrameBoneIterator *it)
	{
		++(*it);
		return *it != frame->GetBones().end();
	}
	DLLS2 const char *us2_animation_frame_bone_iterator_get_name(FrameBoneIterator *it) {return (*it)->first.c_str();}
	DLLS2 const source2::resource::FrameBone *us2_animation_frame_bone_iterator_get_bone(FrameBoneIterator *it) {return &(*it)->second;}

	DLLS2 void us2_animation_frame_bone_get_position(const source2::resource::FrameBone *frameBone,float *outPos)
	{
		memcpy(outPos,&frameBone->position,sizeof(frameBone->position));
	}
	DLLS2 void us2_animation_frame_bone_get_rotation(const source2::resource::FrameBone *frameBone,float *outRot)
	{
		memcpy(outRot,&frameBone->rotation,sizeof(frameBone->rotation));
	}
}
