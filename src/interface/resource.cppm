// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-FileCopyrightText: (c) 2015 Steam Database
// SPDX-License-Identifier: MIT

module;

#include "definitions.hpp"

export module source2:resource;

import :block;

export namespace source2::resource {
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
		std::shared_ptr<Block> ConstructFromType(std::string input);
		std::shared_ptr<ResourceData> ConstructResourceType();

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
