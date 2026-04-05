#include <volk.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <optional>
#include <set>
#include <stdexcept>
#include <vector>

namespace
{
constexpr uint32_t kWindowWidth = 800;
constexpr uint32_t kWindowHeight = 600;
const std::vector<const char*> kDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

struct QueueFamilyIndices
{
    std::optional<uint32_t> GraphicsFamily;
    std::optional<uint32_t> PresentFamily;

    [[nodiscard]] bool IsComplete() const
    {
        return GraphicsFamily.has_value() && PresentFamily.has_value();
    }
};

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR Capabilities{};
    std::vector<VkSurfaceFormatKHR> Formats;
    std::vector<VkPresentModeKHR> PresentModes;
};

const char* VkResultToString(VkResult result)
{
    switch (result)
    {
    case VK_SUCCESS: return "VK_SUCCESS";
    case VK_NOT_READY: return "VK_NOT_READY";
    case VK_TIMEOUT: return "VK_TIMEOUT";
    case VK_EVENT_SET: return "VK_EVENT_SET";
    case VK_EVENT_RESET: return "VK_EVENT_RESET";
    case VK_INCOMPLETE: return "VK_INCOMPLETE";
    case VK_ERROR_OUT_OF_HOST_MEMORY: return "VK_ERROR_OUT_OF_HOST_MEMORY";
    case VK_ERROR_OUT_OF_DEVICE_MEMORY: return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
    case VK_ERROR_INITIALIZATION_FAILED: return "VK_ERROR_INITIALIZATION_FAILED";
    case VK_ERROR_DEVICE_LOST: return "VK_ERROR_DEVICE_LOST";
    case VK_ERROR_MEMORY_MAP_FAILED: return "VK_ERROR_MEMORY_MAP_FAILED";
    case VK_ERROR_LAYER_NOT_PRESENT: return "VK_ERROR_LAYER_NOT_PRESENT";
    case VK_ERROR_EXTENSION_NOT_PRESENT: return "VK_ERROR_EXTENSION_NOT_PRESENT";
    case VK_ERROR_FEATURE_NOT_PRESENT: return "VK_ERROR_FEATURE_NOT_PRESENT";
    case VK_ERROR_INCOMPATIBLE_DRIVER: return "VK_ERROR_INCOMPATIBLE_DRIVER";
    case VK_ERROR_SURFACE_LOST_KHR: return "VK_ERROR_SURFACE_LOST_KHR";
    case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
    case VK_SUBOPTIMAL_KHR: return "VK_SUBOPTIMAL_KHR";
    case VK_ERROR_OUT_OF_DATE_KHR: return "VK_ERROR_OUT_OF_DATE_KHR";
    default: return "VK_UNKNOWN_ERROR";
    }
}

[[noreturn]] void ThrowIfFailed(VkResult result, const char* message)
{
    if (result == VK_SUCCESS)
    {
        throw std::logic_error("ThrowIfFailed appelé avec un succès Vulkan");
    }

    throw std::runtime_error(std::string(message) + " (" + VkResultToString(result) + ")");
}

std::vector<const char*> GetRequiredInstanceExtensions()
{
    uint32_t extensionCount = 0;
    const char** extensions = glfwGetRequiredInstanceExtensions(&extensionCount);

    if (extensions == nullptr || extensionCount == 0)
    {
        const char* description = nullptr;
        const int glfwError = glfwGetError(&description);
        throw std::runtime_error(
            "GLFW n'a pas fourni les extensions Vulkan requises (GLFW " +
            std::to_string(glfwError) + ": " + (description ? description : "erreur inconnue") + ")");
    }

    return { extensions, extensions + extensionCount };
}

QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

    for (uint32_t i = 0; i < queueFamilyCount; ++i)
    {
        if ((queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
        {
            indices.GraphicsFamily = i;
        }

        VkBool32 presentSupport = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
        if (presentSupport == VK_TRUE)
        {
            indices.PresentFamily = i;
        }

        if (indices.IsComplete())
        {
            break;
        }
    }

    return indices;
}

bool CheckDeviceExtensionSupport(VkPhysicalDevice physicalDevice)
{
    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> missingExtensions(kDeviceExtensions.begin(), kDeviceExtensions.end());
    for (const VkExtensionProperties& extension : availableExtensions)
    {
        missingExtensions.erase(extension.extensionName);
    }

    return missingExtensions.empty();
}

SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.Capabilities);

    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
    if (formatCount > 0)
    {
        details.Formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, details.Formats.data());
    }

    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
    if (presentModeCount > 0)
    {
        details.PresentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, details.PresentModes.data());
    }

    return details;
}

