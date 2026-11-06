#include "arm_import_runtime.h"
#include <algorithm>
#include <chrono>
#include <cctype>
#include <climits>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <vector>

namespace destinia {
namespace {
constexpr uint32_t kHeapBase = 0x30000000;
constexpr uint32_t kHeapSize = 0x04000000;
constexpr uint32_t kTestBase = 0x33ff0000;
constexpr uint32_t kReturn = 0x20002000;
constexpr uint32_t kMaxTransfer = 0x100000;
const char *const kImplemented[] = {
    "__aeabi_idiv", "__aeabi_idivmod", "__aeabi_ldivmod",
    "__aeabi_lmul", "__aeabi_uidiv", "__aeabi_uidivmod",
    "memcpy", "memset", "strlen", "strcpy", "strncmp",
    "strstr", "malloc", "free", "clock",
    "fopen", "fclose", "fread", "fwrite", "fseek", "ftell", "remove",
    "sprintf"
};
}

ArmImportRuntime::ArmImportRuntime(
    uc_engine *engine, const std::map<std::string, uint32_t> &addresses)
    : uc_(engine), names_(addresses) {
    for (const auto &entry : names_) reverse_[entry.second] = entry.first;
    for (const auto *name : kImplemented) {
        if (names_.count(name)) ++implemented_;
    }
}

ArmImportRuntime::~ArmImportRuntime() {
    for (const auto &entry : files_) std::fclose(entry.second);
    if (hook_) uc_hook_del(uc_, hook_);
    if (jniHook_) uc_hook_del(uc_, jniHook_);
    if (profileHook_) uc_hook_del(uc_, profileHook_);
    if (fastFillHook_) uc_hook_del(uc_, fastFillHook_);
    if (fastSpriteHook_) uc_hook_del(uc_, fastSpriteHook_);
}

unsigned ArmImportRuntime::implementedCount() const { return implemented_; }

const std::map<uint32_t,uint64_t> &ArmImportRuntime::profiledBlocks() const {
    return profileBlocks_;
}
std::pair<uint64_t,uint64_t> ArmImportRuntime::directAccessCounts() const {
    return {directFillRows_,directSpriteColumns_};
}

void ArmImportRuntime::onProfileBlock(
    uc_engine *,uint64_t address,uint32_t,void *user) {
    ++static_cast<ArmImportRuntime *>(user)->profileBlocks_[
        static_cast<uint32_t>(address)&~1u];
}

// The original Thumb pixel loop at +0xac70 writes one RGB565 pixel
// per instruction iteration. Replace only that loop (not its clipping,
// blend mode, bounds checks or caller) with one emulated-memory write.
// The patch lives in Unicorn RAM; the original .so on disk is unchanged.
bool ArmImportRuntime::enableNativeFillRect(
    uint32_t imageBase, std::string &error) {
    const uint32_t loop=imageBase+0x0000ac70;
    uint16_t original=0;
    if (!readBytes(loop,&original,sizeof(original)) ||
        original!=0x801d) {
        error="RGB565 hot loop signature mismatch";
        return false;
    }
    const uint16_t skipPixelLoop=0xe002; // Thumb: b +4 (to loop+8).
    if (!writeBytes(loop,&skipPixelLoop,sizeof(skipPixelLoop))) {
        error="Nao foi possivel ativar fast fill ARM32";
        return false;
    }
    const uc_err e=uc_hook_add(
        uc_,&fastFillHook_,UC_HOOK_CODE,
        reinterpret_cast<void *>(onFastFillRow),this,loop,loop);
    if (e!=UC_ERR_OK) {
        writeBytes(loop,&original,sizeof(original));
        error=std::string("Hook de preenchimento: ")+uc_strerror(e);
        return false;
    }
    return true;
}

void ArmImportRuntime::onFastFillRow(
    uc_engine *, uint64_t, uint32_t, void *opaque) {
    auto *runtime=static_cast<ArmImportRuntime *>(opaque);
    uint32_t begin=0,end=0,color=0;
    if (!runtime->readReg(UC_ARM_REG_R3,begin) ||
        !runtime->readReg(UC_ARM_REG_R2,end) ||
        !runtime->readReg(UC_ARM_REG_R5,color)) {
        runtime->fail("RGB565 fill: falha nos registradores"); return;
    }
    if (end<=begin || (end-begin)>400u*240u*2u ||
        ((end-begin)&1u)) {
        runtime->fail("RGB565 fill: intervalo ARM32 invalido"); return;
    }
    const bool direct=runtime->hasDirectHeap_ &&
        begin>=kHeapBase &&
        static_cast<uint64_t>(end)<=
            static_cast<uint64_t>(kHeapBase)+kHeapSize;
    if (direct) {
        uint8_t *dst=runtime->directHeap_.data()+begin-kHeapBase;
        const uint8_t lo=static_cast<uint8_t>(color);
        const uint8_t hi=static_cast<uint8_t>(color>>8);
        for (size_t i=0;i<end-begin;i+=2) {
            dst[i]=lo;
            dst[i+1]=hi;
        }
        ++runtime->directFillRows_;
    } else {
        std::vector<uint16_t> pixels(
            (end-begin)/2,static_cast<uint16_t>(color));
        if (!runtime->writeBytes(begin,pixels.data(),end-begin))
            runtime->fail("RGB565 fill: destino ARM32 invalido");
    }
}

// Optimized version of the Thumb image/palette column at Grp_drawClipWImageP
// +0x206. The surrounding sprite clipping and rendering modes remain ARM32.
bool ArmImportRuntime::enableNativeSpriteColumn(
    uint32_t imageBase, std::string &error) {
    const uint32_t loop=imageBase+0x0000bd96;
    uint16_t original=0;
    if (!readBytes(loop,&original,sizeof(original)) || original!=0x5cf2) {
        error="Sprite ARM32 hot loop signature mismatch";
        return false;
    }
    const uint16_t skipColumn=0xe00c; // Thumb b to +0x1c, 0xbdb2.
    if (!writeBytes(loop,&skipColumn,sizeof(skipColumn))) {
        error="Nao foi possivel ativar native sprite";
        return false;
    }
    const uc_err e=uc_hook_add(
        uc_,&fastSpriteHook_,UC_HOOK_CODE,
        reinterpret_cast<void *>(onFastSpriteColumn),this,loop,loop);
    if (e!=UC_ERR_OK) {
        writeBytes(loop,&original,sizeof(original));
        error=std::string("Sprite hook: ")+uc_strerror(e);
        return false;
    }
    return true;
}

void ArmImportRuntime::onFastSpriteColumn(
    uc_engine *,uint64_t,uint32_t,void *opaque) {
    auto *runtime=static_cast<ArmImportRuntime *>(opaque);
    uint32_t src=0,dst=0,palette=0,grp=0,stack=0,col=0;
    if (!runtime->readReg(UC_ARM_REG_R6,src) ||
        !runtime->readReg(UC_ARM_REG_R5,dst) ||
        !runtime->readReg(UC_ARM_REG_R7,palette) ||
        !runtime->readReg(UC_ARM_REG_R8,grp) ||
        !runtime->readReg(UC_ARM_REG_SP,stack) ||
        !runtime->readReg(UC_ARM_REG_R3,col)) {
        runtime->fail("Sprite ARM32: registradores invalidos"); return;
    }
    uint32_t countRaw=0,stepRaw=0,key=0;
    if (!runtime->readBytes(stack+0x78,&countRaw,4) ||
        !runtime->readBytes(stack+0x24,&stepRaw,4) ||
        !runtime->readBytes(grp+0x50,&key,4)) {
        runtime->fail("Sprite ARM32: metadados invalidos"); return;
    }
    const int32_t count=static_cast<int32_t>(countRaw);
    const int32_t step=static_cast<int32_t>(stepRaw);
    if (col!=0 || count<1 || count>1024 || !step) {
        runtime->fail("Sprite ARM32: coluna fora dos limites"); return;
    }
    const int64_t last=static_cast<int64_t>(dst)+
        static_cast<int64_t>(count-1)*step;
    const int64_t first=std::min<int64_t>(dst,last);
    const int64_t after=std::max<int64_t>(dst,last)+2;
    if (first<1 || after>UINT32_MAX ||
        after-first>1024*1024) {
        runtime->fail("Sprite ARM32: framebuffer fora dos limites"); return;
    }
    std::vector<uint8_t> indices(static_cast<size_t>(count));
    std::vector<uint16_t> colors(256);
    if (!runtime->readBytes(src,indices.data(),indices.size()) ||
        !runtime->readBytes(palette,colors.data(),colors.size()*2)) {
        runtime->fail("Sprite ARM32: leitura de indices/paleta invalida"); return;
    }
    const bool direct=runtime->hasDirectHeap_ &&
        first>=kHeapBase &&
        after<=static_cast<int64_t>(kHeapBase)+kHeapSize;
    if (direct) {
        uint8_t *heap=runtime->directHeap_.data();
        const int64_t base=static_cast<int64_t>(dst)-kHeapBase;
        for (int32_t i=0;i<count;++i) {
            const uint16_t color=colors[indices[static_cast<size_t>(i)]];
            if (color==key) continue;
            const size_t offset=static_cast<size_t>(
                base+static_cast<int64_t>(i)*step);
            heap[offset]=static_cast<uint8_t>(color);
            heap[offset+1]=static_cast<uint8_t>(color>>8);
        }
        ++runtime->directSpriteColumns_;
    } else {
        std::vector<uint8_t> pixels(static_cast<size_t>(after-first));
        if (!runtime->readBytes(
                static_cast<uint32_t>(first),pixels.data(),pixels.size())) {
            runtime->fail("Sprite ARM32: leitura de pixels invalida"); return;
        }
        for (int32_t i=0;i<count;++i) {
            const uint16_t color=colors[indices[static_cast<size_t>(i)]];
            if (color==key) continue;
            const auto position=static_cast<size_t>(
                static_cast<int64_t>(dst)+static_cast<int64_t>(i)*step-first);
            pixels[position]=static_cast<uint8_t>(color);
            pixels[position+1]=static_cast<uint8_t>(color>>8);
        }
        if (!runtime->writeBytes(
                static_cast<uint32_t>(first),pixels.data(),pixels.size())) {
            runtime->fail("Sprite ARM32: escrita de pixels invalida"); return;
        }
    }
    const uint32_t nextDst=static_cast<uint32_t>(
        static_cast<int64_t>(dst)+static_cast<int64_t>(count)*step);
    if (!runtime->writeReg(UC_ARM_REG_R5,nextDst) ||
        !runtime->writeReg(UC_ARM_REG_R3,static_cast<uint32_t>(count))) {
        runtime->fail("Sprite ARM32: falha ao atualizar coluna");
    }
}

bool ArmImportRuntime::readReg(int reg, uint32_t &value) {
    return uc_reg_read(uc_, reg, &value) == UC_ERR_OK;
}

bool ArmImportRuntime::writeReg(int reg, uint32_t value) {
    return uc_reg_write(uc_, reg, &value) == UC_ERR_OK;
}

bool ArmImportRuntime::readBytes(uint32_t address, void *data, size_t bytes) {
    return uc_mem_read(uc_, address, data, bytes) == UC_ERR_OK;
}

bool ArmImportRuntime::writeBytes(
    uint32_t address, const void *data, size_t bytes) {
    return uc_mem_write(uc_, address, data, bytes) == UC_ERR_OK;
}

bool ArmImportRuntime::readCString(
    uint32_t address, std::string &value) {
    value.clear();
    if (!address) return false;
    for (size_t i = 0; i < 4096; ++i) {
        char ch = 0;
        if (!readBytes(address + static_cast<uint32_t>(i), &ch, 1)) return false;
        if (!ch) return true;
        value.push_back(ch);
    }
    return false;
}

bool ArmImportRuntime::writeCString(
    uint32_t address, const std::string &value) {
    return value.size() < 4096 &&
        writeBytes(address, value.c_str(), value.size() + 1);
}

bool ArmImportRuntime::resolvePath(
    const std::string &guest, std::string &path) {
    static const std::string prefix =
        "/data/data/game.destiniaeng/files/";
    std::string relative=guest;
    if (relative.compare(0,prefix.size(),prefix)==0)
        relative=relative.substr(prefix.size());
    else if (!relative.empty() && relative[0]=='/')
        return false;

    if (relative.empty() || relative.find('\\')!=std::string::npos)
        return false;

    const std::filesystem::path sub(relative);
    if (sub.is_absolute()) return false;
    for (const auto &piece:sub) {
        if (piece=="." || piece==".." || piece.empty()) return false;
    }
    const char *home=std::getenv("HOME");
    if (!home || !*home) return false;
    const auto sandbox=std::filesystem::path(home)/"Documents"/"Destinia";
    std::error_code ec;
    std::filesystem::create_directories(sandbox,ec);
    if (ec) return false;
    path=(sandbox/sub).string();
    return true;
}

void ArmImportRuntime::fail(const std::string &reason) {
    if (lastError_.empty()) lastError_ = reason;
    uc_emu_stop(uc_);
}

bool ArmImportRuntime::install(std::string &error) {
#if defined(DESTINIA_NATIVE_HEAP)
    const bool useDirectHeap=true;
#else
    const bool useDirectHeap=std::getenv("DESTINIA_DIRECT_HEAP")!=nullptr;
#endif
    uc_err mapped=UC_ERR_OK;
    if (useDirectHeap) {
        try {
            directHeap_.resize(kHeapSize,0);
        } catch (const std::bad_alloc &) {
            error="Sem memoria para mapear heap ARM32 diretamente";
            return false;
        }
        mapped=uc_mem_map_ptr(
            uc_,kHeapBase,kHeapSize,UC_PROT_ALL,directHeap_.data());
        if (mapped==UC_ERR_OK) hasDirectHeap_=true;
    } else {
        mapped=uc_mem_map(uc_,kHeapBase,kHeapSize,UC_PROT_ALL);
    }
    if (mapped != UC_ERR_OK) {
        error = std::string("Heap ARM32: ") + uc_strerror(mapped);
        return false;
    }
    const uc_err hooked = uc_hook_add(
        uc_, &hook_, UC_HOOK_CODE,
        reinterpret_cast<void *>(onCode), this,
        0x20000000, 0x20000fff);
    if (hooked != UC_ERR_OK) {
        error = std::string("Hook ARM32: ") + uc_strerror(hooked);
        return false;
    }
    return true;
}

void ArmImportRuntime::onCode(
    uc_engine *, uint64_t address, uint32_t, void *user) {
    static_cast<ArmImportRuntime *>(user)->dispatch(
        static_cast<uint32_t>(address));
}

void ArmImportRuntime::dispatch(uint32_t address) {
    const auto it = reverse_.find(address);
    if (it == reverse_.end()) {
        fail("Endereco de stub desconhecido");
        return;
    }
    const std::string &name = it->second;
    uint32_t a=0,b=0,c=0,d=0;
    if (!readReg(UC_ARM_REG_R0,a) || !readReg(UC_ARM_REG_R1,b) ||
        !readReg(UC_ARM_REG_R2,c) || !readReg(UC_ARM_REG_R3,d)) {
        fail("Falha ao ler registradores ARM32");
        return;
    }
    uint32_t result = 0;
    bool setR0 = true;
    auto bytes = [this](uint32_t address, uint32_t size,
                        std::vector<uint8_t> &out) {
        if (size > kMaxTransfer) return false;
        out.resize(size);
        return !size || readBytes(address, out.data(), size);
    };
    auto divSigned = [this](uint32_t dividend, uint32_t divisor,
                            uint32_t &quotient, uint32_t &remainder) {
        const int32_t x = static_cast<int32_t>(dividend);
        const int32_t y = static_cast<int32_t>(divisor);
        if (!y) { fail("Divisao ARM32 por zero"); return false; }
        const int64_t q = static_cast<int64_t>(x) / y;
        const int64_t r = static_cast<int64_t>(x) % y;
        quotient = static_cast<uint32_t>(q);
        remainder = static_cast<uint32_t>(r);
        return true;
    };

    if (name == "__aeabi_idiv" || name == "__aeabi_idivmod") {
        uint32_t rem=0;
        if (!divSigned(a,b,result,rem)) return;
        if (name == "__aeabi_idivmod" && !writeReg(UC_ARM_REG_R1,rem)) {
            fail("R1 indisponivel"); return;
        }
    } else if (name == "__aeabi_uidiv" ||
               name == "__aeabi_uidivmod") {
        if (!b) { fail("Divisao ARM32 por zero"); return; }
        result = a / b;
        if (name == "__aeabi_uidivmod" &&
            !writeReg(UC_ARM_REG_R1,a%b)) {
            fail("R1 indisponivel"); return;
        }
    } else if (name == "__aeabi_lmul") {
        const uint64_t x=(uint64_t(b)<<32)|a;
        const uint64_t y=(uint64_t(d)<<32)|c;
        const uint64_t product=x*y;
        result=static_cast<uint32_t>(product);
        if (!writeReg(UC_ARM_REG_R1,static_cast<uint32_t>(product>>32))) {
            fail("R1 indisponivel"); return;
        }
    } else if (name == "__aeabi_ldivmod") {
        const int64_t x=static_cast<int64_t>((uint64_t(b)<<32)|a);
        const int64_t y=static_cast<int64_t>((uint64_t(d)<<32)|c);
        if (!y) { fail("Divisao longa ARM32 por zero"); return; }
        const __int128 quotient=static_cast<__int128>(x)/y;
        const __int128 remainder=static_cast<__int128>(x)%y;
        const uint64_t q=static_cast<uint64_t>(quotient);
        const uint64_t r=static_cast<uint64_t>(remainder);
        result=static_cast<uint32_t>(q);
        if (!writeReg(UC_ARM_REG_R1,static_cast<uint32_t>(q>>32)) ||
            !writeReg(UC_ARM_REG_R2,static_cast<uint32_t>(r)) ||
            !writeReg(UC_ARM_REG_R3,static_cast<uint32_t>(r>>32))) {
            fail("Registradores de divisao longa indisponiveis"); return;
        }
    } else if (name == "memcpy" || name == "memset") {
        if (c > kMaxTransfer) { fail("Copia de memoria excede limite"); return; }
        std::vector<uint8_t> buffer;
        if (name == "memcpy") {
            if (!bytes(b,c,buffer)) { fail("Origem memcpy invalida"); return; }
        } else {
            buffer.assign(c,static_cast<uint8_t>(b));
        }
        if (c && !writeBytes(a,buffer.data(),c)) {
            fail("Destino de memoria invalido"); return;
        }
        result=a;
    } else if (name == "strlen" || name == "strcpy" ||
               name == "strncmp" || name == "strstr") {
        std::string left,right;
        if (!readCString(a,left)) { fail(name+": string ARM invalida"); return; }
        if (name == "strlen") {
            result=static_cast<uint32_t>(left.size());
        } else if (name == "strcpy") {
            if (!readCString(b,right) || !writeCString(a,right)) {
                fail("strcpy invalido"); return;
            }
            result=a;
        } else {
            if (!readCString(b,right)) { fail(name+": string ARM invalida"); return; }
            if (name == "strncmp") {
                const int cmp=std::strncmp(left.c_str(),right.c_str(),c);
                result=static_cast<uint32_t>(cmp);
            } else {
                const size_t pos=left.find(right);
                result=pos==std::string::npos ? 0 :
                    a+static_cast<uint32_t>(pos);
            }
        }
    } else if (name == "sprintf") {
        std::string format, output;
        if (!readCString(b,format)) {
            fail("sprintf: formato ARM invalido"); return;
        }
        uint32_t argument=2;
        auto nextArgument = [&](uint32_t &value) {
            if (argument==2) value=c;
            else if (argument==3) value=d;
            else {
                uint32_t sp=0;
                if (!readReg(UC_ARM_REG_SP,sp) ||
                    !readBytes(sp+4*(argument-4),&value,4))
                    return false;
            }
            ++argument;
            return true;
        };
        for (size_t i=0;i<format.size();++i) {
            if (format[i]!='%') { output.push_back(format[i]); continue; }
            ++i;
            if (i==format.size()) {
                fail("sprintf: formato incompleto"); return;
            }
            if (format[i]=='%') { output.push_back('%'); continue; }
            std::string flags;
            while (i<format.size() &&
                   std::strchr("-+ #0",format[i])) flags.push_back(format[i++]);
            int width=-1, precision=-1;
            if (i<format.size() && format[i]=='*') {
                uint32_t v=0;
                if (!nextArgument(v)) {
                    fail("sprintf: argumento width invalido"); return;
                }
                width=static_cast<int32_t>(v);
                ++i;
                if (width<0) {
                    flags.push_back('-');
                    if (width==INT_MIN) { fail("sprintf: width invalido"); return; }
                    width=-width;
                }
            } else {
                while (i<format.size() &&
                       std::isdigit(static_cast<unsigned char>(format[i]))) {
                    if (width<0) width=0;
                    width=width*10+(format[i++]-'0');
                    if (width>1024) { fail("sprintf: width excessivo"); return; }
                }
            }
            if (i<format.size() && format[i]=='.') {
                ++i; precision=0;
                if (i<format.size() && format[i]=='*') {
                    uint32_t v=0;
                    if (!nextArgument(v)) {
                        fail("sprintf: argumento precision invalido"); return;
                    }
                    precision=static_cast<int32_t>(v);
                    ++i;
                } else {
                    while (i<format.size() &&
                           std::isdigit(static_cast<unsigned char>(format[i]))) {
                        precision=precision*10+(format[i++]-'0');
                        if (precision>1024) {
                            fail("sprintf: precision excessiva"); return;
                        }
                    }
                }
                if (precision<0) precision=-1;
            }
            while (i<format.size() &&
                   std::strchr("hlLzjt",format[i])) ++i;
            if (i==format.size() || width>1024 || precision>1024) {
                fail("sprintf: especificador invalido"); return;
            }
            const char conv=format[i];
            std::string spec="%"+flags;
            if (width>=0) spec+=std::to_string(width);
            if (precision>=0) spec+="."+std::to_string(precision);
            char buffer[2048]={};
            int written=0;
            uint32_t val=0;
            if (!nextArgument(val)) {
                fail("sprintf: argumento invalido"); return;
            }
            if (conv=='d' || conv=='i') {
                written=std::snprintf(buffer,sizeof(buffer),
                    (spec+"d").c_str(),static_cast<int32_t>(val));
            } else if (std::strchr("uxXo",conv)) {
                written=std::snprintf(buffer,sizeof(buffer),
                    (spec+conv).c_str(),val);
            } else if (conv=='c') {
                written=std::snprintf(buffer,sizeof(buffer),
                    (spec+"c").c_str(),static_cast<int>(val));
            } else if (conv=='s') {
                std::string guest;
                if (val && !readCString(val,guest)) {
                    fail("sprintf: parametro string invalido"); return;
                }
                written=std::snprintf(buffer,sizeof(buffer),
                    (spec+"s").c_str(),val ? guest.c_str() : "(null)");
            } else if (conv=='p') {
                written=std::snprintf(buffer,sizeof(buffer),
                    "0x%x",val);
            } else {
                fail(std::string("sprintf: conversao nao suportada %")+conv);
                return;
            }
            if (written<0 || static_cast<size_t>(written)>=sizeof(buffer)) {
                fail("sprintf: formatacao excedeu limite"); return;
            }
            output.append(buffer,static_cast<size_t>(written));
            if (output.size()>4095) {
                fail("sprintf: texto excede 4095 bytes"); return;
            }
        }
        if (!writeCString(a,output)) {
            fail("sprintf: destino ARM invalido"); return;
        }
        result=static_cast<uint32_t>(output.size());
    } else if (name == "malloc") {
        const uint32_t size=(std::max(a,1u)+15u)&~15u;
        if (size>kHeapSize || heapNext_>kHeapBase+kHeapSize-size) {
            result=0;
        } else {
            result=heapNext_;
            heapNext_+=size;
        }
    } else if (name == "free") {
        setR0=false; // free nao retorna valor; alocador linear temporario
    } else if (name == "clock") {
        static const auto start=std::chrono::steady_clock::now();
        const auto micros=std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now()-start).count();
        result=static_cast<uint32_t>(micros);
    } else if (name == "fopen") {
        std::string guest, mode, path;
        if (!readCString(a,guest) || !readCString(b,mode)) {
            fail("fopen: argumentos invalidos"); return;
        }
        static const char *const allowed[] = {
            "rb", "wb", "ab", "r+b", "w+b", "a+b",
            "r", "w", "a", "rb+", "wb+", "ab+"
        };
        bool safeMode=false;
        for (const auto *candidate : allowed)
            if (mode==candidate) safeMode=true;
        if (!safeMode || !resolvePath(guest,path)) {
            result=0;
        } else {
            FILE *file=std::fopen(path.c_str(),mode.c_str());
            if (!file) result=0;
            else {
                result=nextFile_;
                nextFile_+=16;
                files_[result]=file;
            }
        }
    } else if (name == "fclose") {
        auto it=files_.find(a);
        result=static_cast<uint32_t>(-1);
        if (it!=files_.end()) {
            result=static_cast<uint32_t>(std::fclose(it->second));
            files_.erase(it);
        }
    } else if (name == "fread" || name == "fwrite") {
        auto it=files_.find(d);
        const uint64_t length=uint64_t(b)*c;
        if (length>kMaxTransfer) {
            fail(name+": transferencia excede limite"); return;
        }
        if (!b || !c || it==files_.end()) {
            result=0;
        } else {
            std::vector<uint8_t> buffer(static_cast<size_t>(length));
            if (name=="fwrite") {
                if (!readBytes(a,buffer.data(),buffer.size())) {
                    fail("fwrite: origem ARM invalida"); return;
                }
                result=static_cast<uint32_t>(
                    std::fwrite(buffer.data(),b,c,it->second));
            } else {
                result=static_cast<uint32_t>(
                    std::fread(buffer.data(),b,c,it->second));
                if (result && !writeBytes(a,buffer.data(),size_t(result)*b)) {
                    fail("fread: destino ARM invalido"); return;
                }
            }
        }
    } else if (name == "fseek") {
        auto it=files_.find(a);
        result=it==files_.end() ? static_cast<uint32_t>(-1) :
            static_cast<uint32_t>(std::fseek(
                it->second,static_cast<int32_t>(b),static_cast<int>(c)));
    } else if (name == "ftell") {
        auto it=files_.find(a);
        result=it==files_.end() ? static_cast<uint32_t>(-1) :
            static_cast<uint32_t>(std::ftell(it->second));
    } else if (name == "remove") {
        std::string guest,path;
        if (!readCString(a,guest)) {
            fail("remove: string ARM invalida"); return;
        }
        result=resolvePath(guest,path) ?
            static_cast<uint32_t>(std::remove(path.c_str())) :
            static_cast<uint32_t>(-1);
    } else if (name == "__android_log_print") {
        result=0; // logging nativo sera integrado depois
    } else if (name == "__stack_chk_fail") {
        fail("Stack canary ARM32 violado"); return;
    } else {
        fail("Import ARM32 ainda nao implementado: "+name);
        return;
    }

