//mineutils库的线程相关工具
//注：如果为qnx6.6平台编译，可能需要为编译器开启宏: _GLIBCXX_USE_NANOSLEEP
#pragma once
#ifndef THREAD_HPP_MINEUTILS
#define THREAD_HPP_MINEUTILS

#include<atomic>
#include<condition_variable>
#include<functional>
#include<future>
#include<map>
#include<mutex>
#include<queue>
#include<stdexcept>
#include<thread>
#include<vector>

#include"base.hpp"
#include"type.hpp"

namespace mineutils
{
    /*--------------------------------------------用户接口--------------------------------------------*/

    namespace mthrd
    {
        //简易自旋锁，适用于临界区操作非常少的情况，线程安全
        class SpinLock
        {
        private:
            class Guard;

        public:
            SpinLock() = default;

            void lock();
            void unlock();

            /*  使用RAII方式对局部区域加锁
                - 用法：auto guard = spin_lock.lockGuard();
                @return 一个非线程安全的私有类RGuard对象，只能用auto推导；构造时加锁，调用release函数或析构时解锁  */
            SpinLock::Guard lockGuard();

            SpinLock(const SpinLock& tmp_lock) = delete;
            SpinLock& operator=(const SpinLock& tmp_lock) = delete;

        private:
            std::atomic_flag lock_flag_ = ATOMIC_FLAG_INIT;
        };

        //基于mutex实现的读写锁，线程安全
        class ReadWriteMutex
        {
        private:
            class RGuard;
            class WGuard;

        public:
            ReadWriteMutex() = default;

            void lockRead();
            void unlockRead();

            /*  使用RAII方式对局部区域加读锁
                - 用法：auto guard = rw_lock.lockReadGuard();
                @return 一个非线程安全的私有类RGuard对象，只能用auto推导；构造时加锁，调用release函数或析构时解锁  */
            ReadWriteMutex::RGuard lockReadGuard();

            void lockWrite();
            void unlockWrite();

            /*  使用RAII方式对局部区域加写锁
                - 用法：auto guard = rw_lock.lockWriteGuard();
                @return 一个非线程安全的私有类RGuard对象，只能用auto推导；构造时加锁，调用release函数或析构时解锁  */
            ReadWriteMutex::WGuard lockWriteGuard();

            //禁止拷贝和移动
            ReadWriteMutex(const ReadWriteMutex& tmp_lock) = delete;
            ReadWriteMutex& operator=(const ReadWriteMutex& tmp_lock) = delete;

        private:
            std::mutex mtx_;
            std::condition_variable cv_;
            unsigned int num_readers_ = 0;
            bool is_writing_ = false;
        };

        class ThreadPool;

        //任务的future，Ret必须为void或可拷贝构造的类型，且不可为引用类型
        /*  任务的future
            - Ret不可为引用类型
            - Ret必须为void或可拷贝构造的类型
            - 由于编译器的设定，对于非引用的基本类型，Ret不保留顶层cv修饰（如对于返回const int的函数应使用TaskFuture<int>） */
        template<class Ret>
        class TaskFuture
        {
        public:
            //构造一个无效的TaskFuture对象
            template<class RetU = Ret, typename std::enable_if<std::is_same<RetU, Ret>::value && !std::is_reference<RetU>::value && (std::is_void<RetU>::value || std::is_copy_constructible<RetU>::value), int>::type = 0>
            TaskFuture() {};

            //判断任务是否为有效状态；线程安全
            bool valid() const;
            //判断任务是否结束，如果任务为无效状态会返回true；线程安全
            bool finished() const;
            //等待任务结束，如果任务为无效状态会立即返回；线程安全
            void wait() const;

            /*  等待并获取任务结果的指针，避免抛出异常
                - 线程安全
                - 任务无效时返回nullptr
                - Ret为void时也返回nullptr
                - 返回的指针在TaskFuture对象更新或析构时失效
                @return 任务返回值的指针  */
            const Ret* getPtr() const;
            //导出标准库future
            std::shared_future<Ret> toFuture() const;

            //支持移动禁止拷贝
            TaskFuture(TaskFuture<Ret>&& task_future) noexcept;
            TaskFuture& operator=(TaskFuture<Ret>&& task_future) noexcept;

