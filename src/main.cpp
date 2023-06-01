
#include <array>
#include <iostream>
#include <string>
#include <thread>

#include "glm/glm.hpp"
#include "gsl/gsl"

#include "RAII_Samples/utils/shaders.hpp"
#include "RAII_Samples/utils/utils.hpp"
#include "SPIRV/GlslangToSpv.h"
#include "samples/utils/geometries.hpp"
#include "samples/utils/math.hpp"

#include "vulkan/vulkan.hpp"

#include "SDL_vulkan.h"
#include "sdlpp.hpp"

const std::string AppName { "VulkanTest" };
const std::string EngineName { "No Engine" };

auto main() -> int
{
    std::cout << "App Ready" << std::endl;

    try {

        vk::raii::Context context;

        sdl::Init init( SDL_INIT_VIDEO );

        const auto Width = 800;
        const auto Height = 600;

        vk::Extent2D extent { Width, Height };
        sdl::Window w( AppName.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, Width, Height, SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN );

        // SDL method
        {
            // get total extensions for this device
            uint32_t extensionCount = 0;
            auto res = vk::enumerateInstanceExtensionProperties( nullptr, &extensionCount, nullptr );

            if ( res == vk::Result::eSuccess ) {

                // prepare an empty vector for the extension names as const zero terminated strings
                std::vector<gsl::czstring> extStrArr {};

                // enumerate all extensions available
                std::vector<vk::ExtensionProperties> readExtensions( extensionCount );
                res = vk::enumerateInstanceExtensionProperties( nullptr, &extensionCount, readExtensions.data() );
                if ( res == vk::Result::eSuccess ) {

                    // print and add each extension to the array
                    for ( const auto& extension : readExtensions ) {
                        extStrArr.push_back( extension.extensionName );
                    }
                }

                //auto resultExt = SDL_Vulkan_GetInstanceExtensions( w.get(), &extensionCount, extStrArr.data() );
                //if ( resultExt != SDL_TRUE ) {
                //    std::cout << "ERROR with extension request in SDL_Vulkan_GetInstanceExtensions" << std::endl;
                //    return 1;
                //}

                // initialize the vk::ApplicationInfo structure
                vk::ApplicationInfo applicationInfo( AppName.c_str(), 1, EngineName.c_str(), 1, VK_API_VERSION_1_2 );
                // initialize the vk::InstanceCreateInfo
                vk::InstanceCreateInfo instanceCreateInfo( {}, &applicationInfo, {}, {}, extensionCount, extStrArr.data(), nullptr );

                // create an Instance
                vk::raii::Instance instance( context, instanceCreateInfo );

#if !defined( NDEBUG )
                vk::raii::DebugUtilsMessengerEXT debugUtilsMessenger( instance, vk::su::makeDebugUtilsMessengerCreateInfoEXT() );
#endif

                // create the surface data based on the window
                VkSurfaceKHR rawSurface;
                SDL_Vulkan_CreateSurface( w.get(), static_cast<VkInstance>( *instance ), &rawSurface );

                vk::raii::SurfaceKHR surface { instance, rawSurface };

                // setup device
                vk::raii::PhysicalDevice physicalDevice = vk::raii::PhysicalDevices( instance ).front();

                // find the index of the first queue family that supports graphics
                uint32_t graphicsQueueFamilyIndex = vk::su::findGraphicsQueueFamilyIndex( physicalDevice.getQueueFamilyProperties() );

                // create a Device
                float queuePriority = 0.0f;
                vk::DeviceQueueCreateInfo deviceQueueCreateInfo( {}, graphicsQueueFamilyIndex, 1, &queuePriority );
                vk::DeviceCreateInfo deviceCreateInfo( {}, deviceQueueCreateInfo );

                //vk::raii::Device device( physicalDevice, deviceCreateInfo );

                std::pair<uint32_t, uint32_t> graphicsAndPresentQueueFamilyIndex = vk::raii::su::findGraphicsAndPresentQueueFamilyIndex( physicalDevice, surface );
                vk::raii::Device device = vk::raii::su::makeDevice( physicalDevice, graphicsAndPresentQueueFamilyIndex.first, vk::su::getDeviceExtensions() );

                vk::raii::CommandPool commandPool = vk::raii::CommandPool( device, { vk::CommandPoolCreateFlagBits::eResetCommandBuffer, graphicsAndPresentQueueFamilyIndex.first } );
                vk::raii::CommandBuffer commandBuffer = vk::raii::su::makeCommandBuffer( device, commandPool );

                vk::raii::Queue graphicsQueue( device, graphicsAndPresentQueueFamilyIndex.first, 0 );
                vk::raii::Queue presentQueue( device, graphicsAndPresentQueueFamilyIndex.second, 0 );

                vk::raii::su::SwapChainData swapChainData( physicalDevice,
                    device,
                    surface,
                    extent,
                    vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc,
                    {},
                    graphicsAndPresentQueueFamilyIndex.first,
                    graphicsAndPresentQueueFamilyIndex.second );

                vk::raii::su::DepthBufferData depthBufferData( physicalDevice, device, vk::Format::eD16Unorm, extent );

                vk::raii::su::BufferData uniformBufferData( physicalDevice, device, sizeof( glm::mat4x4 ), vk::BufferUsageFlagBits::eUniformBuffer );
                glm::mat4x4 mvpcMatrix = vk::su::createModelViewProjectionClipMatrix( extent );
                vk::raii::su::copyToDevice( uniformBufferData.deviceMemory, mvpcMatrix );

                vk::raii::DescriptorSetLayout descriptorSetLayout = vk::raii::su::makeDescriptorSetLayout( device, { { vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex } } );
                vk::raii::PipelineLayout pipelineLayout( device, { {}, *descriptorSetLayout } );

                vk::Format colorFormat = vk::su::pickSurfaceFormat( physicalDevice.getSurfaceFormatsKHR( *surface ) ).format;
                vk::raii::RenderPass renderPass = vk::raii::su::makeRenderPass( device, colorFormat, depthBufferData.format );

                glslang::InitializeProcess();
                vk::raii::ShaderModule vertexShaderModule = vk::raii::su::makeShaderModule( device, vk::ShaderStageFlagBits::eVertex, vertexShaderText_PC_C );
                vk::raii::ShaderModule fragmentShaderModule = vk::raii::su::makeShaderModule( device, vk::ShaderStageFlagBits::eFragment, fragmentShaderText_C_C );
                glslang::FinalizeProcess();

                std::vector<vk::raii::Framebuffer> framebuffers = vk::raii::su::makeFramebuffers( device, renderPass, swapChainData.imageViews, &depthBufferData.imageView, extent );

                vk::raii::su::BufferData vertexBufferData( physicalDevice, device, sizeof( coloredCubeData ), vk::BufferUsageFlagBits::eVertexBuffer );
                vk::raii::su::copyToDevice( vertexBufferData.deviceMemory, coloredCubeData, sizeof( coloredCubeData ) / sizeof( coloredCubeData[0] ) );

                vk::raii::DescriptorPool descriptorPool = vk::raii::su::makeDescriptorPool( device, { { vk::DescriptorType::eUniformBuffer, 1 } } );
                vk::raii::DescriptorSet descriptorSet = std::move( vk::raii::DescriptorSets( device, { *descriptorPool, *descriptorSetLayout } ).front() );
                vk::raii::su::updateDescriptorSets(
                    device, descriptorSet, { { vk::DescriptorType::eUniformBuffer, uniformBufferData.buffer, VK_WHOLE_SIZE, nullptr } }, {} );

                vk::raii::PipelineCache pipelineCache( device, vk::PipelineCacheCreateInfo() );
                vk::raii::Pipeline graphicsPipeline = vk::raii::su::makeGraphicsPipeline( device,
                    pipelineCache,
                    vertexShaderModule,
                    nullptr,
                    fragmentShaderModule,
                    nullptr,
                    vk::su::checked_cast<uint32_t>( sizeof( coloredCubeData[0] ) ),
                    { { vk::Format::eR32G32B32A32Sfloat, 0 }, { vk::Format::eR32G32B32A32Sfloat, 16 } },
                    vk::FrontFace::eClockwise,
                    true,
                    pipelineLayout,
                    renderPass );

                /* VULKAN_KEY_START */

                // Get the index of the next available swapchain image:
                vk::raii::Semaphore imageAcquiredSemaphore( device, vk::SemaphoreCreateInfo() );

                vk::Result result;
                uint32_t imageIndex;
                std::tie( result, imageIndex ) = swapChainData.swapChain.acquireNextImage( vk::su::FenceTimeout, *imageAcquiredSemaphore );
                assert( result == vk::Result::eSuccess );
                assert( imageIndex < swapChainData.images.size() );

                commandBuffer.begin( {} );

                std::array<vk::ClearValue, 2> clearValues;
                clearValues[0].color = vk::ClearColorValue( 0.3f, 0.2f, 0.2f, 0.2f );
                clearValues[1].depthStencil = vk::ClearDepthStencilValue( 1.0f, 0 );
                vk::RenderPassBeginInfo renderPassBeginInfo( *renderPass, *framebuffers[imageIndex], vk::Rect2D( vk::Offset2D( 0, 0 ), extent ), clearValues );
                commandBuffer.beginRenderPass( renderPassBeginInfo, vk::SubpassContents::eInline );
                commandBuffer.bindPipeline( vk::PipelineBindPoint::eGraphics, *graphicsPipeline );
                commandBuffer.bindDescriptorSets( vk::PipelineBindPoint::eGraphics, *pipelineLayout, 0, { *descriptorSet }, nullptr );

                commandBuffer.bindVertexBuffers( 0, { *vertexBufferData.buffer }, { 0 } );
                commandBuffer.setViewport(
                    0, vk::Viewport( 0.0f, 0.0f, static_cast<float>( extent.width ), static_cast<float>( extent.height ), 0.0f, 1.0f ) );
                commandBuffer.setScissor( 0, vk::Rect2D( vk::Offset2D( 0, 0 ), extent ) );

                commandBuffer.draw( 12 * 3, 1, 0, 0 );
                commandBuffer.endRenderPass();
                commandBuffer.end();

                vk::raii::Fence drawFence( device, vk::FenceCreateInfo() );

                vk::PipelineStageFlags waitDestinationStageMask( vk::PipelineStageFlagBits::eColorAttachmentOutput );
                vk::SubmitInfo submitInfo( *imageAcquiredSemaphore, waitDestinationStageMask, *commandBuffer );
                graphicsQueue.submit( submitInfo, *drawFence );

                while ( vk::Result::eTimeout == device.waitForFences( { *drawFence }, VK_TRUE, vk::su::FenceTimeout ) )
                    ;

                vk::PresentInfoKHR presentInfoKHR( nullptr, *swapChainData.swapChain, imageIndex );
                result = presentQueue.presentKHR( presentInfoKHR );
                switch ( result ) {
                case vk::Result::eSuccess:
                    break;
                case vk::Result::eSuboptimalKHR:
                    std::cout << "vk::Queue::presentKHR returned vk::Result::eSuboptimalKHR !\n";
                    break;
                default:
                    assert( false ); // an unexpected result is returned !
                }
                std::this_thread::sleep_for( std::chrono::milliseconds( 1000 ) );

                /* VULKAN_KEY_END */

                device.waitIdle();
            }
        }

        // vulkan method
        /*{
            // get total extensions for this device
            uint32_t extensionCount = 0;
            auto res = vk::enumerateInstanceExtensionProperties( nullptr, &extensionCount, nullptr );

            if ( res == vk::Result::eSuccess ) {

                // prepare an empty vector for the extension names as const zero terminated strings
                std::vector<gsl::czstring> extStrArr {};

                // enumerate all extensions available
                std::vector<vk::ExtensionProperties> readExtensions( extensionCount );
                res = vk::enumerateInstanceExtensionProperties( nullptr, &extensionCount, readExtensions.data() );
                if ( res == vk::Result::eSuccess ) {

                    // print and add each extension to the array
                    for ( const auto& extension : readExtensions ) {
                        extStrArr.push_back( extension.extensionName );
                    }
                }

                std::cout << "Extensions found:" << '\n';
                for ( const auto& extName : extStrArr ) {
                    std::cout << '\t' << extName << '\n';
                }
            }
        }*/

        sdl::EventHandler e {};

        auto done = false;
        e.keyDown = [&done]( const SDL_KeyboardEvent& evt ) {
            if ( evt.keysym.sym == SDL_KeyCode::SDLK_ESCAPE ) {
                done = true;
            }
        };
        e.quit = [&done]( const SDL_QuitEvent& ) {
            done = true;
        };

        while ( !done ) {
            while ( e.poll() ) {
            }
        }

        SDL_Quit();

        //SDL_Init( SDL_INIT_EVERYTHING );
        //auto window = SDL_CreateWindow( AppName.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN );

        // start glfw
        /*auto uniqueInst = vkfw::initUnique( {} );

        if ( vkfw::vulkanSupported() ) {

            // initialize the vk::ApplicationInfo structure
            vk::ApplicationInfo applicationInfo( AppName.c_str(), 1, EngineName.c_str(), 1, VK_API_VERSION_1_2 );

            // get total extensions for this device
            uint32_t extensionCount = 0;
            auto res = vk::enumerateInstanceExtensionProperties( nullptr, &extensionCount, nullptr );

            if ( res == vk::Result::eSuccess ) {

                // prepare an empty vector for the extension names as const zero terminated strings
                std::vector<gsl::czstring> extStrArr {};

                // enumerate all extensions available
                std::vector<vk::ExtensionProperties> readExtensions( extensionCount );
                res = vk::enumerateInstanceExtensionProperties( nullptr, &extensionCount, readExtensions.data() );
                if ( res == vk::Result::eSuccess ) {

                    // print and add each extension to the array
                    for ( const auto& extension : readExtensions ) {
                        extStrArr.push_back( extension.extensionName );
                    }
                }

                std::cout << "Extensions found:" << '\n';
                for ( const auto& extName : extStrArr ) {
                    std::cout << '\t' << extName << '\n';
                }
                // initialize the vk::InstanceCreateInfo
                vk::InstanceCreateInfo instanceCreateInfo( {}, &applicationInfo, {}, {}, extensionCount, extStrArr.data(), nullptr );

                // create an Instance
                vk::Instance instance = vk::createInstance( instanceCreateInfo );

                // enumerate the physicalDevices
                vk::PhysicalDevice physicalDevice = instance.enumeratePhysicalDevices().front();

                // destroy it again
                instance.destroy();
            }
        }*/
    } catch ( vk::SystemError& err ) {
        std::cout << "vk::SystemError: " << err.what() << std::endl;
        exit( -1 );
    } catch ( std::exception& err ) {
        std::cout << "std::exception: " << err.what() << std::endl;
        exit( -1 );
    } catch ( ... ) {
        std::cout << "unknown error\n";
        exit( -1 );
    }

    //vkfw::terminate();

    return 0;
}