    if (setR0 && !writeReg(UC_ARM_REG_R0,result))
        fail("Falha ao gravar R0");
}

bool ArmImportRuntime::invoke(
    const char *name, uint32_t a0, uint32_t a1,
    uint32_t a2, uint32_t a3, uint32_t &result,
    std::string &error) {
    auto it=names_.find(name);
    if (it==names_.end()) { error=std::string("Import ausente: ")+name; return false; }
    lastError_.clear();
    if (!writeReg(UC_ARM_REG_R0,a0) || !writeReg(UC_ARM_REG_R1,a1) ||
        !writeReg(UC_ARM_REG_R2,a2) || !writeReg(UC_ARM_REG_R3,a3) ||
        !writeReg(UC_ARM_REG_LR,kReturn)) {
        error="Nao foi possivel inicializar registradores";
        return false;
    }
    const uc_err e=uc_emu_start(uc_,it->second,kReturn,0,8);
    if (!lastError_.empty()) { error=lastError_; return false; }
    if (e!=UC_ERR_OK) {
        error=std::string("Emulacao de ")+name+": "+uc_strerror(e);
        return false;
    }
    if (!readReg(UC_ARM_REG_R0,result)) {
        error="Nao foi possivel ler retorno ARM32";
        return false;
    }
    return true;
}

