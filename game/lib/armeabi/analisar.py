import sys
from capstone import *
from elftools.elf.elffile import ELFFile

def analyze_so(filename):
    try:
        with open(filename, 'rb') as f:
            elffile = ELFFile(f)
            arm_insns = set()
            thumb_insns = set()

            for section in elffile.iter_sections():
                if section['sh_flags'] & 0x4: # SHF_EXECINSTR (Seção executável)
                    data = section.data()
                    addr = section['sh_addr']
                    
                    # Tentar ARM
                    md_arm = Cs(CS_ARCH_ARM, CS_MODE_ARM)
                    for i in md_arm.disasm(data, addr):
                        arm_insns.add(i.mnemonic)
                    
                    # Tentar Thumb
                    md_thumb = Cs(CS_ARCH_ARM, CS_MODE_THUMB)
                    for i in md_thumb.disasm(data, addr):
                        thumb_insns.add(i.mnemonic)

            print(f"--- ARM Opcodes Unique ({len(arm_insns)}) ---")
            print(", ".join(sorted(arm_insns)))
            print(f"\n--- THUMB Opcodes Unique ({len(thumb_insns)}) ---")
            print(", ".join(sorted(thumb_insns)))

    except Exception as e:
        print(f"Erro: {e}")

if __name__ == '__main__':
    analyze_so('libdestinia_jni.so')