// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-FileCopyrightText: (c) 2015 Steam Database
// SPDX-License-Identifier: MIT

module;

#include "definitions.hpp"

export module source2:block;

import :core;

export namespace source2::resource {
	class DLLUS2 Block : public std::enable_shared_from_this<Block> {
	  public:
		virtual BlockType GetType() const = 0;
		virtual void Read(const Resource &resource, ufile::IFile &f) = 0;
		virtual void DebugPrint(std::stringstream &ss, const std::string &t = "") const = 0;
		uint32_t GetOffset() const;
		uint32_t GetSize() const;
		void SetOffset(uint32_t offset);
		void SetSize(uint32_t size);
	  private:
		uint32_t m_offset = 0u;
		uint32_t m_size = 0u;
	};
};
