#pragma once
#include <unicorn/unicorn.h>
#include <cstdint>
#include <cstdio>
#include <map>
#include <string>

// Ponte minima entre a ABI ARM32 Android e a memoria isolada do Unicorn.
// Nenhum ponteiro ARM32 e convertido diretamente para ponteiro nativo.
namespace destinia {
class ArmImportRuntime {
public:
    explicit ArmImportRuntime(uc_engine *engine,
                             const std::map<std::string, uint32_t> &addresses);
    ~ArmImportRuntime();
    bool install(std::string &error);
    bool selfTest(std::string &error);
    unsigned implementedCount() const;
private:
    static void onCode(uc_engine *, uint64_t, uint32_t, void *);
    void dispatch(uint32_t address);
    bool readCString(uint32_t address, std::string &value);
    bool writeCString(uint32_t address, const std::string &value);
    bool resolvePath(const std::string &guest, std::string &path);
    bool readReg(int reg, uint32_t &value);
    bool writeReg(int reg, uint32_t value);
    bool readBytes(uint32_t address, void *data, size_t bytes);
    bool writeBytes(uint32_t address, const void *data, size_t bytes);
    void fail(const std::string &reason);
    bool invoke(const char *name, uint32_t a0, uint32_t a1,
                uint32_t a2, uint32_t a3, uint32_t &result,
                std::string &error);
    uc_engine *uc_;
    uc_hook hook_ = 0;
    std::map<std::string, uint32_t> names_;
    std::map<uint32_t, std::string> reverse_;
    uint32_t heapNext_ = 0x30000000;
    uint32_t nextFile_ = 0x5000;
    std::map<uint32_t, FILE *> files_;
    std::string lastError_;
    unsigned implemented_ = 0;
};
}
