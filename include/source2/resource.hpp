/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __SOURCE2_RESOURCE_HPP__
#define __SOURCE2_RESOURCE_HPP__

#include "source2/block.hpp"
#include <vector>
#include <string>

namespace source2::resource
{
	class ResourceData;
	class Resource
	{
	public:
		Block *FindBlock(BlockType type);
		const std::vector<std::shared_ptr<Block>> &GetBlocks() const;
		void Read(std::shared_ptr<VFilePtrInternal> f);
		std::shared_ptr<source2::resource::Block> ConstructFromType(std::string input);
		std::shared_ptr<source2::resource::ResourceData> ConstructResourceType();
	private:
		ResourceType m_resourceType = ResourceType::Unknown;
		std::vector<std::shared_ptr<Block>> m_blocks = {};
		uint16_t m_version = 0u;
	};
};

#endif
