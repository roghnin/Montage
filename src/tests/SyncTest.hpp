#ifndef SYNCTEST_HPP
#define SYNCTEST_HPP

#include "MapChurnTest.hpp"
#include "Recoverable.hpp"

template <class K, class V>
class MapSyncTest: public MapChurnTest<K,V>{
public:
    Recoverable* rec;
    int ps;
    int range;

    MapSyncTest(int p_gets, int p_puts, int p_inserts, int p_removes, 
        int p_sync, int range, int prefill):
        MapChurnTest<K,V>(p_gets, p_puts, p_inserts, p_removes, range, prefill), ps(p_sync), range(range){}
    MapSyncTest(int p_gets, int p_puts, int p_inserts, int p_removes, 
        int p_sync, int range):
        MapChurnTest<K,V>(p_gets, p_puts, p_inserts, p_removes, range), ps(p_sync), range(range){}
    
    void init(GlobalTestConfig* gtc){
        MapChurnTest<K,V>::init(gtc);
        rec = dynamic_cast<Recoverable*>(this->getRideable());
        if (!rec){
            errexit("MapSyncTest must be run on Recoverable type object.");
        }
    }
    void parInit(GlobalTestConfig* gtc, LocalTestConfig* ltc){
        MapChurnTest<K,V>::parInit(gtc, ltc);
    }
    int execute(GlobalTestConfig* gtc, LocalTestConfig* ltc){
        auto time_up = gtc->finish;
	
        int ops = 0;
        uint64_t r = ltc->seed;
        std::mt19937_64 gen_k(r);
        std::mt19937_64 gen_p(r+1);
        std::mt19937_64 gen_s(r+2);

        int tid = ltc->tid;

        // atomic_thread_fence(std::memory_order_acq_rel);
        //broker->threadInit(gtc,ltc);
        auto now = std::chrono::high_resolution_clock::now();

        while(std::chrono::duration_cast<std::chrono::microseconds>(time_up - now).count()>0){

            r = abs((long)gen_k()%range);
            // r = abs(rand_nums[(k_idx++)%1000]%range);
            int p = abs((long)gen_p()%100);
            // int p = abs(rand_nums[(p_idx++)%1000]%100);
            
            this->operation(r, p, tid);

            int s = abs((long)gen_s()%100);
            if (s < ps){
                rec->sync();
            }
            
            ops++;
            if (ops % 512 == 0){
                now = std::chrono::high_resolution_clock::now();
            }
            // TODO: replace this with __rdtsc
            // or use hrtimer (high-resolution timer API in linux.)
        }
        return ops;
    }
};

#endif