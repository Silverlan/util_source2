// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-FileCopyrightText: (c) 2015 Steam Database
// SPDX-License-Identifier: MIT

module;

#include <string>
#include <vector>
#include <cinttypes>
#include "definitions.hpp"

export module source2:redi;

import :block;

export namespace source2::resource {
	class DLLUS2 REDIBlock : public Block {
	  public:
		virtual BlockType GetType() const override;
		virtual void DebugPrint(std::stringstream &ss, const std::string &t = "") const = 0;
	};

	class DLLUS2 InputDependencies : public REDIBlock {
	  public:
		struct InputDependency {
			std::string contentRelativeFilename;
			std::string contentSearchPath;
			uint32_t fileCRC;
			uint32_t flags;

			void DebugPrint(std::stringstream &ss, const std::string &t = "") const;
		};
		virtual void Read(const Resource &resource, ufile::IFile &f) override;
		virtual void DebugPrint(std::stringstream &ss, const std::string &t = "") const override;
	  private:
		std::vector<InputDependency> m_inputDependencies {};
	};

	class DLLUS2 AdditionalInputDependencies : public InputDependencies {};

	class DLLUS2 ArgumentDependencies : public REDIBlock {
	  public:
		struct ArgumentDependency {
			std::string parameterName;
			std::string parameterType;
			uint32_t fingerprint = 0;
			uint32_t fingerprintDefault = 0;

			void DebugPrint(std::stringstream &ss, const std::string &t = "") const;
		};
		virtual void Read(const Resource &resource, ufile::IFile &f) override;
		virtual void DebugPrint(std::stringstream &ss, const std::string &t = "") const override;
	  private:
		std::vector<ArgumentDependency> m_argumentDependencies {};
	};

	class DLLUS2 SpecialDependencies : public REDIBlock {
	  public:
		struct SpecialDependency {
			std::string string;
			std::string compilerIdentifier;
			uint32_t fingerprint = 0;
			uint32_t userData = 0;

			void DebugPrint(std::stringstream &ss, const std::string &t = "") const;
		};
		static ResourceType DetermineResourceTypeByCompilerIdentifier(const SpecialDependency &input);
		virtual void Read(const Resource &resource, ufile::IFile &f) override;
		virtual void DebugPrint(std::stringstream &ss, const std::string &t = "") const override;
		const std::vector<SpecialDependency> &GetSpecialDependencies() const;
	  private:
		std::vector<SpecialDependency> m_specialDependencies {};
	};

	class DLLUS2 CustomDependencies : public REDIBlock {
	  public:
		virtual void Read(const Resource &resource, ufile::IFile &f) override;
		virtual void DebugPrint(std::stringstream &ss, const std::string &t = "") const override;
	};

	class DLLUS2 AdditionalRelatedFiles : public REDIBlock {
	  public:
		struct AdditionalRelatedFile {
			std::string contentRelativeFilename;
			std::string contentSearchPath;

			void DebugPrint(std::stringstream &ss, const std::string &t = "") const;
		};
		virtual void Read(const Resource &resource, ufile::IFile &f) override;
		virtual void DebugPrint(std::stringstream &ss, const std::string &t = "") const override;
	  private:
		std::vector<AdditionalRelatedFile> m_additionalRelatedFiles {};
	};

	class DLLUS2 ChildResourceList : public REDIBlock {
	  public:
		struct ReferenceInfo {
			uint64_t id = 0;
			std::string resourceName;

			void DebugPrint(std::stringstream &ss, const std::string &t = "") const;
		};
		virtual void Read(const Resource &resource, ufile::IFile &f) override;
		virtual void DebugPrint(std::stringstream &ss, const std::string &t = "") const override;
	  private:
		std::vector<ReferenceInfo> m_references {};
	};

	class DLLUS2 ExtraIntData : public REDIBlock {
	  public:
		struct EditIntData {
			std::string name;
			int32_t value = 0;

			void DebugPrint(std::stringstream &ss, const std::string &t = "") const;
		};
		virtual void Read(const Resource &resource, ufile::IFile &f) override;
		virtual void DebugPrint(std::stringstream &ss, const std::string &t = "") const override;
	  private:
		std::vector<EditIntData> m_editIntData {};
	};

	class DLLUS2 ExtraFloatData : public REDIBlock {
	  public:
		struct EditFloatData {
			std::string name;
			float value = 0;

			void DebugPrint(std::stringstream &ss, const std::string &t = "") const;
		};
		virtual void Read(const Resource &resource, ufile::IFile &f) override;
		virtual void DebugPrint(std::stringstream &ss, const std::string &t = "") const override;
	  private:
		std::vector<EditFloatData> m_editFloatData {};
	};

	class DLLUS2 ExtraStringData : public REDIBlock {
	  public:
		struct EditStringData {
			std::string name;
			std::string value;

			void DebugPrint(std::stringstream &ss, const std::string &t = "") const;
		};
		virtual void Read(const Resource &resource, ufile::IFile &f) override;
		virtual void DebugPrint(std::stringstream &ss, const std::string &t = "") const override;
	  private:
		std::vector<EditStringData> m_editStringData {};
	};
};
