#include <cstdio>
#include <cstring>

extern "C" int destinia_probe_elf(
    const char *path, char *out, size_t capacity);

int main(int argc, char **argv) {
    if (argc != 2) {
        std::fprintf(stderr, "Uso: probe_host_test <libdestinia_jni.so>\n");
        return 2;
    }

    char output[2048] = {};
    const int ok = destinia_probe_elf(argv[1], output, sizeof(output));
    std::printf("=== TESTE AUTOMATICO DO CARREGADOR ARM32 ===\n%s\n", output);

    const char *required[] = {
        "ELF CARREGADO",
        "Imagem: 568 KB",
        "Simbolos: 747",
        "Relocacoes: 213",
        "Relocacoes aplicadas: 213",
        "Imports externos vinculados: 26",
        "Tipos nao suportados: 0",
        "Imports distintos: 26",
        "Memoria Unicorn: OK",
        "Testes ARM32: OK",
        "Codigo original: OK",
        "InitGame: OK",
        "JniRun: OK",
        "Pixels nao zero:",
    };

    if (ok != 1) {
        std::fprintf(stderr, "FALHOU: carregador retornou erro.\n");
        return 1;
    }

    for (const char *expected : required) {
        if (!std::strstr(output, expected)) {
            std::fprintf(stderr, "FALHOU: resultado ausente: %s\n", expected);
            return 1;
        }
    }

    std::puts("PASSOU: ELF, initGame, jniRun e primeiro framebuffer RGB565.");
    return 0;
}
