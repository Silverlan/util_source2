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
		virtual void Read(const Resource &resource,std::shared_ptr<VFilePtrInternal> f)=0;
		virtual void DebugPrint(std::stringstream &ss,const std::string &t="") const=0;
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
