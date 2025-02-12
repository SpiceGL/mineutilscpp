//mineutils库的便利时间相关工具
//注：如果为qnx6.6平台编译，可能需要为编译器开启宏: _GLIBCXX_USE_NANOSLEEP
#pragma once
#ifndef TIME_HPP_MINEUTILS
#define TIME_HPP_MINEUTILS

#include<array>
#include<chrono>
#include<iostream>
#include<list>
#include<map>
#include<stdio.h>
#include<string>
#include<string.h>
#include<thread>
#include<time.h>
#include<type_traits>

#include"base.hpp"


namespace mineutils
{
    /*--------------------------------------------用户接口--------------------------------------------*/

    //基于<chrono>库的简易计时函数封装
    namespace mtime
    {
        //时间单位
        enum Unit
        {
            ns = 1 << 0,
            us = 1 << 1,
            ms = 1 << 2,
            s = 1 << 3
        };

        //日期时间信息
        struct DateTime
        {
            int year = 0;    // 1900-现今
            int yday = 0;    // 1-366
            int month = 0;    // 1-12
            int mday = 0;    // 1-31
            int wday = 0;    // 1-7
            int hour = 0;    // 0-23
            int minute = 0;    // 0-59
            int second = 0;    // 0-60

            int isdst = 0;   // 1代表夏令时，0代表非夏令时，其他代表未知
            int isutc = 0;    // 1代表UTC时间，0代表非UTC时间，其他代表未知
            bool valid = false;  //上述信息是否有效
        };

        /*  为DateTime对象添加对operator<< 的支持，也添加了对mstr::toStr的支持
            - 按照"2025-01-07 17:18:35 STD"的格式，STD根据实际可能为DST、UTC、Invalid等  */
        std::ostream& operator<<(std::ostream& cout_obj, const DateTime& date_time);


        //时间点，可获取时间点对应的实际日期和时间，也可计算两个时间点之间的时长
        class TimePoint
        {
        public:
            //创建一个0时刻的时间点
            TimePoint() = default;

            DateTime localTime() const;
            DateTime utcTime() const;

            //根据unit计算从tp到当前时间点之间的时长
            template<Unit unit>
            long long since(const TimePoint& tp) const;

            //在当前时间点上增加时间段，正负皆可，在qnx660上实际精度为1000ns以上
            template<Unit unit>
            TimePoint& add(long long duration);

        private:
            TimePoint(std::chrono::steady_clock::time_point&& now_steady, std::chrono::system_clock::time_point&& now_system)
                :steady_tp_(std::move(now_steady)), system_tp_(std::move(now_system)) {}

            std::chrono::steady_clock::time_point steady_tp_;
            std::chrono::system_clock::time_point system_tp_;
            friend TimePoint now();
        };

        //获得当前时间点
        TimePoint now();

        //线程休眠(秒)
        void sleep(long long t);

        //线程休眠(毫秒)
        void msleep(long long t);

        //线程休眠(微秒)
        void usleep(long long t);

        //线程休眠(纳秒)
        void nsleep(long long t);


        //只有开启时TimeCounter系列类的统计功能才生效，作用范围为该二进制模块
        void enableGlobalTimeCounter(bool enabled);

        //用于统计各个代码段的在一定循环次数的平均消耗时间，非线程安全
        class MeanTimeCounter
        {
        private:
            class Guard;

        public:
            MeanTimeCounter() = default;
            /*  构造MeanTimeCounter类
                @param target_count: 每轮统计次数，小于1的值会被置为1
                @param print_header: 打印时的头信息，为空时表示无头信息
                @param enabled: 是否开启计时功能
                @param unit: 计时单位  */
            MeanTimeCounter(int target_count, std::string print_header = "", bool enabled = true);

            /*  本轮统计开始，应在目标统计代码段前调用，与段后markEnd成对出现
                @param codeblock_tag: 要统计的代码段的tag   */
            void markStart(const std::string& codeblock_tag);

            /*  本轮统计结束，应在目标统计代码段后调用，与段前markStart成对出现
                @param codeblock_tag: 要统计的代码段的tag   */
            void markEnd(const std::string& codeblock_tag);

