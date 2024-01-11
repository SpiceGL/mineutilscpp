//mineutils库的便利时间相关工具
//注：如果为qnx6.6平台编译，可能需要为编译器开启宏: _GLIBCXX_USE_NANOSLEEP
#pragma once
#ifndef TIME_HPP_MINEUTILS
#define TIME_HPP_MINEUTILS

#include<chrono>
#include<map>
#include<string>
#include<thread>
#include<vector>


namespace mineutils
{
    //基于<chrono>库的简易计时函数封装
    namespace mtime
    {
        using TimePoint = std::chrono::high_resolution_clock::time_point;   //时间点
        using Duration = decltype(TimePoint() - TimePoint());   //时间段

        //获取当前时间点(mtime::time_point)
        inline mtime::TimePoint now()
        {
            return std::chrono::high_resolution_clock::now();
        }

        //将“时间段(mtime::Duration)”类型转化为以秒为单位的数字
        inline long long s(const mtime::Duration& t)
        {
            return std::chrono::duration_cast<std::chrono::seconds>(t).count();
        }

        //将“时间段(mtime::Duration)”类型转化为以毫秒为单位的数字
        inline long long ms(const mtime::Duration& t)
        {
            return std::chrono::duration_cast<std::chrono::milliseconds>(t).count();
        }

        //将“时间段(mtime::Duration)”类型转化为以微秒为单位的数字
        inline long long us(const mtime::Duration& t)
        {
            return std::chrono::duration_cast<std::chrono::microseconds>(t).count();
        }

        //将“时间段(mtime::Duration)”类型转化为以纳秒为单位的数字
        inline long long ns(const mtime::Duration& t)
        {
            return std::chrono::duration_cast<std::chrono::nanoseconds>(t).count();
        }

        //进程休眠(秒)
        inline void sleep(const int& t)
        {
            std::this_thread::sleep_for(std::chrono::seconds(t));
        }

