/********
 **
 **  SPDX-License-Identifier: MIT
 **
 **  Copyright (c) 2017-2022 James M. Putnam <putnamjm.design@gmail.com>
 **
 **/

/********
 **
 **  heap.h: libmu heap
 **
 **/
#if !defined(LIBMU_HEAP_HEAP_H_)
#define LIBMU_HEAP_HEAP_H_

#include <algorithm>
#include <cassert>
#include <cinttypes>
#include <cstdio>
#include <iostream>
#include <list>
#include <string>
#include <utility>

#include "libmu/platform/platform.h"

#include "libmu/type.h"

namespace libmu {
namespace heap {

class Env;

using platform::Platform;

using SYS_CLASS = core::Type::SYS_CLASS;

class Heap {
 public: /* tag */
  enum class HeapInfo : uint64_t {};

  /** * make HeapInfo **/
  /* reloc offset (uint32_t), size (uint16_t), ref bits (uint8_t), and tag
   * (uint8_t) */
  static constexpr HeapInfo MakeHeapInfo(size_t size, SYS_CLASS tag) {
    return static_cast<HeapInfo>((static_cast<uint16_t>((size + 7) / 8) << 16) |
                                 static_cast<uint8_t>(tag));
  }

  /** * dump HeapInfo **/
  static void Dump(HeapInfo hinfo) {
    printf(
        "\n0x%016llx: reloc: 0x%llx size: %zu refbits: 0x%x sys class: 0x%x\n",
        hinfo, Reloc(hinfo), Size(hinfo), RefBits(hinfo),
        static_cast<uint8_t>(SysClass(hinfo)));
  }

 private:
  std::string filename_; /* mapped file */
  size_t pagesz_;        /* page size for this heap */
  size_t npages_;        /* number of pages in the heap */
  char* uaddr_;          /* user virtual address */
  char* alloc_;          /* alloc barrier */
  HeapInfo* conses_;     /* gc caching */

 public:
  /** * SYS_CLASS from HeapInfo **/
  static constexpr SYS_CLASS SysClass(HeapInfo hinfo) {
    return static_cast<SYS_CLASS>(static_cast<uint64_t>(hinfo) & 0xff);
  }

  /** * HeapInfo from HeapInfo, SYS_CLASS **/
  static constexpr HeapInfo SysClass(HeapInfo hinfo, SYS_CLASS tag) {
    return static_cast<HeapInfo>((static_cast<uint64_t>(hinfo) & ~(0xffULL)) |
                                 static_cast<uint8_t>(tag));
  }

  /** * get ref bits **/
  static constexpr uint8_t RefBits(HeapInfo hinfo) {
    return (static_cast<uint64_t>(hinfo) >> 8) & 0xff;
  }

  /** * set ref bits **/
  static constexpr HeapInfo RefBits(HeapInfo hinfo, uint8_t refbits) {
    return static_cast<HeapInfo>(
        (static_cast<uint64_t>(hinfo) & ~(0xffULL << 8)) | refbits << 8);
  }

  /** * get heap object size **/
  static constexpr size_t Size(HeapInfo hinfo) {
    return 8 * ((static_cast<uint64_t>(hinfo) >> 16) & 0xffff);
  }
  /** * set heap object size **/
  static constexpr HeapInfo Size(HeapInfo hinfo, uint32_t size) {
    return static_cast<HeapInfo>(
        (static_cast<uint64_t>(hinfo) & ~(0xffffULL << 16)) |
        ((((size + 7) / 8) & 0xffff) << 16));
  }

  /** * get reloc **/
  static constexpr uint64_t Reloc(HeapInfo hinfo) {
    return 8 * ((static_cast<uintptr_t>(hinfo) >> 32) & 0xffffffff);
  }
  /** * set reloc **/
  static HeapInfo Reloc(HeapInfo hinfo, uint64_t reloc) {
    return static_cast<HeapInfo>(
        (static_cast<uint64_t>(hinfo) & ~(0xffffffffULL << 32)) |
        (((reloc / 8) & 0xffffffff) << 32));
  }

 public:
  size_t nobjects_;               /* number of objects in the heap */
  std::vector<uint32_t>* nalloc_; /* allocated counts */
  std::vector<uint32_t>* nfree_;  /* free counts */

  constexpr size_t size() { return pagesz_ * npages_; }
  constexpr size_t alloc() { return alloc_ - uaddr_; }

  void* Alloc(size_t, SYS_CLASS);

  auto Gc() -> size_t;
  auto ClearRefBits() -> void;
  auto FindFree(size_t, SYS_CLASS) -> HeapInfo*;

  /** * is this cadder in the heap? **/
  auto in_heap(void* caddr) -> bool {
    return (uint64_t)caddr >= (uint64_t)uaddr_ &&
           (uint64_t)caddr < (uint64_t)uaddr_ + npages_ * pagesz_;
  }

  size_t room();
  size_t room(SYS_CLASS);

  explicit Heap();
};

} /* namespace heap */
} /* namespace libmu */

#endif /* LIBMU_HEAP_HEAP_H_ */