            /*  使用RAII方式安全记录一段代码的耗时，自动调用markStart和markEnd
                - 用例：auto guard = time_counter.markGuard("codeblock_tag")
                @param codeblock_tag: 要统计的代码段的tag
                @return 一个私有类Guard对象，只能用auto推导；在返回时记录开始时间，在调用release或析构时记录结束时间  */
            MeanTimeCounter::Guard markGuard(std::string codeblock_tag);

            /*  在每个被统计的代码段达到目标统计次数后，打印其平均消耗时间并重新开始统计此段代码  */
            template<Unit unit>
            void printOnTargetCount();

            //支持移动禁止拷贝
            MeanTimeCounter(MeanTimeCounter&& rvalue) = default;
            MeanTimeCounter& operator=(MeanTimeCounter&& rvalue) = default;

        private:
            class SingleCounter;

            int target_count_ = 1;
            std::string print_header_;
            bool self_enabled_ = true;
            bool final_enabled_ = true;

            //有依赖关系
            std::list<std::string> tags_;
            std::map<std::string, SingleCounter> counter_map_;
        };


        /*  统计并打印该对象从获取资源到释放资源之间的时间消耗         
            - 用例：TimeCounterGuard<mtime::ms> guard("tag");    //获取资源 
            - 用例：guard.reset("tag");    //释放旧资源，获取新资源  */
        template<Unit unit>
        class TimeCounterGuard
        {
        public:
            //构造一个空的对象，无资源
            TimeCounterGuard() = default;
            /*  构造对象，获取资源，统计从当前到资源释放之间的时间消耗
                @param codeblock_tag: 要计时的代码块标识符  */
            TimeCounterGuard(std::string codeblock_tag);

            //提前释放资源并将对象置空
            void release();
            //释放旧资源，获取新资源
            void reset(std::string codeblock_tag);

            //禁止拷贝和移动
            TimeCounterGuard(const TimeCounterGuard<unit>& obj) = delete;
            TimeCounterGuard<unit>& operator=(const TimeCounterGuard<unit>& obj) = delete;
            //释放资源
            ~TimeCounterGuard();

        private:
            void create(std::string& codeblock_tag);    //不检查有效性并创建资源
            void destroy();    //释放资源，不修改有效性

            bool valid_ = false;
            std::chrono::steady_clock::time_point start_t_;
            std::string codeblock_tag_;
        };

        /*  设置从获取资源到释放资源的代码段的最短时间
            - 用例1：MinTimeGuard<mtime::ms> guard(50);    //获取资源，保证从当前到释放资源之间的代码段时间不低于50毫秒
            - 用例2：guard.reset(50);    //释放旧资源，获取新资源 
            注意：在QNX660的GCC4.7.3上，实际精度为1000ns以上   */
        template<Unit unit>
        class MinTimeGuard
        {
        public:
            //构造空对象，无资源
            MinTimeGuard() = default;
            /*  构造对象，获取资源，统计从当前到资源释放之间的时间消耗
                @param target_time: 目标时长  */
            MinTimeGuard(long long target_time);

            //提前释放资源并将对象置空
            void release();
            //释放旧资源，获取新资源
            void reset(long long target_time);

            //禁止拷贝和移动
            MinTimeGuard(const MinTimeGuard<unit>& obj) = delete;
            MinTimeGuard<unit>& operator=(const MinTimeGuard<unit>& obj) = delete;
            //释放资源
            ~MinTimeGuard();

        private:
            void create(long long target_time);    //不检查有效性并创建资源
            void destroy();    //释放资源，不修改有效性

            bool valid_ = false;
            std::chrono::steady_clock::time_point target_tp_;
        };
    }








    /*--------------------------------------------内部实现--------------------------------------------*/

    namespace mtime
    {
        inline std::array<char, 32> _MINE_REF_WHEN_THREAD_LOCAL _createFmtDateTime()
        {
            _MINE_THREAD_LOCAL_IF_HAVE std::array<char, 32> fmt_date;
            return fmt_date;
        }
        _MINE_NOREMOVE const auto _MINE_REF_WHEN_THREAD_LOCAL _fmt_datetime = _createFmtDateTime();

