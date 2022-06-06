#pragma once

#include <memory>
#include <mutex>
#include <thread>

#include <kx/kx.h>

namespace qbind {

// forward
void initialise();

namespace memory {

/**
 * @brief Control threading specific memory options.
 *
 * Implements memory functions:
 *  - V m9(V)
 *  - I setm(I).
 * Also implements memory-system initialisation using:
 *  - I khp(S, I);
 */
class ThreadMemoryManager
{
public:
    ~ThreadMemoryManager()
    {
        std::lock_guard<std::mutex> lk(m_mutex);
        m_active_managers -= 1;

        // turn symbol lock off when at one manager
        if (m_active_managers == 1)
            setm(0);

        // release the memory allocated for the threads pool
        m9();

        // std::cout << "Main thread " << *m_main_thread << std::endl;
        // std::cout << "Destructing Memory Manager " << std::this_thread::get_id() << std::endl;
        // std::cout << "Will be " << m_active_managers << " left" << std::endl;
    }

private:
    ThreadMemoryManager()
    {
        std::lock_guard<std::mutex> lk(m_mutex);
        if (!m_main_thread)
            m_main_thread = std::make_unique<std::thread::id>(std::this_thread::get_id());
        m_active_managers += 1;

        // TODO: Can this be global or does it need to be thread local?
        // Initialise the memory system.
        khp("", -1);

        // turn symbol lock on when at two managers
        if (m_active_managers == 2)
            setm(1);

        // std::cout << "Main thread " << *m_main_thread << std::endl;
        // std::cout << "Constructing Memory Manager " << std::this_thread::get_id() << std::endl;
        // std::cout << "Constructed " << m_active_managers << " manager" << std::endl;
    }

    static std::unique_ptr<std::thread::id> m_main_thread;
    static size_t m_active_managers;
    static std::mutex m_mutex;

    friend void qbind::initialise();
};

std::unique_ptr<std::thread::id> ThreadMemoryManager::m_main_thread;
size_t ThreadMemoryManager::m_active_managers;
std::mutex ThreadMemoryManager::m_mutex;

} // qbind::memory

// Initialise the memory system once and only once per thread.
// Ensures the de-initialisation on thread termination too.
void initialise()
{
    thread_local qbind::memory::ThreadMemoryManager mm;
}

} // qbind
