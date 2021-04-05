#include "gc.hpp"

using namespace Vault;
  
MemInfo* Vault::getMemInfo(void* block) {
  return (MemInfo*)((uint64_t)block - (uint64_t)sizeof(MemInfo));
}