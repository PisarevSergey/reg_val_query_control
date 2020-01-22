#pragma once

namespace smart_pointers
{
  template <typename T>
  class auto_pointer
  {
  public:
    explicit auto_pointer(T* object = nullptr) : ptr{ object }
    {}

    ~auto_pointer()
    {
      delete ptr;
      ptr = nullptr;
    }

    auto_pointer(const auto_pointer&) = delete;
    auto_pointer& operator=(const auto_pointer&) = delete;

    void reset(T* new_ptr = nullptr)
    {
      if (new_ptr != ptr)
      {
        delete ptr;
        ptr = new_ptr;
      }
    }

    T* get()
    {
      return ptr;
    }

    T* operator->()
    {
      return ptr;
    }

  private:
    T* ptr;
  };
}
