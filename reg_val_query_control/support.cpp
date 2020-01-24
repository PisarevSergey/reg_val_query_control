#include "rvqc_common.h"

const void* support::align_up(const void* to_align_ptr, const ULONG alignment)
{
  const ULONG_PTR to_align{ reinterpret_cast<ULONG_PTR>(to_align_ptr) };
  static_assert(sizeof(to_align_ptr) == sizeof(to_align), "wrong size");
  return reinterpret_cast<const void*>((to_align / alignment + ((to_align % alignment) ? 1 : 0)) * alignment);
}
