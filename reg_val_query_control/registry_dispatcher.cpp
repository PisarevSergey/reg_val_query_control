#include "rvqc_common.h"
#include "registry_dispatcher.tmh"

namespace registry_dispatcher_cpp
{
  class registry_dispatcher_impl : public registry_dispatcher::dispatcher
  {
  public:
    NTSTATUS callback(REG_NOTIFY_CLASS /*reg_op*/, void* /*reg_op_data*/)
    {
      info_message(REGISTRY_DISPATCHER, "in callback");

      return STATUS_SUCCESS;
    }
  };

  class top_dispatcher : public registry_dispatcher_impl
  {
  public:
    void* __cdecl operator new(size_t sz)
    {
      return ExAllocatePoolWithTag(NonPagedPool, sz, 'sidR');
    }
  };
}

void __cdecl registry_dispatcher::dispatcher::operator delete(void* p)
{
  if (p)
  {
    ExFreePool(p);
  }
}

registry_dispatcher::dispatcher* registry_dispatcher::create_dispatcher(NTSTATUS& stat)
{
  auto p{new registry_dispatcher_cpp::top_dispatcher};

  stat = p ? STATUS_SUCCESS : STATUS_INSUFFICIENT_RESOURCES;

  return p;
}
