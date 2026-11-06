#include <cstdio>
#include <cstring>
#include <vector>
#include <fstream>
#include <cstdint>

extern "C" int destinia_game_open(
    const char *path, char *out, size_t capacity,
    uint8_t *frameOut, size_t frameCapacity);
extern "C" int destinia_game_step(
    uint8_t *frameOut, size_t frameCapacity, uint32_t *nextMs,
    char *message, size_t capacity);
extern "C" int destinia_game_touch(
    uint32_t x, uint32_t y, uint32_t type,
    char *message, size_t capacity);
extern "C" void destinia_game_close(void);

int main(int argc, char **argv) {
    if (argc != 2) {
        std::fprintf(stderr, "Uso: probe_host_test <libdestinia_jni.so>\n");
        return 2;
    }

    char output[2048] = {};
    std::vector<uint8_t> frame(400u*240u*2u,0);
    const int ok = destinia_game_open(
        argv[1], output, sizeof(output), frame.data(), frame.size());
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
        "Quadros:",
        "Cores distintas:",
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

    const char *value=std::strstr(output,"Cores distintas: ");
    unsigned colors=0;
    if (!value || std::sscanf(value,"Cores distintas: %u",&colors)!=1) {
        std::fprintf(stderr,"FALHOU: nao foi possivel ler cores RGB565.\n");
        return 1;
    }
    std::printf("Cores RGB565 diferentes: %u\n",colors);
    std::ofstream ppm("/tmp/destinia-frame.ppm",std::ios::binary);
    if (!ppm) {
        std::fprintf(stderr,"FALHOU: nao foi possivel salvar imagem.\n");
        return 1;
    }
    ppm << "P6\n400 240\n255\n";
    for (size_t i=0;i<frame.size();i+=2) {
        const uint16_t rgb=uint16_t(frame[i]) |
            (uint16_t(frame[i+1]) << 8);
        const unsigned char pixel[3]={
            static_cast<unsigned char>(((rgb>>11)&31)*255/31),
            static_cast<unsigned char>(((rgb>>5)&63)*255/63),
            static_cast<unsigned char>((rgb&31)*255/31)
        };
        ppm.write(reinterpret_cast<const char *>(pixel),3);
    }
    ppm.close();
    if (!ppm) {
        std::fprintf(stderr,"FALHOU: imagem incompleta.\n");
        return 1;
    }
    char message[512]={};
    uint32_t waitMs=0;
    if (!destinia_game_step(
            frame.data(),frame.size(),&waitMs,message,sizeof(message))) {
        std::fprintf(stderr,"FALHOU: sessao persistente: %s\n",message);
        destinia_game_close();
        return 1;
    }
    std::printf("Quadro persistente: OK, espera=%u ms\n",waitMs);
    if (!destinia_game_touch(200,120,0,message,sizeof(message)) ||
        !destinia_game_touch(200,120,1,message,sizeof(message))) {
        std::fprintf(stderr,"FALHOU: toque no jogo: %s\n",message);
        destinia_game_close();
        return 1;
    }
    destinia_game_close();
    std::puts("PASSOU: initGame, jniRun, sessao persistente, toque e RGB565.");
    return 0;
}
