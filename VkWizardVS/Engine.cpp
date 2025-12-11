#include "Engine.h"
#include "RustTypes.h"
#include "Input.h"
#include <iostream>

using namespace vkwiz;

void vkwiz::Engine::run() {
	draw();
	for (;;) {
		input::update();
		if (input::shouldQuit()) break;
	}
	std::cout << "Exiting engine loop." << std::endl;
}

vk::raii::Instance vkwiz::Engine::createInstance() {
	constexpr vk::ApplicationInfo appInfo{
		.pApplicationName = "VkWizard",
		.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		.pEngineName = "No Engine",
		.engineVersion = VK_MAKE_VERSION(1, 0, 0),
		.apiVersion = vk::ApiVersion14
	};

	u32 extensionCount = 0;
	auto requiredExtensions = window->getRequiredVulkanExtensions(&extensionCount);
	auto supportedExtensions = vk::enumerateInstanceExtensionProperties();
	// TODO: check for supported extensions

#ifdef NDEBUG
	auto layers = std::vector<const char*>{};
#else
	auto layers = std::vector<const char*>{ "VK_LAYER_KHRONOS_validation" };
	std::cout << "Enabling validation layers..." << std::endl;
#endif

	vk::InstanceCreateInfo createInfo{
		.pApplicationInfo = &appInfo,
		.enabledLayerCount = static_cast<u32>(layers.size()),
		.ppEnabledLayerNames = layers.data(),
		.enabledExtensionCount = static_cast<u32>(extensionCount),
		.ppEnabledExtensionNames = requiredExtensions,
	};

	auto instance = vk::raii::Instance(vkContext, createInfo);
	std::cout << "Vulkan instance created." << std::endl;
	return instance;
}

void transition_image_layout(
	vk::raii::CommandBuffer& commandBuffer,
	vk::Image image,
	vk::ImageLayout oldLayout,
	vk::ImageLayout newLayout,
	vk::AccessFlags2 srcAccessMask,
	vk::AccessFlags2 dstAccessMask,
	vk::PipelineStageFlags2 srcStageMask,
	vk::PipelineStageFlags2 dstStageMask
) {
	vk::ImageMemoryBarrier2 barrier = {
		.srcStageMask = srcStageMask,
		.srcAccessMask = srcAccessMask,
		.dstStageMask = dstStageMask,
		.dstAccessMask = dstAccessMask,
		.oldLayout = oldLayout,
		.newLayout = newLayout,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = image,
		.subresourceRange = {
			.aspectMask = vk::ImageAspectFlagBits::eColor,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1
		}
	};
	vk::DependencyInfo dependencyInfo = {
		.dependencyFlags = {},
		.imageMemoryBarrierCount = 1,
		.pImageMemoryBarriers = &barrier
	};
	commandBuffer.pipelineBarrier2(dependencyInfo);
}

void vkwiz::Engine::createCommandBuffer()
{
	auto commandPool = &device_->commandPool();
	auto device = &device_->getDevice();
	vk::CommandBufferAllocateInfo allocInfo{
		.commandPool = *commandPool,
		.level = vk::CommandBufferLevel::ePrimary,
		.commandBufferCount = 1
	};
	commandBuffer_ = std::move(vk::raii::CommandBuffers(*device, allocInfo).front());
}

void vkwiz::Engine::recordCommandBuffer(vk::Image image, vk::ImageView imageView)
{
	auto extent = swapChain->extent();
	commandBuffer_.begin({});

	transition_image_layout(
		commandBuffer_,
		image,
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eColorAttachmentOptimal,
		{},
		vk::AccessFlagBits2::eColorAttachmentWrite,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput
	);

	vk::RenderingAttachmentInfo colorAttachment{
		.imageView = imageView,
		.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
		.loadOp = vk::AttachmentLoadOp::eClear,
		.storeOp = vk::AttachmentStoreOp::eStore,
		.clearValue = vk::ClearColorValue(0.1f, 0.1f, 0.1f, 1.0f),
	};
	vk::RenderingInfo renderingInfo{
		.renderArea = vk::Rect2D{ {0, 0}, extent },
		.layerCount = 1,
		.colorAttachmentCount = 1,
		.pColorAttachments = &colorAttachment,
	};
	commandBuffer_.beginRendering(renderingInfo);

	commandBuffer_.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline->getPipeline());

	commandBuffer_.setViewport(
		0,
		vk::Viewport(
			0.0f,
			0.0f,
			static_cast<f32>(extent.width),
			static_cast<f32>(extent.height),
			0.0f,
			1.0f
		)
	);

	commandBuffer_.setScissor(
		0,
		vk::Rect2D{
			.offset = vk::Offset2D{ 0, 0 },
			.extent = extent
		}
	);

	commandBuffer_.draw(3, 1, 0, 0);

	commandBuffer_.endRendering();

	transition_image_layout(
		commandBuffer_,
		image,
		vk::ImageLayout::eColorAttachmentOptimal,
		vk::ImageLayout::ePresentSrcKHR,
		vk::AccessFlagBits2::eColorAttachmentWrite,
		{},
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::PipelineStageFlagBits2::eBottomOfPipe
	);

	commandBuffer_.end();
}

void vkwiz::Engine::createSyncObjects()
{
	auto device = &device_->getDevice();
	presentCompleteSemaphore = vk::raii::Semaphore(*device, vk::SemaphoreCreateInfo());
	renderCompleteSemaphore = vk::raii::Semaphore(*device, vk::SemaphoreCreateInfo());
	drawFence = vk::raii::Fence(*device, vk::FenceCreateInfo{ .flags = vk::FenceCreateFlagBits::eSignaled });
}

void vkwiz::Engine::draw()
{
	auto [image, imageView] = swapChain->acquireImage(presentCompleteSemaphore, nullptr);
	recordCommandBuffer(image, imageView);
	// Reset fence
}

vkwiz::Engine::Engine() {
	window = std::make_unique<Window>("VkWizard");
	auto windowExtent = window->getExtent();
	vkInstance = createInstance();
	surface = window->getVulkanSurface(vkInstance);
	device_ = std::make_unique<Device>(vkInstance, surface);
	swapChain = std::make_unique<SwapChain>(*device_, surface, windowExtent);
	pipeline = std::make_unique<Pipeline>(*device_, *swapChain, "shaders/shader.spv", windowExtent);

	createCommandBuffer();
	createSyncObjects();
}