        private:
            template<class RetU, typename std::enable_if<std::is_void<RetU>::value, int>::type = 0>
            const RetU* getPtrDispatch() const;
            template<class RetU, typename std::enable_if<!std::is_void<RetU>::value, int>::type = 0>
            const RetU* getPtrDispatch() const;

#if defined(__GNUC__) && !_mgccMinVersion(4, 8, 1)  //for qnx660
            mutable std::shared_future<Ret> future_state_;
#else
            std::shared_future<Ret> future_state_;
#endif
            friend class mthrd::ThreadPool;

        public:
            //等待并获取任务结果，如果任务为无效状态会抛出std::runtime_error异常；线程安全
            _mdeprecated(R"(Deprecated! Please use mem_function "tryGet" instead.)") Ret get();
        };


        //简易线程池，在rv1126上执行一个任务大概会引入接近200us的时间开销
        class ThreadPool
        {
        public:
            /*  构造ThreadPool对象
                @param pool_size: 线程池线程数量，不小于1  */
            ThreadPool(uint32_t pool_size);

            /*  添加一个任务到线程池中并异步执行(会拷贝所有输入用于储存)，规则涵盖std::bind的要求且更严格；线程安全
                推荐用法:
                - addTask(function or &function, args...)
                - addTask(&class::mem_function, &class_obj, args...)
                - addTask(&class::static_mem_function, args...)
                - addTask(functor, args...)
                注意，经测试QNX的g++4.7.3对C++11特性支持不全，以下情况可能直接在模板内部编译错误而非触发SFINAE特性：
                - 仿函数作为Fn，但被类似std::reference_wrapper的第三方引用包装传递时
                - 仿函数作为Fn，但匹配Args...的operator()为私有或受保护的成员时

                @param func: 任务函数。
                    - 任务函数的返回类型Ret可以为void
                    - Ret不为void时，必须支持拷贝构造，且不可为引用
                    - func为函数对象时，自身的去引用类型必须支持使用自身的左值和右值对象进行构造，且不可为volatile类型
                    - func为函数对象时，operator=的CV限定符必须与自身限定一致
                    - func为成员函数时，CV限定符必须与它的对象实例一致
                @param args...: 任务函数的参数。
                    - 需要左值引用传递的参数必须用std::ref或std::cref显式引用否则实际为值传递
                    - 非C数组参数和非std::ref或std::cref传递的参数，它的去引用类型必须支持使用自身的左值和右值对象进行构造
                @return 任务结果的future状态，用于查询任务状态、等待任务结束以及获取任务返回值，注意ThreadPool对象析构后未执行的任务TaskFuture失效  */
            template<class Fn, class... Args, class Ret = typename mtype::StdBindTraits<Fn, Args...>::ReturnType, typename std::enable_if<std::is_same<Ret, typename mtype::StdBindTraits<Fn, Args...>::ReturnType>::value && !std::is_reference<Ret>::value && (std::is_void<Ret>::value || std::is_copy_constructible<Ret>::value), int>::type = 0>
            TaskFuture<Ret> addTask(Fn&& func, Args&&... args);

            //是否线程池当前已占满
            bool full();

            //禁止拷贝和移动
            ThreadPool(const ThreadPool& thd_pool) = delete;
            ThreadPool& operator=(const ThreadPool& thd_pool) = delete;
            //执行完当前正在执行的任务，放弃队列里剩余的任务，释放资源
            ~ThreadPool();

        private:
            void worker();

            uint32_t pool_size_;
            std::queue<std::function<void()>> task_queue_;
            std::vector<std::thread> work_thds_;
            std::atomic<uint32_t> working_task_num_{ 0 };

            std::mutex task_mtx_;
            std::condition_variable cond_var_;
            std::atomic<bool> need_abort_;
        };



        //跨线程暂停，使用条件变量实现以代替循环sleep
        class ThreadPauser
        {
        public:
            ThreadPauser();

            /*  设置暂停点
                - 收到暂停信号后，线程会在此处暂停等待
                - 收到继续信号后，线程会解除等待
                - 线程安全
                @param point_id: 用户指定每个暂停点的id，不应重复  */
            void setPausePoint(uint8_t point_id);

            //发出暂停信号。线程安全
            void pause();

            //发出继续信号。线程安全
            void resume();

            //获取当前某个暂停点是否已进入暂停状态；线程安全
            bool isPaused(uint8_t point_id);

            //禁止拷贝和移动
            ThreadPauser(const ThreadPauser& tmp) = delete;
            ThreadPauser& operator=(const ThreadPauser& tmp) = delete;

            ~ThreadPauser();

        private:
            std::mutex mtx_;
            std::condition_variable cond_;
            SpinLock splk_;
            std::atomic<bool> need_pause_;
            std::unordered_map<uint8_t, bool> state_map_;
        };
    }










