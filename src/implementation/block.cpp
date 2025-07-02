// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-FileCopyrightText: (c) 2015 Steam Database
// SPDX-License-Identifier: MIT

module;

#include <cinttypes>

module source2;

using namespace source2;

uint32_t resource::Block::GetOffset() const { return m_offset; }
uint32_t resource::Block::GetSize() const { return m_size; }
void resource::Block::SetOffset(uint32_t offset) { m_offset = offset; }
void resource::Block::SetSize(uint32_t size) { m_size = size; }
