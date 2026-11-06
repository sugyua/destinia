#include "../../common/elf_image.h"
#include "arm_import_runtime.h"
#include <unicorn/unicorn.h>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>
#include <set>
#include <map>

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

        size_t applied = 0;
        size_t pending = 0;
        size_t unsupported = 0;
        std::set<std::string> imports;

        const uint32_t stubBase = 0x20000000;
        const uint32_t guardAddress = 0x20001000;
        std::map<std::string, uint32_t> stubAddresses;

        for (const auto &r : image.relocations) {
            uint32_t value = 0;
            std::memcpy(&value, image.bytes.data() + r.offset, 4);

            if (r.type == 23) {
                value += static_cast<uint32_t>(base);
            } else if (r.type == 2 || r.type == 21 || r.type == 22) {
                const auto &sym = image.symbols[r.symbol];

                if (sym.shndx == 0) {
                    const std::string &name = image.names[r.symbol];
                    imports.insert(name);

                    uint32_t address = guardAddress;
                    if (name != "__stack_chk_guard") {
                        auto it = stubAddresses.find(name);
                        if (it == stubAddresses.end()) {
                            address = stubBase +
                                static_cast<uint32_t>(stubAddresses.size()) * 4;
                            stubAddresses[name] = address;
                        } else {
                            address = it->second;
                        }
                    }

                    value = (r.type == 2)
                        ? value + address : address;

                    ++pending;
                    std::memcpy(image.bytes.data() + r.offset, &value, 4);
                    ++applied;
                    continue;
                }

                const uint32_t address =
                    static_cast<uint32_t>(base) + sym.value;

                value = (r.type == 2) ? value + address : address;
            } else {
                ++unsupported;
                continue;
            }

            std::memcpy(image.bytes.data() + r.offset, &value, 4);
            ++applied;
        }

        uc_engine *uc = nullptr;

        uc_err err = uc_open(UC_ARCH_ARM, UC_MODE_ARM, &uc);
        if (err != UC_ERR_OK) {
            throw std::runtime_error(uc_strerror(err));
        }

        err = uc_mem_map(uc, base, image.bytes.size(), UC_PROT_ALL);

        if (err == UC_ERR_OK) {
            err = uc_mem_map(uc, stubBase, 0x2000, UC_PROT_ALL);
        }

        if (err == UC_ERR_OK) {
            std::vector<uint32_t> stubs(
                stubAddresses.size(), 0xE12FFF1E);

            err = uc_mem_write(
                uc, stubBase, stubs.data(),
                stubs.size() * sizeof(uint32_t));
        }

        if (err == UC_ERR_OK) {
            const uint32_t guard = 0xA5C3F00D;
            err = uc_mem_write(
                uc, guardAddress, &guard, sizeof(guard));
        }

        if (err == UC_ERR_OK) {
            err = uc_mem_write(
                uc, base, image.bytes.data(), image.bytes.size());
        }

        uint8_t verify[16] = {};

        if (err == UC_ERR_OK) {
            err = uc_mem_read(uc, base, verify, sizeof(verify));
        }

        if (err != UC_ERR_OK) {
            uc_close(uc);
            throw std::runtime_error(uc_strerror(err));
        }

        if (std::memcmp(verify, image.bytes.data(), sizeof(verify))) {
            uc_close(uc);
            throw std::runtime_error("Verificacao da memoria falhou");
        }

        size_t nonzeroPixels=0;
        uint32_t nextFrameMs=0;
        {
            destinia::ArmImportRuntime runtime(uc, stubAddresses);
            std::string runtimeError;
            if (!runtime.install(runtimeError) ||
                !runtime.selfTest(runtimeError)) {
                throw std::runtime_error(
                    "Falha no teste de imports ARM32: " + runtimeError);
            }

            uint32_t originalAddress=0;
            for (size_t i=0; i<image.symbols.size(); ++i) {
                if (image.symbols[i].shndx != 0 &&
                    image.names[i] == "getMyCash") {
                    originalAddress = static_cast<uint32_t>(base) +
                        image.symbols[i].value;
                    break;
                }
            }
            if (!originalAddress) {
                throw std::runtime_error(
                    "Exportacao original getMyCash nao encontrada");
            }
            uint32_t gameValue=0;
            if (!runtime.runOriginalExport(
                    originalAddress,gameValue,runtimeError)) {
                throw std::runtime_error(
                    "Falha ao executar getMyCash original: "+runtimeError);
            }

            uint32_t initAddress=0;
            for (size_t i=0; i<image.symbols.size(); ++i) {
                if (image.symbols[i].shndx != 0 &&
                    image.names[i] ==
                    "Java_game_destiniaeng_GameThread_initGame") {
                    initAddress=static_cast<uint32_t>(base) +
                        image.symbols[i].value;
                    break;
                }
            }
            if (!initAddress) {
                throw std::runtime_error("initGame original ausente");
            }
            const std::filesystem::path libraryPath(path);
            auto assets=libraryPath.parent_path()/"assets";
            if (!std::filesystem::is_directory(assets))
                assets=libraryPath.parent_path().parent_path()
                    .parent_path()/"assets";
            if (!runtime.runGameInit(
                    initAddress,assets.string(),runtimeError)) {
                throw std::runtime_error(
                    "Falha no initGame original: "+runtimeError);
            }

            uint32_t frameAddress=0;
            for (size_t i=0; i<image.symbols.size(); ++i) {
                if (image.symbols[i].shndx!=0 &&
                    image.names[i]==
                    "Java_game_destiniaeng_GameThread_jniRun") {
                    frameAddress=static_cast<uint32_t>(base) +
                        image.symbols[i].value;
                    break;
                }
            }
            if (!frameAddress)
                throw std::runtime_error("jniRun original ausente");
            std::vector<uint8_t> pixels;
            if (!runtime.runGameFrame(
                    frameAddress,0,pixels,nextFrameMs,runtimeError)) {
                throw std::runtime_error(
                    "Falha no jniRun original: "+runtimeError);
            }
            for (size_t at=0; at+1<pixels.size(); at+=2)
                if (pixels[at] || pixels[at+1]) ++nonzeroPixels;
        }

        uc_close(uc);

        std::snprintf(
            out, capacity,
            "ELF CARREGADO\n"
            "Destinia ARM32\n"
            "Imagem: %zu KB\n"
            "Simbolos: %zu\n"
            "Relocacoes: %zu\n"
            "Relocacoes aplicadas: %zu\n"
            "Imports externos vinculados: %zu\n"
            "Tipos nao suportados: %zu\n"
            "Imports distintos: %zu\n"
            "Memoria Unicorn: OK\n"
            "Testes ARM32: OK\n"
            "Codigo original: OK\n"
            "InitGame: OK\n"
            "JniRun: OK\n"
            "Pixels nao zero: %zu\n"
            "Proximo quadro: %u ms",
            image.bytes.size() / 1024,
            image.symbols.size(),
            image.relocations.size(),
            applied,
            pending,
            unsupported,
            imports.size(),
            nonzeroPixels,
            nextFrameMs
        );

        return 1;

    } catch (const std::exception &e) {
        std::snprintf(out, capacity, "ELF FALHOU\n%s", e.what());
        return 0;
    }
}