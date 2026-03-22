#include "Mad-RHI/Common.h"

namespace mad::rhi {

// RefCounter

void RefCounter::AddStrongRef()
{
    m_Strong.fetch_add(1, std::memory_order_relaxed);
}

bool RefCounter::ReleaseStrongRef()
{
    int prev = m_Strong.fetch_sub(1, std::memory_order_acq_rel);
    assert(prev > 0);
    if (prev == 1)
    {
        DestroyObject();
        if (m_Weak.load(std::memory_order_acquire) == 0)
            delete this;
        return true;
    }

    return false;
}

void RefCounter::AddWeakRef()
{
    m_Weak.fetch_add(1, std::memory_order_relaxed);
}

void RefCounter::ReleaseWeakRef()
{
    int prev = m_Weak.fetch_sub(1, std::memory_order_acq_rel);
    assert(prev > 0);
    if (prev == 1 && m_Strong.load(std::memory_order_acquire) == 0)
        delete this;
}

bool RefCounter::TryAddStrongRef()
{
    int count = m_Strong.load(std::memory_order_relaxed);
    while (count > 0)
    {
        if (m_Strong.compare_exchange_weak(count, count + 1,
                std::memory_order_acq_rel, std::memory_order_relaxed))
            return true;
    }
    return false;
}

void RefCounter::DestroyObject()
{
    if (m_pObject)
    {
        Object* obj = m_pObject;
        m_pObject = nullptr;
        delete obj;
    }
}

}