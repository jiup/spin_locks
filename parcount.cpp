#include <iostream>
#include <cstring>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>
#include <stdatomic.h>

class lock;

class cpp_mutex_lock;

class tas_lock;

class eback_tas_lock;

class ticket_lock;

class pback_ticket_lock;

class mcs_lock;

class k42_mcs_lock;

class clh_lock;

class k42_clh_lock;

void set_affinity(pthread_t &thread, int i, int cores, int step);

void run_tests(int t_cnt, int iter_cnt, int cores, int step);

void test(lock &lock, int t_cnt, int iter_cnt, int cores, int step);

class lock {
public:
    virtual void acquire() = 0;

    virtual void release() = 0;
};

// C++ mutex
class cpp_mutex_lock : public lock {
private:
    std::mutex lock;

public:
    cpp_mutex_lock() = default;

    cpp_mutex_lock(cpp_mutex_lock const &other) {}

    void acquire() override {
        lock.lock();
    }

    void release() override {
        lock.unlock();
    }
};

// naive TAS lock
class tas_lock : public lock {
private:
    std::atomic_flag f = ATOMIC_FLAG_INIT;

public:
    tas_lock() = default;

    tas_lock(tas_lock const &other) {}

    void acquire() override {
        while (f.test_and_set());
    }

    void release() override {
        f.clear();
    }
};

// TAS lock with well-tuned exponential backoff
// you’ll need to experiment with different base, multiplier, and cap values
// todo
class eback_tas_lock : public lock {
public:
    eback_tas_lock() = default;

    eback_tas_lock(eback_tas_lock const &other) {}

    void acquire() override {}

    void release() override {}
};

// naive ticket lock
// todo
class ticket_lock : public lock {
public:
    ticket_lock() = default;

    ticket_lock(ticket_lock const &other) {}

    void acquire() override {}

    void release() override {}
};

// ticket lock with well-tuned proportional backoff
// todo
class pback_ticket_lock : public lock {
public:
    pback_ticket_lock() = default;

    pback_ticket_lock(pback_ticket_lock const &other) {}

    void acquire() override {}

    void release() override {}
};

// MCS lock
// todo
class mcs_lock : public lock {
public:
    mcs_lock() = default;

    mcs_lock(mcs_lock const &other) {}

    void acquire() override {}

    void release() override {}
};

// "K42" MCS lock with standard interface
// todo
class k42_mcs_lock : public lock {
public:
    k42_mcs_lock() = default;

    k42_mcs_lock(k42_mcs_lock const &other) {}

    void acquire() override {}

    void release() override {}
};

// CLH lock
// todo
class clh_lock : public lock {
public:
    clh_lock() = default;

    clh_lock(clh_lock const &other) {}

    void acquire() override {}

    void release() override {}
};

// “K42” CLH lock
// todo
class k42_clh_lock : public lock {
public:
    k42_clh_lock() = default;

    k42_clh_lock(k42_clh_lock const &other) {}

    void acquire() override {}

    void release() override {}
};


int main(int argc, char *argv[]) {
    int t_cnt = 4, iter_cnt = 10000, cores = 0, step = 1;

    for (int i = 1; i < argc - 1; i++) {
        try {
            if (!strcmp(argv[i], "-t")) {
                t_cnt = std::stoi(argv[++i]);
            } else if (!strcmp(argv[i], "-i")) {
                iter_cnt = std::stoi(argv[++i]);
            } else if (!strcmp(argv[i], "-c")) {
                cores = std::stoi(argv[++i]);
                if (cores < 0) cores = 0;
            } else if (!strcmp(argv[i], "-s")) {
                step = std::stoi(argv[++i]);
                if (step < 0) step = 1;
            }
        } catch (const std::exception &e) {
            std::cerr << "ERR: invalid input.";
            std::cerr << " (" << e.what() << ")\n";
            return -1;
        }
    }
    std::cout << "[-t] number of threads:    " << t_cnt << std::endl;
    std::cout << "[-i] number of iterations: " << iter_cnt << std::endl;
    if (cores > 0) {
        int hardware_concurrency = std::thread::hardware_concurrency();
        if (cores > hardware_concurrency / step) cores = hardware_concurrency / step;
        std::cout << "[-c] cpu affinity mask:    " << cores << "/";
        std::cout << hardware_concurrency << std::endl;
        std::cout << "[-s] scattered on core:    ";
        for (int i = 0; i < cores; i++) std::cout << i * step << " ";
        std::cout << std::endl;
    }
    std::cout << std::endl;
    run_tests(t_cnt, iter_cnt, cores, step);
    return 0;
}

