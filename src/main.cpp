
#include <array>
#include <iostream>
#include <string>

#include "glm/glm.hpp"
#include "gsl/gsl"
#include "vkfw/vkfw.hpp"

//#include "RAII_Samples/utils/utils.hpp"
#include "vulkan/vulkan.hpp"

#include "SDL_vulkan.h"
#include "sdlpp.hpp"

const std::string AppName { "VulkanTest" };
const std::string EngineName { "No Engine" };

auto main() -> int
{
    std::cout << "App Ready" << std::endl;

    try {

        //vk::raii::Context context;

        sdl::Init init( SDL_INIT_VIDEO );

        const auto Width = 800;
        const auto Height = 600;
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

                auto resultExt = SDL_Vulkan_GetInstanceExtensions( w.get(), &extensionCount, extStrArr.data() );
                if ( resultExt != SDL_TRUE ) {
                    std::cout << "ERROR with extension request in SDL_Vulkan_GetInstanceExtensions" << std::endl;
                    return 1;
                }

                // initialize the vk::ApplicationInfo structure
                vk::ApplicationInfo applicationInfo( AppName.c_str(), 1, EngineName.c_str(), 1, VK_API_VERSION_1_2 );
                // initialize the vk::InstanceCreateInfo
                vk::InstanceCreateInfo instanceCreateInfo( {}, &applicationInfo, {}, {}, extensionCount, extStrArr.data(), nullptr );

                // create an Instance
                vk::Instance instance = vk::createInstance( instanceCreateInfo );

                //vk::DebugUtilsMessengerEXT debugUtilsMessenger = instance.createDebugUtilsMessengerEXT( makeDebugUtilsMessengerCreateInfoEXT() );
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