bool ArmImportRuntime::runOriginalExport(
    uint32_t address, uint32_t &result, std::string &error) {
    if (!address) {
        error="Endereco de exportacao ARM32 invalido";
        return false;
    }
    const uint32_t stackBase=0x7f000000;
    const uint32_t stackSize=0x20000;
    const uc_err mapped=uc_mem_map(
        uc_,stackBase,stackSize,UC_PROT_ALL);
    if (mapped!=UC_ERR_OK) {
        error=std::string("Pilha ARM32: ")+uc_strerror(mapped);
        return false;
    }
    lastError_.clear();
    if (!writeReg(UC_ARM_REG_R0,0) ||
        !writeReg(UC_ARM_REG_R1,0) ||
        !writeReg(UC_ARM_REG_R2,0) ||
        !writeReg(UC_ARM_REG_R3,0) ||
        !writeReg(UC_ARM_REG_SP,stackBase+stackSize-16) ||
        !writeReg(UC_ARM_REG_LR,kReturn)) {
        error="Nao inicializou os registradores do jogo";
        return false;
    }
    const uc_err e=uc_emu_start(uc_,address,kReturn,0,20000);
    if (!lastError_.empty()) { error=lastError_; return false; }
    if (e!=UC_ERR_OK) {
        error=std::string("Codigo Destinia: ")+uc_strerror(e);
        return false;
    }
    uint32_t pc=0;
    if (!readReg(UC_ARM_REG_PC,pc) || pc!=kReturn) {
        error="Funcao original nao retornou dentro do limite";
        return false;
    }
    if (!readReg(UC_ARM_REG_R0,result)) {
        error="Nao leu retorno do codigo original";
        return false;
    }
    return true;
}