    /*--------------------------------------------内部实现--------------------------------------------*/

    namespace mthrd
    {
        class SpinLock::Guard
        {
        public:
            //只允许移动构造，不允许移动赋值和拷贝
            Guard(Guard&& tmp) noexcept
            {
                this->resource_ = tmp.resource_;
                tmp.resource_ = nullptr;
            }

            ~Guard()
            {
                if (this->resource_)
                    this->resource_->unlock();
            }

            void release()
            {
                if (this->resource_)
                    this->resource_->unlock();
                this->resource_ = nullptr;
            }
        private:
            Guard(SpinLock* resource)
            {
                this->resource_ = resource;
                this->resource_->lock();
            }

            SpinLock* resource_ = nullptr;
            friend SpinLock;
        };

        inline void SpinLock::lock()
        {
            int spin_count = 0;
            const int max_spin_count = 1000; // 设置合理的最大自旋次数
            while (this->lock_flag_.test_and_set(std::memory_order_acquire))
            {
                ++spin_count;
                if (spin_count > 1000)
                {
#if defined(__GNUC__) && !_mgccMinVersion(4, 8, 1)  //for qnx660
                    std::this_thread::sleep_for(std::chrono::nanoseconds(1));
#else 
                    std::this_thread::yield(); // 自旋次数过多，让出CPU
#endif 
                    spin_count = 0;
                }
            }
        }

        inline void SpinLock::unlock()
        {
            this->lock_flag_.clear(std::memory_order_release);
        }

        inline SpinLock::Guard SpinLock::lockGuard()
        {
            return SpinLock::Guard(this);
        }

        //class ReadWriteMutex::RGuard
        //{
        //public:
        //    RGuard(RGuard&& tmp) noexcept
        //    {
        //        //只会在同一个线程执行，不需要太强的内存屏障
        //        this->resource_.store(tmp.resource_.load(std::memory_order_relaxed), std::memory_order_relaxed);
        //        tmp.resource_.store(nullptr, std::memory_order_relaxed);
        //    }
        //    ~RGuard()
        //    {
        //        ReadWriteMutex* ptr;
        //        if (ptr = this->resource_.load(std::memory_order_acquire), ptr)
        //            ptr->unlockRead();
        //    }
        //    void release()
        //    {
        //        ReadWriteMutex* ptr;
        //        if (ptr = this->resource_.exchange(nullptr, std::memory_order_acq_rel), ptr)
        //            ptr->unlockRead();
        //    }
        //private:
        //    RGuard(ReadWriteMutex* resource)
        //    {
        //        this->resource_.store(resource, std::memory_order_release);
        //        this->resource_.load(std::memory_order_acquire)->lockRead();
        //    }

        //    std::atomic<ReadWriteMutex*> resource_{ nullptr };
        //    friend ReadWriteMutex;
        //};

        class ReadWriteMutex::RGuard
        {
        public:
            //只允许移动构造，不允许移动赋值和拷贝
            RGuard(RGuard&& tmp) noexcept
            {
                this->resource_ = tmp.resource_;
                tmp.resource_ = nullptr;
            }
            ~RGuard()
            {
                if (this->resource_)
                    this->resource_->unlockRead();
            }
            void release()
            {
                if (this->resource_)
                    this->resource_->unlockRead();
                this->resource_ = nullptr;
            }
        private:
            RGuard(ReadWriteMutex* resource)
            {
                this->resource_ = resource;
                this->resource_->lockRead();
            }

            ReadWriteMutex* resource_ = nullptr;
            friend ReadWriteMutex;
        };

        class ReadWriteMutex::WGuard
        {
        public:
            //只允许移动构造，不允许移动赋值和拷贝
            WGuard(WGuard&& tmp) noexcept
            {
                this->resource_ = tmp.resource_;
                tmp.resource_ = nullptr;
            }
            ~WGuard()
            {
                if (this->resource_)
                    this->resource_->unlockWrite();
            }
            void release()
            {
                if (this->resource_)
                    this->resource_->unlockWrite();
                this->resource_ = nullptr;
            }
        private:
            WGuard(ReadWriteMutex* resource)
            {
                this->resource_ = resource;
                this->resource_->lockWrite();
            }

            ReadWriteMutex* resource_ = nullptr;
            friend ReadWriteMutex;
        };

        inline void ReadWriteMutex::lockRead()
        {
            std::unique_lock<std::mutex> lk(this->mtx_);
            while (this->is_writing_)
                this->cv_.wait(lk);
            this->num_readers_++;
        }

