#include "../common/portuguese_text.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
namespace fs=std::filesystem;
std::string json(const std::string&s){std::string o="\"";for(unsigned char c:s){if(c=='"'||c=='\\'){o+='\\';o+=c;}else if(c=='\n')o+="\\n";else if(c=='\r')o+="\\r";else if(c=='\t')o+="\\t";else o+=c;}return o+'"';}
int main(int argc,char**argv){if(argc!=2){std::cerr<<"Uso: export_portuguese_web PASTA_ASSETS\n";return 1;}std::set<std::string> originals;for(const auto&pair:destinia::portugueseText())originals.insert(pair.first);
 for(const auto& file:fs::directory_iterator(argv[1])){
  const auto name=file.path().filename().string();if(!destinia::isPortugueseAsset(name))continue;
  std::ifstream f(file.path(),std::ios::binary);std::vector<uint8_t>b((std::istreambuf_iterator<char>(f)),{});
  if(file.path().extension()==".tdt"){size_t p=2;unsigned count=b[0]|(b[1]<<8);for(unsigned i=0;i<count && p+2<=b.size();++i){unsigned n=b[p]|(b[p+1]<<8);p+=2;if(p+n>b.size())break;originals.insert(std::string((char*)b.data()+p,n));p+=n;}}
  else {for(size_t p=0;p<b.size();){while(p<b.size() && !b[p])++p;auto start=p;while(p<b.size() && b[p])++p;if(p>start && p-start<=512)originals.insert(std::string((char*)b.data()+start,p-start));}}
 }
 std::cout<<"// Generated from common/portuguese_text.h and the known English resource strings.\nexport const portugueseRules={\n";
 for(const auto&text:originals){if(std::any_of(text.begin(),text.end(),[](unsigned char c){return (c<32 && c!='\n'&&c!='\r'&&c!='\t') || c>126;}))continue;auto translated=destinia::translatePortugueseString(text);if(translated!=text)std::cout<<json(text)<<":"<<json(translated)<<",\n";}
 std::cout<<"};\n";
}
