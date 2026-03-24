#pragma once

#include <atomic>
#include <assert.h>

namespace mad::rhi {

class Object;
    
class RefCounter
{
friend class Object;
template<typename T> friend class ObjectBase;
template<typename T> friend class WeakPtr;
template<typename T> friend class RefPtr;

public:
    void AddStrongRef();
    bool ReleaseStrongRef();

    void AddWeakRef();
    void ReleaseWeakRef();

    bool TryAddStrongRef();

    int GetStrongRefCount() const { return m_Strong.load(std::memory_order_relaxed); }

private:

    Object* m_pObject = nullptr;

    std::atomic<int> m_Strong = 0;
    std::atomic<int> m_Weak = 0;

    void DestroyObject();

};

class Object
{
public:
    virtual ~Object() = default;

    virtual void AddRef() = 0;
    virtual void Release() = 0;
    virtual RefCounter* GetRefCounter() = 0;

};

template<typename BaseInterface>
class ObjectBase : public BaseInterface
{
public:
    ObjectBase()
    {
        m_pRefCounter = new RefCounter();
        m_pRefCounter->m_pObject = this;
        m_pRefCounter->m_Strong.store(1, std::memory_order_relaxed);
    }

    void AddRef() override
    {
        m_pRefCounter->AddStrongRef();
    }

    void Release() override
    {
        m_pRefCounter->ReleaseStrongRef();
    }

    RefCounter* GetRefCounter() override 
    { 
        return m_pRefCounter; 
    }

protected:
    ~ObjectBase() override = default;

private:
    RefCounter* m_pRefCounter = nullptr;

};

template<typename T>
class RefPtr
{
public:
    RefPtr() = default;
    RefPtr(std::nullptr_t) noexcept {}

    explicit RefPtr(T* ptr) : m_Ptr(ptr)
    {
        if (m_Ptr) m_Ptr->AddRef();
    }

    ~RefPtr()
    {
        if (m_Ptr) m_Ptr->Release();
    }

    RefPtr(const RefPtr& other) : m_Ptr(other.m_Ptr)
    {
        if (m_Ptr) m_Ptr->AddRef();
    }

    template<typename U>
    RefPtr(const RefPtr<U>& other) : m_Ptr(other.m_Ptr)
    {
        if (m_Ptr) m_Ptr->AddRef();
    }

    RefPtr& operator=(const RefPtr& other)
    {
        if (m_Ptr != other.m_Ptr)
        {
            if (m_Ptr) m_Ptr->Release();
            m_Ptr = other.m_Ptr;
            if (m_Ptr) m_Ptr->AddRef();
        }
        return *this;
    }

    RefPtr(RefPtr&& other) noexcept : m_Ptr(other.m_Ptr)
    {
        other.m_Ptr = nullptr;
    }

    template<typename U>
    RefPtr(RefPtr<U>&& other) noexcept : m_Ptr(other.m_Ptr)
    {
        other.m_Ptr = nullptr;
    }

    RefPtr& operator= (RefPtr&& other) noexcept
    {
        if (m_Ptr != other.m_Ptr)
        {
            if (m_Ptr) m_Ptr->Release();
            m_Ptr = other.m_Ptr;
            other.m_Ptr = nullptr;
        }
        return *this;
    }

    void Attach(T* ptr)
    {
        if (m_Ptr) m_Ptr->Release();
        m_Ptr = ptr;
    }

    T* Detach()
    {
        T* p = m_Ptr;
        m_Ptr = nullptr;
        return p;
    }

    void Reset()
    {
        if (m_Ptr) m_Ptr->Release();
        m_Ptr = nullptr;
    }

    T* operator->() const { return m_Ptr; }
    T& operator*() const { return *m_Ptr; }
    T* Get() const { return m_Ptr; }

    T** operator&()
    {
        assert(m_Ptr == nullptr);
        return &m_Ptr;
    }

    explicit operator bool() const { return m_Ptr != nullptr; }

    bool operator==(const RefPtr& other) const { return m_Ptr == other.m_Ptr; }
    bool operator!=(const RefPtr& other) const { return m_Ptr != other.m_Ptr; }

private:
    template<typename U> friend class RefPtr;
    template<typename U> friend class WeakPtr; 

    T* m_Ptr = nullptr;

};

template<typename T>
class WeakPtr
{
public:
    WeakPtr() = default;

    WeakPtr(T* ptr) : m_pRefCounter(ptr ? ptr->GetRefCounter() : nullptr)
    {
        if (m_pRefCounter) m_pRefCounter->AddWeakRef();
    }

    WeakPtr(const RefPtr<T> refPtr) : WeakPtr(refPtr->Get()) {}

    ~WeakPtr()
    {
        if (m_pRefCounter) m_pRefCounter->ReleaseWeakRef();
    }

    WeakPtr(const WeakPtr& other) : m_pRefCounter(other.m_pRefCounter)
    {
        if (m_pRefCounter) m_pRefCounter->AddWeakRef();
    }

    WeakPtr& operator=(const WeakPtr& other)
    {
        if (m_pRefCounter != other.m_pRefCounter)
        {
            if (m_pRefCounter) m_pRefCounter->ReleaseWeakRef();
            m_pRefCounter = other.m_pRefCounter;
            if (m_pRefCounter) m_pRefCounter->AddWeakRef();
        }
        return *this;
    }

    RefPtr<T> Lock()
    {
        if (m_pRefCounter && m_pRefCounter->TryAddStrongRef())
        {
            RefPtr<T> result;
            result.Attach(static_cast<T*>(m_pRefCounter->m_pObject));
            return result;
        }
        return {};
    }

    bool Expired() const
    {
        return !m_pRefCounter || m_pRefCounter->GetStrongRefCount() == 0;
    }

private:
    RefCounter* m_pRefCounter = nullptr;

};

template<typename T, typename... Args>
RefPtr<T> MakeRef(Args&... args)
{
    T* obj = new T(std::forward<Args>(args)...);
    RefPtr<T> ptr;
    ptr.Attach(obj);
    return ptr;
}

}