void set_affinity(pthread_t &thread, int __unused i, const int __unused cores, const int __unused step) {
#ifdef __linux__
    if (cores == 0) return;
    cpu_set_t c_set;
    CPU_ZERO(&c_set);
    CPU_SET(step * i % cores, &c_set);
    if (pthread_setaffinity_np(thread, sizeof(cpu_set_t), &c_set)) {
        std::cerr << "cannot set affinity of thread#" << i;
        std::cerr << " to core #" << (i % cores) << std::endl;
    }
#endif
}

void run_tests(int t_cnt, int iter_cnt, const int cores, const int step) {
    std::cout << "running test_cpp_mutex..." << std::endl;
    auto cpp_mutex = cpp_mutex_lock();
    test(cpp_mutex, t_cnt, iter_cnt, cores, step);

    std::cout << "\nrunning test_naive_tas..." << std::endl;
    auto tas = tas_lock();
    test(tas, t_cnt, iter_cnt, cores, step);

    std::cout << "\nrunning test_eback_tas..." << std::endl;
    auto eback_tas = eback_tas_lock();
    test(eback_tas, t_cnt, iter_cnt, cores, step);

    std::cout << "\nrunning test_naive_ticket..." << std::endl;
    auto ticket = ticket_lock();
    test(ticket, t_cnt, iter_cnt, cores, step);

    std::cout << "\nrunning test_pback_ticket..." << std::endl;
    auto pback_ticket = pback_ticket_lock();
    test(pback_ticket, t_cnt, iter_cnt, cores, step);

    std::cout << "\nrunning test_mcs..." << std::endl;
    auto mcs = mcs_lock();
    test(mcs, t_cnt, iter_cnt, cores, step);

    std::cout << "\nrunning test_k42_mcs..." << std::endl;
    auto k42_mcs = k42_mcs_lock();
    test(k42_mcs, t_cnt, iter_cnt, cores, step);

    std::cout << "\nrunning test_clh..." << std::endl;
    auto clh = clh_lock();
    test(clh, t_cnt, iter_cnt, cores, step);

    std::cout << "\nrunning test_k42_clh..." << std::endl;
    auto k42_clh = k42_clh_lock();
    test(k42_clh, t_cnt, iter_cnt, cores, step);
}

void test(::lock &lock, const int t_cnt, const int iter_cnt, const int cores, const int step) {
    struct t_state {
        ::lock *lock;
        std::atomic_bool *start;
        int *counter;
        int iter_cnt;
    };
    pthread_t threads[t_cnt];
    int counter = 0;
    std::atomic_bool start;
    t_state params = {&lock, &start, &counter, iter_cnt};
    for (int i = 0; i < t_cnt; i++) {
        pthread_create(&(threads[i]), nullptr, [](void *args) -> void * {
            auto *state = static_cast<struct t_state *>(args);
            std::atomic_bool *trigger = state->start;
            int *cnt = state->counter, it_cnt = state->iter_cnt;
            while (!trigger->load());
            for (int t = 0; t < it_cnt; t++) {
                state->lock->acquire();
                *cnt = *cnt + 1;
                state->lock->release();
            }
            return nullptr;
        }, &params);
        set_affinity(threads[i], i, cores, step);
    }
    start = true;
    auto start_t = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < t_cnt; i++) {
        pthread_join(threads[i], nullptr);
    }
    auto end_t = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> diff = end_t - start_t;
    std::cout << "result of count: " << counter << std::endl;
    std::cout << "completed in " << diff.count() << " ms" << std::endl;
}