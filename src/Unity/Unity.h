#pragma once

#include "../memory.h"

#include "../logger.h"
#include "../offsets.h"

class Unity
{
private:
        Memory &mem;

public:
        Unity(Memory &mem);
        bool init();

        Address TimeManager_VA;

        float get_time_scale();
        void set_time_scale(float newValue);

        float get_delta_time();
        void set_delta_time(float newValue);
};

Unity::Unity(Memory &memory) : mem(memory) {}

bool Unity::init()
{

        Address pat = mem.pattern_scan_module_section("UnityPlayer.dll", ".text", SIG_UNITYPLAYER_TIMEMANAGER);
        if (pat <= 0)
        {
                Logger::warn("Failed to find pattern for TimeManager");
                return false;
        }
        pat = mem.relative_to_absolute(pat, 19);

        mem.Read<Address>(pat, TimeManager_VA);
        Logger::debug("TimeManager VA: 0x%013llX\n", TimeManager_VA);

        return true;
}
float Unity::get_time_scale()
{
        float result = 0; // declare like this to not worry about memory management
        if (!mem.Read(TimeManager_VA + OFFSET_UNITYPLAYER_TIMEMANAGER_TIMESCALE, result))
                Logger::warn("Failed to read time scale.\n");
        return result;
}
void Unity::set_time_scale(float newValue)
{
        if (!mem.Write(TimeManager_VA + OFFSET_UNITYPLAYER_TIMEMANAGER_TIMESCALE, newValue))
                Logger::warn("Failed to write time scale.\n");
}
float Unity::get_delta_time()
{
        float result = 0;
        if (!mem.Read(TimeManager_VA + OFFSET_UNITYPLAYER_TIMEMANAGER_DELTATIME, result))
                Logger::warn("Failed to read delta time.\n");
        return result;
}
void Unity::set_delta_time(float newValue)
{
        if (!mem.Write(TimeManager_VA + OFFSET_UNITYPLAYER_TIMEMANAGER_DELTATIME, newValue))
                Logger::warn("Failed to write delta time.\n");
}