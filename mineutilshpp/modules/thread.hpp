//mineutils库的线程相关工具
//注：如果为qnx6.6平台编译，可能需要为编译器开启宏: _GLIBCXX_USE_NANOSLEEP
#pragma once
#ifndef THREAD_HPP_MINEUTILS
#define THREAD_HPP_MINEUTILS

#include<atomic>
#include<condition_variable>
#include<functional>
#include<future>
#include<mutex>
#include<queue>
#include<stdexcept>
#include<thread>
#include<vector>

#include"time.hpp"
#include"log.hpp"

namespace mineutils
{
    /*--------------------------------------------用户接口--------------------------------------------*/

    namespace mthread
    {
        //任务结果状态，线程安全
        template<class Ret>
        class TaskRetState
        {
        public:
            //构造一个无效的TaskRetState对象
            TaskRetState() = default;
            explicit TaskRetState(std::future<Ret>&& future_state) noexcept;
            TaskRetState(TaskRetState<Ret>&& future_state) noexcept;
            TaskRetState& operator=(TaskRetState<Ret>&& future_state) noexcept;

            //判断任务是否为有效状态
            bool valid();
            //判断任务是否结束，如果任务为无效状态会返回true
            bool finished();
            //等待任务结束，如果任务为无效状态会立即返回
            void wait();
            //等待并获取任务结果，只可调用一次，如果任务为无效状态会抛出std::runtime_error异常
            Ret get();

            TaskRetState(const TaskRetState<Ret>& future_state) = delete;
            TaskRetState& operator=(const TaskRetState<Ret>& future_state) = delete;
        private:
            std::future<Ret> future_state_;
        };

        using TaskState = TaskRetState<void>;


        //简易线程池
        class ThreadPool
        {
        public:
            /*  构造ThreadPool对象
                @param pool_size: 线程池线程数量，不小于1
                @param wakeup_period_ms: 定时唤醒工作线程执行任务的周期，单位毫秒，不小于1   */
            ThreadPool(int pool_size, long long wakeup_period_ms = 100);

            /*  添加一个任务到线程池中并异步执行，线程安全
                --addTask(func, arg1, arg2...)
                --addTask(&class::func, &class_obj, arg1, arg2...)
                @param func: 任务函数，注意在任务完成前保证参数的生命周期
                @param args: 任务函数的参数，要传递引用应使用std::ref显式指定，即使有默认值的参数也要提供
                @return 任务结果状态，用于查询任务状态、等待任务结束以及获取任务返回值，注意ThreadPool对象析构后任务状态失效   */
            template<class Fn, class... Args, class Ret = decltype(std::bind(std::declval<Fn>(), std::forward<Args>(std::declval<Args>())...)())>
            TaskRetState<Ret> addTask(Fn&& func, Args&&... args);

            ThreadPool(const ThreadPool& thd_pool) = delete;
            ThreadPool& operator=(const ThreadPool& thd_pool) = delete;
            ~ThreadPool();

        private:
            void manager();
            void worker();

            int pool_size_;
            long long wakeup_period_ms_;
            std::thread wakeup_thd_;
            std::vector<std::thread> work_thds_;

            std::mutex task_mtx_;
            std::condition_variable cond_var_;
            std::queue<std::function<void()>> task_queue_;

            std::atomic<bool> need_abort_;
            std::atomic<bool> aborted_;
        };
    }






    /*--------------------------------------------内部实现--------------------------------------------*/

    namespace mthread
    {
        template<class Ret>
        inline TaskRetState<Ret>::TaskRetState(std::future<Ret>&& future_state) noexcept
        {
            this->future_state_ = std::move(future_state);
        }

        template<class Ret>
        inline TaskRetState<Ret>::TaskRetState(TaskRetState<Ret>&& tmp_state) noexcept
        {
            *this = std::move(tmp_state);
        }

