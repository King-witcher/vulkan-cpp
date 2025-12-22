#include "Engine.h"
#include "RustTypes.h"
#include "Input.h"

#include <iostream>

using namespace vkwiz;

void vkwiz::Engine::run() {
	for (;;) {
		draw();
		input::update();
		if (input::shouldQuit()) break;
	}
	device_.vkDevice().waitIdle();
	std::cout << "Exiting engine loop." << std::endl;
}

vk::raii::Instance vkwiz::Engine::createInstance() const {
	vk::ApplicationInfo appInfo;
	appInfo.setPApplicationName("VkWizard");
	appInfo.setApplicationVersion(vk::makeVersion(1, 0, 0));
	appInfo.setPEngineName("No Engine");
	appInfo.setEngineVersion(vk::makeVersion(1, 0, 0));
	appInfo.setApiVersion(vk::ApiVersion14);


#ifdef NDEBUG
	auto layers = std::vector<const char*>{};
#else
	auto layers = std::vector<const char*>{ "VK_LAYER_KHRONOS_validation" };
	std::cout << "Enabling validation layers..." << std::endl;
#endif

	auto requiredExtensions = window_.getRequiredVulkanExtensions();
	// TODO: check for supported extensions
	vk::InstanceCreateInfo createInfo;
	createInfo.setPApplicationInfo(&appInfo);
	createInfo.setPEnabledLayerNames(layers);
	createInfo.setPEnabledExtensionNames(requiredExtensions);

	auto instance = vkContext_.createInstance(createInfo);
	std::cout << "Vulkan instance created." << std::endl;
	return instance;
}

void vkwiz::Engine::createSyncObjects()
{
	auto imageCount = swapChain_.imageCount();
	for (size_t i = 0; i < imageCount; i++)
	{
		renderFinishedSemaphores_.emplace_back(device_.vkDevice(), vk::SemaphoreCreateInfo());
	}

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		presentCompleteSemaphores_.emplace_back(device_.vkDevice(), vk::SemaphoreCreateInfo());
		inFlightFences_.emplace_back(device_.vkDevice(), vk::FenceCreateInfo{ .flags = vk::FenceCreateFlagBits::eSignaled });
	}
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

void vkwiz::Engine::recordCommandBuffer(vk::raii::CommandBuffer& commandBuffer, vk::Image image, vk::ImageView imageView)
{
	auto extent = swapChain_.extent();
	commandBuffer.begin({});

	transition_image_layout(
		commandBuffer,
		image,
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eColorAttachmentOptimal,
		{},
		vk::AccessFlagBits2::eColorAttachmentWrite,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput
	);

	vk::RenderingAttachmentInfo colorAtt{};
	colorAtt.setImageView(imageView);
	colorAtt.setImageLayout(vk::ImageLayout::eColorAttachmentOptimal);
	colorAtt.setLoadOp(vk::AttachmentLoadOp::eClear);
	colorAtt.setStoreOp(vk::AttachmentStoreOp::eStore);
	colorAtt.setClearValue(vk::ClearColorValue(0.05f, 0.05f, 0.1f, 1.0f));

	std::array colorAtts = { colorAtt };

	vk::RenderingInfo renderingInfo{};
	renderingInfo.setRenderArea({ {0, 0}, extent });
	renderingInfo.setLayerCount(1);
	renderingInfo.setColorAttachments(colorAtts);
	commandBuffer.beginRendering(renderingInfo);

	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline_.vkPipeline());

	commandBuffer.setViewport(
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

	commandBuffer.setScissor(
		0,
		vk::Rect2D{
			.offset = vk::Offset2D{ 0, 0 },
			.extent = extent
		}
	);

	commandBuffer.draw(3, 1, 0, 0);

	commandBuffer.endRendering();

	transition_image_layout(
		commandBuffer,
		image,
		vk::ImageLayout::eColorAttachmentOptimal,
		vk::ImageLayout::ePresentSrcKHR,
		vk::AccessFlagBits2::eColorAttachmentWrite,
		{},
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::PipelineStageFlagBits2::eBottomOfPipe
	);

	commandBuffer.end();
}

void vkwiz::Engine::draw()
{
	device_.waitForFence(inFlightFences_[frameIndex_]);
	device_.resetFence(inFlightFences_[frameIndex_]);

	auto [image, imageView, imageIndex] = swapChain_.acquireImage(presentCompleteSemaphores_[frameIndex_], nullptr);

	vkCommandbuffers_[frameIndex_].reset();
	recordCommandBuffer(vkCommandbuffers_[frameIndex_], image, imageView);

	auto presentSemaphore = *presentCompleteSemaphores_[frameIndex_];
	auto renderSemaphore = *renderFinishedSemaphores_[imageIndex];
	auto commandBuffer = *vkCommandbuffers_[frameIndex_];

	vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);

	// Render
	vk::SubmitInfo submitInfo{};
	submitInfo.setWaitSemaphores({ presentSemaphore });
	submitInfo.setPWaitDstStageMask(&waitDestinationStageMask);
	submitInfo.setCommandBuffers({ commandBuffer });
	submitInfo.setSignalSemaphores({ renderSemaphore });

	device_.submitGraphics(submitInfo, *inFlightFences_[frameIndex_]);

	// Present
	auto& vkSwapChain = swapChain_.vkSwapChain();

	vk::PresentInfoKHR presentInfo{};
	presentInfo.setWaitSemaphores({ renderSemaphore });
	presentInfo.setSwapchains({ *vkSwapChain });
	presentInfo.setImageIndices({ imageIndex });

	device_.present(presentInfo);
	frameIndex_ = (frameIndex_ + 1) % MAX_FRAMES_IN_FLIGHT;
}

vkwiz::Engine::Engine() {
	window_.setPosition(-1400, 200);
	auto windowExtent = window_.getExtent();
	createSyncObjects();
}