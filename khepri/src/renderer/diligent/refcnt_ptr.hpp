#pragma once

#include <memory>
#include <type_traits>

namespace khepri::renderer::diligent {

/**
 * Custom implementation of Diligent::RefCntAutoPtr.
 *
 * This implementation fixes one VERY annoying property: unlike Diligent's RefCntAutoPtr,
 * dereferencing a `const RefCntPtr<T>` does NOT produce a `const T*`.
 *
 * This corrected behavior is the same behavior for C++'s raw and smart pointers and enables us to
 * implement better const correctness.
 */
template <typename T>
class RefCntPtr
{
public:
    RefCntPtr() noexcept = default;

    // Constructor that takes ownership
    explicit RefCntPtr(T* obj) noexcept : m_object{obj}
    {
        if (m_object) {
            m_object->AddRef();
        }
    }

    // Copy constructor
    RefCntPtr(const RefCntPtr& other) noexcept : m_object{other.m_object}
    {
        if (m_object) {
            m_object->AddRef();
        }
    }

    // Copy constructor from derived type
    template <typename DerivedType,
              typename = typename std::enable_if<std::is_base_of<T, DerivedType>::value>::type>
    RefCntPtr(const RefCntPtr<DerivedType>& other) noexcept : RefCntPtr<T>{other.m_object}
    {
    }

    // Move constructor
    RefCntPtr(RefCntPtr&& other) noexcept : m_object{std::move(other.m_object)}
    {
        // Make sure original pointer has no references to the object
        other.m_object = nullptr;
    }

    // Move constructor from derived type
    template <typename DerivedType,
              typename = typename std::enable_if<std::is_base_of<T, DerivedType>::value>::type>
    RefCntPtr(RefCntPtr<DerivedType>&& other) noexcept : m_object{std::move(other.m_object)}
    {
        other.m_object = nullptr;
    }

    ~RefCntPtr()
    {
        release();
    }

    void swap(RefCntPtr& other) noexcept
    {
        std::swap(m_object, other.m_object);
    }

    void attach(T* obj) noexcept
    {
        release();
        m_object = obj;
    }

    T* detach() noexcept
    {
        T* obj   = m_object;
        m_object = nullptr;
        return obj;
    }

    void release() noexcept
    {
        if (m_object) {
            m_object->Release();
            m_object = nullptr;
        }
    }

    RefCntPtr& operator=(T* obj) noexcept
    {
        if (m_object != obj) {
            if (m_object) {
                m_object->Release();
            }
            m_object = obj;
            if (m_object) {
                m_object->AddRef();
            }
        }
        return *this;
    }

    RefCntPtr& operator=(const RefCntPtr& other) noexcept
    {
        return operator=(other.m_object);
    }

    template <typename DerivedType,
              typename = typename std::enable_if<std::is_base_of<T, DerivedType>::value>::type>
    RefCntPtr& operator=(const RefCntPtr<DerivedType>& other) noexcept
    {
        return *this = static_cast<T*>(other.m_object);
    }

    RefCntPtr& operator=(RefCntPtr&& other) noexcept
    {
        if (m_object != other.m_object) {
            attach(other.detach());
        }
        return *this;
    }

    template <typename DerivedType,
              typename = typename std::enable_if<std::is_base_of<T, DerivedType>::value>::type>
    RefCntPtr& operator=(RefCntPtr<DerivedType>&& other) noexcept
    {
        if (m_object != other.m_object)
            attach(other.detach());

        return *this;
    }

    bool operator!() const noexcept
    {
        return m_object == nullptr;
    }

    explicit operator bool() const noexcept
    {
        return m_object != nullptr;
    }

    bool operator==(const RefCntPtr& other) const noexcept
    {
        return m_object == other.m_object;
    }

    bool operator!=(const RefCntPtr& other) const noexcept
    {
        return m_object != other.m_object;
    }

    bool operator<(const RefCntPtr& other) const noexcept
    {
        return static_cast<const T*>(*this) < static_cast<const T*>(other);
    }

    T& operator*() const noexcept
    {
        return *m_object;
    }

    T* raw_ptr() const noexcept
    {
        return m_object;
    }

    operator T*() const noexcept
    {
        return raw_ptr();
    }

    T* operator->() const noexcept
    {
        return m_object;
    }

private:
    // Helper to make sure that expressions such as `&ref_cnt_ptr` can be passed as argument to
    // functions expecting a `T**`: `operator&` creates a temporary object that attaches the new
    // pointer to the parent RefCntPtr upon destruction.
    template <typename DstType>
    class PtrAssignmentHelper
    {
    public:
        explicit PtrAssignmentHelper(RefCntPtr& refcnt_ptr) noexcept
            : m_raw_ptr{static_cast<DstType*>(refcnt_ptr)}, m_refcnt_ptr{std::addressof(refcnt_ptr)}
        {
        }

        PtrAssignmentHelper(PtrAssignmentHelper&& other) noexcept
            : m_raw_ptr{other.m_raw_ptr}, m_refcnt_ptr{other.m_refcnt_ptr}
        {
            other.m_refcnt_ptr = nullptr;
            other.m_raw_ptr    = nullptr;
        }

        ~PtrAssignmentHelper()
        {
            if (m_refcnt_ptr && *m_refcnt_ptr != static_cast<T*>(m_raw_ptr)) {
                m_refcnt_ptr->attach(static_cast<T*>(m_raw_ptr));
            }
        }

        DstType* operator*() const noexcept
        {
            return m_raw_ptr;
        }

        operator DstType**() noexcept
        {
            return &m_raw_ptr;
        }

        operator const DstType**() const noexcept
        {
            return &m_raw_ptr;
        }

    private:
        DstType*   m_raw_ptr;
        RefCntPtr* m_refcnt_ptr;

        PtrAssignmentHelper(const PtrAssignmentHelper&)            = delete;
        PtrAssignmentHelper& operator=(const PtrAssignmentHelper&) = delete;
        PtrAssignmentHelper& operator=(PtrAssignmentHelper&&)      = delete;
    };

public:
    PtrAssignmentHelper<T> operator&()
    {
        return PtrAssignmentHelper<T>(*this);
    }

    const PtrAssignmentHelper<const T> operator&() const
    {
        return PtrAssignmentHelper<T>(*this);
    }

private:
    template <typename OtherType>
    friend class RefCntPtr;

    T* m_object = nullptr;
};

} // namespace khepri::renderer::diligent
