/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "source2/resource_edit_info.hpp"
#include "source2/redi/redi.hpp"
#include <fsys/filesystem.h>

using namespace source2;

#pragma optimize("",off)
std::shared_ptr<resource::REDIBlock> resource::ResourceEditInfo::ConstructStruct(REDIStruct id)
{
	switch (id)
	{
	case REDIStruct::InputDependencies:
		return std::make_shared<InputDependencies>();
	case REDIStruct::AdditionalInputDependencies:
		return std::make_shared<AdditionalInputDependencies>();
	case REDIStruct::ArgumentDependencies:
		return std::make_shared<ArgumentDependencies>();
	case REDIStruct::SpecialDependencies:
		return std::make_shared<SpecialDependencies>();
	case REDIStruct::CustomDependencies:
		return std::make_shared<CustomDependencies>();
	case REDIStruct::AdditionalRelatedFiles:
		return std::make_shared<AdditionalRelatedFiles>();
	case REDIStruct::ChildResourceList:
		return std::make_shared<ChildResourceList>();
	case REDIStruct::ExtraIntData:
		return std::make_shared<ExtraIntData>();
	case REDIStruct::ExtraFloatData:
		return std::make_shared<ExtraFloatData>();
	case REDIStruct::ExtraStringData:
		return std::make_shared<ExtraStringData>();
	}

	throw new std::invalid_argument{"Unknown struct in REDI block."};
}

resource::REDIBlock &resource::ResourceEditInfo::GetStruct(REDIStruct type)
{
	return *m_structs.at(umath::to_integral(type));
}
BlockType resource::ResourceEditInfo::GetType() const {return BlockType::REDI;}
void resource::ResourceEditInfo::Read(std::shared_ptr<VFilePtrInternal> f)
{
	f->Seek(GetOffset());
	for(auto i=decltype(m_structs.size()){0u};i<m_structs.size();++i)
	{
		auto block = ConstructStruct(static_cast<REDIStruct>(i));
		auto offset = f->Tell();
		block->SetOffset(offset +f->Read<uint32_t>());
		block->SetSize(f->Read<uint32_t>());
		offset = f->Tell();
		block->Read(f);
		m_structs.at(i) = block;
		f->Seek(offset);
	}
}

///////////

BlockType resource::ResourceExtRefList::GetType() const {return BlockType::RERL;}
void resource::ResourceExtRefList::Read(std::shared_ptr<VFilePtrInternal> f)
{
	f->Seek(GetOffset());

	auto offset = f->Read<uint32_t>();
	auto size = f->Read<uint32_t>();

	if (size == 0)
		return;

	f->Seek(f->Tell() +offset -sizeof(uint32_t) *2);
	m_resourceReferenceInfos.resize(size);
	for(auto i=decltype(size){0u};i<size;++i)
	{
		auto &resInfo = m_resourceReferenceInfos.at(i);
		resInfo.id = f->Read<uint64_t>();

		auto previousPosition = f->Tell();

		// jump to string
		// offset is counted from current position,
		// so we will need to add 8 to position later
		f->Seek(previousPosition +f->Read<int64_t>());

		resInfo.name = f->ReadString(); // TODO: UTF8

		f->Seek(previousPosition +sizeof(int64_t)); // 8 is to account for string offset
	}
}
#pragma optimize("",on)
