#ifndef __AFS_UTIL_H__
#define __AFS_UTIL_H__

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sched.h>
#include <assert.h>
#include <sys/stat.h>

#define MB (1024*1024)
#define GB (1024*1024*1024)

#define BYTESIZE 8

#define DEBUG false

#define IS_CLICK false // Kosarak, Retail

#define IS_SYNTHETIC false // synthetic
#define INTERCEPT 0.1
#define FLOWNUM 100000
#define SLOPE 3.56 // skewness: 1000
//#define SLOPE 2.1 // skewness: 100
//#define SLOPE 1.12 // skewness: 10
//#define SLOPE 0.848 // skewness: 1

//#define TRACE_PATH "./data/retail/100000ms/records.dat"
//#define NEXT_TRACE_PATH "./data/retail/100000ms/records.dat" // do not support heavy changer

//#define TRACE_PATH "./data/kosarak/100000ms/records.dat"
//#define NEXT_TRACE_PATH "./data/kosarak/100000ms/records.dat" // do not support heavy changer

// TRACE_PATH is related with epoch length
#define TRACE_PATH "./data/caida2018/2500ms/split_0.bin"
#define NEXT_TRACE_PATH "./data/caida2018/2500ms/split_1.bin"

//#define TRACE_PATH "./data/imc2010univ1/100000ms/split_0.bin"
//#define NEXT_TRACE_PATH "./data/imc2010univ1/100000ms/split_1.bin"

//#define TRACE_PATH "./data/imc2010univ2/100000ms/split_0.bin"
//#define NEXT_TRACE_PATH "./data/imc2010univ2/100000ms/split_1.bin"

//#define TRACE_PATH "./data/caida2018/5ms/split_0.bin"
//#define NEXT_TRACE_PATH "./data/caida2018/5ms/split_1.bin"

//#define TRACE_PATH "./data/caida2018/100ms/split_0.bin"
//#define NEXT_TRACE_PATH "./data/caida2018/100ms/split_1.bin"

//#define HH_THRESHOLD 1150
//#define HC_THRESHOLD 108
#define HH_THRESHOLD 12
#define HC_THRESHOLD 2
#define DDOS_RATIO 0.25
#define MRAC_MAXK 5
#define MRAC_MAXNUM 5

#define deleteptr(x) \
	do { \
		delete x; \
		x = NULL; \
	} while (0)

// Branch predict
#define afs_likely(x) __builtin_expect ((x), 1)
#define afs_unlikely(x) __builtin_expect ((x), 0)

// ##: connection (macro will not expand), e.g. f(n) x##n; f(1) -> x1
// #: add "", e.g. f(n) #n; f(123) -> "123"
// #@: add '', e.g. f(n) #@n; f(1) -> '1'
// "" __VA_ARGS__: like ## __VA_ARGS__, avoid __VA_ARGS__ is empty
#define afs_assert(x, ...) \
    do {\
        if (afs_unlikely(!(x))) {\
            fprintf (stderr, "Assertion failed at %s (%s:%d): ", #x, \
                __FILE__, __LINE__);\
            fprintf (stderr, "" __VA_ARGS__);\
            fflush(stderr);\
            exit(EXIT_FAILURE);\
        }\
    } while (0) 

// do while(0): add scope manually
#define LOG_MSG(...) \
    do { \
        fprintf(stderr, "" __VA_ARGS__); \
        fflush(stderr);\
    } while (0)
#define LOG_WARN(...) \
    do { \
        fprintf(stderr, "[WARN] " __VA_ARGS__); \
        fflush(stderr);\
    } while(0)
#define LOG_ERR(...) \
    do { \
        fprintf(stderr, "[ERROR] "  __VA_ARGS__); \
        exit(EXIT_FAILURE); \
    } while (0)

#ifdef DEBUG
#define LOG_DEBUG(...) \
    do { \
        fprintf(stderr, "[DEBUG]" __VA_ARGS__); \
        fflush(stderr); \
    } while (0)
#else
#define LOG_DEBUG(...)
#endif

static inline uint64_t now_us ()
{
    //  Use POSIX gettimeofday function to get precise time.
    struct timeval tv;
    gettimeofday (&tv, NULL);
    return (tv.tv_sec * (uint64_t) 1000000 + tv.tv_usec);
}

static inline void *stopwatch_start ()
{
    uint64_t *watch = (uint64_t*) malloc (sizeof (uint64_t));
    *watch = now_us ();
    return (void*) watch;
}

static inline unsigned long stopwatch_stop (void *watch_)
{
    uint64_t end = now_us ();
    uint64_t start = *(uint64_t*) watch_;
    free (watch_);
    return (unsigned long) (end - start);
}

static inline uint32_t abs_minus(uint32_t a, uint32_t b) {
	if (a > b) {
		return a - b;
	}
	else {
		return b - a;
	}
}

FILE* fopen_with_dirs(const char* filename, const char* mode);

//#define CPU_CORE 16
//static void pin_to_cpu(int cpu) {
//	cpu_set_t cpuset;	// set of CPUs allocating to the LVRM
//
//    /*
//	if (getuid()) {
//        // exit if user is not root
//        LOG_ERR("Root required\n");
//		return;
//	}
//    */
//
//	CPU_ZERO(&cpuset);
//    CPU_SET(cpu, &cpuset);
//    if (pthread_setaffinity_np(pthread_self(), sizeof(cpuset), &cpuset) < 0) {
//        fprintf(stderr, "set thread affinity failed\n");
//    }
//    cpu_set_t get;
//    CPU_ZERO(&get);
//    if (pthread_getaffinity_np(pthread_self(), sizeof(get), &get) < 0) {
//        fprintf(stderr, "get thread affinity failed\n");
//    }
//    for (int j = 0; j < CPU_CORE; j++) {
//        if (CPU_ISSET(j, &get)) {
//            LOG_MSG("    running in processor %d\n", j);
//        }
//    }
//}

#endif // AFS_UTIL_HPP_