bool ArmImportRuntime::readAssetName(
    uint32_t handle, std::string &name) {
    const auto it=arrays_.find(handle);
    if (it==arrays_.end()) return false;
    name.assign(it->second.begin(),it->second.end());
    if (name.empty() || name.find('\\')!=std::string::npos)
        return false;
    const std::filesystem::path sub(name);
    if (sub.is_absolute()) return false;
    for (const auto &part:sub) {
        if (part=="." || part==".." || part.empty()) return false;
    }
    return true;
}

bool ArmImportRuntime::installJni(std::string &error) {
    constexpr uint32_t jniBase=0x21000000;
    constexpr uint32_t vmBase=0x21001000;
    constexpr uint32_t envObject=0x40000000;
    constexpr uint32_t envTable=0x40000100;
    constexpr uint32_t vmObject=0x40001000;
    constexpr uint32_t vmTable=0x40001100;
    uc_err e=uc_mem_map(uc_,jniBase,0x2000,UC_PROT_ALL);
    if (e==UC_ERR_OK) e=uc_mem_map(
        uc_,envObject,0x10000,UC_PROT_ALL);
    if (e!=UC_ERR_OK) {
        error=std::string("Mapa JNI: ")+uc_strerror(e);
        return false;
    }
    const uint32_t bxlr=0xe12fff1e;
    std::vector<uint32_t> jniFunctions(235);
    for (unsigned i=0;i<jniFunctions.size();++i) {
        jniFunctions[i]=jniBase+4*i;
        if (!writeBytes(jniFunctions[i],&bxlr,sizeof(bxlr))) {
            error="Falha na tabela de stubs JNI"; return false;
        }
    }
    std::vector<uint32_t> vmFunctions(8);
    for (unsigned i=0;i<vmFunctions.size();++i) {
        vmFunctions[i]=vmBase+4*i;
        if (!writeBytes(vmFunctions[i],&bxlr,sizeof(bxlr))) {
            error="Falha na tabela JavaVM"; return false;
        }
    }
    if (!writeBytes(envObject,&envTable,4) ||
        !writeBytes(envTable,jniFunctions.data(),jniFunctions.size()*4) ||
        !writeBytes(vmObject,&vmTable,4) ||
        !writeBytes(vmTable,vmFunctions.data(),vmFunctions.size()*4)) {
        error="Falha nos ponteiros JNI/JavaVM"; return false;
    }
    e=uc_hook_add(uc_,&jniHook_,UC_HOOK_CODE,
                  reinterpret_cast<void *>(onJniCode),this,
                  jniBase,vmBase+0x100);
    if (e!=UC_ERR_OK) {
        error=std::string("Hook JNI: ")+uc_strerror(e);
        return false;
    }
    return true;
}

