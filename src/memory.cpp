#include "memory.h"
ProcessState Memory::get_proc_state() { return proc.state(); }
ProcessInstance<> &Memory::get_proc() { return proc; };
bool Memory::is_key_pressed(int VK)
{
        std::lock_guard<std::mutex> l(mutex);
        return keyboard.is_down(VK);
}
void Memory::set_key_down(int VK, bool down)
{
        std::lock_guard<std::mutex> l(mutex);
        keyboard.set_down(VK, down);
}

bool Memory::os_init()
{
        Inventory *inventory = inventory_scan();

        ConnectorInstance<> conninst;
        if (inventory_create_connector(inventory, "qemu", "",
                                       &conninst))
        {
                Logger::warn("Failed to create connector.\n");
                return false;
        }
        conn = &conninst;
        if (inventory_create_os(inventory, "win32", "", conn, &os))
        {
                Logger::warn("Failed to create os.\n");
                return false;
        }

        inventory_free(inventory);
        os.keyboard(&keyboard);
        return true;
}

void Memory::open_process(const char *name)
{

        if (conn == nullptr || &os == nullptr)
        {
                if (!os_init())
                {
                        Logger::warn("Failed to init os. Exiting..\n");
                        exit(1);
                }
        }
        if (os.process_by_name(CSliceRef<uint8_t>(name), &proc))
        {
                Logger::warn("Failed to open process by name %s\n", name);
                exit(1);
        }
        Logger::info("Opened process (PID %d) %s %s\n", proc.info()->pid,
                     proc.info()->name, proc.info()->command_line);
        Logger::debug("It's primary module %s, is at 0x%013llX\n",
                      get_primary_module().name, get_primary_module().base);
}
Address Memory::get_base_module_address()
{
        Address addr;
        proc.primary_module_address(&addr);
        return addr;
}
ModuleInfo Memory::get_primary_module()
{
        ModuleInfo m;
        proc.primary_module(&m);
        return m;
}
ModuleInfo Memory::get_module(const char *name)
{
        ModuleInfo out;
        if (proc.module_by_name(name, &out))
        {
                Logger::warn("Failed getting module info for: %s\n", name);
                exit(1);
        }
        Logger::debug(
            "Memory::get_module(\"%s\"): Address: 0x%013llX Base: 0x%013llX (%d), Size:%d\n",
            name, out.address, out.base, out.base, out.size);
        return out;
}
Address Memory::pattern_scan(Address start, size_t size, const char *pattern)
{
        uint8_t *bytes = new uint8_t[size];
        if (!Read<uint8_t>(start, *bytes, size))
        {
                Logger::warn("Read error, partial read likely (#fuckpaging), proceeding regardless..\n");
        }

        size_t psres = find_pattern(bytes, size, pattern);
        if (psres == -1)
        {
                Logger::warn("Failed to find pattern.\n");
                return -1;
        }

        Address result = (Address)(start + (Address)psres);
        Logger::debug("Pattern scan result: 0x%013llX\n", result);
        return result;
}
Address Memory::pattern_scan_module_section(const char *moduleName, const char *sectionName, const char *pattern)
{
        ModuleInfo mod = get_module(moduleName);
        SectionInfo section;
        if (proc.module_section_by_name(&mod, CSliceRef<uint8_t>(sectionName), &section) != 0)
        {
                Logger::warn("Failed to get module section.\n");
                return -1;
        }
        Logger::debug("%s section %s at %016llX with len = %d\n", moduleName, section.name,
                      section.base, section.size);

        return pattern_scan(section.base, section.size, pattern);
}
Address Memory::pattern_scan_module(const char *moduleName, const char *pattern)
{
        ModuleInfo mod = get_module(moduleName);
        return pattern_scan(mod.base, mod.size, pattern);
}

// Thanks xoner
// Credits: learn_more, stevemk14ebr

inline bool isMatch(const PBYTE addr, const PBYTE pat, const PBYTE msk)
{
        size_t n = 0;
        while (addr[n] == pat[n] || msk[n] == (BYTE)'?')
        {
                if (!msk[++n])
                {
                        return true;
                }
        }
        return false;
}
size_t find_pattern(const PBYTE rangeStart, size_t len, const char *pattern)
{
        size_t l = strlen(pattern);
        PBYTE patt_base = static_cast<PBYTE>(malloc(l >> 1));
        PBYTE msk_base = static_cast<PBYTE>(malloc(l >> 1));
        PBYTE pat = patt_base;
        PBYTE msk = msk_base;
        if (pat && msk)
        {
                l = 0;
                while (*pattern)
                {
                        if (*pattern == ' ')
                                pattern++;
                        if (!*pattern)
                                break;
                        if (*(PBYTE)pattern == (BYTE)'\?')
                        {
                                *pat++ = 0;
                                *msk++ = '?';
                                pattern +=
                                    ((*(PWORD)pattern == (WORD)'\?\?') ? 2 : 1);
                        }
                        else
                        {
                                *pat++ = getByte(pattern);
                                *msk++ = 'x';
                                pattern += 2;
                        }
                        l++;
                }
                *msk = 0;
                pat = patt_base;
                msk = msk_base;
                for (size_t n = 0; n < (len - l); ++n)
                {
                        if (isMatch(rangeStart + n, patt_base, msk_base))
                        {
                                free(patt_base);
                                free(msk_base);
                                return n;
                        }
                }
                free(patt_base);
                free(msk_base);
        }
        return -1;
}
Address Memory::relative_to_absolute(Address address, int offset_to_delta)
{
        if (!address)
                return 0;
        int delta;
        Read<int>(address + offset_to_delta, delta);
        return (Address)(address + offset_to_delta + 4 + delta);
}