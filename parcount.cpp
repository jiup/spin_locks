#include <iostream>
#include <cstring>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>

void set_affinity(pthread_t &thread, int i, int cores, int step);

void run_tests(int t_cnt, int iter_cnt, int cores, int step);

void test_cpp_mutex(int t_cnt, int iter_cnt, int cores, int step);

void test_naive_tas(int t_cnt, int iter_cnt, int cores, int step);

void test_eback_tas(int t_cnt, int iter_cnt, int cores, int step);

void test_naive_ticket(int t_cnt, int iter_cnt, int cores, int step);

void test_pback_ticket(int t_cnt, int iter_cnt, int cores, int step);

void test_mcs(int t_cnt, int iter_cnt, int cores, int step);

void test_k42_mcs(int t_cnt, int iter_cnt, int cores, int step);

void test_clh(int t_cnt, int iter_cnt, int cores, int step);

void test_k42_clh(int t_cnt, int iter_cnt, int cores, int step);


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

void set_affinity(pthread_t &thread, int i, const int cores, const int step) {
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
    test_cpp_mutex(t_cnt, iter_cnt, cores, step);
    std::cout << "\nrunning test_naive_tas..." << std::endl;
    test_naive_tas(t_cnt, iter_cnt, cores, step);
    std::cout << "\nrunning test_eback_tas..." << std::endl;
    test_eback_tas(t_cnt, iter_cnt, cores, step);
    std::cout << "\nrunning test_naive_ticket..." << std::endl;
    test_naive_ticket(t_cnt, iter_cnt, cores, step);
    std::cout << "\nrunning test_pback_ticket..." << std::endl;
    test_pback_ticket(t_cnt, iter_cnt, cores, step);
    std::cout << "\nrunning test_mcs..." << std::endl;
    test_mcs(t_cnt, iter_cnt, cores, step);
    std::cout << "\nrunning test_k42_mcs..." << std::endl;
    test_k42_mcs(t_cnt, iter_cnt, cores, step);
    std::cout << "\nrunning test_clh..." << std::endl;
    test_clh(t_cnt, iter_cnt, cores, step);
    std::cout << "\nrunning test_k42_clh..." << std::endl;
    test_k42_clh(t_cnt, iter_cnt, cores, step);
}

// a C++ mutex
void test_cpp_mutex(const int t_cnt, const int iter_cnt, const int cores, const int step) {
    struct t_state {
        std::mutex *lock;
        std::atomic_bool *start;
        int *counter;
        int iter_cnt;
    };
    pthread_t threads[t_cnt];
    int counter = 0;
    std::mutex lock;
    std::atomic_bool start;
    t_state params = {&lock, &start, &counter, iter_cnt};
    for (int i = 0; i < t_cnt; i++) {
        pthread_create(&(threads[i]), nullptr, [](void *args) -> void * {
            auto *state = static_cast<struct t_state *>(args);
            std::mutex *l = state->lock;
            std::atomic_bool *trigger = state->start;
            int *cnt = state->counter, it_cnt = state->iter_cnt;
            while (!trigger->load());
            for (int t = 0; t < it_cnt; t++) {
                l->lock();
                *cnt = *cnt + 1;
                l->unlock();
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

// a naive TAS lock
void test_naive_tas(const int t_cnt, const int iter_cnt, const int cores, const int step) {
    // todo
}

// a TAS lock with well-tuned exponential backoff (you’ll need to experiment with different base, multiplier, and cap values)
void test_eback_tas(const int t_cnt, const int iter_cnt, const int cores, const int step) {
    // todo
}

// a naive ticket lock
void test_naive_ticket(const int t_cnt, const int iter_cnt, const int cores, const int step) {
    // todo
}

// a ticket lock with well-tuned proportional backoff
void test_pback_ticket(const int t_cnt, const int iter_cnt, const int cores, const int step) {
    // todo
}

// an MCS lock
void test_mcs(const int t_cnt, const int iter_cnt, const int cores, const int step) {
    // todo
}

// a "K42" MCS lock (with standard interface)
void test_k42_mcs(const int t_cnt, const int iter_cnt, const int cores, const int step) {
    // todo
}

// a CLH lock
void test_clh(const int t_cnt, const int iter_cnt, const int cores, const int step) {
    // todo
}

// a “K42” CLH lock
void test_k42_clh(const int t_cnt, const int iter_cnt, const int cores, const int step) {
    // todo
}