#include "Engine.h"
#include "RustTypes.h"
#include "Input.h"

#include <iostream>

using namespace vkwiz;

void vkwiz::Engine::run() {
	for (;;) {
		input::update();
		if (input::shouldQuit()) break;
		draw();
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

void vkwiz::Engine::recordCommandBuffer(vk::Image image, vk::ImageView imageView)
{
	auto extent = swapChain_.extent();
	vkCommandbuffer_.begin({});

	transition_image_layout(
		vkCommandbuffer_,
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
	vkCommandbuffer_.beginRendering(renderingInfo);

	vkCommandbuffer_.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline_.vkPipeline());

	vkCommandbuffer_.setViewport(
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

	vkCommandbuffer_.setScissor(
		0,
		vk::Rect2D{
			.offset = vk::Offset2D{ 0, 0 },
			.extent = extent
		}
	);

	vkCommandbuffer_.draw(3, 1, 0, 0);

	vkCommandbuffer_.endRendering();

	transition_image_layout(
		vkCommandbuffer_,
		image,
		vk::ImageLayout::eColorAttachmentOptimal,
		vk::ImageLayout::ePresentSrcKHR,
		vk::AccessFlagBits2::eColorAttachmentWrite,
		{},
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::PipelineStageFlagBits2::eBottomOfPipe
	);

	vkCommandbuffer_.end();
}

void vkwiz::Engine::draw()
{
	device_.waitForFence(drawFence_);
	device_.resetFence(drawFence_);

	auto [image, imageView, imageIndex] = swapChain_.acquireImage(presentCompleteSemaphore_, nullptr);

	vkCommandbuffer_.reset();
	recordCommandBuffer(image, imageView);

	auto presentSemaphore = *presentCompleteSemaphore_;
	auto renderSemaphore = *renderCompleteSemaphore_;
	auto commandBuffer = *vkCommandbuffer_;

	vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);

	vk::SubmitInfo submitInfo;
	submitInfo.setWaitSemaphores({ presentSemaphore });
	submitInfo.setPWaitDstStageMask(&waitDestinationStageMask);
	submitInfo.setCommandBuffers({ commandBuffer });
	submitInfo.setSignalSemaphores({ renderSemaphore });

	device_.submitGraphics(submitInfo, *drawFence_);
	// Testar com o while
	//device_->waitForFence(*drawFence);

	// Present
	auto& vkSwapChain = swapChain_.vkSwapChain();

	vk::PresentInfoKHR presentInfo;
	presentInfo.setWaitSemaphores({ renderSemaphore });
	presentInfo.setSwapchains({ *vkSwapChain });
	presentInfo.setImageIndices({ imageIndex });

	device_.present(presentInfo);
}

vkwiz::Engine::Engine() {
	window_.setPosition(-1400, 200);
	auto windowExtent = window_.getExtent();

	vkCommandbuffer_ = std::move(device_.allocateCommandBuffers(1)[0]);
}