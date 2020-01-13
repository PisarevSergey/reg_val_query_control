#include "rvqc_common.h"
#include "driver.tmh"

namespace driver_cpp
{
  class wpp_tracing_driver : public driver
  {
  public:
    wpp_tracing_driver()
    {
      WPP_INIT_TRACING(0, 0);
    }

    ~wpp_tracing_driver()
    {
      WPP_CLEANUP(0);
    }
  };

  class fltmgr_driver : public wpp_tracing_driver
  {
  public:
    fltmgr_driver(NTSTATUS& stat, PDRIVER_OBJECT driver) : filter(nullptr)
    {
      FLT_REGISTRATION freg = { 0 };
      freg.Size = sizeof(freg);
      freg.Version = FLT_REGISTRATION_VERSION;
      freg.FilterUnloadCallback = unload;
      stat = FltRegisterFilter(driver, &freg, &filter);
      if (NT_SUCCESS(stat))
      {
        info_message(DRIVER, "FltRegisterFilter success");
      }
      else
      {
        error_message(DRIVER, "FltRegisterFilter failed with status %!STATUS!", stat);
      }
    }

    ~fltmgr_driver()
    {
      if (filter)
      {
        verbose_message(DRIVER, "unregistering minifilter");
        FltUnregisterFilter(filter);
        verbose_message(DRIVER, "minifilter unregistering finished");
      }
    }

    static NTSTATUS unload(FLT_FILTER_UNLOAD_FLAGS)
    {
      info_message(DRIVER, "unloading");
      delete get_driver();

      return STATUS_SUCCESS;
    }

  private:
    PFLT_FILTER filter;
  };


  class top_driver : public fltmgr_driver
  {
  public:
    top_driver(NTSTATUS& stat, PDRIVER_OBJECT driver) : fltmgr_driver(stat, driver)
    {}

    void* __cdecl operator new(size_t, void* p) { return p; }
  };

  char driver_memory[sizeof(top_driver)];
}

driver* get_driver()
{
  return reinterpret_cast<driver*>(driver_cpp::driver_memory);
}

NTSTATUS create_driver(PDRIVER_OBJECT driver)
{
  NTSTATUS stat(STATUS_UNSUCCESSFUL);

  new (driver_cpp::driver_memory) driver_cpp::top_driver(stat, driver);
  if (NT_SUCCESS(stat))
  {
    info_message(DRIVER, "driver_cpp::top_driver success");
  }
  else
  {
    error_message(DRIVER, "driver_cpp::top_driver failed with status %!STATUS!", stat); // WPP registration always successful
    delete get_driver();
  }

  return stat;
}
