#include "common.h"
#include "value_modifier.tmh"

using support::is_entity_inside_buffer;
using win_kernel_lib::string_facility::string;

namespace value_modifier_cpp
{
  class modifier_with_rules : public value_modifier::modifier
  {
  public:
    modifier_with_rules(NTSTATUS& stat) : r_manager{ rule_manager::create_ruler(stat) }
    {}

    ~modifier_with_rules()
    {
      delete r_manager;
      r_manager = nullptr;
    }

    NTSTATUS set_rules(unsigned __int32 number_of_rules, um_km_common::key_rule_header* rules_from_um, ULONG rules_size) override
    {
      NTSTATUS stat{ STATUS_SUCCESS };

      const void* const first_byte_after_rules_buffer{ win_kernel_lib::pointer_manipulation::add_to_ptr<void, um_km_common::key_rule_header>(rules_from_um, rules_size) };

      const void* current_position{ rules_from_um };
      for (decltype(number_of_rules) i{ 0 }; (i < number_of_rules) && NT_SUCCESS(stat); ++i)
      {
        if (!is_entity_inside_buffer(current_position, first_byte_after_rules_buffer, current_position, sizeof(um_km_common::key_rule_header)))
        {
          stat = STATUS_INVALID_PARAMETER;
          error_message(VALUE_MODIFIER, "rules buffer too small");
          break;
        }

        auto_pointer<const UNICODE_STRING, pool_deleter> key_path;
        stat = support::query_name_for_user_mode_key_handle(static_cast<const um_km_common::key_rule_header*>(current_position)->key.handle_val, key_path);
        if (NT_SUCCESS(stat))
        {
          verbose_message(VALUE_MODIFIER, "key path for rule %d is %wZ", i, key_path.get());
        }
        else
        {
          error_message(VALUE_MODIFIER, "support::query_name_for_user_mode_key_handle failed with status %!STATUS!", stat);
          break;
        }

        rule_facility::rule* r{ new rule_facility::rule };
        if (r)
        {
          info_message(VALUE_MODIFIER, "rule allocated");
        }
        else
        {
          error_message(VALUE_MODIFIER, "failed to allocate memory for rule");
          stat = STATUS_INSUFFICIENT_RESOURCES;
          break;
        }

        r->set_reg_key(key_path);

        auto val_num{ static_cast<const um_km_common::key_rule_header*>(current_position)->number_of_values };
        auto current_value_name{ static_cast<const um_km_common::key_rule_header*>(current_position)->get_first_value_name() };
        for (decltype(val_num) j{ 0 }; j < val_num; ++j)
        {
          if (!is_entity_inside_buffer(current_position, first_byte_after_rules_buffer, current_value_name, sizeof(*current_value_name)))
          {
            stat = STATUS_INVALID_PARAMETER;
            break;
          }

          if (!is_entity_inside_buffer(current_position, first_byte_after_rules_buffer, current_value_name, sizeof(*current_value_name) + current_value_name->buffer_size_in_bytes))
          {
            stat = STATUS_INVALID_PARAMETER;
            break;
          }

          auto_pointer<UNICODE_STRING, pool_deleter> cur_val_name_us{ win_kernel_lib::unicode_strings::createStringCopy(current_value_name->get_buffer(),
            current_value_name->buffer_size_in_bytes,
            'mnlV') };
          if (cur_val_name_us.get())
          {
            verbose_message(VALUE_MODIFIER, "memory for value name successfully allocated");
          }
          else
          {
            error_message(VALUE_MODIFIER, "failed to allocate memory for value name");
            stat = STATUS_INSUFFICIENT_RESOURCES;
            break;
          }

          stat = r->add_value_name(cur_val_name_us);
          if (NT_SUCCESS(stat))
          {
            verbose_message(VALUE_MODIFIER, "successfully added value name to rule");
          }
          else
          {
            error_message(VALUE_MODIFIER, "failed to add value name to rule with status %!STATUS!", stat);
            break;
          }

          current_value_name = static_cast<const um_km_common::counted_string*>(current_value_name->first_byte_after_string());
        }

        if (NT_SUCCESS(stat))
        {
          verbose_message(VALUE_MODIFIER, "successfully parsed value names for rule %d", i);
        }
        else
        {
          stat = STATUS_INVALID_PARAMETER;
          delete r;
          break;
        }

        if (r_manager->add_rule_to_list(r))
        {
          FLT_ASSERT(NT_SUCCESS(stat));
          verbose_message(VALUE_MODIFIER, "rule successfully inserted in rule list");
        }
        else
        {
          error_message(VALUE_MODIFIER, "failed to insert rule in list");
          stat = STATUS_INSUFFICIENT_RESOURCES;
          delete r;
          break;
        }

        current_position = current_value_name;
      }

#if DBG
      if (NT_SUCCESS(stat))
      {
        ASSERT(first_byte_after_rules_buffer == current_position);
      }
#endif

      return stat;
    }

  private:
    rule_manager::ruler* r_manager;
  };

  class modifier_impl : public modifier_with_rules
  {
  public:
    modifier_impl(NTSTATUS& stat) : modifier_with_rules{ stat }
    {}

    void modify(reg_data_decoding::decoded_data& data) override
    {
      PCUNICODE_STRING key_path{nullptr};
      modify(key_path, data);
    }

    void modify(PCUNICODE_STRING& key_path, reg_data_decoding::decoded_data& data) override
    {
      NTSTATUS stat{ key_path ? STATUS_SUCCESS : CmCallbackGetKeyObjectID(get_driver()->get_reg_cookie(), data.key_object, nullptr, &key_path) };

      do
      {
        if (NT_SUCCESS(stat))
        {
          verbose_message(VALUE_MODIFIER, "key path is %wZ", key_path);
        }
        else
        {
          error_message(VALUE_MODIFIER, "CmCallbackGetKeyObjectID failed with status %!STATUS!", stat);
          key_path = nullptr;
          break;
        }



      } while (false);

    }
  };

  class top_modifier final : public modifier_impl
  {
  public:
    top_modifier(NTSTATUS& stat) : modifier_impl{ stat }
    {}

    void* __cdecl operator new(size_t, void* p)
    {
      return p;
    }
  };

  char modifier_memory[sizeof(top_modifier)];
}

value_modifier::modifier::~modifier() {}

void __cdecl value_modifier::modifier::operator delete(void*)
{}


value_modifier::modifier* value_modifier::create_modifier(NTSTATUS& stat)
{
  stat = STATUS_SUCCESS;
  auto m{ new (value_modifier_cpp::modifier_memory) value_modifier_cpp::top_modifier{stat} };
  if (NT_SUCCESS(stat))
  {
  }
  else
  {
    delete m;
    m = nullptr;
  }

  return m;
}
