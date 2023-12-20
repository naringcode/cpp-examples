#pragma once

#include <atomic>

extern thread_local uint32_t tls_threadID;

// W -> W -> W (연속되는 Write OK)
// W -> R (OK)
// R -> W (NO)
class RWLock
{
public:
    enum : uint64_t
    {
        kMaximumTimeWait  = 10'000,
        kMaximumSpinCount = 5000,

        // 상위 32비트는 소유자 ID
        kWriteOwnerMask = 0xFFFF'FFFF'0000'0000,

        // 하위 32비트는 Read의 공유 상태
        kReadSharedCountMask = 0x0000'0000'FFFF'FFFF,

        kEmptyState = 0x0000'0000'0000'0000
    };

public:
    RWLock() = default;
    ~RWLock() = default;

public:
    RWLock(const RWLock&) = delete;
    RWLock& operator=(const RWLock&) = delete;

    RWLock(RWLock&&) = delete;
    RWLock& operator=(RWLock&&) = delete;

public:
    void WriteLock();
    void WriteUnlock();

    void ReadLock();
    void ReadUnlock();

private:
    std::atomic<uint64_t> _lockState = kEmptyState;
    uint64_t              _writeCnt  = 0;
};

class ReadLockGuard
{
public:
    ReadLockGuard(RWLock& lock) : _lock(lock) { _lock.ReadLock(); }
    ~ReadLockGuard() { _lock.ReadUnlock(); }

private:
    RWLock&  _lock;
};

class WriteLockGuard
{
public:
    WriteLockGuard(RWLock& lock) : _lock(lock) { _lock.WriteLock(); }
    ~WriteLockGuard() { _lock.WriteUnlock(); }

private:
    RWLock& _lock;
};