        inline std::ostream& operator<<(std::ostream& cout_obj, const DateTime& date_time)
        {
            auto _MINE_REF_WHEN_THREAD_LOCAL fmt_date = _createFmtDateTime();

            const char* fmt = nullptr;
            if (!date_time.valid)
                fmt = "%04d-%02d-%02d %02d:%02d:%02d Invalid";
            else if (date_time.isutc)
                fmt = "%04d-%02d-%02d %02d:%02d:%02d UTC";
            else if (date_time.isdst)
                fmt = "%04d-%02d-%02d %02d:%02d:%02d DST";
            else fmt = "%04d-%02d-%02d %02d:%02d:%02d STD";

            snprintf(fmt_date.data(), sizeof(fmt_date), fmt, date_time.year, date_time.month, date_time.mday, date_time.hour, date_time.minute, date_time.second);
            cout_obj << fmt_date.data();
            return cout_obj;
        }


        inline DateTime TimePoint::localTime() const
        {
            auto& now = this->system_tp_;
            time_t now_time_t = std::chrono::system_clock::to_time_t(now);

            DateTime date_time;
            date_time.valid = false;
            tm buf;
#if defined(_MSC_VER)
            errno_t err = localtime_s(&buf, &now_time_t);
            if (err != 0)
            {
                mprintfW("Got Invalid DateTime!\n");
                return date_time;
            }
#else
            if (localtime_r(&now_time_t, &buf) == nullptr)
            {
                mprintfW("Got Invalid DateTime!\n");
                return date_time;
            }
#endif
            date_time.year = buf.tm_year + 1900;
            date_time.yday = buf.tm_yday + 1;
            date_time.month = buf.tm_mon + 1;
            date_time.mday = buf.tm_mday;
            date_time.wday = buf.tm_wday + 1;
            date_time.hour = buf.tm_hour;
            date_time.minute = buf.tm_min;
            date_time.second = buf.tm_sec;
            date_time.isdst = buf.tm_isdst;
            date_time.isutc = 0;
            date_time.valid = true;

            return date_time;
        }

        inline DateTime TimePoint::utcTime() const
        {
            auto& now = this->system_tp_;
            auto time_t_now = std::chrono::system_clock::to_time_t(now);
            DateTime date_time;
            tm buf;
#if defined(_MSC_VER)
            // Windows 特定实现
            errno_t err = gmtime_s(&buf, &time_t_now);
            if (err != 0)
            {
                mprintfW("Got Invalid DateTime!\n");
                return date_time;
            }
#else
            // POSIX 特定实现
            if (gmtime_r(&time_t_now, &buf) == nullptr)
            {
                mprintfW("Got Invalid DateTime!\n");
                return date_time;
            }
#endif
            date_time.year = buf.tm_year + 1900;
            date_time.yday = buf.tm_yday + 1;
            date_time.month = buf.tm_mon + 1;
            date_time.mday = buf.tm_mday;
            date_time.wday = buf.tm_wday + 1;
            date_time.hour = buf.tm_hour;
            date_time.minute = buf.tm_min;
            date_time.second = buf.tm_sec;
            date_time.isdst = 0;
            date_time.isutc = 1;
            date_time.valid = true;

            return date_time;
        }

        template<Unit unit>
        inline long long TimePoint::since(const TimePoint& tp) const
        {
            switch (unit)
            {
            case Unit::s:
                return std::chrono::duration_cast<std::chrono::seconds>(this->steady_tp_ - tp.steady_tp_).count();
            case Unit::ms:
                return std::chrono::duration_cast<std::chrono::milliseconds>(this->steady_tp_ - tp.steady_tp_).count();
            case Unit::us:
                return std::chrono::duration_cast<std::chrono::microseconds>(this->steady_tp_ - tp.steady_tp_).count();
            case Unit::ns:
                return std::chrono::duration_cast<std::chrono::nanoseconds>(this->steady_tp_ - tp.steady_tp_).count();
            default:
                return std::chrono::duration_cast<std::chrono::seconds>(this->steady_tp_ - tp.steady_tp_).count();
            }
        }

