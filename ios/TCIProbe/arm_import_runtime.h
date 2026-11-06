#pragma once
#include <unicorn/unicorn.h>
#include <cstdint>
#include <cstdio>
#include <map>
#include <vector>
#include <vector>
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
    bool enableNativeFillRect(uint32_t imageBase, std::string &error);
    bool enableNativeSpriteColumn(uint32_t imageBase, std::string &error);
    bool selfTest(std::string &error);
    bool selfTestJniBufferReuse(std::string &error);
    bool runOriginalExport(uint32_t address, uint32_t &result,
                           std::string &error);
    bool runGameInit(uint32_t address, const std::string &assetsRoot,
                     std::string &error);
    bool runGameFrame(uint32_t address, uint32_t frameNumber,
                      std::vector<uint8_t> &frameBuffer, uint32_t &nextMs,
                      std::string &error);
    bool runGameTouch(uint32_t address, uint32_t x, uint32_t y,
                      uint32_t type, std::string &error);
    unsigned implementedCount() const;
    const std::map<uint32_t,uint64_t> &profiledBlocks() const;
private:
    static void onCode(uc_engine *, uint64_t, uint32_t, void *);
    static void onProfileBlock(uc_engine *, uint64_t, uint32_t, void *);
    static void onFastFillRow(uc_engine *, uint64_t, uint32_t, void *);
    static void onFastSpriteColumn(uc_engine *, uint64_t, uint32_t, void *);
    static void onJniCode(uc_engine *, uint64_t, uint32_t, void *);
    void dispatchJni(uint32_t address);
    bool installJni(std::string &error);
    bool readAssetName(uint32_t handle, std::string &name);
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
    uc_hook jniHook_ = 0;
    uc_hook profileHook_ = 0;
    uc_hook fastFillHook_ = 0;
    uc_hook fastSpriteHook_ = 0;
    std::map<uint32_t,uint64_t> profileBlocks_;
    std::map<std::string, uint32_t> names_;
    std::map<uint32_t, std::string> reverse_;
    std::vector<uint8_t> directHeap_;
    bool hasDirectHeap_ = false;
    uint64_t directSpriteColumns_ = 0;
    uint64_t directFillRows_ = 0;
    uint32_t heapNext_ = 0x30000000;
    uint32_t nextFile_ = 0x5000;
    uint32_t nextArray_ = 0x1000;
    uint32_t screenArray_ = 0;
    uint32_t nextClass_ = 0x100;
    std::map<uint32_t,std::vector<uint8_t>> arrays_;
    std::map<uint32_t,uint32_t> arrayPointers_;
    std::map<uint32_t,uint32_t> arrayBufferCapacities_;
    // Buffers JNI desalocados podem ser reutilizados por outras arrays.
    // A chave e a capacidade e o valor e o endereco ARM32.
    std::multimap<uint32_t,uint32_t> reusableJniBuffers_;
    std::string assetsRoot_;
    std::map<uint32_t, FILE *> files_;
    std::string lastError_;
    unsigned implemented_ = 0;
};
}
