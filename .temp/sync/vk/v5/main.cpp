#include <fstream>
#include <iostream>

// Comment this to disable VMA support
//#define WITH_VMA

#include <vulkan/vulkan.hpp>

#ifdef WITH_VMA
#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"
#endif

#include "pgm.h"

int main()
{
	try {
		std::cout << "Hello Vulkan Compute" << std::endl;
		vk::ApplicationInfo AppInfo{
		    "VulkanCompute",   // Application Name
		    1,		       // Application Version
		    nullptr,	       // Engine Name or nullptr
		    0,		       // Engine Version
		    VK_API_VERSION_1_1 // Vulkan API version
		};

		const std::vector<const char *> Layers = {
		    "VK_LAYER_KHRONOS_validation"};
		vk::InstanceCreateInfo InstanceCreateInfo(
		    vk::InstanceCreateFlags(), // Flags
		    &AppInfo,		       // Application Info
		    Layers.size(),	       // Layers count
		    Layers.data());	       // Layers
		vk::Instance Instance = vk::createInstance(InstanceCreateInfo);

		vk::PhysicalDevice PhysicalDevice =
		    Instance.enumeratePhysicalDevices().front();
		vk::PhysicalDeviceProperties DeviceProps =
		    PhysicalDevice.getProperties();
		std::cout << "Device Name    : " << DeviceProps.deviceName
			  << std::endl;
		const uint32_t ApiVersion = DeviceProps.apiVersion;
		std::cout << "Vulkan Version : " << VK_VERSION_MAJOR(ApiVersion)
			  << "." << VK_VERSION_MINOR(ApiVersion) << "."
			  << VK_VERSION_PATCH(ApiVersion) << std::endl;
		vk::PhysicalDeviceLimits DeviceLimits = DeviceProps.limits;
		std::cout << "Max Compute Shared Memory Size: "
			  << DeviceLimits.maxComputeSharedMemorySize / 1024
			  << " KB" << std::endl;

		std::vector<vk::QueueFamilyProperties> QueueFamilyProps =
		    PhysicalDevice.getQueueFamilyProperties();
		auto PropIt = std::find_if(
		    QueueFamilyProps.begin(), QueueFamilyProps.end(),
		    [](const vk::QueueFamilyProperties &Prop) {
			    return Prop.queueFlags &
				   vk::QueueFlagBits::eCompute;
		    });
		const uint32_t ComputeQueueFamilyIndex =
		    std::distance(QueueFamilyProps.begin(), PropIt);
		std::cout << "Compute Queue Family Index: "
			  << ComputeQueueFamilyIndex << std::endl;

		// Just to avoid a warning from the Vulkan Validation Layer
		const float QueuePriority = 1.0f;
		vk::DeviceQueueCreateInfo DeviceQueueCreateInfo(
		    vk::DeviceQueueCreateFlags(), // Flags
		    ComputeQueueFamilyIndex,	  // Queue Family Index
		    1,				  // Number of Queues
		    &QueuePriority);
		vk::DeviceCreateInfo DeviceCreateInfo(
		    vk::DeviceCreateFlags(), // Flags
		    DeviceQueueCreateInfo);  // Device Queue Create Info struct
		vk::Device Device =
		    PhysicalDevice.createDevice(DeviceCreateInfo);

		PGMData src_pgm;
		readPGM("./input.pgm", &src_pgm);
		PGMData dst_pgm;
		dst_pgm.width = 900;  // src_pgm.width;
		dst_pgm.height = 300; // src_pgm.height;
		dst_pgm.max_gray = src_pgm.max_gray;
		dst_pgm.ptr =
		    (unsigned char *)malloc(dst_pgm.width * dst_pgm.height);

		const uint32_t NumElementsSrc = src_pgm.width * src_pgm.height;
		const uint32_t NumElementsDst = dst_pgm.width * dst_pgm.height;
		const uint32_t BufferSizeSrc = NumElementsSrc * sizeof(int32_t);
		const uint32_t BufferSizeDst = NumElementsDst * sizeof(int32_t);

		vk::BufferCreateInfo BufferCreateInfoSrc{
		    vk::BufferCreateFlags(),		     // Flags
		    BufferSizeSrc,			     // Size
		    vk::BufferUsageFlagBits::eStorageBuffer, // Usage
		    vk::SharingMode::eExclusive,	     // Sharing mode
		    1,			     // Number of queue family indices
		    &ComputeQueueFamilyIndex // List of queue family indices
		};

		vk::BufferCreateInfo BufferCreateInfoDst{
		    vk::BufferCreateFlags(),		     // Flags
		    BufferSizeDst,			     // Size
		    vk::BufferUsageFlagBits::eStorageBuffer, // Usage
		    vk::SharingMode::eExclusive,	     // Sharing mode
		    1,			     // Number of queue family indices
		    &ComputeQueueFamilyIndex // List of queue family indices
		};

#ifdef WITH_VMA
		VmaAllocatorCreateInfo AllocatorInfo = {};
		AllocatorInfo.vulkanApiVersion = DeviceProps.apiVersion;
		AllocatorInfo.physicalDevice = PhysicalDevice;
		AllocatorInfo.device = Device;
		AllocatorInfo.instance = Instance;

		VmaAllocator Allocator;
		vmaCreateAllocator(&AllocatorInfo, &Allocator);

		VkBuffer InBufferRaw;
		VkBuffer OutBufferRaw;

		VmaAllocationCreateInfo AllocationInfo = {};
		AllocationInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

		VmaAllocation InBufferAllocation;
		vmaCreateBuffer(
		    Allocator,
		    &static_cast<VkBufferCreateInfo>(BufferCreateInfo),
		    &AllocationInfo, &InBufferRaw, &InBufferAllocation,
		    nullptr);

		AllocationInfo.usage = VMA_MEMORY_USAGE_GPU_TO_CPU;
		VmaAllocation OutBufferAllocation;
		vmaCreateBuffer(
		    Allocator,
		    &static_cast<VkBufferCreateInfo>(BufferCreateInfo),
		    &AllocationInfo, &OutBufferRaw, &OutBufferAllocation,
		    nullptr);

		vk::Buffer InBuffer = InBufferRaw;
		vk::Buffer OutBuffer = OutBufferRaw;

		int32_t *InBufferPtr = nullptr;
		vmaMapMemory(Allocator, InBufferAllocation,
			     reinterpret_cast<void **>(&InBufferPtr));
		for (int32_t I = 0; I < NumElements; ++I) {
			InBufferPtr[I] = I;
		}
		vmaUnmapMemory(Allocator, InBufferAllocation);
#else
		vk::Buffer InBuffer = Device.createBuffer(BufferCreateInfoSrc);
		vk::Buffer OutBuffer = Device.createBuffer(BufferCreateInfoDst);

		vk::MemoryRequirements InBufferMemoryRequirements =
		    Device.getBufferMemoryRequirements(InBuffer);
		vk::MemoryRequirements OutBufferMemoryRequirements =
		    Device.getBufferMemoryRequirements(OutBuffer);

		vk::PhysicalDeviceMemoryProperties MemoryProperties =
		    PhysicalDevice.getMemoryProperties();

		uint32_t MemoryTypeIndex = uint32_t(~0);
		vk::DeviceSize MemoryHeapSize = uint32_t(~0);
		for (uint32_t CurrentMemoryTypeIndex = 0;
		     CurrentMemoryTypeIndex < MemoryProperties.memoryTypeCount;
		     ++CurrentMemoryTypeIndex) {
			vk::MemoryType MemoryType =
			    MemoryProperties
				.memoryTypes[CurrentMemoryTypeIndex];
			if ((vk::MemoryPropertyFlagBits::eHostVisible &
			     MemoryType.propertyFlags) &&
			    (vk::MemoryPropertyFlagBits::eHostCoherent &
			     MemoryType.propertyFlags)) {
				MemoryHeapSize =
				    MemoryProperties
					.memoryHeaps[MemoryType.heapIndex]
					.size;
				MemoryTypeIndex = CurrentMemoryTypeIndex;
				break;
			}
		}

		std::cout << "Memory Type Index: " << MemoryTypeIndex
			  << std::endl;
		std::cout << "Memory Heap Size : "
			  << MemoryHeapSize / 1024 / 1024 / 1024 << " GB"
			  << std::endl;

		vk::MemoryAllocateInfo InBufferMemoryAllocateInfo(
		    InBufferMemoryRequirements.size, MemoryTypeIndex);
		vk::MemoryAllocateInfo OutBufferMemoryAllocateInfo(
		    OutBufferMemoryRequirements.size, MemoryTypeIndex);
		vk::DeviceMemory InBufferMemory =
		    Device.allocateMemory(InBufferMemoryAllocateInfo);
		vk::DeviceMemory OutBufferMemory =
		    Device.allocateMemory(InBufferMemoryAllocateInfo);

		int32_t *InBufferPtr = static_cast<int32_t *>(
		    Device.mapMemory(InBufferMemory, 0, BufferSizeSrc));
		for (int32_t I = 0; I < NumElementsSrc; ++I) {
			InBufferPtr[I] = src_pgm.ptr[I];
		}
		Device.unmapMemory(InBufferMemory);

		Device.bindBufferMemory(InBuffer, InBufferMemory, 0);
		Device.bindBufferMemory(OutBuffer, OutBufferMemory, 0);
#endif
		//******************************** 1
		std::vector<char> ShaderContents1;
		if (std::ifstream ShaderFile{
			"downscale.spv", std::ios::binary | std::ios::ate}) {
			const size_t FileSize = ShaderFile.tellg();
			ShaderFile.seekg(0);
			ShaderContents1.resize(FileSize, '\0');
			ShaderFile.read(ShaderContents1.data(), FileSize);
		}

		vk::ShaderModuleCreateInfo ShaderModuleCreateInfo1(
		    vk::ShaderModuleCreateFlags(), // Flags
		    ShaderContents1.size(),	   // Code size
		    reinterpret_cast<const uint32_t *>(
			ShaderContents1.data())); // Code
		vk::ShaderModule ShaderModule1 =
		    Device.createShaderModule(ShaderModuleCreateInfo1);

		// ******************************** 2
		std::vector<char> ShaderContents2;
		if (std::ifstream ShaderFile{
			"convolution.spv", std::ios::binary | std::ios::ate}) {
			const size_t FileSize = ShaderFile.tellg();
			ShaderFile.seekg(0);
			ShaderContents2.resize(FileSize, '\0');
			ShaderFile.read(ShaderContents2.data(), FileSize);
		}

		vk::ShaderModuleCreateInfo ShaderModuleCreateInfo2(
		    vk::ShaderModuleCreateFlags(), // Flags
		    ShaderContents2.size(),	   // Code size
		    reinterpret_cast<const uint32_t *>(
			ShaderContents2.data())); // Code
		vk::ShaderModule ShaderModule2 =
		    Device.createShaderModule(ShaderModuleCreateInfo2);

		const std::vector<vk::DescriptorSetLayoutBinding>
		    DescriptorSetLayoutBinding = {
			{0, vk::DescriptorType::eStorageBuffer, 1,
			 vk::ShaderStageFlagBits::eCompute},
			{1, vk::DescriptorType::eStorageBuffer, 1,
			 vk::ShaderStageFlagBits::eCompute}};
		vk::DescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo(
		    vk::DescriptorSetLayoutCreateFlags(),
		    DescriptorSetLayoutBinding);
		vk::DescriptorSetLayout DescriptorSetLayout =
		    Device.createDescriptorSetLayout(
			DescriptorSetLayoutCreateInfo);

		vk::PipelineLayoutCreateInfo PipelineLayoutCreateInfo(
		    vk::PipelineLayoutCreateFlags(), DescriptorSetLayout);
		vk::PipelineLayout PipelineLayout =
		    Device.createPipelineLayout(PipelineLayoutCreateInfo);
		vk::PipelineCache PipelineCache =
		    Device.createPipelineCache(vk::PipelineCacheCreateInfo());

		// ************************1*******************************
		vk::PipelineShaderStageCreateInfo PipelineShaderCreateInfo1(
		    vk::PipelineShaderStageCreateFlags(), // Flags
		    vk::ShaderStageFlagBits::eCompute,	  // Stage
		    ShaderModule1,			  // Shader Module
		    "main");
		vk::ComputePipelineCreateInfo ComputePipelineCreateInfo1(
		    vk::PipelineCreateFlags(), // Flags
		    PipelineShaderCreateInfo1, // Shader Create Info struct
		    PipelineLayout);	       // Pipeline Layout
		vk::Pipeline ComputePipeline1 = Device.createComputePipeline(
		    PipelineCache, ComputePipelineCreateInfo1);

		// ************************2*******************************
		vk::PipelineShaderStageCreateInfo PipelineShaderCreateInfo2(
		    vk::PipelineShaderStageCreateFlags(), // Flags
		    vk::ShaderStageFlagBits::eCompute,	  // Stage
		    ShaderModule2,			  // Shader Module
		    "main");
		vk::ComputePipelineCreateInfo ComputePipelineCreateInfo2(
		    vk::PipelineCreateFlags(), // Flags
		    PipelineShaderCreateInfo2, // Shader Create Info struct
		    PipelineLayout);	       // Pipeline Layout
		vk::Pipeline ComputePipeline2 = Device.createComputePipeline(
		    PipelineCache, ComputePipelineCreateInfo2);

		// ************************end*******************************
		vk::DescriptorPoolSize DescriptorPoolSize(
		    vk::DescriptorType::eStorageBuffer, 2);
		vk::DescriptorPoolCreateInfo DescriptorPoolCreateInfo(
		    vk::DescriptorPoolCreateFlags(), 1, DescriptorPoolSize);
		vk::DescriptorPool DescriptorPool =
		    Device.createDescriptorPool(DescriptorPoolCreateInfo);

		vk::DescriptorSetAllocateInfo DescriptorSetAllocInfo(
		    DescriptorPool, 1, &DescriptorSetLayout);
		const std::vector<vk::DescriptorSet> DescriptorSets =
		    Device.allocateDescriptorSets(DescriptorSetAllocInfo);
		vk::DescriptorSet DescriptorSet = DescriptorSets.front();
		vk::DescriptorBufferInfo InBufferInfo(
		    InBuffer, 0, NumElementsSrc * sizeof(int32_t));
		vk::DescriptorBufferInfo OutBufferInfo(
		    OutBuffer, 0, NumElementsDst * sizeof(int32_t));

		const std::vector<vk::WriteDescriptorSet> WriteDescriptorSets =
		    {
			{DescriptorSet, 0, 0, 1,
			 vk::DescriptorType::eStorageBuffer, nullptr,
			 &InBufferInfo},
			{DescriptorSet, 1, 0, 1,
			 vk::DescriptorType::eStorageBuffer, nullptr,
			 &OutBufferInfo},
		    };
		Device.updateDescriptorSets(WriteDescriptorSets, {});

		vk::CommandPoolCreateInfo CommandPoolCreateInfo(
		    vk::CommandPoolCreateFlags(
			VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT),
		    ComputeQueueFamilyIndex);
		vk::CommandPool CommandPool =
		    Device.createCommandPool(CommandPoolCreateInfo);

		vk::CommandBufferAllocateInfo CommandBufferAllocInfo(
		    CommandPool,		      // Command Pool
		    vk::CommandBufferLevel::ePrimary, // Level
		    2);				      // Num Command Buffers
		const std::vector<vk::CommandBuffer> CmdBuffers =
		    Device.allocateCommandBuffers(CommandBufferAllocInfo);

		// **************************1
		vk::CommandBuffer CmdBuffer = CmdBuffers.front();

		vk::CommandBufferBeginInfo CmdBufferBeginInfo(
		    vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
		CmdBuffer.begin(CmdBufferBeginInfo);
		CmdBuffer.bindPipeline(vk::PipelineBindPoint::eCompute,
				       ComputePipeline1);
		CmdBuffer.bindDescriptorSets(
		    vk::PipelineBindPoint::eCompute, // Bind point
		    PipelineLayout,		     // Pipeline Layout
		    0,				     // First descriptor set
		    {DescriptorSet},		     // List of descriptor sets
		    {});			     // Dynamic offsets
		CmdBuffer.dispatch(src_pgm.width, src_pgm.height, 1);
		CmdBuffer.end();

		vk::Queue Queue = Device.getQueue(ComputeQueueFamilyIndex, 0);
		vk::Fence Fence1 = Device.createFence(vk::FenceCreateInfo());

		vk::SubmitInfo SubmitInfo(
		    0,		 // Num Wait Semaphores
		    nullptr,	 // Wait Semaphores
		    nullptr,	 // Pipeline Stage Flags
		    1,		 // Num Command Buffers
		    &CmdBuffer); // List of command buffers
		Queue.submit({SubmitInfo}, Fence1);
		Device.waitForFences({Fence1},	    // List of fences
				     true,	    // Wait All
				     uint64_t(-1)); // Timeout
		// ***********************************2
		CmdBuffer.begin(CmdBufferBeginInfo);
		CmdBuffer.bindPipeline(vk::PipelineBindPoint::eCompute,
				       ComputePipeline2);
		CmdBuffer.bindDescriptorSets(
		    vk::PipelineBindPoint::eCompute, // Bind point
		    PipelineLayout,		     // Pipeline Layout
		    0,				     // First descriptor set
		    {DescriptorSet},		     // List of descriptor sets
		    {});			     // Dynamic offsets
		CmdBuffer.dispatch(dst_pgm.width, dst_pgm.height, 1);
		CmdBuffer.end();

		vk::Fence Fence2 = Device.createFence(vk::FenceCreateInfo());

		Queue.submit({SubmitInfo}, Fence2);
		Device.waitForFences({Fence2},	    // List of fences
				     true,	    // Wait All
				     uint64_t(-1)); // Timeout

#ifdef WITH_VMA
		vmaMapMemory(Allocator, InBufferAllocation,
			     reinterpret_cast<void **>(&InBufferPtr));
		for (uint32_t I = 0; I < NumElements; ++I) {
			std::cout << InBufferPtr[I] << " ";
		}
		std::cout << std::endl;
		vmaUnmapMemory(Allocator, InBufferAllocation);

		int32_t *OutBufferPtr = nullptr;
		vmaMapMemory(Allocator, OutBufferAllocation,
			     reinterpret_cast<void **>(&OutBufferPtr));
		for (uint32_t I = 0; I < NumElements; ++I) {
			std::cout << OutBufferPtr[I] << " ";
		}
		std::cout << std::endl;
		vmaUnmapMemory(Allocator, OutBufferAllocation);

		struct BufferInfo {
			VkBuffer Buffer;
			VmaAllocation Allocation;
		};

		// Lets allocate a couple of buffers to see how they are layed
		// out in memory
		auto AllocateBuffer = [Allocator, ComputeQueueFamilyIndex](
					  size_t SizeInBytes,
					  VmaMemoryUsage Usage) {
			vk::BufferCreateInfo BufferCreateInfo{
			    vk::BufferCreateFlags(),		     // Flags
			    SizeInBytes,			     // Size
			    vk::BufferUsageFlagBits::eStorageBuffer, // Usage
			    vk::SharingMode::eExclusive, // Sharing mode
			    1, // Number of queue family indices
			    &ComputeQueueFamilyIndex // List of queue family
						     // indices
			};

			VmaAllocationCreateInfo AllocationInfo = {};
			AllocationInfo.usage = Usage;

			BufferInfo Info;
			vmaCreateBuffer(
			    Allocator,
			    &static_cast<VkBufferCreateInfo>(BufferCreateInfo),
			    &AllocationInfo, &Info.Buffer, &Info.Allocation,
			    nullptr);

			return Info;
		};

		auto DestroyBuffer = [Allocator](BufferInfo Info) {
			vmaDestroyBuffer(Allocator, Info.Buffer,
					 Info.Allocation);
		};

		constexpr size_t MB = 1024 * 1024;
		BufferInfo B1 =
		    AllocateBuffer(4 * MB, VMA_MEMORY_USAGE_CPU_TO_GPU);
		BufferInfo B2 =
		    AllocateBuffer(10 * MB, VMA_MEMORY_USAGE_GPU_TO_CPU);
		BufferInfo B3 =
		    AllocateBuffer(20 * MB, VMA_MEMORY_USAGE_GPU_ONLY);
		BufferInfo B4 =
		    AllocateBuffer(100 * MB, VMA_MEMORY_USAGE_CPU_ONLY);

		{
			VmaStats Stats;
			char *StatsString = nullptr;
			vmaBuildStatsString(Allocator, &StatsString, true);
			{
				std::ofstream OutStats{"VmaStats_2.json"};
				OutStats << StatsString;
			}
			vmaFreeStatsString(Allocator, StatsString);
		}

		DestroyBuffer(B1);
		DestroyBuffer(B2);
		DestroyBuffer(B3);
		DestroyBuffer(B4);
#else
		int32_t *OutBufferPtr = static_cast<int32_t *>(
		    Device.mapMemory(InBufferMemory, 0, BufferSizeDst));
		std::cout << "src " << src_pgm.width << " " << src_pgm.height
			  << " dst " << dst_pgm.width << " " << dst_pgm.height
			  << std::endl;
		for (uint32_t I = 0; I < NumElementsDst; ++I) {
			dst_pgm.ptr[I] = (unsigned char)OutBufferPtr[I];
		}

		writePGM("./output.pgm", &dst_pgm);

		Device.unmapMemory(InBufferMemory);

#endif

#ifdef WITH_VMA
		char *StatsString = nullptr;
		vmaBuildStatsString(Allocator, &StatsString, true);
		{
			std::ofstream OutStats{"VmaStats.json"};
			OutStats << StatsString;
		}
		vmaFreeStatsString(Allocator, StatsString);

		vmaDestroyBuffer(Allocator, InBuffer, InBufferAllocation);
		vmaDestroyBuffer(Allocator, OutBuffer, OutBufferAllocation);
		vmaDestroyAllocator(Allocator);
#else
		Device.freeMemory(InBufferMemory);
		Device.freeMemory(OutBufferMemory);
		Device.destroyBuffer(InBuffer);
		Device.destroyBuffer(OutBuffer);
#endif

		Device.resetCommandPool(CommandPool,
					vk::CommandPoolResetFlags());
		Device.destroyFence(Fence1);
		Device.destroyFence(Fence2);
		Device.destroyDescriptorSetLayout(DescriptorSetLayout);
		Device.destroyPipelineLayout(PipelineLayout);
		Device.destroyPipelineCache(PipelineCache);
		Device.destroyShaderModule(ShaderModule1);
		Device.destroyShaderModule(ShaderModule2);
		Device.destroyPipeline(ComputePipeline1);
		Device.destroyPipeline(ComputePipeline2);
		Device.destroyDescriptorPool(DescriptorPool);
		Device.destroyCommandPool(CommandPool);
		Device.destroy();
		Instance.destroy();
	} catch (const std::exception &Exception) {
		std::cout << "Error: " << Exception.what() << std::endl;
	}

	return 0;
}