﻿//mineutils库的线程相关工具
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

#include"base.hpp"
#include"time.hpp"
#include"log.hpp"

namespace mineutils
{
    /*--------------------------------------------用户接口--------------------------------------------*/

    namespace mthread
    {
        //任务结果状态
        template<class Ret>
        class TaskRetState
        {
        public:
            //构造一个无效的TaskRetState对象
            TaskRetState() = default;
            explicit TaskRetState(std::future<Ret>&& future_state) noexcept;
            TaskRetState(TaskRetState<Ret>&& future_state) noexcept;
            TaskRetState& operator=(TaskRetState<Ret>&& future_state) noexcept;

            //判断任务是否为有效状态；线程安全
            bool valid();
            //判断任务是否结束，如果任务为无效状态会返回true；线程安全
            bool finished();
            //等待任务结束，如果任务为无效状态会立即返回；线程安全
            void wait();
            //等待并获取任务结果，只可调用一次，如果任务为无效状态会抛出std::runtime_error异常；线程安全
            Ret get();

            TaskRetState(const TaskRetState<Ret>& future_state) = delete;    //不支持拷贝构造
            TaskRetState& operator=(const TaskRetState<Ret>& future_state) = delete;   //不支持拷贝赋值
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

            /*  添加一个任务到线程池中并异步执行(会拷贝所有输入用于储存)，规则涵盖std::bind的要求且更严格；线程安全
                @param func: 任务函数。要求其参数类型不能为右值引用，返回类型必须为void或支持使用自身的右值赋值。如果Fn是一个函数对象(functor)类型，那么它的去引用类型必须支持使用自身的左值和右值对象进行构造
                @param args...: 任务函数的参数。需要左值引用传递的参数必须用std::ref或std::cref显式引用，其他参数的去引用类型必须支持使用自身的左值和右值对象进行构造
                @return 任务结果状态，用于查询任务状态、等待任务结束以及获取任务返回值，注意ThreadPool对象析构后任务状态失效     
                @用法:
                - addTask(function or &function, arg1, arg2...)
                - addTask(&class::function, &class_obj, arg1, arg2...)
                - addTask(functor, arg1, arg2...)  */
            template<class Fn, class... Args, class Ret = typename mtype::StdBindChecker<Fn, Args...>::ReturnType, typename std::enable_if<std::is_same<Ret, typename mtype::StdBindChecker<Fn, Args...>::ReturnType>::value && (std::is_void<Ret>::value || std::is_move_assignable<typename std::remove_reference<Ret>::type>::value), int>::type = 0>
            TaskRetState<Ret> addTask(Fn&& func, Args&&... args);

            ThreadPool(const ThreadPool& thd_pool) = delete;
            ThreadPool& operator=(const ThreadPool& thd_pool) = delete;
            ThreadPool(ThreadPool&& thd_pool) = delete;
            ThreadPool& operator=(ThreadPool&& thd_pool) = delete;
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

        //跨线程暂停，使用条件变量实现以代替循环sleep
        class ThreadPauser
        {
        public:
            ThreadPauser();

            //设置暂停点，当调用ThreadPauser::pause的时候就会在此处暂停；线程安全
            void setPausePoint();

            //发出暂停信号，在所有暂停点卡住线程；线程安全
            void pause();

            //发出继续信号，唤醒所有暂停点；线程安全
            void resume();

            //获取当前是否为paused状态
            bool isPaused();

            ThreadPauser(const ThreadPauser& tmp) = delete;
            ThreadPauser& operator=(const ThreadPauser& tmp) = delete;
            ThreadPauser(ThreadPauser&& tmp) = delete;
            ThreadPauser& operator=(ThreadPauser&& tmp) = delete;

        private:
            std::mutex mtx_;
            std::condition_variable cond_;
            std::atomic<bool> need_pause_;
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

        template<class Fn, class... Args, class Ret, typename std::enable_if<std::is_same<Ret, typename mtype::StdBindChecker<Fn, Args...>::ReturnType>::value && (std::is_void<Ret>::value || std::is_move_assignable<typename std::remove_reference<Ret>::type>::value), int>::type>
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


        inline ThreadPauser::ThreadPauser()
        {
            this->need_pause_ = false;
        }

        inline void ThreadPauser::setPausePoint()
        {
            std::unique_lock<std::mutex> lk(this->mtx_);
            if (this->need_pause_)
            {
                this->cond_.wait(lk, [this]() {return !this->need_pause_; });
            }
        }

        inline void ThreadPauser::pause()
        {
            this->need_pause_ = true;
        }

        inline void ThreadPauser::resume()
        {
            std::lock_guard<std::mutex> lk(this->mtx_);
            this->need_pause_ = false;
            this->cond_.notify_all();
        }

        inline bool ThreadPauser::isPaused()
        {
            return this->need_pause_.load();
        }
    }
}

#endif // !THREAD_HPP_MINEUTILS