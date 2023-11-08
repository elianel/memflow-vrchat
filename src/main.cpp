#include <thread>
#include <chrono>
#include <memflow.hpp>
#include "logger.h"
#include "memory.h"
#include "Unity/Unity.h"
#include "vk_keys.h"
#include "offsets.h"

Memory *pMemory;
Unity *pUnity;

int main(int argc, char *argv[])
{
        // log_init(LevelFilter::LevelFilter_Info); //memflow log level
        Logger::setLogLevel(LogLevel::LOG_DEBUG);
        Memory mem;
        mem.open_process("VRChat.exe");
        pMemory = &mem;

        Unity unity(mem);
        if (!unity.init())
        {
                Logger::warn("Failed to init unity.");
                return 1;
        }

        pUnity = &unity;

        while (true)
        {
                std::this_thread::sleep_for(std::chrono::milliseconds(25));
                if (!pMemory->is_key_pressed(VK_SHIFT))
                        continue;
                if (pMemory->is_key_pressed(VK_LEFT))
                {
                        Logger::info("Toggled speed hack ON\n");
                        pUnity->set_time_scale(10.0f);
                }
                if (pMemory->is_key_pressed(VK_RIGHT))
                {
                        Logger::info("Toggled speed hack OFF\n");
                        pUnity->set_time_scale(1.0);
                }
        }
        return 0;
}