void ArmImportRuntime::onJniCode(
    uc_engine *,uint64_t address,uint32_t,void *user) {
    static_cast<ArmImportRuntime *>(user)->dispatchJni(
        static_cast<uint32_t>(address));
}

void ArmImportRuntime::dispatchJni(uint32_t address) {
    const uint32_t jniBase=0x21000000;
    const uint32_t vmBase=0x21001000;
    const bool vm=address>=vmBase;
    const uint32_t index=(address-(vm ? vmBase : jniBase))/4;
    uint32_t a=0,b=0,c=0,d=0;
    if (!readReg(UC_ARM_REG_R0,a) || !readReg(UC_ARM_REG_R1,b) ||
        !readReg(UC_ARM_REG_R2,c) || !readReg(UC_ARM_REG_R3,d)) {
        fail("JNI: registradores indisponiveis"); return;
    }
    uint32_t result=0;
    bool writeResult=true;

    if (vm) {
        if (index==6 || index==4) {
            const uint32_t envObject=0x40000000;
            if (!writeBytes(b,&envObject,4)) {
                fail("JavaVM: ponteiro GetEnv invalido"); return;
            }
        } else {
            fail("JavaVM sem implementacao: "+std::to_string(index));
            return;
        }
    } else if (index==6) { // FindClass
        std::string name;
        if (!readCString(b,name)) {
            fail("JNI FindClass: nome invalido"); return;
        }
        result=nextClass_++;
    } else if (index==21) { // NewGlobalRef
        result=b ? b : 1;
    } else if (index==113) { // GetStaticMethodID
        std::string method;
        if (!readCString(c,method)) {
            fail("JNI GetStaticMethodID: nome invalido"); return;
        }
        const std::map<std::string,uint32_t> known = {
            {"playVib",1},{"gotoURL",2},{"getAssetResSize",3},
            {"getAssetRes",4},{"playSound",5},{"setSoundVolume",6},
            {"close",7},{"buyCash",8},{"logEvent",9}
        };
        const auto found=known.find(method);
        result=found==known.end() ? 0 : found->second;
    } else if (index==171) { // GetArrayLength
        const auto it=arrays_.find(b);
        result=it==arrays_.end() ? 0 :
            static_cast<uint32_t>(it->second.size());
    } else if (index==176) { // NewByteArray
        if (b>kMaxTransfer) {
            fail("JNI NewByteArray excedeu memoria"); return;
        }
        result=nextArray_++;
        arrays_[result].assign(b,0);
    } else if (index==184) { // GetByteArrayElements
        const auto it=arrays_.find(b);
        if (it==arrays_.end()) result=0;
        else {
            const uint32_t size=static_cast<uint32_t>(it->second.size());
            const uint32_t aligned=(std::max(size,1u)+15u)&~15u;
            // JNI_RELEASE libera a copia ARM32 para reuso: anteriormente,
            // cada quadro alocava outra area e esgotava os 64 MiB.
            auto reusable=reusableJniBuffers_.lower_bound(aligned);
            bool recycled=reusable!=reusableJniBuffers_.end();
            uint32_t capacity=aligned;
            if (recycled) {
                capacity=reusable->first;
                result=reusable->second;
                reusableJniBuffers_.erase(reusable);
            } else {
                if (aligned>kHeapSize ||
                    heapNext_>kHeapBase+kHeapSize-aligned) {
                    fail("JNI sem heap para GetByteArrayElements"); return;
                }
                result=heapNext_;
                heapNext_+=aligned;
            }
            if (size && !writeBytes(result,it->second.data(),size)) {
                fail("JNI falhou ao copiar byteArray"); return;
            }
            if (c) {
                const uint32_t copied=1;
                if (!writeBytes(c,&copied,4)) {
                    fail("JNI isCopy invalido"); return;
                }
            }
            arrayPointers_[result]=b;
            arrayBufferCapacities_[result]=capacity;
        }
    } else if (index==192) { // ReleaseByteArrayElements
        const auto ptr=arrayPointers_.find(c);
        const auto it=arrays_.find(b);
        if (ptr==arrayPointers_.end() || ptr->second!=b ||
            it==arrays_.end() || (d!=0 && d!=1 && d!=2)) {
            fail("JNI ReleaseByteArrayElements invalido"); return;
        }
        // JNI_ABORT=2 descarta as alteracoes. JNI_COMMIT=1 copia,
        // mas o ponteiro continua valido ate o release final.
        if (d!=2 && !it->second.empty() &&
            !readBytes(c,it->second.data(),it->second.size())) {
            fail("JNI ReleaseByteArrayElements: copia invalida"); return;
        }
        if (d!=1) {
            const auto capacity=arrayBufferCapacities_.find(c);
            if (capacity==arrayBufferCapacities_.end()) {
                fail("JNI ReleaseByteArrayElements: capacidade ausente");
                return;
            }
            reusableJniBuffers_.emplace(capacity->second,c);
            arrayBufferCapacities_.erase(capacity);
            arrayPointers_.erase(ptr);
        }
        writeResult=false;
    } else if (index==200 || index==208) { // Get/SetByteArrayRegion
        const auto it=arrays_.find(b);
        uint32_t sp=0, guestBuffer=0;
        if (!readReg(UC_ARM_REG_SP,sp) ||
            !readBytes(sp,&guestBuffer,4)) {
            fail("JNI ByteArrayRegion: argumento na pilha invalido");
            return;
        }
        if (it==arrays_.end() || uint64_t(c)+d>it->second.size()) {
            fail("JNI ByteArrayRegion: limites invalidos"); return;
        }
        if (d) {
            uint8_t *data=it->second.data()+c;
            const bool copied=index==200 ?
                writeBytes(guestBuffer,data,d) :
                readBytes(guestBuffer,data,d);
            if (!copied) {
                fail("JNI ByteArrayRegion: ponteiro ARM invalido");
                return;
            }
        }
        writeResult=false;
    } else if (index==219) { // GetJavaVM
        const uint32_t vmObject=0x40001000;
        if (!writeBytes(b,&vmObject,4)) {
            fail("JNI GetJavaVM: endereco invalido"); return;
        }
    } else if (index==129 || index==114) { // Assets
        if (c!=3 && c!=4) {
            fail("JNI: metodo estatico nao suportado "+std::to_string(c));
            return;
        }
        std::string asset;
        if (!readAssetName(d,asset)) {
            fail("JNI: nome do asset invalido"); return;
        }
        const auto source=std::filesystem::path(assetsRoot_)/asset;
        if (index==129 && c==3) {
            std::error_code ec;
            const auto n=std::filesystem::file_size(source,ec);
            result=ec || n>UINT32_MAX ? 0 : static_cast<uint32_t>(n);
        } else if (index==114 && c==4) {
            std::ifstream stream(source,std::ios::binary|std::ios::ate);
            if (!stream) result=0;
            else {
                const auto length=stream.tellg();
                if (length<0 || length>static_cast<std::streamoff>(kMaxTransfer)) {
                    fail("Asset maior que 1 MiB"); return;
                }
                std::vector<uint8_t> data(static_cast<size_t>(length));
                stream.seekg(0);
                if (!data.empty() && !stream.read(
                    reinterpret_cast<char *>(data.data()),length)) {
                    fail("Falha na leitura de asset"); return;
                }
                result=nextArray_++;
                arrays_[result]=std::move(data);
            }
        } else {
            fail("JNI: assinatura de acesso a asset inesperada");
            return;
        }
    } else if (index==141) { // CallStaticVoidMethod
        // Som, vibracao, anuncios e compras serao tratados no frontend.
        writeResult=false;
    } else {
        fail("JNI ainda nao implementado: "+std::to_string(index));
        return;
    }
    if (writeResult && !writeReg(UC_ARM_REG_R0,result))
        fail("JNI falhou ao retornar R0");
}

