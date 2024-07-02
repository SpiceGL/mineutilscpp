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
#include<thread>
#include<vector>

#include"time.hpp"
#include"log.hpp"

namespace mineutils
{
    
    namespace mthread
    {
        /*--------------------------------------------用户接口--------------------------------------------*/
        class TaskState
        {
        public:
            TaskState() = default;
            explicit TaskState(std::future<void>&& future_state) noexcept;
            TaskState(TaskState&& future_state) noexcept;
            TaskState& operator=(TaskState&& future_state) noexcept;

            //判断任务是否结束，无参构造后为true
            bool finished();           
            //等待任务结束
            void wait();   

            TaskState(const TaskState& future_state) = delete;
            TaskState& operator=(const TaskState& future_state) = delete;
        private:
            std::future<void> future_state_;
        };


        //简易线程池
        class ThreadPool
        {
        public:
            /*  构造ThreadPool对象
                @param pool_size: 线程池线程数量，不小于1
                @param wakeup_period_ms: 定时唤醒工作线程执行任务的周期，单位毫秒，不小于1   */
            ThreadPool(int pool_size, long long wakeup_period_ms = 100);

            /*  添加一个任务到线程池中并异步执行，任务的返回值无法获取  
                --addTask(func, arg1, arg2...)  
                --addTask(&class::func, &class_obj, arg1, arg2...)
                @param func: 任务函数，注意在任务完成前保证其生命周期
                @param args: 任务函数的参数，传递指针时应在任务完成前保证其生命周期，要传递引用应使用std::ref显式指定
                @return 任务状态，用于查询任务是否结束或等待任务结束，注意ThreadPool析构后会将所有未执行任务的状态设置为结束   */  
            template<class Fn, class... Args>
            TaskState addTask(const Fn& func, Args&&... args);
         
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
            std::queue<std::promise<void>> promise_queue_;
            
            std::atomic<bool> need_abort_;
            std::atomic<bool> aborted_;       
        };
    







        /*--------------------------------------------内部实现--------------------------------------------*/


        inline TaskState::TaskState(std::future<void>&& future_state) noexcept
        {
            this->future_state_ = std::move(future_state);
        }

        inline TaskState::TaskState(TaskState&& tmp_state) noexcept
        {
            *this = std::move(tmp_state);
        }

        inline TaskState& TaskState::operator=(TaskState&& tmp_state) noexcept
        {
            this->future_state_ = std::move(tmp_state.future_state_);
            return *this;
        }

        inline bool TaskState::finished()
        {
            if (this->future_state_.valid())
                return this->future_state_.wait_for(std::chrono::nanoseconds(0)) == std::future_status::ready;
            return true;
        }

        inline void TaskState::wait()
        {
            if (this->future_state_.valid())
                this->future_state_.wait();
        }


        inline ThreadPool::ThreadPool(int pool_size, long long wakeup_period_ms)
        {
            if (pool_size <= 0)
            {
                printfW("Invalid param value pool_size:%d, which will be set to 1.\n", pool_size);
                pool_size = 1;
            }
            if (wakeup_period_ms <= 0)
            {
                printfW("Invalid param value wakeup_period_ms:%lld, which will be set to 100.\n", wakeup_period_ms);
                wakeup_period_ms = 1;
            }
            this->pool_size_ = pool_size;
            this->wakeup_period_ms_ = 100;
            this->task_queue_ = {};
            this->promise_queue_ = {};
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
            {
                std::lock_guard<std::mutex> lk(this->task_mtx_);
                while (!this->promise_queue_.empty())
                {
                    std::promise<void>& task_promise = this->promise_queue_.front();
                    task_promise.set_value();
                    this->promise_queue_.pop();
                }
            }           
            this->aborted_ = true;
            this->wakeup_thd_.join();
            printfN("Destroyed.\n");
        }

        template<class Fn, class... Args>
        inline TaskState ThreadPool::addTask(const Fn& func, Args&&... args)
        {
            std::function<void()> task = std::bind(func, std::forward<Args>(args)...);
            //std::function<void()> task = [&](){func(std::forward<Args>(args)...); };
            std::lock_guard<std::mutex> lk(this->task_mtx_);
            std::promise<void> promise_state;
            TaskState state(promise_state.get_future());

            this->task_queue_.emplace(std::move(task));
            this->promise_queue_.emplace(std::move(promise_state));
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
                std::promise<void> task_promise;
                {
                    std::unique_lock<std::mutex> lk(this->task_mtx_);
                    while (!this->need_abort_ && this->task_queue_.empty())
                    {
                        this->cond_var_.wait(lk);
                    }
                    if (this->need_abort_ || this->task_queue_.empty())
                        continue;
                    task = this->task_queue_.front();
                    task_promise = std::move(this->promise_queue_.front());
                    this->task_queue_.pop();
                    this->promise_queue_.pop();
                }
                task();
                task_promise.set_value();
            }
        }
    }
}

#endif // !THREAD_HPP_MINEUTILS