        inline void ReadWriteMutex::unlockRead()
        {
            {
                std::lock_guard<std::mutex> lk(this->mtx_);
                this->num_readers_--;
                if (this->num_readers_ != 0)
                    return;
            }
            this->cv_.notify_one();
        }

        inline ReadWriteMutex::RGuard ReadWriteMutex::lockReadGuard()
        {
            return ReadWriteMutex::RGuard(this);
        }

        inline void ReadWriteMutex::lockWrite()
        {
            std::unique_lock<std::mutex> lk(this->mtx_);
            while (this->is_writing_ || this->num_readers_ != 0)
                this->cv_.wait(lk);
            this->is_writing_ = true;
        }

        inline void ReadWriteMutex::unlockWrite()
        {
            {
                std::lock_guard<std::mutex> lk(this->mtx_);
                this->is_writing_ = false;
            }
            this->cv_.notify_all();
        }

        inline ReadWriteMutex::WGuard ReadWriteMutex::lockWriteGuard()
        {
            return ReadWriteMutex::WGuard(this);
        }

        template<class Ret>
        inline TaskFuture<Ret>::TaskFuture(TaskFuture<Ret>&& tmp_state) noexcept
        {
            *this = std::move(tmp_state);
        }

        template<class Ret>
        inline TaskFuture<Ret>& TaskFuture<Ret>::operator=(TaskFuture<Ret>&& tmp_state) noexcept
        {
            this->future_state_ = std::move(tmp_state.future_state_);
            return *this;
        }

        template<class Ret>
        inline bool TaskFuture<Ret>::valid() const
        {
            return this->future_state_.valid();
        }

        template<class Ret>
        inline bool TaskFuture<Ret>::finished() const
        {
            if (this->future_state_.valid())
                return this->future_state_.wait_for(std::chrono::nanoseconds(0)) == std::future_status::ready;
            mprintfW("Task is invalid, so the function returns value:true!\n");
            return true;
        }

        template<class Ret>
        inline void TaskFuture<Ret>::wait() const
        {
            if (this->future_state_.valid())
                this->future_state_.wait();
            else mprintfW("Task is invalid, so the function returns directly!\n");
        }

        template<class Ret>
        inline const Ret* TaskFuture<Ret>::getPtr() const
        {
            return this->getPtrDispatch<Ret>();
        }

        template<class Ret>
        inline std::shared_future<Ret> TaskFuture<Ret>::toFuture() const
        {        
            return this->future_state_;
        }

        template<class Ret>
        template<class RetU, typename std::enable_if<std::is_void<RetU>::value, int>::type>
        inline const RetU* TaskFuture<Ret>::getPtrDispatch() const
        {
            return nullptr;
        }

        template<class Ret>
        template<class RetU, typename std::enable_if<!std::is_void<RetU>::value, int>::type>
        inline const RetU* TaskFuture<Ret>::getPtrDispatch() const
        {
            if (this->future_state_.valid())
                return &this->future_state_.get();
            else return nullptr;
        }

        template<class Ret>
        inline Ret TaskFuture<Ret>::get()
        {
            if (this->future_state_.valid())
                return this->future_state_.get();
            else throw std::runtime_error("Error: Task is invalid!");
        }


        inline ThreadPool::ThreadPool(uint32_t pool_size)
        {
            if (pool_size <= 0)
            {
                mprintfW("Invalid param value pool_size:%d, which will be set to 1.\n", pool_size);
                pool_size = 1;
            }
            this->pool_size_ = pool_size;
            std::queue<std::function<void()>> empty_queue;
            this->task_queue_.swap(empty_queue);
            this->need_abort_ = false;

            this->work_thds_.resize(pool_size);
            for (uint32_t i = 0; i < pool_size; ++i)
            {
                this->work_thds_[i] = std::thread(&ThreadPool::worker, this);
            }
        }

        inline ThreadPool::~ThreadPool()
        {
            {
                std::unique_lock<std::mutex> lk(this->task_mtx_);
                this->need_abort_ = true;
                std::queue<std::function<void()>> tmp;
                this->task_queue_.swap(tmp);
            }
            this->cond_var_.notify_all();
            for (auto& thd : this->work_thds_)
            {
                if (thd.joinable())
                    thd.join();
            }
        }

