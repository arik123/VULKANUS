#include <optional>
#include <set>
#include "Window.h"

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

class HelloTriangleApplication {
public:
    void run() {
        wnd.create();
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    Window wnd;
    vk::Instance instance;
    vk::DebugUtilsMessengerEXT debugMessenger;
    vk::DispatchLoaderDynamic dynamicDispatcher;
    vk::PhysicalDevice physicalDevice;
    vk::Device device;
	vk::Queue graphicsQueue;
    vk::SurfaceKHR surface;
	vk::Queue presentQueue;
    vk::SwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    vk::Format swapChainImageFormat;
    vk::Extent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;
    void initVulkan() {
        createInstance();
        setupDebugMessenger();
        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();
        createSwapChain();
        createImageViews();
    }
    void createInstance() {
        if (enableValidationLayers && !checkValidationLayerSupport()) {
            throw std::runtime_error("validation layers requested, but not available!");
        }
        vk::ApplicationInfo app_info_info("Hello Triangle", VK_MAKE_VERSION(1, 0, 0),
            "No Engine", VK_MAKE_VERSION(1, 0, 0), VK_API_VERSION_1_0);
        auto reqExtensions = getRequiredExtensions();
        auto createInfo = vk::InstanceCreateInfo()
            .setPApplicationInfo(&app_info_info)
            .setPEnabledExtensionNames(reqExtensions)
            .setPEnabledLayerNames(validationLayers);
    	
        if (enableValidationLayers) {
            createInfo.setPEnabledLayerNames(validationLayers);
        	vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo;
            populateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = reinterpret_cast<VkDebugUtilsMessengerCreateInfoEXT*>(&debugCreateInfo);
        }
        instance = vk::createInstance(createInfo);
        dynamicDispatcher = vk::DispatchLoaderDynamic(instance, vkGetInstanceProcAddr);
        auto extensions = vk::enumerateInstanceExtensionProperties();
        std::cout << "available extensions:\n";

        for (const auto& extension : extensions) {
            std::cout << '\t' << extension.extensionName << '\n';
        }
    }
    bool checkValidationLayerSupport() {
        auto availableLayers = vk::enumerateInstanceLayerProperties();
        for (const char* layerName : validationLayers) {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) {
                return false;
            }
        }
        return true;
    }
    std::vector<const char*> getRequiredExtensions() {
#ifdef _WIN32
        std::vector<const char*> reqExtensions = {
            "VK_KHR_surface",
            "VK_KHR_win32_surface"
        };
#endif

        if (enableValidationLayers) {
            reqExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return reqExtensions;
    }
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) {
        if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
            std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
        }
        
