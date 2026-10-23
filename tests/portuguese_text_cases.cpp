#include "../common/portuguese_text.h"
#include <cassert>
#include <iostream>
using namespace destinia;
int main() {
 auto log=[](const std::string&){};
 assert(isPortugueseAsset("help_touch_en.tdt"));
 assert(isPortugueseAsset("mapname0_en.tdt"));
 assert(translatePortugueseString("North Darkwoods - 1")=="Bosque Sombrio Norte - 1");
 assert(translatePortugueseString("Basic Controls")=="Controles basicos");
 assert(translatePortugueseString("Inventory")=="Inventario");
 assert(translatePortugueseString("A new skill acquired.")=="Nova habilidade adquirida.");
 assert(translatePortugueseString("Unknown location 123")=="Unknown location 123");
 // TDT lengths are rebuilt instead of truncating a longer translation.
 std::vector<uint8_t> table={1,0,9,0,'I','n','v','e','n','t','o','r','y'};
 assert(applyPortugueseText(true,"gametxt_en.tdt",table,log));
 assert(table[2]==10 && std::string(table.begin()+4,table.end())=="Inventario");
 auto unchanged=table;assert(!applyPortugueseText(false,"gametxt_en.tdt",table,log));assert(table==unchanged);
 table={1,0,9,0,'I','n'};unchanged=table;
 assert(!translateTable(table));assert(table==unchanged);
 // A GDT field with insufficient capacity retains its original bytes.
 std::vector<uint8_t> shortField={'I','n','v','e','n','t','o','r','y',0,0x81};unchanged=shortField;
 assert(!translateGdt("item_en.gdt",shortField,log));assert(shortField==unchanged);
 std::vector<uint8_t> padded={'I','n','v','e','n','t','o','r','y',0,0,0,0,0,0x81};
 assert(translateGdt("item_en.gdt",padded,log));assert(padded.size()==15 && padded.back()==0x81);
 assert(std::string(reinterpret_cast<char*>(padded.data()))=="Inventario");
 std::cout<<"PORTUGUESE_TEXT_OK\n";
}
