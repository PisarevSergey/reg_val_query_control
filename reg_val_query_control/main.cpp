#include "rvqc_common.h"
#include "main.tmh"

extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT driver, PUNICODE_STRING)
{
  ExInitializeDriverRuntime(DrvRtPoolNxOptIn);

  return create_driver(driver);
}
