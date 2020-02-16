/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __SOURCE2_RESOURCE_EDIT_INFO_HPP__
#define __SOURCE2_RESOURCE_EDIT_INFO_HPP__

#include "util_source2.hpp"
#include "block.hpp"
#include <array>
#include <memory>
#include <mathutil/umath.h>

namespace source2::resource
{
	class REDIBlock;
	class ResourceEditInfo
		: public Block
	{
	public:
		static std::shared_ptr<REDIBlock> ConstructStruct(REDIStruct id);

		REDIBlock &GetStruct(REDIStruct type);
		virtual BlockType GetType() const override;
		virtual void Read(std::shared_ptr<VFilePtrInternal> f) override;
	private:
		std::array<std::shared_ptr<REDIBlock>,umath::to_integral(REDIStruct::Count)> m_structs = {};
	};

	class ResourceExtRefList
		: public Block
	{
	public:
		struct ResourceReferenceInfo
		{
			uint64_t id = 0;
			std::string name;
		};
		virtual BlockType GetType() const override;
		virtual void Read(std::shared_ptr<VFilePtrInternal> f) override;
	private:
		std::vector<ResourceReferenceInfo> m_resourceReferenceInfos = {};
	};
};

#endif
