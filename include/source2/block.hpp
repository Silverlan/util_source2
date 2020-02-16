/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __SOURCE2_BLOCK_HPP__
#define __SOURCE2_BLOCK_HPP__

#include "util_source2.hpp"
#include <memory>

namespace source2::resource
{
	class Block
		: public std::enable_shared_from_this<Block>
	{
	public:
		virtual BlockType GetType() const=0;
		virtual void Read(std::shared_ptr<VFilePtrInternal> f)=0;
		uint32_t GetOffset() const;
		uint32_t GetSize() const;
		void SetOffset(uint32_t offset);
		void SetSize(uint32_t size);
	private:
		uint32_t m_offset = 0u;
		uint32_t m_size = 0u;
	};
};

#endif
