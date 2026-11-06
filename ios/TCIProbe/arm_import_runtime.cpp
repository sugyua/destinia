#include "arm_import_runtime.h"
#include <algorithm>
#include <chrono>
#include <climits>
#include <cstdio>
#include <cstring>
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
    "strstr", "malloc", "free", "clock"
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
    return true;
}
} // namespace destinia