bool ArmImportRuntime::selfTestJniBufferReuse(std::string &error) {
    constexpr uint32_t kJniGet=0x21000000+184u*4u;
    constexpr uint32_t kJniRelease=0x21000000+192u*4u;
    constexpr uint32_t kArrayBytes=400u*240u*2u;
    const uint32_t handle=nextArray_++;
    arrays_[handle].assign(kArrayBytes,0x5a);
    const uint32_t heapBefore=heapNext_;
    uint32_t firstPointer=0;
    auto call=[&](uint32_t address,uint32_t ptr,
                  uint32_t mode,uint32_t &result) {
        lastError_.clear();
        if (!writeReg(UC_ARM_REG_R0,0x40000000) ||
            !writeReg(UC_ARM_REG_R1,handle) ||
            !writeReg(UC_ARM_REG_R2,ptr) ||
            !writeReg(UC_ARM_REG_R3,mode) ||
            !writeReg(UC_ARM_REG_LR,kReturn)) {
            error="JNI stress: registradores invalidos";
            return false;
        }
        const uc_err status=uc_emu_start(uc_,address,kReturn,0,8);
        if (!lastError_.empty()) {
            error="JNI stress: "+lastError_;
            return false;
        }
        if (status!=UC_ERR_OK || !readReg(UC_ARM_REG_R0,result)) {
            error="JNI stress: execucao ARM32 falhou";
            return false;
        }
        return true;
    };
    for (uint32_t i=0; i<450; ++i) {
        uint32_t ptr=0,ignored=0;
        if (!call(kJniGet,0,0,ptr) || !ptr) {
            if (error.empty()) error="JNI stress: GetByteArrayElements nulo";
            return false;
        }
        if (i==0) firstPointer=ptr;
        else if (ptr!=firstPointer) {
            error="JNI stress: buffer nao foi reutilizado";
            return false;
        }
        if (i==0) {
            const uint8_t updated=0xa7;
            if (!writeBytes(ptr,&updated,1)) {
                error="JNI stress: nao foi possivel alterar buffer";
                return false;
            }
        }
        if (!call(kJniRelease,ptr,0,ignored)) return false;
        if (i==0 && arrays_[handle][0]!=0xa7) {
            error="JNI stress: release nao copiou para o array";
            return false;
        }
    }
    const uint32_t allocated=heapNext_-heapBefore;
    arrays_.erase(handle);
    if (allocated>kArrayBytes+16) {
        error="JNI stress: consumo crescente de heap ("+
            std::to_string(allocated)+")";
        return false;
    }
    return true;
}

