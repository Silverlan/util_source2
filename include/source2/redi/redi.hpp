/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __SOURCE2_REDI_HPP__
#define __SOURCE2_REDI_HPP__

#include "util_source2.hpp"
#include "source2/block.hpp"
#include <string>
#include <vector>
#include <cinttypes>

namespace source2::resource
{
	class REDIBlock
		: public Block
	{
	public:
		virtual BlockType GetType() const override;
	};

	class InputDependencies
		: public REDIBlock
	{
	public:
		struct InputDependency
		{
			std::string contentRelativeFilename;
			std::string contentSearchPath;
			uint32_t fileCRC;
			uint32_t flags;
		};
		virtual void Read(std::shared_ptr<VFilePtrInternal> f) override;
	private:
		std::vector<InputDependency> m_inputDependencies {};
	};

	class AdditionalInputDependencies
		: public InputDependencies
	{};

	class ArgumentDependencies
		: public REDIBlock
	{
	public:
		struct ArgumentDependency
		{
			std::string parameterName;
			std::string parameterType;
			uint32_t fingerprint = 0;
			uint32_t fingerprintDefault = 0;
		};
		virtual void Read(std::shared_ptr<VFilePtrInternal> f) override;
	private:
		std::vector<ArgumentDependency> m_argumentDependencies {};
	};

	class SpecialDependencies
		: public REDIBlock
	{
	public:
		struct SpecialDependency
		{
			std::string string;
			std::string compilerIdentifier;
			uint32_t fingerprint = 0;
			uint32_t userData = 0;
		};
		static ResourceType DetermineResourceTypeByCompilerIdentifier(const SpecialDependency &input);
		virtual void Read(std::shared_ptr<VFilePtrInternal> f) override;
		const std::vector<SpecialDependency> &GetSpecialDependencies() const;
	private:
		std::vector<SpecialDependency> m_specialDependencies {};
	};

	class CustomDependencies
		: public REDIBlock
	{
	public:
		virtual void Read(std::shared_ptr<VFilePtrInternal> f) override;
	};

	class AdditionalRelatedFiles
		: public REDIBlock
	{
	public:
		struct AdditionalRelatedFile
		{
			std::string contentRelativeFilename;
			std::string contentSearchPath;
		};
		virtual void Read(std::shared_ptr<VFilePtrInternal> f) override;
	private:
		std::vector<AdditionalRelatedFile> m_additionalRelatedFiles {};
	};

	class ChildResourceList
		: public REDIBlock
	{
	public:
		struct ReferenceInfo
		{
			uint64_t id = 0;
			std::string resourceName;
		};
		virtual void Read(std::shared_ptr<VFilePtrInternal> f) override;
	private:
		std::vector<ReferenceInfo> m_references {};
	};

	class ExtraIntData
		: public REDIBlock
	{
	public:
		struct EditIntData
		{
			std::string name;
			int32_t value = 0;
		};
		virtual void Read(std::shared_ptr<VFilePtrInternal> f) override;
	private:
		std::vector<EditIntData> m_editIntData {};
	};

	class ExtraFloatData
		: public REDIBlock
	{
	public:
		struct EditFloatData
		{
			std::string name;
			float value = 0;
		};
		virtual void Read(std::shared_ptr<VFilePtrInternal> f) override;
	private:
		std::vector<EditFloatData> m_editFloatData {};
	};

	class ExtraStringData
		: public REDIBlock
	{
	public:
		struct EditStringData
		{
			std::string name;
			std::string value;
		};
		virtual void Read(std::shared_ptr<VFilePtrInternal> f) override;
	private:
		std::vector<EditStringData> m_editStringData {};
	};
};

#endif
