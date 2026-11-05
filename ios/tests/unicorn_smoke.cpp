#include <unicorn/unicorn.h>
#include <cstdint>
#include <cstdio>

int main() {
    uc_engine* uc = nullptr;

    uc_err err = uc_open(UC_ARCH_ARM, UC_MODE_ARM, &uc);
    if (err != UC_ERR_OK) {
        std::printf("FALHA: uc_open: %s\n", uc_strerror(err));
        return 1;
    }

    const uint64_t address = 0x10000;

    // ARM: mov r0, #42
    const uint8_t code[] = {0x2A, 0x00, 0xA0, 0xE3};

    err = uc_mem_map(uc, address, 0x1000, UC_PROT_ALL);

    if (err == UC_ERR_OK)
        err = uc_mem_write(uc, address, code, sizeof(code));

    if (err == UC_ERR_OK)
        err = uc_emu_start(
            uc,
            address,
            address + sizeof(code),
            1000000,
            0
        );

    uint32_t result = 0;

    if (err == UC_ERR_OK)
        err = uc_reg_read(uc, UC_ARM_REG_R0, &result);

    uc_close(uc);

    if (err != UC_ERR_OK) {
        std::printf("FALHA: %s\n", uc_strerror(err));
        return 2;
    }

    std::printf("ARM R0 = %u\n", result);

    if (result != 42) {
        std::puts("FALHA: resultado incorreto");
        return 3;
    }

    std::puts("SUCESSO: Unicorn ARM funcionando");
    return 0;
}
