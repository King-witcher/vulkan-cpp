#include "Engine.h"
#include "RustTypes.h"
#include "Input.h"
#include "Mesh.h"

#include <iostream>
#include <memory>

using namespace gd;

void gd::Engine::run()
{
	std::vector<gd::Mesh::Vertex> vertices = {
		{{0.0f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
		{{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
		{{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},
	};
	gd::Mesh model(vertices);
	auto dataSize = vertices.size() * sizeof(gd::Mesh::Vertex);
	auto [buffer, mem] = device_.alloc(dataSize);
	auto ptr = *mem.mapMemory(0, dataSize);
	memcpy(ptr, vertices.data(), dataSize);
	mem.unmapMemory();
	for (;;)
	{
		draw(buffer, vertices.size());
		input::update();
		if (input::shouldQuit())
			break;
	}
	device_.vkDevice().waitIdle();
	std::cout << "Exiting engine loop." << std::endl;
}

vk::raii::Instance gd::Engine::createInstance() const
{
	vk::ApplicationInfo appInfo;
	appInfo.setPApplicationName("VkWizard");
	appInfo.setApplicationVersion(vk::makeVersion(1, 0, 0));
	appInfo.setPEngineName("No Engine");
	appInfo.setEngineVersion(vk::makeVersion(1, 0, 0));
	appInfo.setApiVersion(vk::ApiVersion14);

#ifdef _DEBUG
	auto layers = std::vector<const char *>{"VK_LAYER_KHRONOS_validation"};
	std::cout << "Enabling validation layers..." << std::endl;
#else
	auto layers = std::vector<const char *>{};
#endif

	auto requiredExtensions = window_.getRequiredVulkanExtensions();
	// TODO: check for supported extensions
	vk::InstanceCreateInfo createInfo{};
	createInfo.setPApplicationInfo(&appInfo);
	createInfo.setPEnabledLayerNames(layers);
	createInfo.setPEnabledExtensionNames(requiredExtensions);

	auto instance = std::move(*vkContext_.createInstance(createInfo));
	std::cout << "Vulkan instance created." << std::endl;
	return instance;
}

void gd::Engine::createSyncObjects()
{
	auto &vkDevice = device_.vkDevice();
	// auto imageCount = swapChain_->imageCount();
	auto fenceCreateInfo = vk::FenceCreateInfo{};
	fenceCreateInfo.setFlags(vk::FenceCreateFlagBits::eSignaled);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		auto presentSemaphore = std::move(*vkDevice.createSemaphore({}));
		auto fence = std::move(*vkDevice.createFence(fenceCreateInfo));
		presentCompleteSemaphores_.push_back(std::move(presentSemaphore));
		inFlightFences_.push_back(std::move(fence));
	}
}

void transition_image_layout(
	vk::raii::CommandBuffer &commandBuffer,
	vk::Image image,
	vk::ImageLayout oldLayout,
	vk::ImageLayout newLayout,
	vk::AccessFlags2 srcAccessMask,
	vk::AccessFlags2 dstAccessMask,
	vk::PipelineStageFlags2 srcStageMask,
	vk::PipelineStageFlags2 dstStageMask)
{
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
			.layerCount = 1}};
	vk::DependencyInfo dependencyInfo = {
		.dependencyFlags = {},
		.imageMemoryBarrierCount = 1,
		.pImageMemoryBarriers = &barrier};
	commandBuffer.pipelineBarrier2(dependencyInfo);
}

void gd::Engine::recordCommandBuffer(
	vk::raii::CommandBuffer &commandBuffer,
	vk::Image image,
	vk::ImageView imageView,
	vk::raii::Buffer &vertexBuffer,
	u32 vertices)
{
	auto extent = swapChain_->extent();
	commandBuffer.begin({});

	transition_image_layout(
		commandBuffer,
		image,
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eColorAttachmentOptimal,
		{},
		vk::AccessFlagBits2::eColorAttachmentWrite,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput);

	vk::RenderingAttachmentInfo colorAtt{};
	colorAtt.setImageView(imageView);
	colorAtt.setImageLayout(vk::ImageLayout::eColorAttachmentOptimal);
	colorAtt.setLoadOp(vk::AttachmentLoadOp::eClear);
	colorAtt.setStoreOp(vk::AttachmentStoreOp::eStore);
	colorAtt.setClearValue(vk::ClearColorValue(0.05f, 0.05f, 0.1f, 1.0f));

	std::array colorAtts = {colorAtt};

	vk::RenderingInfo renderingInfo{};
	renderingInfo.setRenderArea({{0, 0}, extent});
	renderingInfo.setLayerCount(1);
	renderingInfo.setColorAttachments(colorAtts);
	commandBuffer.beginRendering(renderingInfo);

	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline_.vkPipeline());
	commandBuffer.bindVertexBuffers(0, *vertexBuffer, {0}); // See v2

	commandBuffer.setViewport(
		0,
		vk::Viewport(
			0.0f,
			0.0f,
			static_cast<f32>(extent.width),
			static_cast<f32>(extent.height),
			0.0f,
			1.0f));

	commandBuffer.setScissor(
		0,
		vk::Rect2D{
			.offset = vk::Offset2D{0, 0},
			.extent = extent});

	commandBuffer.draw(vertices, 1, 0, 0);

	commandBuffer.endRendering();

	transition_image_layout(
		commandBuffer,
		image,
		vk::ImageLayout::eColorAttachmentOptimal,
		vk::ImageLayout::ePresentSrcKHR,
		vk::AccessFlagBits2::eColorAttachmentWrite,
		{},
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::PipelineStageFlagBits2::eBottomOfPipe);

	commandBuffer.end();
}

void gd::Engine::draw(vk::raii::Buffer &vertexBuffer, u32 vertices)
{
	device_.waitForFence(inFlightFences_[inFlightIndex]);
	auto presentReady = *presentCompleteSemaphores_[inFlightIndex];
	auto &frame = swapChain_->acquireNextFrame(presentReady);
	device_.resetFence(inFlightFences_[inFlightIndex]);

	vkCommandbuffers_[inFlightIndex].reset();

	recordCommandBuffer(vkCommandbuffers_[inFlightIndex], frame.getImage(), frame.getImageView(), vertexBuffer, vertices);

	auto renderReady = frame.getRenderReadySemaphore();
	auto commandBuffer = *vkCommandbuffers_[inFlightIndex];

	// Render
	vk::SubmitInfo submitInfo{};
	vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
	submitInfo.setWaitDstStageMask(waitDestinationStageMask);
	submitInfo.setWaitSemaphores(presentReady);
	submitInfo.setCommandBuffers(commandBuffer);
	submitInfo.setSignalSemaphores(renderReady);
	device_.submitGraphics(submitInfo, *inFlightFences_[inFlightIndex]);

	// Present
	swapChain_->present(frame);

	inFlightIndex = (inFlightIndex + 1) % MAX_FRAMES_IN_FLIGHT;
}

gd::Engine::Engine()
{
	window_.setPosition(-1400, 200);
	// auto windowExtent = window_.extent();
	createSyncObjects();
}