        return VK_FALSE;
    }
    void populateDebugMessengerCreateInfo(vk::DebugUtilsMessengerCreateInfoEXT& createInfo) {
        createInfo = vk::DebugUtilsMessengerCreateInfoEXT()
    		.setMessageSeverity(vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo
                | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError)
            .setMessageType(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation)
            .setPfnUserCallback(debugCallback);
    }
    void setupDebugMessenger() {
        if (!enableValidationLayers) return;
        vk::DebugUtilsMessengerCreateInfoEXT createInfo;
        populateDebugMessengerCreateInfo(createInfo);
        debugMessenger = instance.createDebugUtilsMessengerEXT(createInfo, nullptr,dynamicDispatcher);
    }
    bool checkDeviceExtensionSupport(vk::PhysicalDevice device) {
        auto availableExtensions = device.enumerateDeviceExtensionProperties();

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for (const auto& extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }
    bool isDeviceSuitable(vk::PhysicalDevice device) {
        QueueFamilyIndices indices = findQueueFamilies(device);

        bool extensionsSupported = checkDeviceExtensionSupport(device);
        bool swapChainAdequate = false;
        if (!extensionsSupported) return false;
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        return indices.isComplete() && swapChainAdequate;
    }
	void createSurface() {
    	
        auto win32SurfaceCreateInfo = vk::Win32SurfaceCreateInfoKHR()
            .setHwnd(wnd.getHandle())
            .setHinstance(GetModuleHandle(NULL));
        surface = instance.createWin32SurfaceKHR(win32SurfaceCreateInfo);
    }
    void pickPhysicalDevice() {
        auto devices = instance.enumeratePhysicalDevices();
        if (devices.size() == 0) {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }
        for (const auto& device : devices) {
            if (isDeviceSuitable(device)) {
                physicalDevice = device;
                break;
            }
        }

        if (!physicalDevice) {
            throw std::runtime_error("failed to find a suitable GPU!");
        }
    	
    }

    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;
    	bool isComplete() {
			return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice device) {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
    	
		auto queueFamilies = device.getQueueFamilyProperties();
        int i = 0;
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
                indices.graphicsFamily = i;
            }
            if(device.getSurfaceSupportKHR(i, surface))
            {
                indices.presentFamily = i;
            }
            if (indices.isComplete()) {
                break;
            }
            i++;
        }
        return indices;
    }
    void createLogicalDevice() {
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            auto queueCreateInfo = vk::DeviceQueueCreateInfo()
                .setQueueFamilyIndex(queueFamily)
                .setQueueCount(1)
                .setPQueuePriorities(&queuePriority);
            queueCreateInfos.emplace_back(queueCreateInfo);
        }
        vk::PhysicalDeviceFeatures deviceFeatures{};
        auto createInfo = vk::DeviceCreateInfo()
            .setQueueCreateInfos(queueCreateInfos)
    		.setPEnabledFeatures(&deviceFeatures)
    		.setPEnabledExtensionNames(deviceExtensions);
        device = physicalDevice.createDevice(createInfo);
        graphicsQueue = device.getQueue(indices.graphicsFamily.value(), 0);
        presentQueue = device.getQueue(indices.presentFamily.value(), 0);
    }
    struct SwapChainSupportDetails {
        vk::SurfaceCapabilitiesKHR capabilities;
        std::vector<vk::SurfaceFormatKHR> formats;
        std::vector<vk::PresentModeKHR> presentModes;
    };
    SwapChainSupportDetails querySwapChainSupport(vk::PhysicalDevice device) {
        SwapChainSupportDetails details;
        details.capabilities = device.getSurfaceCapabilitiesKHR(surface);
        details.formats = device.getSurfaceFormatsKHR(surface);
        details.presentModes= device.getSurfacePresentModesKHR(surface);
        return details;
    }
    vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) {
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
                return availableFormat;
            }
        }
        return availableFormats[0];
    }
    vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes) {
        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
                return availablePresentMode;
            }
        }
        return vk::PresentModeKHR::eFifo;
    }
    vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities) {
        if (capabilities.currentExtent.width != UINT32_MAX) return capabilities.currentExtent;
        RECT window;
        GetClientRect(wnd.getHandle(), &window); //WINDOWS
        const int width = window.right, height = window.bottom;

        vk::Extent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

        return actualExtent;
    }
    void createSwapChain() {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

        vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        vk::PresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        vk::Extent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }
        auto createInfo = vk::SwapchainCreateInfoKHR()
            .setSurface(surface)
    		.setMinImageCount(imageCount)
    		.setImageFormat(surfaceFormat.format)
    		.setImageColorSpace(surfaceFormat.colorSpace)
    		.setImageExtent(extent)
    		.setImageArrayLayers(1)
    		.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
    		.setPreTransform(swapChainSupport.capabilities.currentTransform)
    		.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
    		.setPresentMode(presentMode)
    		.setClipped(1)
    		.setOldSwapchain(nullptr);
    	    	
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
        uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

        if (indices.graphicsFamily != indices.presentFamily) {
            createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else {
            createInfo.imageSharingMode = vk::SharingMode::eExclusive;
        }

        swapChain = device.createSwapchainKHR(createInfo);
        swapChainImages = device.getSwapchainImagesKHR(swapChain);
        swapChainImageFormat = surfaceFormat.format;
        swapChainExtent = extent;
    }
    void createImageViews() {
        swapChainImageViews.resize(swapChainImages.size());
        for (size_t i = 0; i < swapChainImages.size(); i++) {
            auto createInfo = vk::ImageViewCreateInfo()
        		.setImage(swapChainImages[i])
        		.setViewType(vk::ImageViewType::e2D)
        		.setFormat(swapChainImageFormat)
        		.setComponents({vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity , vk::ComponentSwizzle::eIdentity , vk::ComponentSwizzle::eIdentity})
        		.setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
            swapChainImageViews[i] = device.createImageView(createInfo);
        }

    }
    void mainLoop() {

    }

    void cleanup() {
        for (auto imageView : swapChainImageViews) {
            device.destroyImageView(imageView);
        }
        device.destroySwapchainKHR(swapChain);
        device.destroy();
        instance.destroySurfaceKHR(surface);
    	if (enableValidationLayers)
    	{
            instance.destroyDebugUtilsMessengerEXT(debugMessenger, nullptr, dynamicDispatcher);
    	}
        instance.destroy();
        wnd.destroy();
    }
};