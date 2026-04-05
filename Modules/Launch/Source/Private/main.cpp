#include "vulkan/vulkan.h"
#include "GLFW/glfw3.h"

#include <iostream>
#include <vector>
#include <cstring>

int main()
{
    // ---- Init GLFW ----
    glfwInitVulkanLoader(vkGetInstanceProcAddr);

    if (!glfwInit())
    {
        std::cerr << "GLFW init failed\n";
        return -1;
    }

    // IMPORTANT : laisser GLFW choisir (Wayland ou X11)
    glfwWindowHint(GLFW_PLATFORM, GLFW_ANY_PLATFORM);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Vulkan", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Window creation failed\n";
        return -1;
    }

    // ---- Vulkan Instance ----
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.apiVersion = VK_API_VERSION_1_3;

    uint32_t glfwExtCount = 0;
    const char** glfwExt = glfwGetRequiredInstanceExtensions(&glfwExtCount);

    if (!glfwExt)
    {
        std::cerr << "GLFW returned NULL extensions\n";
        return -1;
    }

    std::cout << "GLFW requested extensions:\n";
    for (uint32_t i = 0; i < glfwExtCount; i++)
        std::cout << "  " << glfwExt[i] << "\n";

    // ---- Get available Vulkan extensions ----
    uint32_t availCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &availCount, nullptr);

    std::vector<VkExtensionProperties> avail(availCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &availCount, avail.data());

    std::cout << "Available instance extensions:\n";
    for (const auto &e : avail)
        std::cout << "  " << e.extensionName << "\n";

    // ---- Filter valid extensions ----
    std::vector<const char*> enabledExt;

    for (uint32_t i = 0; i < glfwExtCount; i++)
    {
        bool found = false;

        for (const auto& e : avail)
        {
            if (strcmp(glfwExt[i], e.extensionName) == 0)
            {
                found = true;
                break;
            }
        }

        if (found)
        {
            enabledExt.push_back(glfwExt[i]);
        }
        else
        {
            std::cout << "Skipping unsupported: " << glfwExt[i] << "\n";
        }
    }

    std::cout << "Enabled extensions:\n";
    for (const auto &e : enabledExt)
        std::cout << "  " << e << "\n";

    // ---- Create instance ----
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExt.size());
    createInfo.ppEnabledExtensionNames = enabledExt.data();
    createInfo.enabledLayerCount = 0;

    VkInstance instance;
    VkResult res = vkCreateInstance(&createInfo, nullptr, &instance);

    if (res != VK_SUCCESS)
    {
        std::cerr << "Failed to create instance: " << res << "\n";
        return -1;
    }

    std::cout << "Vulkan instance created\n";

    // ---- Create surface (auto platform) ----
    VkSurfaceKHR surface;
    VkResult surfRes = glfwCreateWindowSurface(instance, window, nullptr, &surface);
    if (surfRes != VK_SUCCESS)
    {
        std::cerr << "Failed to create surface: " << surfRes << "\n";
        return -1;
    }

    std::cout << "Surface created (platform OK)\n";

    // ---- Select GPU ----
    uint32_t gpuCount = 0;
    vkEnumeratePhysicalDevices(instance, &gpuCount, nullptr);

    if (gpuCount == 0)
    {
        std::cerr << "No GPU found\n";
        return -1;
    }

    std::vector<VkPhysicalDevice> gpus(gpuCount);
    vkEnumeratePhysicalDevices(instance, &gpuCount, gpus.data());

    VkPhysicalDevice physicalDevice = gpus[0];

    // ---- Queue ----
    uint32_t queueCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, nullptr);

    std::vector<VkQueueFamilyProperties> queues(queueCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, queues.data());

    uint32_t graphicsIndex = -1;

    for (uint32_t i = 0; i < queueCount; i++)
    {
        if (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            graphicsIndex = i;
            break;
        }
    }

    if (graphicsIndex == (uint32_t)-1)
    {
        std::cerr << "No graphics queue found\n";
        return -1;
    }

    float priority = 1.0f;

    VkDeviceQueueCreateInfo queueInfo{};
    queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo.queueFamilyIndex = graphicsIndex;
    queueInfo.queueCount = 1;
    queueInfo.pQueuePriorities = &priority;

    VkDeviceCreateInfo deviceInfo{};
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.queueCreateInfoCount = 1;
    deviceInfo.pQueueCreateInfos = &queueInfo;

    VkDevice device;
    if (vkCreateDevice(physicalDevice, &deviceInfo, nullptr, &device) != VK_SUCCESS)
    {
        std::cerr << "Failed to create device\n";
        return -1;
    }

    VkQueue queue;
    vkGetDeviceQueue(device, graphicsIndex, 0, &queue);

    std::cout << "Device + queue ready\n";

    // ---- Main loop ----
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }

    // ---- Cleanup ----
    vkDestroyDevice(device, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}