        template<Unit unit>
        inline TimePoint& TimePoint::add(long long duration)
        {
            switch (unit)
            {
            case mineutils::mtime::Unit::s:
                this->steady_tp_ = this->steady_tp_ + std::chrono::seconds(duration);
                this->system_tp_ = this->system_tp_ + std::chrono::seconds(duration);
                break;
            case mineutils::mtime::ms:
                this->steady_tp_ = this->steady_tp_ + std::chrono::milliseconds(duration);
                this->system_tp_ = this->system_tp_ + std::chrono::milliseconds(duration);
                break;
            case mineutils::mtime::Unit::us:
                this->steady_tp_ = this->steady_tp_ + std::chrono::microseconds(duration);
                this->system_tp_ = this->system_tp_ + std::chrono::microseconds(duration);
                break;
            case mineutils::mtime::Unit::ns:    //system_tp_不支持 + nanoseconds
#if defined(__GNUC__) && !_mgccMinVersion(4, 8, 1)  //for qnx660
                this->steady_tp_ = this->steady_tp_ + std::chrono::microseconds(duration / 1000 + 1);
#else
                this->steady_tp_ = this->steady_tp_ + std::chrono::nanoseconds(duration);
#endif
                this->system_tp_ = this->system_tp_ + std::chrono::microseconds(duration / 1000 + 1);
                break;
            default:
                this->steady_tp_ = this->steady_tp_ + std::chrono::milliseconds(duration);
                this->system_tp_ = this->system_tp_ + std::chrono::milliseconds(duration);
                break;
            }
            return *this;
        }

        //inline long long TimePoint<unit>::operator-(const TimePoint<unit>& tp)
        //{
        //    switch (unit)
        //    {
        //    case Unit::s:
        //        return std::chrono::duration_cast<std::chrono::seconds>(this->steady_tp_ - tp.steady_tp_).count();
        //    case Unit::ms:
        //        return std::chrono::duration_cast<std::chrono::milliseconds>(this->steady_tp_ - tp.steady_tp_).count();
        //    case Unit::us:
        //        return std::chrono::duration_cast<std::chrono::microseconds>(this->steady_tp_ - tp.steady_tp_).count();
        //    case Unit::ns:
        //        return std::chrono::duration_cast<std::chrono::nanoseconds>(this->steady_tp_ - tp.steady_tp_).count();
        //    default:
        //        return std::chrono::duration_cast<std::chrono::seconds>(this->steady_tp_ - tp.steady_tp_).count();
        //    }
        //}

        //template<Unit unit>
        //template<Unit other_unit>
        //inline TimePoint<unit>::TimePoint(const TimePoint<other_unit>& tp)
        //{
        //    *this = tp;
        //}

        //template<Unit unit>
        //template<Unit other_unit>
        //inline TimePoint<unit>& TimePoint<unit>::operator=(const TimePoint<other_unit>& tp)
        //{
        //    this->steady_tp_ = tp.steady_tp_;
        //    this->system_tp_ = tp.system_tp_;
        //    return *this;
        //}

        inline TimePoint now()
        {
            return TimePoint(std::chrono::steady_clock::now(), std::chrono::system_clock::now());
        }


        //线程休眠(秒)
        inline void sleep(long long t)
        {
            if (t > 0)
                std::this_thread::sleep_for(std::chrono::seconds(t));
        }

        //线程休眠(毫秒)
        inline void msleep(long long t)
        {
            if (t > 0)
                std::this_thread::sleep_for(std::chrono::milliseconds(t));
        }

        //线程休眠(微秒)
        inline void usleep(long long t)
        {
            if (t > 0)
                std::this_thread::sleep_for(std::chrono::microseconds(t));
        }

        //线程休眠(纳秒)
        inline void nsleep(long long t)
        {
            if (t > 0)
                std::this_thread::sleep_for(std::chrono::nanoseconds(t));
        }


        inline std::atomic<bool>& _getGlobalTimeCounterEnabled()
        {
            static std::atomic<bool> g_timecounter_on(true);
            return g_timecounter_on;
        }
        _MINE_NOREMOVE const std::atomic<bool>& _g_timecounter_on = _getGlobalTimeCounterEnabled();

        inline void enableGlobalTimeCounter(bool enabled)
        {
            mtime::_getGlobalTimeCounterEnabled().store(enabled, std::memory_order_relaxed);
        }