        template<class Fn, class... Args, class Ret, typename std::enable_if<std::is_same<Ret, typename mtype::StdBindTraits<Fn, Args...>::ReturnType>::value && !std::is_reference<Ret>::value && (std::is_void<Ret>::value || std::is_copy_constructible<Ret>::value), int>::type>
        inline TaskFuture<Ret> ThreadPool::addTask(Fn&& func, Args&&... args)
        {
            auto task = std::make_shared<std::packaged_task<Ret()>>(std::bind(std::forward<Fn>(func), std::forward<Args>(args)...));
            TaskFuture<Ret> state;
            state.future_state_ = task->get_future();
            {
                std::lock_guard<std::mutex> lk(this->task_mtx_);
                this->task_queue_.emplace([task]() {(*task)(); });
            }
            this->cond_var_.notify_one();
            return state;
        }

        inline bool ThreadPool::full()
        {
            std::lock_guard<std::mutex> lk(this->task_mtx_);
            return (this->working_task_num_.load(std::memory_order_acquire) + this->task_queue_.size()) >= this->pool_size_;
        }

        inline void ThreadPool::worker()
        {
            std::function<void()> task;
            while (!this->need_abort_)
            {
                {
                    std::unique_lock<std::mutex> lk(this->task_mtx_);
                    while (!this->need_abort_ && this->task_queue_.empty())
                    {
                        this->cond_var_.wait(lk);
                    }
                    if (this->need_abort_)
                        break;
                    task = std::move(this->task_queue_.front());
                    this->task_queue_.pop();
                }
                this->working_task_num_.fetch_add(1, std::memory_order_release);
                task();
                this->working_task_num_.fetch_add(-1, std::memory_order_release);
            }
        }


        inline ThreadPauser::ThreadPauser()
        {
            this->need_pause_.store(false);
        }

        inline ThreadPauser::~ThreadPauser()
        {
            this->resume();
        }

        inline void ThreadPauser::setPausePoint(uint8_t point_id)
        {
            if (this->need_pause_.load(std::memory_order_relaxed))
            {
                std::unique_lock<std::mutex> lk(this->mtx_);
                while (this->need_pause_.load(std::memory_order_relaxed))
                {
                    this->splk_.lock();
                    this->state_map_[point_id] = true;
                    this->splk_.unlock();
                    this->cond_.wait(lk);
                }
            }
            this->splk_.lock();
            this->state_map_[point_id] = false;
            this->splk_.unlock();
        }

        inline void ThreadPauser::pause()
        {
            this->need_pause_.store(true, std::memory_order_relaxed);
        }

        inline void ThreadPauser::resume()
        {
            {
                std::lock_guard<std::mutex> lk(this->mtx_);
                this->need_pause_.store(false, std::memory_order_relaxed);
            }
            this->cond_.notify_all();  //据说notify内部，先加锁cond_var内部锁，再通知，因此不用担心cond_var在lk解锁后进入wait前，notify触发而导致错过
        }

        inline bool ThreadPauser::isPaused(uint8_t point_id)
        {
            {
                auto lk_guard = this->splk_.lockGuard();
                auto it = this->state_map_.find(point_id);
                if (it != this->state_map_.end())
                    return it->second;
            }
            mprintfW("The param point_id not found, so the function returns value:false!\n");
            return false;
        }
    }



#ifdef MINEUTILS_TEST_MODULES
    namespace _mthreadcheck
    {
        inline void SpinLockTest()
        {
            mthrd::SpinLock splk;
            char strs[] = "Hello World";

            bool func1_check_ret = true;
            auto func1 = [&strs, &splk, &func1_check_ret]()
            {
                for (int i = 0; i < 10000; i++)
                {
                    auto guard = splk.lockGuard();
                    strs[0] = 'W';
                    strs[1] = 'o';
                    strs[2] = 'r';
#if !defined(__GNUC__) || (defined(__GNUC__) && _mgccMinVersion(4, 8, 1))  
                    std::this_thread::yield();
#endif
                    strs[3] = 'l';
                    strs[4] = 'd';
                    func1_check_ret = func1_check_ret && (std::string("World World") == strs);
                }
            };

            bool func2_check_ret = true;
            auto func2 = [&strs, &splk, &func2_check_ret]()
            {
                for (int i = 0; i < 10000; i++)
                {
                    auto guard = splk.lockGuard();
                    strs[0] = 'H';
                    strs[1] = 'e';
                    strs[2] = 'l';
#if !defined(__GNUC__) || (defined(__GNUC__) && _mgccMinVersion(4, 8, 1))  
                    std::this_thread::yield();
#endif
                    strs[3] = 'l';
                    strs[4] = 'o';
                    func2_check_ret = func2_check_ret && (std::string("Hello World") == strs);
                }
            };
            std::thread thd1(func1);
            std::thread thd2(func2);
            thd1.join();
            thd2.join();

            if (!(func1_check_ret && func2_check_ret)) mprintfE(R"(Failed when check SpinLock)""\n");
        }