        template<class Ret>
        inline TaskRetState<Ret>& TaskRetState<Ret>::operator=(TaskRetState<Ret>&& tmp_state) noexcept
        {
            this->future_state_ = std::move(tmp_state.future_state_);
            return *this;
        }

        template<class Ret>
        inline bool TaskRetState<Ret>::valid()
        {
            return this->future_state_.valid();
        }

        template<class Ret>
        inline bool TaskRetState<Ret>::finished()
        {
            if (this->future_state_.valid())
                return this->future_state_.wait_for(std::chrono::nanoseconds(0)) == std::future_status::ready;
            mprintfW("Task is invalid, so the function returns value:true!\n");
            return true;
        }

        template<class Ret>
        inline void TaskRetState<Ret>::wait()
        {
            if (this->future_state_.valid())
                this->future_state_.wait();
            else mprintfW("Task is invalid, so the function returns directly!\n");
        }

        template<class Ret>
        inline Ret TaskRetState<Ret>::get()
        {
            if (this->future_state_.valid())
                return this->future_state_.get();
            else throw std::runtime_error("Error: Task is invalid!");
        }

        inline ThreadPool::ThreadPool(int pool_size, long long wakeup_period_ms)
        {
            if (pool_size <= 0)
            {
                mprintfW("Invalid param value pool_size:%d, which will be set to 1.\n", pool_size);
                pool_size = 1;
            }
            if (wakeup_period_ms <= 0)
            {
                mprintfW("Invalid param value wakeup_period_ms:%lld, which will be set to 100.\n", wakeup_period_ms);
                wakeup_period_ms = 100;
            }
            this->pool_size_ = pool_size;
            this->wakeup_period_ms_ = wakeup_period_ms;
            std::queue<std::function<void()>> empty_queue;
            this->task_queue_.swap(empty_queue);
            this->need_abort_ = false;
            this->aborted_ = false;

            this->work_thds_.resize(pool_size);
            for (int i = 0; i < pool_size; ++i)
            {
                this->work_thds_[i] = std::thread(&ThreadPool::worker, this);
            }
            this->wakeup_thd_ = std::thread(&ThreadPool::manager, this);    
        }

        inline ThreadPool::~ThreadPool()
        {
            this->need_abort_ = true;
            this->cond_var_.notify_all();
            for (auto& thd : this->work_thds_)
            {
                if (thd.joinable())
                    thd.join();
            }
        
            this->aborted_ = true;
            this->wakeup_thd_.join();
            mprintfN("Destroyed.\n");
        }

        template<class Fn, class... Args, class Ret>
        inline TaskRetState<Ret> ThreadPool::addTask(Fn&& func, Args&&... args)
        {
            auto task = std::make_shared<std::packaged_task<Ret()>>(std::bind(std::forward<Fn>(func), std::forward<Args>(args)...));
            TaskRetState<Ret> state(task->get_future());

            std::lock_guard<std::mutex> lk(this->task_mtx_);                     
            this->task_queue_.emplace([task]() {(*task)(); });
            this->cond_var_.notify_one();
            return state;
        }

        inline void ThreadPool::manager()
        {
            while (!this->need_abort_)
            {
                {
                    std::lock_guard<std::mutex> lk(this->task_mtx_);
                    if (!this->task_queue_.empty())
                        this->cond_var_.notify_one();
                }
                mtime::msleep(this->wakeup_period_ms_);
            }
            while (!this->aborted_)
            {
                this->cond_var_.notify_all();
                mtime::msleep(this->wakeup_period_ms_);
            }
        }

        inline void ThreadPool::worker()
        {
            while (!this->need_abort_)
            {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lk(this->task_mtx_);
                    while (!this->need_abort_ && this->task_queue_.empty())
                    {
                        this->cond_var_.wait(lk);
                    }
                    if (this->need_abort_ || this->task_queue_.empty())
                        continue;
                    task = this->task_queue_.front();
                    this->task_queue_.pop();
                }
                task();
            }
        }
    }
}

#endif // !THREAD_HPP_MINEUTILS