        class MeanTimeCounter::Guard
        {
        public:
            Guard(Guard&& tmp) noexcept
            {
                this->codeblock_tag_ = std::move(tmp.codeblock_tag_);
                this->resource_ = tmp.resource_;
                tmp.resource_ = nullptr;
            }

            ~Guard()
            {
                if (this->resource_)
                    this->resource_->markEnd(this->codeblock_tag_);
            }

            void release()
            {
                if (this->resource_)
                    this->resource_->markEnd(this->codeblock_tag_);
                this->resource_ = nullptr;
            }

            Guard(const Guard& tmp) = delete;
            Guard& operator=(const Guard& tmp) = delete;
            Guard& operator=(Guard&& tmp) = delete;

        private:
            Guard(MeanTimeCounter* resource, std::string& codeblock_tag)
            {
                resource->markStart(codeblock_tag);
                this->resource_ = resource;
                this->codeblock_tag_ = std::move(codeblock_tag);
            }

            MeanTimeCounter* resource_ = nullptr;
            std::string codeblock_tag_;
            friend MeanTimeCounter;
        };

        class MeanTimeCounter::SingleCounter
        {
        public:
            SingleCounter() = default;

            explicit SingleCounter(int target_count, const char* print_head, const char* codeblock_tag)
            {
                this->target_count_ = target_count;
                if (print_head)
                {
                    this->final_tag_.reserve(strlen(print_head) + strlen(codeblock_tag) + 5);  //算上\0
                    this->final_tag_.append(print_head).append(": ").append(codeblock_tag);
                }
                else
                {
                    this->final_tag_.reserve(strlen(codeblock_tag) + 1);  //算上\0
                    this->final_tag_.append(codeblock_tag);
                }
                this->codeblock_tag_ = codeblock_tag;
            }

            void markStart()
            {
                this->start_t_ = std::chrono::steady_clock::now();
                this->addstart_times_ += 1;
            }

            void markEnd()
            {
                this->time_cost_ += (std::chrono::steady_clock::now() - this->start_t_);
                this->addend_times_ += 1;        
            }

            template<Unit unit>
            void printOnTargetCount()
            {
                if (!(this->addend_times_ == this->addstart_times_))
                {
                    mprintfW("Function \"'markStart(%s)\" and function \"markEnd(%s)\" should be called the same number of times!\n", this->codeblock_tag_, this->codeblock_tag_);
                    return;
                }
                if (this->finish())
                {
                    long long mean_time_cost;
                    const char* msg = nullptr;

                    switch (unit)
                    {
                    case Unit::s:
                        mean_time_cost = std::chrono::duration_cast<std::chrono::seconds>(this->time_cost_).count() / this->addend_times_;
                        msg = "%s mean cost time %llds in %d counts\n";
                        break;
                    case Unit::ms:
                        mean_time_cost = std::chrono::duration_cast<std::chrono::milliseconds>(this->time_cost_).count() / this->addend_times_;
                        msg = "%s mean cost time %lldms in %d counts\n";
                        break;
                    case Unit::us:
                        mean_time_cost = std::chrono::duration_cast<std::chrono::microseconds>(this->time_cost_).count() / this->addend_times_;
                        msg = "%s mean cost time %lldus in %d counts\n";
                        break;
                    case Unit::ns:
                        mean_time_cost = std::chrono::duration_cast<std::chrono::nanoseconds>(this->time_cost_).count() / this->addend_times_;
                        msg = "%s mean cost time %lldns in %d counts\n";
                        break;
                    default:
                        mean_time_cost = std::chrono::duration_cast<std::chrono::milliseconds>(this->time_cost_).count() / this->addend_times_;
                        msg = "%s mean cost time %lldms in %d counts\n";
                        break;
                    }
                    printf(msg, this->final_tag_.c_str(), mean_time_cost, this->addend_times_);
                    this->restart();
                }
            }

        private:
            bool finish()
            {
                return this->addend_times_ >= this->target_count_;
            }

            void restart()
            {
                this->addstart_times_ = 0;
                this->addend_times_ = 0;
                this->time_cost_ = std::chrono::nanoseconds(0);
            }

            int addstart_times_ = 0;
            int addend_times_ = 0;
            std::chrono::nanoseconds time_cost_{ 0 };
            std::chrono::steady_clock::time_point start_t_;