        inline void ReadWriteMutexTest()
        {
            mthrd::ReadWriteMutex rwlk;
            char strs[] = "Hello World";

            bool func1_check_ret = true;
            auto func1 = [&strs, &rwlk, &func1_check_ret]()
            {
                for (int i = 0; i < 10000; i++)
                {
                    auto guard = rwlk.lockReadGuard();
                    strs[0] = 'W';
                    strs[1] = 'o';
                    strs[2] = 'r';
                    strs[3] = 'l';
                    strs[4] = 'd';
                    func1_check_ret = func1_check_ret && (std::string("World World") == strs);
                }
            };

            bool func2_check_ret = true;
            auto func2 = [&strs, &rwlk, &func2_check_ret]()
            {
                for (int i = 0; i < 10000; i++)
                {
                    auto guard = rwlk.lockReadGuard();
                    strs[0] = 'H';
                    strs[1] = 'e';
                    strs[2] = 'l';
                    strs[3] = 'l';
                    strs[4] = 'o';
                    func2_check_ret = func2_check_ret && (std::string("Hello World") == strs);
                }
            };
            {
                std::thread thd1(func1);
                std::thread thd2(func2);
                thd1.join();
                thd2.join();
                //读-读之间不会加锁，因此两个结果中可能存在false
                if (func1_check_ret && func2_check_ret) mprintfW(R"(Maybe failed this time when check: ReadWriteMutexTest Read-Read Lock. Try again!)""\n");
            }

            memcpy(strs, "Hello World", 11);
            bool func3_check_ret = true;
            auto func3 = [&strs, &rwlk, &func3_check_ret]()
            {
                for (int i = 0; i < 10000; i++)
                {
                    auto guard = rwlk.lockWriteGuard();
                    strs[0] = 'W';
                    strs[1] = 'o';
                    strs[2] = 'r';
                    strs[3] = 'l';
                    strs[4] = 'd';
                    func3_check_ret = func3_check_ret && (std::string("World World") == strs);
                }
            };

            bool func4_check_ret = true;
            auto func4 = [&strs, &rwlk, &func4_check_ret]()
            {
                for (int i = 0; i < 10000; i++)
                {
                    auto guard = rwlk.lockWriteGuard();
                    strs[0] = 'H';
                    strs[1] = 'e';
                    strs[2] = 'l';
                    strs[3] = 'l';
                    strs[4] = 'o';
                    func4_check_ret = func4_check_ret && (std::string("Hello World") == strs);
                }
            };

            {
                func1_check_ret = true;
                func4_check_ret = true;
                std::thread thd1(func1);
                std::thread thd4(func4);
                thd1.join();
                thd4.join();

                if (!(func1_check_ret && func4_check_ret)) mprintfE(R"(Failed when check: ReadWriteMutexTest Read-Write Lock)""\n");
            }

            {
                func3_check_ret = true;
                func4_check_ret = true;
                std::thread thd3(func3);
                std::thread thd4(func4);
                thd3.join();
                thd4.join();
                if (!(func3_check_ret && func4_check_ret)) mprintfE(R"(Failed when check: ReadWriteMutexTest Write-Write Lock)""\n");
            }
        }

        inline void ThreadPoolTest()
        {
            mthrd::ThreadPool thread_pool(4);
            mthrd::TaskFuture<void> state1 = thread_pool.addTask([](int x) {std::this_thread::sleep_for(std::chrono::seconds(1)); }, 1);
            auto ft = state1.toFuture();
            ft.wait();
            if (state1.getPtr()) mprintfE(R"(Failed when check: thread_pool.addTask 1)""\n");

            mthrd::TaskFuture<int> state2 = thread_pool.addTask([](int x) {std::this_thread::sleep_for(std::chrono::seconds(1)); return x; }, 1);
            if (*state2.getPtr() != 1) mprintfE(R"(Failed when check: thread_pool.addTask 2)""\n");
        }

        inline void check()
        {
            printf("\n--------------------check mthrd start--------------------\n");
            SpinLockTest();
            ReadWriteMutexTest();
            ThreadPoolTest();
            printf("---------------------check mthrd end---------------------\n\n");
        }
    }
#endif


}

#endif // !THREAD_HPP_MINEUTILS