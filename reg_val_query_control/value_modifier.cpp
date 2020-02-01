#include "value_modifier.h"

namespace value_modifier_cpp
{
  class modifier_with_rules : public value_modifier::modifier
  {};

  class modifier_impl : public modifier_with_rules
  {
  public:
    void modify(PCUNICODE_STRING& /*key_path*/, const reg_data_decoding::decoded_data& /*data*/)
    {}
  };

  class top_modifier final : public modifier_impl
  {
  public:
    void* __cdecl operator new(size_t, void* p)
    {
      return p;
    }
  };

  char modifier_memory[sizeof(top_modifier)];
}

value_modifier::modifier::~modifier() {}

void __cdecl operator delete(void*)
{}


value_modifier::modifier* value_modifier::create_modifier(NTSTATUS& stat)
{
  stat = STATUS_SUCCESS;
  return new (value_modifier_cpp::modifier_memory) value_modifier_cpp::top_modifier;
}