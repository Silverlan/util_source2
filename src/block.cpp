/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "source2/block.hpp"

using namespace source2;

uint32_t resource::Block::GetOffset() const {return m_offset;}
uint32_t resource::Block::GetSize() const {return m_size;}
void resource::Block::SetOffset(uint32_t offset) {m_offset = offset;}
void resource::Block::SetSize(uint32_t size) {m_size = size;}