bool ArmImportRuntime::runGameTouch(
    uint32_t address,uint32_t x,uint32_t y,uint32_t type,
    std::string &error) {
    if (!address || assetsRoot_.empty()) {
        error="jniTouch: jogo ainda nao inicializado";
        return false;
    }
    constexpr uint32_t stack=0x7f01ffc0;
    lastError_.clear();
    if (!writeBytes(stack,&type,sizeof(type)) ||
        !writeReg(UC_ARM_REG_R0,0x40000000) ||
        !writeReg(UC_ARM_REG_R1,0x10) ||
        !writeReg(UC_ARM_REG_R2,x) ||
        !writeReg(UC_ARM_REG_R3,y) ||
        !writeReg(UC_ARM_REG_SP,stack) ||
        !writeReg(UC_ARM_REG_LR,kReturn)) {
        error="jniTouch: nao configurou argumentos ARM32";
        return false;
    }
    const uc_err e=uc_emu_start(
        uc_,address,kReturn,5000000,200000000);
    if (!lastError_.empty()) {
        error="jniTouch: "+lastError_;
        return false;
    }
    if (e!=UC_ERR_OK) {
        error=std::string("jniTouch Unicorn: ")+uc_strerror(e);
        return false;
    }
    uint32_t pc=0;
    if (!readReg(UC_ARM_REG_PC,pc) || pc!=kReturn) {
        error="jniTouch nao retornou dentro do limite";
        return false;
    }
    return true;
}

bool ArmImportRuntime::runGameFrame(
    uint32_t address,uint32_t frameNumber,
    std::vector<uint8_t> &frameBuffer,uint32_t &nextMs,
    std::string &error) {
    constexpr size_t kFrameBytes=400*240*2;
    if (!address || assetsRoot_.empty()) {
        error="jniRun: jogo ainda nao inicializado"; return false;
    }
    if (!screenArray_) {
        screenArray_=nextArray_++;
        arrays_[screenArray_].assign(kFrameBytes,0);
    }
    lastError_.clear();
    if (!writeReg(UC_ARM_REG_R0,0x40000000) ||
        !writeReg(UC_ARM_REG_R1,0x10) ||
        !writeReg(UC_ARM_REG_R2,screenArray_) ||
        !writeReg(UC_ARM_REG_R3,frameNumber) ||
        !writeReg(UC_ARM_REG_SP,0x7f01ffc0) ||
        !writeReg(UC_ARM_REG_LR,kReturn)) {
        error="jniRun: nao inicializou registradores ARM32";
        return false;
    }
    const bool profileFrame = frameNumber==48 &&
        std::getenv("DESTINIA_PROFILE_BLOCKS")!=nullptr;
    if (profileFrame) {
        profileBlocks_.clear();
        const uc_err hooked=uc_hook_add(
            uc_,&profileHook_,UC_HOOK_CODE,
            reinterpret_cast<void *>(onProfileBlock),this,
            0x10000000,0x1008ffff);
        if (hooked!=UC_ERR_OK) {
            error=std::string("Profiler ARM32: ")+uc_strerror(hooked);
            return false;
        }
    }
    const uc_err e=uc_emu_start(
        uc_,address,kReturn,60000000,2000000000);
    if (profileHook_) {
        uc_hook_del(uc_,profileHook_);
        profileHook_=0;
    }
    if (!lastError_.empty()) {
        error="jniRun: "+lastError_; return false;
    }
    if (e!=UC_ERR_OK) {
        error=std::string("jniRun Unicorn: ")+uc_strerror(e);
        return false;
    }
    uint32_t pc=0;
    if (!readReg(UC_ARM_REG_PC,pc) || pc!=kReturn) {
        char diagnostic[160];
        std::snprintf(diagnostic,sizeof(diagnostic),
            "jniRun excedeu limite de 60s ou 2B instrucoes (PC=0x%08x)",pc);
        error=diagnostic;
        return false;
    }
    if (!readReg(UC_ARM_REG_R0,nextMs)) {
        error="jniRun: nao foi possivel ler intervalo do proximo quadro";
        return false;
    }
    frameBuffer=arrays_[screenArray_];
    if (frameBuffer.size()!=kFrameBytes) {
        error="jniRun: framebuffer RGB565 inesperado";
        return false;
    }
    return true;
}

