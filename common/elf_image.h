#pragma once
// Valida o ELF antes de resolver imports/aplicar relocacoes. Metadados sao copiados
// para fora da imagem: uma relocacao nao pode alterar o parser durante a iteracao.
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace destinia {
struct ArmElfImage {
  using u32=uint32_t;
  struct Header {
    uint8_t ident[16]; uint16_t type,machine;
    u32 version,entry,phoff,shoff,flags;
    uint16_t ehsize,phentsize,phnum,shentsize,shnum,shstrndx;
  };
  struct Segment { u32 type,offset,vaddr,paddr,filesz,memsz,flags,align; };
  struct Symbol { u32 name,value,size; uint8_t info,other; uint16_t shndx; };
  struct Relocation { u32 offset,type,symbol; };
  std::vector<uint8_t> bytes;
  std::vector<Symbol> symbols;
  std::vector<std::string> names;
  std::vector<Relocation> relocations;
  static constexpr size_t maxSize=64*1024*1024;

  static bool range(uint64_t offset,uint64_t count,uint64_t total) { return offset<=total && count<=total-offset; }
  static void require(bool ok,const char* reason) { if(!ok) throw std::runtime_error(reason); }
  template<class T> static T object(const std::vector<uint8_t>& input,uint64_t offset) {
    require(range(offset,sizeof(T),input.size()),"tabela fora dos limites");
    T value{}; std::memcpy(&value,input.data()+offset,sizeof(T)); return value;
  }
  void parse(const std::vector<uint8_t>& file) {
    static_assert(sizeof(Header)==52 && sizeof(Segment)==32 && sizeof(Symbol)==16,"Layout ELF32 inesperado");
    require(file.size()<=maxSize && file.size()>=sizeof(Header),"tamanho do arquivo invalido");
    const auto eh=object<Header>(file,0);
    require(std::memcmp(eh.ident,"\x7f" "ELF",4)==0 && eh.ident[4]==1 && eh.ident[5]==1 && eh.ident[6]==1,
            "esperado ELF32 little-endian versao 1");
    require(eh.machine==40 && eh.type==3 && eh.version==1,"esperada biblioteca ARM ET_DYN");
    require(eh.ehsize==sizeof(Header) && eh.phentsize==sizeof(Segment) && eh.phnum>0,"cabecalho de segmentos invalido");
    require(range(eh.phoff,uint64_t(eh.phnum)*eh.phentsize,file.size()),"segmentos fora do arquivo");
    std::vector<Segment> segments; uint64_t span=0; Segment dynamic{}; unsigned dynamicCount=0;
    for(unsigned i=0;i<eh.phnum;++i) {
      auto p=object<Segment>(file,uint64_t(eh.phoff)+i*eh.phentsize);
      if(p.type!=1 && p.type!=2) continue;
      require(p.filesz<=p.memsz && range(p.offset,p.filesz,file.size()),"segmento truncado ou filesz maior que memsz");
      require(range(p.vaddr,p.memsz,maxSize),"segmento excede limite de memoria");
      if(p.type==1) { segments.push_back(p); span=std::max(span,uint64_t(p.vaddr)+p.memsz); }
      else { dynamic=p; ++dynamicCount; }
    }
    require(span>0 && dynamicCount==1,"segmentos de carga/dinamico ausentes ou ambiguos");
    span=(span+4095)&~uint64_t(4095);
    require(span<=maxSize,"imagem excede limite de memoria");
    auto loaded=[&](uint64_t offset,uint64_t size) {
      for(const auto& p:segments) if(offset>=p.vaddr && range(offset-p.vaddr,size,p.memsz)) return true;
      return false;
    };
    bytes.assign(static_cast<size_t>(span),0);
    for(const auto& p:segments) if(p.filesz) std::memcpy(bytes.data()+p.vaddr,file.data()+p.offset,p.filesz);
    require(dynamic.filesz>=8 && dynamic.filesz%8==0 && loaded(dynamic.vaddr,dynamic.filesz),"tabela dinamica fora da imagem");
    std::map<int32_t,u32> tags; bool terminated=false;
    for(uint64_t pos=dynamic.vaddr;pos<uint64_t(dynamic.vaddr)+dynamic.filesz;pos+=8) {
      auto tag=object<int32_t>(bytes,pos); auto val=object<u32>(bytes,pos+4);
      if(tag==0) { terminated=true; break; }
      tags[tag]=val;
    }
    require(terminated,"tabela dinamica sem terminador");
    for(auto tag:{4,5,6,10}) require(tags.count(tag)!=0,"metadado dinamico obrigatorio ausente");
    require(!tags.count(11) || tags[11]==16,"tamanho de simbolo incompativel");
    require(!tags.count(19) || tags[19]==8,"tamanho de relocacao incompativel");
    require(!tags.count(20) || tags[20]==17,"relocacoes PLT precisam usar REL");
    const auto hash=tags[4],strtab=tags[5],symtab=tags[6],strsz=tags[10];
    require(loaded(hash,8),"DT_HASH fora da imagem");
    const auto buckets=object<u32>(bytes,hash),nsyms=object<u32>(bytes,uint64_t(hash)+4);
    require(nsyms<=65536,"numero de simbolos excede o limite suportado");
    require(nsyms>0 && loaded(hash,8+uint64_t(buckets)*4+uint64_t(nsyms)*4),"DT_HASH truncado");
    require(loaded(symtab,uint64_t(nsyms)*16),"tabela de simbolos fora da imagem");
    require(strsz>0 && loaded(strtab,strsz),"tabela de strings fora da imagem");
    symbols.clear(); names.clear(); relocations.clear();
    symbols.reserve(nsyms); names.reserve(nsyms);
    size_t nameBytes=0;
    for(u32 i=0;i<nsyms;++i) {
      auto s=object<Symbol>(bytes,uint64_t(symtab)+uint64_t(i)*16);
      require(s.name<strsz,"nome de simbolo fora da tabela");
      const char* text=reinterpret_cast<const char*>(bytes.data()+strtab+s.name);
      const auto end=static_cast<const char*>(std::memchr(text,0,strsz-s.name));
      require(end!=nullptr,"nome de simbolo sem terminador");
      const size_t length=static_cast<size_t>(end-text);
      require(length<=4096 && length<=8*1024*1024-nameBytes,"nomes de simbolos excedem o limite suportado");
      nameBytes+=length;
      if(s.shndx!=0) {
        // Symbols de tamanho zero podem indicar exatamente o fim da imagem.
        require((s.value&~1u)<=span && s.size<=span-(s.value&~1u),"simbolo definido fora da imagem");
      }
      symbols.push_back(s); names.emplace_back(text,end);
    }
    auto readRelocations=[&](u32 off,u32 size) {
      if(!size) return;
      require(size%8==0 && loaded(off,size),"tabela de relocacoes fora da imagem");
      for(uint64_t pos=off;pos<uint64_t(off)+size;pos+=8) {
        const auto target=object<u32>(bytes,pos),info=object<u32>(bytes,pos+4),type=info&255,si=info>>8;
        if(type==0) continue;
        require(target%4==0 && loaded(target,4),"destino de relocacao invalido");
        if(type==2 || type==21 || type==22) require(si<nsyms,"indice de simbolo da relocacao invalido");
        require(relocations.size()<1024*1024,"numero de relocacoes excede o limite suportado");
        relocations.push_back({target,type,si});
      }
    };
    readRelocations(tags[17],tags[18]); readRelocations(tags[23],tags[2]);
  }
};
} // namespace destinia
