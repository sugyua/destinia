#include "../common/attribute_sheet.h"
#include "../common/diagnostics_panel.h"
#include "../common/inventory_panel.h"
#include <cassert>
#include <vector>
int main(){int panels=0,texts=0;destinia::originalPanelPainter=[&](uint8_t*,int,int,int,int,int,int){panels++;return true;};destinia::originalTextPainter=[&](uint8_t*,int,int,int,int,const std::string&,uint16_t,int,int){texts++;return true;};std::vector<uint8_t> frame(400*240*2);destinia::AttributeSheet a;a.toggle();a.paint(frame.data(),400,240,{});assert(panels>0&&texts>0);panels=0;texts=0;destinia::DiagnosticsPanel d;d.toggle();d.paint(frame.data(),400,240);assert(panels>0&&texts>0);panels=0;texts=0;destinia::InventoryPanel p;destinia::paintInventory(frame.data(),400,240,100,p,true,true);assert(panels>1&&texts>5);}
