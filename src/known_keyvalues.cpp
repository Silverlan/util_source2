#include "source2/impl.hpp"
#include "murmur_hash2.hpp"
#include "source2/resource_data.hpp"

#pragma optimize("",off)
const std::unordered_map<uint32_t,std::string> &source2::impl::get_known_keyvalues()
{
	static auto initialized = false;
	static std::unordered_map<uint32_t,std::string> knownKeyValues {};
	if(initialized == false)
	{
		initialized = true;
		std::vector<std::string> list = {
#include "keyvalue_list.txt"
		};
		for(auto &str : list)
		{
			auto hash = murmur2::hash(str,resource::Entity::MURMUR2_SEED);
			knownKeyValues[hash] = str;
		}
	}
	return knownKeyValues;
}
std::optional<std::string> source2::impl::hash_to_keyvalue(uint32_t hash)
{
	auto &knownKeyValues = get_known_keyvalues();
	auto it = knownKeyValues.find(hash);
	return (it != knownKeyValues.end()) ? it->second : std::optional<std::string>{};
}
#pragma optimize("",on)