            int target_count_ = 1;
            std::string final_tag_;
            const char* codeblock_tag_ = nullptr;  
        };


        inline MeanTimeCounter::MeanTimeCounter(int target_count, std::string print_header, bool enabled)
        {
            this->target_count_ = target_count >= 1 ? target_count : 1;
            this->print_header_ = std::move(print_header);
            this->self_enabled_ = enabled;
            this->final_enabled_ = mtime::_getGlobalTimeCounterEnabled().load(std::memory_order_acquire) && enabled;
        }

        inline void MeanTimeCounter::markStart(const std::string& codeblock_tag)
        {
            if (this->final_enabled_)
            {
                if (this->counter_map_.end() == this->counter_map_.find(codeblock_tag))
                {
                    this->tags_.emplace_back(codeblock_tag);
                    this->counter_map_[codeblock_tag] = MeanTimeCounter::SingleCounter(this->target_count_, this->print_header_.c_str(), this->tags_.back().c_str());
                }
                this->counter_map_[codeblock_tag].markStart();
            }
        }

        inline void MeanTimeCounter::markEnd(const std::string& codeblock_tag)
        {
            if (this->final_enabled_)
            {
                if (this->counter_map_.end() == this->counter_map_.find(codeblock_tag))
                {
                    mprintfW("Please call \"markStart(%s)\" before \"markEnd(%s)\"!\n", codeblock_tag.c_str(), codeblock_tag.c_str());
                    return;
                }
                this->counter_map_[codeblock_tag].markEnd();
            }
        }

        inline MeanTimeCounter::Guard MeanTimeCounter::markGuard(std::string codeblock_tag)
        {
            return MeanTimeCounter::Guard(this, codeblock_tag);
        }

        template<Unit unit>
        inline void MeanTimeCounter::printOnTargetCount()
        {
            this->final_enabled_ = mtime::_getGlobalTimeCounterEnabled().load(std::memory_order_relaxed) && this->self_enabled_;
            if (this->final_enabled_)
            {
                for (const std::string& tag : this->tags_)
                {
                    this->counter_map_[tag].printOnTargetCount<unit>();
                }
            }
        }


        template<Unit unit>
        inline TimeCounterGuard<unit>::TimeCounterGuard(std::string codeblock_tag)
        {
            this->create(codeblock_tag);
        }

        template<Unit unit>
        inline void TimeCounterGuard<unit>::release()
        {
            this->destroy();
            this->valid_ = false;
        }
        template<Unit unit>
        inline void TimeCounterGuard<unit>::reset(std::string codeblock_tag)
        {
            this->destroy();
            this->create(codeblock_tag);
        }

        template<Unit unit>
        inline TimeCounterGuard<unit>::~TimeCounterGuard()
        {
            this->destroy();
        }

        template<Unit unit>
        inline void TimeCounterGuard<unit>::create(std::string& codeblock_tag)
        {
            this->start_t_ = std::chrono::steady_clock::now();
            this->codeblock_tag_ = std::move(codeblock_tag);
            this->valid_ = true;
        }