VkSurfaceFormatKHR ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    for (const VkSurfaceFormatKHR& format : availableFormats)
    {
        if (format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return format;
        }
    }

    return availableFormats.front();
}

VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    for (const VkPresentModeKHR presentMode : availablePresentModes)
    {
        if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return presentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D ChooseExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window)
{
    if (capabilities.currentExtent.width != UINT32_MAX)
    {
        return capabilities.currentExtent;
    }

    int width = 0;
    int height = 0;
    glfwGetFramebufferSize(window, &width, &height);

    VkExtent2D extent{};
    extent.width = std::clamp(static_cast<uint32_t>(std::max(width, 1)), capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    extent.height = std::clamp(static_cast<uint32_t>(std::max(height, 1)), capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    return extent;
}

bool IsDeviceSuitable(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    const QueueFamilyIndices indices = FindQueueFamilies(physicalDevice, surface);
    const bool extensionsSupported = CheckDeviceExtensionSupport(physicalDevice);

    bool swapChainAdequate = false;
    if (extensionsSupported)
    {
        const SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(physicalDevice, surface);
        swapChainAdequate = !swapChainSupport.Formats.empty() && !swapChainSupport.PresentModes.empty();
    }

    return indices.IsComplete() && extensionsSupported && swapChainAdequate;
}
} // namespace

int main()
{
    GLFWwindow* window = nullptr;
    VkInstance instance = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue presentQueue = VK_NULL_HANDLE;
    VkSwapchainKHR swapChain = VK_NULL_HANDLE;
    VkFormat swapChainImageFormat = VK_FORMAT_UNDEFINED;
    VkExtent2D swapChainExtent{};
    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;
    VkRenderPass renderPass = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> framebuffers;
    VkCommandPool commandPool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> commandBuffers;
    VkSemaphore imageAvailableSemaphore = VK_NULL_HANDLE;
    VkSemaphore renderFinishedSemaphore = VK_NULL_HANDLE;
    VkFence inFlightFence = VK_NULL_HANDLE;

    try
    {
        if (!glfwInit())
        {
            throw std::runtime_error("Impossible d'initialiser GLFW");
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window = glfwCreateWindow(static_cast<int>(kWindowWidth), static_cast<int>(kWindowHeight), "VectemEngine Vulkan", nullptr, nullptr);
        if (window == nullptr)
        {
            const char* description = nullptr;
            const int glfwError = glfwGetError(&description);
            throw std::runtime_error(
                "Impossible de créer la fenêtre GLFW (GLFW " +
                std::to_string(glfwError) + ": " + (description ? description : "erreur inconnue") + ")");
        }

        if (volkInitialize() != VK_SUCCESS)
        {
            throw std::runtime_error("Volk n'a pas pu charger le loader Vulkan");
        }

        if (glfwVulkanSupported() != GLFW_TRUE)
        {
            const char* description = nullptr;
            const int glfwError = glfwGetError(&description);
            throw std::runtime_error(
                "GLFW ne détecte pas de support Vulkan (GLFW " +
                std::to_string(glfwError) + ": " + (description ? description : "erreur inconnue") + ")");
        }

        const std::vector<const char*> instanceExtensions = GetRequiredInstanceExtensions();

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "VectemEngine";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "VectemEngine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo instanceInfo{};
        instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceInfo.pApplicationInfo = &appInfo;
        instanceInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
        instanceInfo.ppEnabledExtensionNames = instanceExtensions.data();

        const VkResult instanceResult = vkCreateInstance(&instanceInfo, nullptr, &instance);
        if (instanceResult != VK_SUCCESS)
        {
            ThrowIfFailed(instanceResult, "La création de l'instance Vulkan a échoué");
        }

        volkLoadInstance(instance);

        const VkResult surfaceResult = glfwCreateWindowSurface(instance, window, nullptr, &surface);
        if (surfaceResult != VK_SUCCESS)
        {
            ThrowIfFailed(surfaceResult, "La création de la surface GLFW/Vulkan a échoué");
        }

        uint32_t physicalDeviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);
        if (physicalDeviceCount == 0)
        {
            throw std::runtime_error("Aucun GPU compatible Vulkan n'a été détecté");
        }

        std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
        vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data());

        for (const VkPhysicalDevice candidate : physicalDevices)
        {
            if (IsDeviceSuitable(candidate, surface))
            {
                physicalDevice = candidate;
                break;
            }
        }

        if (physicalDevice == VK_NULL_HANDLE)
        {
            throw std::runtime_error("Aucun GPU ne supporte à la fois le rendu et la présentation pour cette surface");
        }

        const QueueFamilyIndices queueFamilies = FindQueueFamilies(physicalDevice, surface);
        std::vector<VkDeviceQueueCreateInfo> queueInfos;
        std::set<uint32_t> uniqueQueueFamilies = {
            queueFamilies.GraphicsFamily.value(),
            queueFamilies.PresentFamily.value()
        };

        constexpr float queuePriority = 1.0f;
        for (const uint32_t familyIndex : uniqueQueueFamilies)
        {
            VkDeviceQueueCreateInfo queueInfo{};
            queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueInfo.queueFamilyIndex = familyIndex;
            queueInfo.queueCount = 1;
            queueInfo.pQueuePriorities = &queuePriority;
            queueInfos.push_back(queueInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures{};

        VkDeviceCreateInfo deviceInfo{};
        deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceInfo.queueCreateInfoCount = static_cast<uint32_t>(queueInfos.size());
        deviceInfo.pQueueCreateInfos = queueInfos.data();
        deviceInfo.pEnabledFeatures = &deviceFeatures;
        deviceInfo.enabledExtensionCount = static_cast<uint32_t>(kDeviceExtensions.size());
        deviceInfo.ppEnabledExtensionNames = kDeviceExtensions.data();

        const VkResult deviceResult = vkCreateDevice(physicalDevice, &deviceInfo, nullptr, &device);
        if (deviceResult != VK_SUCCESS)
        {
            ThrowIfFailed(deviceResult, "La création du périphérique logique a échoué");
        }

        volkLoadDevice(device);
        vkGetDeviceQueue(device, queueFamilies.GraphicsFamily.value(), 0, &graphicsQueue);
        vkGetDeviceQueue(device, queueFamilies.PresentFamily.value(), 0, &presentQueue);

        const SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(physicalDevice, surface);
        const VkSurfaceFormatKHR surfaceFormat = ChooseSurfaceFormat(swapChainSupport.Formats);
        const VkPresentModeKHR presentMode = ChoosePresentMode(swapChainSupport.PresentModes);
        swapChainExtent = ChooseExtent(swapChainSupport.Capabilities, window);

        uint32_t imageCount = swapChainSupport.Capabilities.minImageCount + 1;
        if (swapChainSupport.Capabilities.maxImageCount > 0 && imageCount > swapChainSupport.Capabilities.maxImageCount)
        {
            imageCount = swapChainSupport.Capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR swapChainInfo{};
        swapChainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapChainInfo.surface = surface;
        swapChainInfo.minImageCount = imageCount;
        swapChainInfo.imageFormat = surfaceFormat.format;
        swapChainInfo.imageColorSpace = surfaceFormat.colorSpace;
        swapChainInfo.imageExtent = swapChainExtent;
        swapChainInfo.imageArrayLayers = 1;
        swapChainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        const uint32_t queueFamilyIndices[] = {
            queueFamilies.GraphicsFamily.value(),
            queueFamilies.PresentFamily.value()
        };

        if (queueFamilies.GraphicsFamily != queueFamilies.PresentFamily)
        {
            swapChainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            swapChainInfo.queueFamilyIndexCount = 2;
            swapChainInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            swapChainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        swapChainInfo.preTransform = swapChainSupport.Capabilities.currentTransform;
        swapChainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapChainInfo.presentMode = presentMode;
        swapChainInfo.clipped = VK_TRUE;

        const VkResult swapChainResult = vkCreateSwapchainKHR(device, &swapChainInfo, nullptr, &swapChain);
        if (swapChainResult != VK_SUCCESS)
        {
            ThrowIfFailed(swapChainResult, "La création du swapchain a échoué");
        }

        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
        swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());
        swapChainImageFormat = surfaceFormat.format;

        swapChainImageViews.resize(swapChainImages.size());
        for (size_t i = 0; i < swapChainImages.size(); ++i)
        {
            VkImageViewCreateInfo imageViewInfo{};
            imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            imageViewInfo.image = swapChainImages[i];
            imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            imageViewInfo.format = swapChainImageFormat;
            imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageViewInfo.subresourceRange.baseMipLevel = 0;
            imageViewInfo.subresourceRange.levelCount = 1;
            imageViewInfo.subresourceRange.baseArrayLayer = 0;
            imageViewInfo.subresourceRange.layerCount = 1;

            const VkResult imageViewResult = vkCreateImageView(device, &imageViewInfo, nullptr, &swapChainImageViews[i]);
            if (imageViewResult != VK_SUCCESS)
            {
                ThrowIfFailed(imageViewResult, "La création d'une image view a échoué");
            }
        }

        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = swapChainImageFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        const VkResult renderPassResult = vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass);
        if (renderPassResult != VK_SUCCESS)
        {
            ThrowIfFailed(renderPassResult, "La création du render pass a échoué");
        }

        framebuffers.resize(swapChainImageViews.size());
        for (size_t i = 0; i < swapChainImageViews.size(); ++i)
        {
            VkImageView attachments[] = { swapChainImageViews[i] };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = swapChainExtent.width;
            framebufferInfo.height = swapChainExtent.height;
            framebufferInfo.layers = 1;

            const VkResult framebufferResult = vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffers[i]);
            if (framebufferResult != VK_SUCCESS)
            {
                ThrowIfFailed(framebufferResult, "La création d'un framebuffer a échoué");
            }
        }

        VkCommandPoolCreateInfo commandPoolInfo{};
        commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        commandPoolInfo.queueFamilyIndex = queueFamilies.GraphicsFamily.value();
        commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        const VkResult commandPoolResult = vkCreateCommandPool(device, &commandPoolInfo, nullptr, &commandPool);
        if (commandPoolResult != VK_SUCCESS)
        {
            ThrowIfFailed(commandPoolResult, "La création du command pool a échoué");
        }

        commandBuffers.resize(framebuffers.size());
        VkCommandBufferAllocateInfo commandBufferInfo{};
        commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferInfo.commandPool = commandPool;
        commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

        const VkResult commandBufferResult = vkAllocateCommandBuffers(device, &commandBufferInfo, commandBuffers.data());
        if (commandBufferResult != VK_SUCCESS)
        {
            ThrowIfFailed(commandBufferResult, "L'allocation des command buffers a échoué");
        }

        for (size_t i = 0; i < commandBuffers.size(); ++i)
        {
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

            const VkResult beginResult = vkBeginCommandBuffer(commandBuffers[i], &beginInfo);
            if (beginResult != VK_SUCCESS)
            {
                ThrowIfFailed(beginResult, "Le démarrage d'un command buffer a échoué");
            }

            VkClearValue clearColor{};
            clearColor.color = { { 0.08f, 0.10f, 0.18f, 1.0f } };

            VkRenderPassBeginInfo renderPassBeginInfo{};
            renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassBeginInfo.renderPass = renderPass;
            renderPassBeginInfo.framebuffer = framebuffers[i];
            renderPassBeginInfo.renderArea.offset = { 0, 0 };
            renderPassBeginInfo.renderArea.extent = swapChainExtent;
            renderPassBeginInfo.clearValueCount = 1;
            renderPassBeginInfo.pClearValues = &clearColor;

            vkCmdBeginRenderPass(commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
            vkCmdEndRenderPass(commandBuffers[i]);

            const VkResult endResult = vkEndCommandBuffer(commandBuffers[i]);
            if (endResult != VK_SUCCESS)
            {
                ThrowIfFailed(endResult, "La finalisation d'un command buffer a échoué");
            }
        }

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        const VkResult imageSemaphoreResult = vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphore);
        if (imageSemaphoreResult != VK_SUCCESS)
        {
            ThrowIfFailed(imageSemaphoreResult, "La création du sémaphore d'image disponible a échoué");
        }

        const VkResult renderSemaphoreResult = vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphore);
        if (renderSemaphoreResult != VK_SUCCESS)
        {
            ThrowIfFailed(renderSemaphoreResult, "La création du sémaphore de rendu terminé a échoué");
        }

        const VkResult fenceResult = vkCreateFence(device, &fenceInfo, nullptr, &inFlightFence);
        if (fenceResult != VK_SUCCESS)
        {
            ThrowIfFailed(fenceResult, "La création de la fence a échoué");
        }

        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();

            vkWaitForFences(device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);
            vkResetFences(device, 1, &inFlightFence);

            uint32_t imageIndex = 0;
            const VkResult acquireResult = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
            if (acquireResult != VK_SUCCESS && acquireResult != VK_SUBOPTIMAL_KHR)
            {
                ThrowIfFailed(acquireResult, "L'acquisition d'une image du swapchain a échoué");
            }

            const VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.waitSemaphoreCount = 1;
            submitInfo.pWaitSemaphores = &imageAvailableSemaphore;
            submitInfo.pWaitDstStageMask = waitStages;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &commandBuffers[imageIndex];
            submitInfo.signalSemaphoreCount = 1;
            submitInfo.pSignalSemaphores = &renderFinishedSemaphore;

            const VkResult submitResult = vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFence);
            if (submitResult != VK_SUCCESS)
            {
                ThrowIfFailed(submitResult, "L'envoi de la commande de rendu a échoué");
            }

            VkPresentInfoKHR presentInfo{};
            presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
            presentInfo.waitSemaphoreCount = 1;
            presentInfo.pWaitSemaphores = &renderFinishedSemaphore;
            presentInfo.swapchainCount = 1;
            presentInfo.pSwapchains = &swapChain;
            presentInfo.pImageIndices = &imageIndex;

            const VkResult presentResult = vkQueuePresentKHR(presentQueue, &presentInfo);
            if (presentResult != VK_SUCCESS && presentResult != VK_SUBOPTIMAL_KHR)
            {
                ThrowIfFailed(presentResult, "La présentation de l'image a échoué");
            }
        }

        vkDeviceWaitIdle(device);
    }
    catch (const std::exception& exception)
    {
        std::cerr << "Erreur: " << exception.what() << '\n';
    }

    if (device != VK_NULL_HANDLE)
    {
        vkDeviceWaitIdle(device);
    }

    if (inFlightFence != VK_NULL_HANDLE)
    {
        vkDestroyFence(device, inFlightFence, nullptr);
    }

    if (renderFinishedSemaphore != VK_NULL_HANDLE)
    {
        vkDestroySemaphore(device, renderFinishedSemaphore, nullptr);
    }

    if (imageAvailableSemaphore != VK_NULL_HANDLE)
    {
        vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);
    }

    if (commandPool != VK_NULL_HANDLE)
    {
        vkDestroyCommandPool(device, commandPool, nullptr);
    }

    for (VkFramebuffer framebuffer : framebuffers)
    {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }

    if (renderPass != VK_NULL_HANDLE)
    {
        vkDestroyRenderPass(device, renderPass, nullptr);
    }

    for (VkImageView imageView : swapChainImageViews)
    {
        vkDestroyImageView(device, imageView, nullptr);
    }

    if (swapChain != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(device, swapChain, nullptr);
    }

    if (device != VK_NULL_HANDLE)
    {
        vkDestroyDevice(device, nullptr);
    }

    if (surface != VK_NULL_HANDLE)
    {
        vkDestroySurfaceKHR(instance, surface, nullptr);
    }

    if (instance != VK_NULL_HANDLE)
    {
        vkDestroyInstance(instance, nullptr);
    }

    if (window != nullptr)
    {
        glfwDestroyWindow(window);
    }

    glfwTerminate();
    return 0;
}