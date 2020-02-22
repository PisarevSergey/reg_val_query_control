#include "common.h"

extern "C"
{
  NTSTATUS NTAPI NtDeleteKey(HANDLE KeyHandle);
}

NTSTATUS ntdll::delete_key(HANDLE key)
{
  return NtDeleteKey(key);
}