bool ArmImportRuntime::runGameInit(
    uint32_t address,const std::string &assetsRoot,std::string &error) {
    assetsRoot_=assetsRoot;
    if (!std::filesystem::is_directory(assetsRoot_)) {
        error="Diretorio de assets indisponivel: "+assetsRoot_;
        return false;
    }
    if (!installJni(error)) return false;
    const std::string devId="DESTINIA-IOS-0001";
    const std::string model="iPhone";
    const uint32_t hId=nextArray_++;
    const uint32_t hModel=nextArray_++;
    arrays_[hId].assign(devId.begin(),devId.end());
    arrays_[hModel].assign(model.begin(),model.end());

    const uint32_t stackPtr=0x7f01ffc0;
    const uint32_t extraArgs[5] = {
        hModel,static_cast<uint32_t>(model.size()),400,240,0
    };
    if (!writeBytes(stackPtr,extraArgs,sizeof(extraArgs)) ||
        !writeReg(UC_ARM_REG_R0,0x40000000) ||
        !writeReg(UC_ARM_REG_R1,0x10) ||
        !writeReg(UC_ARM_REG_R2,hId) ||
        !writeReg(UC_ARM_REG_R3,static_cast<uint32_t>(devId.size())) ||
        !writeReg(UC_ARM_REG_SP,stackPtr) ||
        !writeReg(UC_ARM_REG_LR,kReturn)) {
        error="Falha ao configurar argumentos do initGame"; return false;
    }
    lastError_.clear();
    const uc_err e=uc_emu_start(uc_,address,kReturn,0,30000000);
    if (!lastError_.empty()) { error=lastError_; return false; }
    if (e!=UC_ERR_OK) {
        error=std::string("initGame ARM32: ")+uc_strerror(e);
        return false;
    }
    uint32_t pc=0;
    if (!readReg(UC_ARM_REG_PC,pc) || pc!=kReturn) {
        char diagnostic[160];
        std::snprintf(diagnostic,sizeof(diagnostic),
            "initGame excedeu 30000000 instrucoes (PC=0x%08x)",pc);
        error=diagnostic;
        return false;
    }
    return true;
}

bool ArmImportRuntime::selfTest(std::string &error) {
    const char hello[]="Destinia";
    const char needle[]="tin";
    if (!writeBytes(kTestBase,hello,sizeof(hello)) ||
        !writeBytes(kTestBase+64,needle,sizeof(needle))) {
        error="Nao foi possivel preparar strings do teste";
        return false;
    }
    uint32_t result=0, r1=0;
    auto call=[&](const char *name,uint32_t a,uint32_t b,
                  uint32_t c,uint32_t d,uint32_t expected) {
        if (!invoke(name,a,b,c,d,result,error)) return false;
        if (result != expected) {
            error=std::string("Retorno incorreto: ")+name;
            return false;
        }
        return true;
    };
    if (!call("strlen",kTestBase,0,0,0,8) ||
        !call("strstr",kTestBase,kTestBase+64,0,0,kTestBase+3) ||
        !call("__aeabi_uidiv",42,5,0,0,8) ||
        !call("__aeabi_idivmod",static_cast<uint32_t>(-43),5,0,0,
              static_cast<uint32_t>(-8))) return false;
    if (!readReg(UC_ARM_REG_R1,r1) ||
        r1!=static_cast<uint32_t>(-3)) {
        error="Resto idivmod incorreto"; return false;
    }
    if (!call("__aeabi_lmul",3,0,7,0,21) ||
        !call("memset",kTestBase+128,0x41,5,0,kTestBase+128) ||
        !call("memcpy",kTestBase+160,kTestBase,9,0,kTestBase+160) ||
        !call("strncmp",kTestBase,kTestBase+160,8,0,0) ||
        !call("malloc",32,0,0,0,kHeapBase)) return false;
    char copied[9]={};
    char filled[5]={};
    if (!readBytes(kTestBase+160,copied,9) ||
        std::strcmp(copied,hello)!=0 ||
        !readBytes(kTestBase+128,filled,5) ||
        std::memcmp(filled,"AAAAA",5)!=0) {
        error="Memoria apos memcpy/memset incorreta";
        return false;
    }
    if (!invoke("free",kHeapBase,0,0,0,result,error)) return false;

    // Teste completo de arquivo ARM32, isolado no Documents/Destinia.
    const char guestPath[]=
        "/data/data/game.destiniaeng/files/arm32-ci-smoke.bin";
    const char writeMode[]="wb";
    const char readMode[]="rb";
    const char fileData[]="ARM32-FILE";
    if (!writeBytes(kTestBase+256,guestPath,sizeof(guestPath)) ||
        !writeBytes(kTestBase+384,writeMode,sizeof(writeMode)) ||
        !writeBytes(kTestBase+400,readMode,sizeof(readMode)) ||
        !writeBytes(kTestBase+448,fileData,sizeof(fileData))) {
        error="Falha ao preparar teste de arquivos"; return false;
    }
    if (!invoke("fopen",kTestBase+256,kTestBase+384,0,0,result,error))
        return false;
    const uint32_t writer=result;
    if (!writer) { error="Nao abriu arquivo para escrita"; return false; }
    if (!call("fwrite",kTestBase+448,1,sizeof(fileData),writer,
              sizeof(fileData)) ||
        !call("fclose",writer,0,0,0,0))
        return false;
    if (!invoke("fopen",kTestBase+256,kTestBase+400,0,0,result,error))
        return false;
    const uint32_t reader=result;
    if (!reader) { error="Nao abriu arquivo para leitura"; return false; }
    if (!call("fseek",reader,0,0,0,0) ||
        !call("fread",kTestBase+512,1,sizeof(fileData),reader,
              sizeof(fileData)) ||
        !call("fclose",reader,0,0,0,0) ||
        !call("remove",kTestBase+256,0,0,0,0))
        return false;
    const char fmt[]="Score:%04d";
    if (!writeBytes(kTestBase+640,fmt,sizeof(fmt))) {
        error="Falha ao preparar sprintf ARM32"; return false;
    }
    if (!call("sprintf",kTestBase+704,kTestBase+640,42,0,10))
        return false;
    const char expected[]="Score:0042";
    char formatted[sizeof(expected)]={};
    if (!readBytes(kTestBase+704,formatted,sizeof(formatted)) ||
        std::memcmp(formatted,expected,sizeof(expected))!=0) {
        error="Formatacao sprintf ARM32 incorreta"; return false;
    }
    char saved[sizeof(fileData)]={};
    if (!readBytes(kTestBase+512,saved,sizeof(saved)) ||
        std::memcmp(saved,fileData,sizeof(saved))!=0) {
        error="Arquivo ARM32 retornou dados incorretos"; return false;
    }
    return true;
}
} // namespace destinia
