#include "util_source2.hpp"
#include "source2/resource.hpp"
#include "source2/resource_data.hpp"
#include <fsys/filesystem.h>
#include <sharedutils/util_string.h>
#include <mathutil/uvec.h>

#pragma optimize("",off)

std::shared_ptr<source2::resource::Resource> source2::load_resource(std::shared_ptr<VFilePtrInternal> f)
{
	auto resource = std::make_shared<resource::Resource>();
	resource->Read(f);
	return resource;
}
#pragma optimize("",on)
