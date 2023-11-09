#pragma once
#include <memflow.hpp>
#include <mutex>
#include "logger.h"

static ConnectorInstance<> *conn;

/*
        TODO: Cache get_module() results
*/

class Memory
{
private:
        OsInstance<> os;
        ProcessInstance<> proc;
        KeyboardBase<> keyboard;
        std::mutex mutex;

public:
        bool os_init();
        ProcessState get_proc_state();
        void open_process(const char *processName);
        ProcessInstance<> &get_proc();

        Address get_base_module_address();
        ModuleInfo get_module(const char *name);
        ModuleInfo get_primary_module();

        Address pattern_scan(Address start, size_t size, const char *pattern);
        Address pattern_scan_module_section(const char *moduleName, const char *sectionName, const char *pattern);
        Address pattern_scan_module(const char *moduleName, const char *pattern);

        Address relative_to_absolute(Address address, int offset_to_delta = 1);

        bool is_key_pressed(int VK);
        void set_key_down(int VK, bool down);

        template <typename T>
        inline bool Read(Address address, T &out, size_t size = 1);
        template <typename T>
        inline bool Write(Address address, const T &value, size_t size = 1);
};

template <typename T>
inline bool Memory::Read(Address address, T &out, size_t size)
{
        std::lock_guard<std::mutex> l(mutex);
        if (proc.state().tag != ProcessState::Tag::ProcessState_Alive)
                return false;
        CSliceMut<uint8_t> buf;
        buf.len = sizeof(T) * size;
        buf.data = new uint8_t[(int)buf.len];
        int32_t readres = proc.read_raw_into(address, buf);
        std::memcpy(&out, buf.data, buf.len);
        delete[] buf.data;
        return readres == 0;
}
template <typename T>
inline bool Memory::Write(Address address, const T &value, size_t size)
{
        std::lock_guard<std::mutex> l(mutex);
        CSliceRef<uint8_t> buf;
        buf.data = reinterpret_cast<const uint8_t *>(&value);
        buf.len = sizeof(T) * size;
        return proc.state().tag == ProcessState::Tag::ProcessState_Alive && proc.write_raw(address, buf) == 0;
}

// thanks xoner
typedef uint8_t *PBYTE;
typedef uint8_t BYTE;
typedef unsigned long DWORD;
typedef unsigned short WORD;
typedef WORD *PWORD;
#define INRANGE(x, a, b) (x >= a && x <= b)
#define getBits(x) \
        (INRANGE(x, '0', '9') ? (x - '0') : ((x & (~0x20)) - 'A' + 0xa))
#define getByte(x) (getBits(x[0]) << 4 | getBits(x[1]))
size_t find_pattern(const PBYTE rangeStart, size_t len, const char *pattern);