        //进程休眠(毫秒)
        inline void msleep(const int& t)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(t));
        }

        //进程休眠(微秒)
        inline void usleep(const int& t)
        {
            std::this_thread::sleep_for(std::chrono::microseconds(t));
        }

        //进程休眠(纳秒)
        inline void nsleep(const int& t)
        {
            std::this_thread::sleep_for(std::chrono::nanoseconds(t));
        }


        inline bool& _createTimeCounterOn()
        {
            static bool g_timecounter_on = true;
            return g_timecounter_on;
        }

        static bool& _g_timecounter_on = _createTimeCounterOn();   //计时功能全局开关，默认开启，关闭后跳过计时功能以提高程序效率

        //设置后所有TimeCounter系列类的功能将被跳过
        inline void setGlobalTimeCounterOn(bool glob_timecounter_on)
        {
            _g_timecounter_on = glob_timecounter_on;
        }

        //时间单位
        enum class Unit
        {
            s = 0,
            ms = 1,
            us = 2,
            ns = 3
        };

        //统计一定循环次数中某一代码段的时间消耗，线程不安全
        class MeanTimeCounter
        {
        public:
            MeanTimeCounter()
            {
                target_count_times_ = 1;
                time_counter_on_ = true;
            }
            /*  构造MeanTimeCounter类
                @param target_count_times: 目标统计次数，调用MeanTimeCounter::printMeanTimeCost方法后在达到目标次数时会输出平均消耗时间   */
            explicit MeanTimeCounter(int target_count_times, bool time_counter_on = true)
            {
                target_count_times_ = target_count_times;
                time_counter_on_ = time_counter_on;
            }

            //本轮统计开始，应在目标统计代码段前调用，与段后addEnd成对出现 
            void addStart()
            {
                if (_g_timecounter_on && time_counter_on_)
                {
                    start_t_ = mtime::now();
                    addstart_times_ += 1;
                }
            }

            //本轮统计结束，应在目标统计代码段后调用，与段前addStart成对出现
            void addEnd()
            {
                if (_g_timecounter_on && time_counter_on_)
                {
                    end_t_ = mtime::now();
                    time_cost_ += (end_t_ - start_t_);
                    addend_times_ += 1;
                    now_statistical_times_ += 1;
                }
            }

            /*  在达到目标统计次数后输出平均消耗时间，并重新开始统计时间
                @param codeblock_tag: 输出信息中被统计代码段的tag
                @param time_unit: 输出信息中时间统计的单位，输入强枚举类型mtime::Unit的成员
                @return 若达到目标统计次数，则按time_unit返回平均耗时；否则返回-1   */
            long long printMeanTimeCost(const std::string& codeblock_tag, mtime::Unit time_unit = mtime::Unit::ms)
            {
                if (_g_timecounter_on && time_counter_on_)
                {
                    if (!(addend_times_ == addstart_times_))
                    {
                        printf("!Warning!: MeanTimeCounter::%s: Function \'addStart(%s)\' and function \'addEnd(%s)\' should be called the same number of times before function \'%s\'!\n", __func__, codeblock_tag.c_str(), codeblock_tag.c_str(), __func__);
                        return -1;
                    }
                    if (this->finish())
                    {
                        long long mean_time_cost;
                        if (time_unit == mtime::Unit::s)
                        {
                            mean_time_cost = mtime::s(time_cost_) / now_statistical_times_;
                            printf("%s mean cost time %llds in %d counts\n", codeblock_tag.c_str(), mean_time_cost, now_statistical_times_);
                        }
                        else if (time_unit == mtime::Unit::ms)
                        {
                            mean_time_cost = mtime::ms(time_cost_) / now_statistical_times_;
                            printf("%s mean cost time %lldms in %d counts\n", codeblock_tag.c_str(), mean_time_cost, now_statistical_times_);
                        }
                        else if (time_unit == mtime::Unit::us)
                        {
                            mean_time_cost = mtime::us(time_cost_) / now_statistical_times_;
                            printf("%s mean cost time %lldus in %d counts\n", codeblock_tag.c_str(), mean_time_cost, now_statistical_times_);
                        }
                        else if (time_unit == mtime::Unit::ns)
                        {
                            mean_time_cost = mtime::ns(time_cost_) / now_statistical_times_;
                            printf("%s mean cost time %lldns in %d counts\n", codeblock_tag.c_str(), mean_time_cost, now_statistical_times_);
                        }
                        else
                        {
                            mean_time_cost = mtime::ms(time_cost_) / now_statistical_times_;
                            printf("%s mean cost time %lldms in %d counts\n", codeblock_tag.c_str(), mean_time_cost, now_statistical_times_);
                        }
                        this->restart();
                        return mean_time_cost;
                    }
                    else return -1;
                }
                return -1;
            }

            /*  在达到目标统计次数后输出平均消耗时间，并重新开始统计时间
                @param print_head: 输出信息的头部内容，推荐输入调用printMeanTimeCost的函数的名字
                @param codeblock_tag: 输出信息中被统计代码段的tag
                @param time_unit: 输出信息中时间统计的单位，输入强枚举类型mtime::Unit的成员
                @return 若达到目标统计次数，则按time_unit返回平均耗时；否则返回-1   */
            long long printMeanTimeCost(const std::string& print_head, const std::string& codeblock_tag, mtime::Unit time_unit = mtime::Unit::ms)
            {
                if (_g_timecounter_on && time_counter_on_)
                    return printMeanTimeCost(print_head + ": " + codeblock_tag, time_unit);
                return -1;
            }

        private:
            int now_statistical_times_ = 0;
            int target_count_times_;
            int addstart_times_ = 0;
            int addend_times_ = 0;
            bool time_counter_on_;

            mtime::Duration time_cost_ = mtime::Duration(0);
            mtime::TimePoint start_t_;
            mtime::TimePoint end_t_;

            //判断是否完成目标统计次数
            bool finish()
            {
                if (now_statistical_times_ >= target_count_times_)
                    return true;
                else return false;
            }

            //完成目标统计次数并打印统计结果后，重置内部统计计数
            void restart()
            {
                now_statistical_times_ = 0;
                addstart_times_ = 0;
                addend_times_ = 0;
                time_cost_ = mtime::Duration(0);
            }

        };


        //用于分别统计多个代码段的在一定循环次数的平均消耗时间，线程不安全
        //调用addStart和addEnd会带来少量时间损耗(在rv1126上约为12微秒)
        class MultiMeanTimeCounter
        {
        public:
            MultiMeanTimeCounter()
            {
                target_count_times_ = 1;
                time_counter_on_ = true;
            }
            /*  构造MultiMeanTimeCounter类
                @param target_count_times: 目标统计次数
                @param time_counter_on: 计时功能开关，为false会跳过计时功能   */
            explicit MultiMeanTimeCounter(int target_count_times, bool time_counter_on = true)
            {
                target_count_times_ = target_count_times;
                time_counter_on_ = time_counter_on;
            }

            /*  本轮统计开始，应在目标统计代码段前调用，与段后addEnd成对出现
                @param codeblock_tag: 要统计的代码段的tag   */
            void addStart(const std::string& codeblock_tag)
            {
                if (_g_timecounter_on && time_counter_on_)
                {
                    if (time_counter_.end() == time_counter_.find(codeblock_tag))
                    {
                        time_counter_[codeblock_tag] = MeanTimeCounter(target_count_times_);
                        keys_.push_back(codeblock_tag);
                    }
                    time_counter_[codeblock_tag].addStart();
                }
            }

            /*  本轮统计结束，应在目标统计代码段后调用，与段前addStart成对出现
                @param codeblock_tag: 要统计的代码段的tag   */
            void addEnd(const std::string& codeblock_tag)
            {
                if (_g_timecounter_on && time_counter_on_)
                {
                    if (time_counter_.end() == time_counter_.find(codeblock_tag))
                    {
                        printf("!!!Error!!! MultiMeanTimeCounter::%s: Please call \"addStart(%s)\" before \"addEnd(%s)\"!\n", __func__, codeblock_tag.c_str(), codeblock_tag.c_str());
                        return;
                    }
                    time_counter_[codeblock_tag].addEnd();
                }
            }

            /*  在codeblock_tag代码段达到目标统计次数后输出平均消耗时间，并重新开始统计此段代码
                @param codeblock_tag: 输出信息中被统计代码段的tag
                @param time_unit: 输出信息中时间统计的单位，输入强枚举类型mtime::Unit的成员
                @return 若达到目标统计次数，则按time_unit返回平均耗时；否则返回-1   */
            long long printMeanTimeCost(const std::string& codeblock_tag, mtime::Unit time_unit = mtime::Unit::ms)
            {
                if (_g_timecounter_on && time_counter_on_)
                    return time_counter_[codeblock_tag].printMeanTimeCost(codeblock_tag, time_unit);
                return -1;
            }

            /*  在codeblock_tag代码段达到目标统计次数后输出平均消耗时间，并重新开始统计此段代码
                @param print_head: 输出信息的头部内容，推荐输入调用printMeanTimeCost的函数的名字
                @param codeblock_tag: 输出信息中被统计代码段的tag
                @param time_unit: 输出信息中时间统计的单位，输入强枚举类型mtime::Unit的成员
                @return 若达到目标统计次数，则按time_unit返回平均耗时；否则返回-1   */
            long long printMeanTimeCost(const std::string& print_head, const std::string& codeblock_tag, mtime::Unit time_unit = mtime::Unit::ms)
            {
                if (_g_timecounter_on && time_counter_on_)
                    return time_counter_[codeblock_tag].printMeanTimeCost(print_head, codeblock_tag, time_unit);
                return -1;
            }

            /*  在每个被统计的代码段达到目标统计次数后，输出其平均消耗时间并重新开始统计此段代码
                @param time_unit: 输出信息中时间统计的单位，输入强枚举类型mtime::Unit的成员   */
            void printAllMeanTimeCost(mtime::Unit time_unit = mtime::Unit::ms)
            {
                if (_g_timecounter_on && time_counter_on_)
                {
                    for (const std::string& codeblock_tag : keys_)
                    {
                        time_counter_[codeblock_tag].printMeanTimeCost(codeblock_tag, time_unit);
                    }
                }
            }

            /*  在每个被统计的代码段达到目标统计次数后，输出其平均消耗时间并重新开始统计此段代码
                @param print_head: 输出信息的头部内容，推荐输入调用printAllMeanTimeCost的函数的名字
                @param time_unit: 输出信息中时间统计的单位，输入强枚举类型mtime::Unit的成员   */
            void printAllMeanTimeCost(const std::string& print_head, mtime::Unit time_unit = mtime::Unit::ms)
            {
                if (_g_timecounter_on && time_counter_on_)
                {
                    for (const std::string& codeblock_tag : keys_)
                    {
                        time_counter_[codeblock_tag].printMeanTimeCost(print_head, codeblock_tag, time_unit);
                    }
                }
            }

        private:
            int target_count_times_;
            std::map<std::string, MeanTimeCounter> time_counter_;
            std::vector<std::string> keys_;
            bool time_counter_on_;
        };


        //统计代码块的运行时间，在创建对象时开始计时，在析构时停止计时并打印耗时
        class LocalTimeCounter
        {
        public:
            /*  构造LocalTimeCounter类
                @param codeblock_tag: 要计时的代码块标识符
                @param time_unit: 计时单位，强枚举类型mtime::Unit的成员，默认为ms
                @param time_counter_on: 是否开启计时功能，默认为true   */
            explicit LocalTimeCounter(const std::string& codeblock_tag, mtime::Unit time_unit = mtime::Unit::ms, bool time_counter_on = true) :codeblock_tag_(codeblock_tag), time_unit_(time_unit), time_counter_on_(time_counter_on)
            {
                start_t_ = mtime::now();
            }
            /*  构造LocalTimeCounter类
                @param print_head: 输出信息的头部内容，比如使用创建该类对象的函数名
                @param codeblock_tag: 要计时的代码块标识符
                @param time_unit: 计时单位，强枚举类型mtime::Unit的成员，默认为ms
                @param time_counter_on: 是否开启计时功能，默认为true   */
            explicit LocalTimeCounter(const std::string& print_head, const std::string& codeblock_tag, mtime::Unit time_unit = mtime::Unit::ms, bool time_counter_on = true) :codeblock_tag_(print_head + ": " + codeblock_tag), time_unit_(time_unit), time_counter_on_(time_counter_on)
            {
                start_t_ = mtime::now();
            }
            LocalTimeCounter(const LocalTimeCounter& _temp) = delete;
            LocalTimeCounter& operator=(const LocalTimeCounter& _temp) = delete;
            ~LocalTimeCounter()
            {
                if (_g_timecounter_on && time_counter_on_)
                {
                    end_t_ = mtime::now();
                    if (time_unit_ == mtime::Unit::s)
                        printf("%s cost time %llds\n", codeblock_tag_.c_str(), mtime::s(end_t_ - start_t_));
                    else if (time_unit_ == mtime::Unit::ms)
                        printf("%s cost time %lldms\n", codeblock_tag_.c_str(), mtime::ms(end_t_ - start_t_));
                    else if (time_unit_ == mtime::Unit::us)
                        printf("%s cost time %lldus\n", codeblock_tag_.c_str(), mtime::us(end_t_ - start_t_));
                    else if (time_unit_ == mtime::Unit::ns)
                        printf("%s cost time %lldns\n", codeblock_tag_.c_str(), mtime::ns(end_t_ - start_t_));
                    else
                        printf("%s cost time %lldms\n", codeblock_tag_.c_str(), mtime::ms(end_t_ - start_t_));
                }
            }

        private:
            const bool time_counter_on_;
            mtime::TimePoint start_t_;
            mtime::TimePoint end_t_;
            const std::string codeblock_tag_;
            const mtime::Unit time_unit_;
        };
    }
}

#endif // !TIME_HPP_MINEUTILS