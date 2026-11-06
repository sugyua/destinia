#include "arm_import_runtime.h"
#include <algorithm>
#include <chrono>
#include <climits>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <filesystem>
#include <vector>

namespace destinia {
namespace {
constexpr uint32_t kHeapBase = 0x30000000;
constexpr uint32_t kHeapSize = 0x100000;
constexpr uint32_t kTestBase = 0x300f0000;
constexpr uint32_t kReturn = 0x20002000;
constexpr uint32_t kMaxTransfer = 0x100000;
const char *const kImplemented[] = {
    "__aeabi_idiv", "__aeabi_idivmod", "__aeabi_ldivmod",
    "__aeabi_lmul", "__aeabi_uidiv", "__aeabi_uidivmod",
    "memcpy", "memset", "strlen", "strcpy", "strncmp",
    "strstr", "malloc", "free", "clock",
    "fopen", "fclose", "fread", "fwrite", "fseek", "ftell", "remove"
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
}

unsigned ArmImportRuntime::implementedCount() const { return implemented_; }

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
    const uc_err mapped =
        uc_mem_map(uc_, kHeapBase, kHeapSize, UC_PROT_ALL);
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
    char saved[sizeof(fileData)]={};
    if (!readBytes(kTestBase+512,saved,sizeof(saved)) ||
        std::memcmp(saved,fileData,sizeof(saved))!=0) {
        error="Arquivo ARM32 retornou dados incorretos"; return false;
    }
    return true;
}
} // namespace destinia
