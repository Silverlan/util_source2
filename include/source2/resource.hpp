/*
MIT License

Copyright (c) 2021 Silverlan
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

#ifndef __SOURCE2_RESOURCE_HPP__
#define __SOURCE2_RESOURCE_HPP__

#include "source2/block.hpp"
#include <vector>
#include <string>

namespace ufile {
	struct IFile;
};
namespace source2::resource {
	class ResourceData;
	class ResourceIntrospectionManifest;
	class ResourceExtRefList;
	class DLLUS2 Resource {
	  public:
		Resource(const std::function<std::unique_ptr<ufile::IFile>(const std::string &)> &assetFileLoader);
		std::unique_ptr<ufile::IFile> OpenAssetFile(const std::string &path) const;
		std::shared_ptr<Resource> LoadResource(const std::string &path) const;

		Block *FindBlock(BlockType type);
		const Block *FindBlock(BlockType type) const;
		const std::vector<std::shared_ptr<Block>> &GetBlocks() const;
		std::shared_ptr<Block> GetBlock(uint32_t idx) const;
		bool Read(ufile::IFile &f);
		std::shared_ptr<source2::resource::Block> ConstructFromType(std::string input);
		std::shared_ptr<source2::resource::ResourceData> ConstructResourceType();

		ResourceIntrospectionManifest *GetIntrospectionManifest();
		const ResourceIntrospectionManifest *GetIntrospectionManifest() const;

		ResourceExtRefList *GetExternalReferences();
		const ResourceExtRefList *GetExternalReferences() const;

		uint32_t GetVersion() const;
	  private:
		static bool IsHandledResourceType(ResourceType type);
		ResourceType m_resourceType = ResourceType::Unknown;
		std::vector<std::shared_ptr<Block>> m_blocks = {};
		std::function<std::unique_ptr<ufile::IFile>(const std::string &)> m_assetFileLoader = nullptr;
		uint16_t m_version = 0u;
	};
};

#endif
