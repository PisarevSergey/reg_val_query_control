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

  class registry_dispatcher_driver : public wpp_tracing_driver
  {
  public:
    registry_dispatcher_driver(NTSTATUS& stat)
    {
      reg_disp.reset(registry_dispatcher::create_dispatcher(stat));

      if (NT_SUCCESS(stat))
      {
        info_message(DRIVER, "registry_dispatcher::create_dispatcher success");
      }
      else
      {
        error_message(DRIVER, "registry_dispatcher::create_dispatcher failed with status %!STATUS!", stat);
      }
    }

  protected:
    win_kernel_lib::smart_pointers::auto_pointer<registry_dispatcher::dispatcher> reg_disp;
  };

  class registry_callback_driver : public registry_dispatcher_driver
  {
  public:
    registry_callback_driver(NTSTATUS& stat, PDRIVER_OBJECT driver) : registry_dispatcher_driver(stat), callback_registered{ false }
    {
      if (NT_SUCCESS(stat))
      {
        const UNICODE_STRING altitude = RTL_CONSTANT_STRING(L"364297");
        stat = CmRegisterCallbackEx(callback, &altitude, driver, reg_disp.get(), &cookie, nullptr);
        if (NT_SUCCESS(stat))
        {
          callback_registered = true;
          info_message(DRIVER, "CmRegisterCallbackEx success");
        }
        else
        {
          error_message(DRIVER, "CmRegisterCallbackEx failed with status %!STATUS!", stat);
        }
      }
    }

    ~registry_callback_driver()
    {
      if (callback_registered)
      {
        verbose_message(DRIVER, "starting registry callback unregister");
        CmUnRegisterCallback(cookie);
        verbose_message(DRIVER, "finished registry callback unregister");
      }
    }

    PLARGE_INTEGER get_reg_cookie()
    {
      return &cookie;
    }

    union reg_notify_class_caster
    {
      void* ptr;
      REG_NOTIFY_CLASS op;
    };

    static NTSTATUS callback(void* ctx, void* reg_op, void* reg_op_info)
    {
      reg_notify_class_caster caster;
      caster.ptr = reg_op;

      return static_cast<registry_dispatcher::dispatcher*>(ctx)->callback(caster.op, reg_op_info);
    }
  private:
    LARGE_INTEGER cookie;
    bool callback_registered;
  };

  class fltmgr_driver : public registry_callback_driver
  {
  public:
    fltmgr_driver(NTSTATUS& stat, PDRIVER_OBJECT driver) : filter{ nullptr }, registry_callback_driver(stat, driver)
    {
      if (NT_SUCCESS(stat))
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
          filter = nullptr;
          error_message(DRIVER, "FltRegisterFilter failed with status %!STATUS!", stat);
        }
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
      info_message(DRIVER, "starting driver destruction");
      delete get_driver();
      info_message(DRIVER, "finished driver destruction");

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
