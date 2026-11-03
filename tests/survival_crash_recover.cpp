#include "../common/safe_save.h"
int main(int argc,char**argv){if(argc!=2)return 1;try{destinia::SaveTransaction tx;tx.configure(argv[1]);tx.recover();return 0;}catch(...){return 2;}}