        template<Unit unit>
        inline void TimeCounterGuard<unit>::destroy()
        {
            if (this->valid_ && mtime::_getGlobalTimeCounterEnabled().load(std::memory_order_relaxed))
            {
                long long time_cost;
                const char* msg = nullptr;
                switch (unit)
                {
                case mineutils::mtime::Unit::s:
                    time_cost = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - this->start_t_).count();
                    msg = "%s cost time %llds\n";
                    break;
                case mineutils::mtime::ms:
                    time_cost = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - this->start_t_).count();
                    msg = "%s cost time %lldms\n";
                    break;
                case mineutils::mtime::Unit::us:
                    time_cost = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - this->start_t_).count();
                    msg = "%s cost time %lldus\n";
                    break;
                case mineutils::mtime::Unit::ns:
                    time_cost = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - this->start_t_).count();
                    msg = "%s cost time %lldns\n";
                    break;
                default:
                    time_cost = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - this->start_t_).count();
                    msg = "%s cost time %lldms\n";
                    break;
                }
                printf(msg, this->codeblock_tag_.c_str(), time_cost);
            }
        }


        template<Unit unit>
        inline MinTimeGuard<unit>::MinTimeGuard(long long target_time)
        {
            this->create(target_time);
        }

        template<Unit unit>
        inline MinTimeGuard<unit>::~MinTimeGuard()
        {
            this->destroy();
        }

        template<Unit unit>
        inline void MinTimeGuard<unit>::release()
        {
            this->destroy();
            this->valid_ = false;
        }

        template<Unit unit>
        inline void MinTimeGuard<unit>::reset(long long target_time)
        {
            this->destroy();
            this->create(target_time);
        }

        template<Unit unit>
        inline void MinTimeGuard<unit>::create(long long target_time)
        {
            auto now_tp = std::chrono::steady_clock::now();
            switch (unit)
            {
            case mineutils::mtime::Unit::s:
                this->target_tp_ = now_tp + std::chrono::seconds(target_time);
                break;
            case mineutils::mtime::ms:
                this->target_tp_ = now_tp + std::chrono::milliseconds(target_time);
                break;
            case mineutils::mtime::Unit::us:
                this->target_tp_ = now_tp + std::chrono::microseconds(target_time);
                break;
            case mineutils::mtime::Unit::ns:
#if defined(__GNUC__) && !_mgccMinVersion(4, 8, 1)  //for qnx660
                this->target_tp_ = now_tp + std::chrono::microseconds(target_time / 1000 + 1);
#else
                this->target_tp_ = now_tp + std::chrono::nanoseconds(target_time);
#endif
                break;
            default:
                this->target_tp_ = now_tp + std::chrono::milliseconds(target_time);
                break;
            }
            this->valid_ = true;
        }

        template<Unit unit>
        inline void MinTimeGuard<unit>::destroy()
        {
            if (this->valid_)
                std::this_thread::sleep_until(this->target_tp_);
        }

    }


#ifdef MINEUTILS_TEST_MODULES
    namespace _mtimecheck
    {
        inline void MeanTimeCounterTest()
        {

            mtime::MeanTimeCounter time_counter{10, __func__};
            {
                for (int i = 0; i < 10; i++)
                {
                    auto guard = time_counter.markGuard("Guard 2");
                    mtime::msleep(60);
                }
                for (int i = 0; i < 10; i++)
                {
                    auto guard = time_counter.markGuard("Guard 1");
                    mtime::msleep(60);
                }
                printf("User check! Expected output: \n\"MeanTimeCounterTest\": Guard 2 mean cost time 60ms in 10 counts  \n\"MeanTimeCounterTest\": Guard 1 mean cost time 60ms in 10 counts\n");
                printf("Actual output:\n");
                time_counter.printOnTargetCount<mtime::ms>();

            }
        }

        inline void nowTest()
        {
            auto start_t = mtime::now();
            mtime::msleep(60);
            auto end_t = mtime::now();
            //mtime::TimePoint<mtime::Unit::us> start_t1 = start_t;
            //mtime::TimePoint<mtime::Unit::us> end_t1 = end_t;

            printf("User check! Got result:%lldms(60ms)\n", end_t.since<mtime::ms>(start_t));
            //printf("User check! Got result:%lldus(60000us)\n", end_t1 - start_t1);
            auto start_t2 = mtime::now();
            {
                mtime::MinTimeGuard<mtime::ms> guard(60);
                {
                    guard.reset(60);
                }
            }
            auto end_t2 = mtime::now();
            printf("User check! Got result:%lldms(120ms)\n", end_t2.since<mtime::ms>(start_t2));

            auto now_tp = mtime::now();
            mtime::DateTime now_time = now_tp.localTime();
            mtime::DateTime now_utc_time = now_tp.utcTime();
            mtime::DateTime now_time_add_60s = now_tp.add<mtime::s>(60).localTime();
            printf("User checks netx date format below!\n");
            std::cout << now_time << std::endl;
            std::cout << now_utc_time << std::endl;
            std::cout << now_time_add_60s << std::endl;
            std::cout << mtime::DateTime() << std::endl;
            printf("\n");
        }



        inline void check()
        {
            printf("\n--------------------check mtime start--------------------\n\n");
            MeanTimeCounterTest();
            nowTest();
            printf("--------------------check mtime end--------------------\n\n");
        }
    }
#endif
}
 
#endif // !TIME_HPP_MINEUTILS