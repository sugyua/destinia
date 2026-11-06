#include "../../common/elf_image.h"
#include <unicorn/unicorn.h>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

extern "C" int destinia_probe_elf(
    const char *path, char *out, size_t capacity) {

    if (!path || !out || capacity == 0) return 0;

    try {
        std::ifstream input(path, std::ios::binary | std::ios::ate);
        if (!input) throw std::runtime_error("Arquivo nao encontrado");

        const std::streamoff length = input.tellg();
        if (length < 52 ||
            length > static_cast<std::streamoff>(
                destinia::ArmElfImage::maxSize)) {
            throw std::runtime_error("Tamanho ELF invalido");
        }

        std::vector<uint8_t> file(static_cast<size_t>(length));
        input.seekg(0);
        if (!input.read(reinterpret_cast<char *>(file.data()),
                        static_cast<std::streamsize>(file.size()))) {
            throw std::runtime_error("Falha ao ler ELF");
        }

        destinia::ArmElfImage image;
        image.parse(file);

        const uint64_t base = 0x10000000;
        uc_engine *uc = nullptr;

        uc_err err = uc_open(UC_ARCH_ARM, UC_MODE_ARM, &uc);
        if (err != UC_ERR_OK) {
            throw std::runtime_error(uc_strerror(err));
        }

        err = uc_mem_map(uc, base, image.bytes.size(), UC_PROT_ALL);

        if (err == UC_ERR_OK) {
            err = uc_mem_write(
                uc, base, image.bytes.data(), image.bytes.size());
        }

        uint8_t verify[16] = {};

        if (err == UC_ERR_OK) {
            err = uc_mem_read(uc, base, verify, sizeof(verify));
        }

        uc_close(uc);

        if (err != UC_ERR_OK) {
            throw std::runtime_error(uc_strerror(err));
        }

        if (std::memcmp(verify, image.bytes.data(), sizeof(verify))) {
            throw std::runtime_error("Verificacao da memoria falhou");
        }

        std::snprintf(
            out, capacity,
            "ELF CARREGADO\n"
            "Destinia ARM32\n"
            "Imagem: %zu KB\n"
            "Simbolos: %zu\n"
            "Relocacoes: %zu\n"
            "Memoria Unicorn: OK",
            image.bytes.size() / 1024,
            image.symbols.size(),
            image.relocations.size()
        );

        return 1;

    } catch (const std::exception &e) {
        std::snprintf(out, capacity, "ELF FALHOU\n%s", e.what());
        return 0;
    }
}