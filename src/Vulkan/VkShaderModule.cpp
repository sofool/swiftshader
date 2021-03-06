// Copyright 2018 The SwiftShader Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "VkShaderModule.hpp"

#include "spirv-tools/libspirv.hpp"

#include <cstring>

namespace vk {

std::atomic<uint32_t> ShaderModule::serialCounter(1);  // Start at 1, 0 is invalid shader.

ShaderModule::ShaderModule(const VkShaderModuleCreateInfo *pCreateInfo, void *mem)
    : serialID(nextSerialID())
    , code(reinterpret_cast<uint32_t *>(mem))
{
	memcpy(code, pCreateInfo->pCode, pCreateInfo->codeSize);
	wordCount = static_cast<uint32_t>(pCreateInfo->codeSize / sizeof(uint32_t));

#if !defined(NDEBUG) || defined(DCHECK_ALWAYS_ON)
	spvtools::SpirvTools spirvTools(SPIRV_VERSION);
	spvtools::ValidatorOptions validatorOptions = {};
	validatorOptions.SetScalarBlockLayout(true);            // VK_EXT_scalar_block_layout
	validatorOptions.SetUniformBufferStandardLayout(true);  // VK_KHR_uniform_buffer_standard_layout

	ASSERT(spirvTools.Validate(code, wordCount, validatorOptions));  // The SPIR-V code passed to vkCreateShaderModule must be valid (b/158228522)
#endif
}

void ShaderModule::destroy(const VkAllocationCallbacks *pAllocator)
{
	vk::deallocate(code, pAllocator);
}

size_t ShaderModule::ComputeRequiredAllocationSize(const VkShaderModuleCreateInfo *pCreateInfo)
{
	return pCreateInfo->codeSize;
}

}  // namespace vk
