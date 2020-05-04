#include "common.h"
#include "driver.tmh"

using win_kernel_lib::smart_pointers::auto_pointer;

namespace driver_cpp
{
  class memory_allocator_driver : public driver
  {
  public:
    void* __cdecl operator new(size_t, void* p) { return p; }
    void __cdecl operator delete(void*) {}
  };

  class wpp_tracing_driver : public memory_allocator_driver
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
    auto_pointer<registry_dispatcher::dispatcher> reg_disp;
  };

  class registry_callback_driver : public registry_dispatcher_driver
  {
  public:
    registry_callback_driver(NTSTATUS& stat, PDRIVER_OBJECT driver) : registry_dispatcher_driver{ stat }, callback_registered{ false }
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
    fltmgr_driver(NTSTATUS& stat, PDRIVER_OBJECT driver) : filter{ nullptr }, registry_callback_driver{ stat, driver }
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

  protected:
    PFLT_FILTER filter;
  };

  class communication_port_driver : public fltmgr_driver
  {
  public:
    communication_port_driver(NTSTATUS& stat, PDRIVER_OBJECT driver) : server_port{ nullptr }, client_port{nullptr}, fltmgr_driver{ stat, driver }
    {
      if (NT_SUCCESS(stat))
      {
        PSECURITY_DESCRIPTOR desc{ nullptr };
        stat = FltBuildDefaultSecurityDescriptor(&desc, FLT_PORT_ALL_ACCESS);
        if (NT_SUCCESS(stat))
        {
          info_message(DRIVER, "FltBuildDefaultSecurityDescriptor success");

          UNICODE_STRING port_name = RTL_CONSTANT_STRING(um_km_common::communication_port_name);

          OBJECT_ATTRIBUTES oa;
          InitializeObjectAttributes(&oa, &port_name, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, 0, desc);

          stat = FltCreateCommunicationPort(filter,
            &server_port,
            &oa,
            this,
            connect,
            disconnect,
            on_message,
            1);
          if (NT_SUCCESS(stat))
          {
            info_message(DRIVER, "FltCreateCommunicationPort success");
          }
          else
          {
            error_message(DRIVER, "FltCreateCommunicationPort failed with status %!STATUS!", stat);
            server_port = nullptr;
          }

          FltFreeSecurityDescriptor(desc);
        }
        else
        {
          error_message(DRIVER, "FltBuildDefaultSecurityDescriptor failed with status %!STATUS!", stat);
        }
      }
    }

    ~communication_port_driver()
    {
      if (server_port)
      {
        verbose_message(DRIVER, "closing server port");
        FltCloseCommunicationPort(server_port);
        verbose_message(DRIVER, "server port closed");
        server_port = nullptr;
      }
    }

    static NTSTATUS connect(PFLT_PORT client_port_parameter,
      void* server_port_cookie,
      void* connection_context,
      ULONG size_of_context,
      void** connection_port_cookie)
    {
      *connection_port_cookie = server_port_cookie;
      return static_cast<communication_port_driver*>(server_port_cookie)->connect(client_port_parameter, connection_context, size_of_context);
    }

    static void disconnect(void* connection_cookie)
    {
      static_cast<communication_port_driver*>(connection_cookie)->disconnect();
    }

    static NTSTATUS on_message(void* port_cookie,
      void* input_buffer OPTIONAL,
      ULONG input_buffer_length,
      void* output_buffer OPTIONAL,
      ULONG output_buffer_length,
      PULONG return_output_buffer_length)
    {
      return static_cast<communication_port_driver*>(port_cookie)->on_message(input_buffer,
        input_buffer_length,
        output_buffer,
        output_buffer_length,
        return_output_buffer_length);
    }

    NTSTATUS connect(PFLT_PORT client_port_parameter,
      void* /*connection_context*/,
      ULONG /*size_of_context*/)
    {
      info_message(DRIVER, "connecting to communication port");
      client_port = client_port_parameter;
      return STATUS_SUCCESS;
    }

    void disconnect()
    {
      info_message(DRIVER, "disconnecting from communication port");
      info_message(DRIVER, "closing client port");
      FltCloseClientPort(filter, &client_port);
      info_message(DRIVER, "client port closed");
    }

    NTSTATUS on_message(void* input_buffer OPTIONAL,
      ULONG input_buffer_length,
      void* /*output_buffer */OPTIONAL,
      ULONG /*output_buffer_length*/,
      PULONG return_output_buffer_length)
    {
      info_message(DRIVER, "user-mode message callback");

      NTSTATUS stat{ STATUS_SUCCESS };

      void* km_message_copy{ nullptr };

      do
      {
        if (input_buffer_length >= sizeof(um_km_common::request))
        {
          verbose_message(DRIVER, "user-mode request may be valid");
        }
        else
        {
          error_message(DRIVER, "user-mode buffer too small, invalid request");
          stat = STATUS_INVALID_PARAMETER;
          break;
        }

        __try
        {
          ProbeForRead(input_buffer, input_buffer_length, 1);
          verbose_message(DRIVER, "user-mode message memory probe success");

          km_message_copy = ExAllocatePoolWithTag(PagedPool, input_buffer_length, 'pcmK');
          if (km_message_copy)
          {
            verbose_message(DRIVER, "allocated %x bytes at %p for kernel copy of user buffer", input_buffer_length, km_message_copy);
          }
          else
          {
            error_message(DRIVER, "failed to allocate memory for kernel copy of user buffer");
            stat = STATUS_INSUFFICIENT_RESOURCES;
            break;
          }

          RtlCopyMemory(km_message_copy, input_buffer, input_buffer_length);
          verbose_message(DRIVER, "copy user-mode message to kernel buffer success");
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
          stat = GetExceptionCode();
          error_message(DRIVER, "user mode buffer dispatching failed with status %!STATUS!", stat);
          break;
        }

        ASSERT(km_message_copy);

        switch (static_cast<um_km_common::request*>(km_message_copy)->rt)
        {
        case um_km_common::request_type::set_rules:
          stat = reg_disp->set_rules(static_cast<um_km_common::request*>(km_message_copy)->request_fixed_part_header.rh.number_of_rules,
            static_cast<um_km_common::key_rule_header*>(static_cast<um_km_common::request*>(km_message_copy)->get_request_specific_data()),
            input_buffer_length - sizeof(um_km_common::request));
          *return_output_buffer_length = 0;
          break;
        case um_km_common::request_type::clear_rules:
          reg_disp->clear_rules();
          break;
        default:
          stat = STATUS_INVALID_PARAMETER;
          error_message(DRIVER, "invalid request");
          break;
        }

      } while (false);

      if (km_message_copy)
      {
        ExFreePool(km_message_copy);
        km_message_copy = nullptr;
      }

      return stat;
    }
  private:
    PFLT_PORT server_port;
    PFLT_PORT client_port;
  };


  class top_driver final : public communication_port_driver
  {
  public:
    top_driver(NTSTATUS& stat, PDRIVER_OBJECT driver) : communication_port_driver(stat, driver)
    {}
  };

  char driver_memory[sizeof(top_driver)];
}

driver::~driver() {}

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
