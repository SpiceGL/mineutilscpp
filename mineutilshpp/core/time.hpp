// mineutils库的便利时间相关工具
// 注：如果为qnx6.6平台编译，可能需要为编译器开启宏: _GLIBCXX_USE_NANOSLEEP
#pragma once
#ifndef TIME_HPP_MINEUTILS
#define TIME_HPP_MINEUTILS

#include <array>
#include <chrono>
#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <utility>
#include <unordered_map>
#include <stdio.h>
#include <string>
#include <string.h>
#include <thread>
#include <time.h>
#include <type_traits>

#include "base.hpp"
#include "str.hpp"


namespace mineutils
{
/*--------------------------------------------用户接口--------------------------------------------*/


// 基于<chrono>库的简易计时函数封装
namespace mtime
{
    // 用于表示时间的存储数值类型，通常是long long
    using Rep = std::chrono::nanoseconds::rep;
    static_assert(std::is_signed<Rep>::value, "Rep type must be signed type!");

    // using TimeValue
    // 时间单位
    enum Unit : Rep
    {
        NSEC = 1,
        USEC = 1000,
        MSEC = 1000 * 1000,
        SEC = 1000 * 1000 * 1000
    };


    // 日期时间信息，包含常用的日期时间字段和自纪元以来的时间戳成员
    struct DateTime
    {
        int year = 0;    // 1900-现今
        int month = 0;   // 1-12
        int mday = 0;    // 1-31
        int hour = 0;    // 0-23
        int minute = 0;  // 0-59
        int second = 0;  // 0-60

        int isdst = 0;       // 1代表夏令时，0代表非夏令时，其他代表未知
        bool isutc = 0;      // UTC时间or当地时间
        bool valid = false;  // 时间数据是否有效，超出C标准库tm能表示的范围时无效
    };

    /*  为DateTime对象添加对operator<< 的支持，也添加了对mstr::toStr的支持
        - 按照"2025-01-07 17:18:35 UTC"的格式，UTC根据实际可能为DST、Local、Invalid等  */
    std::ostream& operator<<(std::ostream& cout_obj, const DateTime& date_time);


    // 时间点类，合法范围是以纳秒为单位时的Rep类型最小值到最大值，超出范围时会被截断到最小或最大值
    class TimePoint final
    {
    public:
        // 创建一个0时刻的时间点
        TimePoint() = default;

        // 通过指定时间值和单位创建时间点
        template <Unit unit>
        TimePoint& fromTimeValue(Rep time_value);

        template <Unit unit>
        Rep toTimeValue() const;

        /*  将本时间点转换为日期时间
            - 因为chrono和time_t的有效时间范围并不保证一致，因此转换可能失败，通过DateTime的valid成员判断转换是否成功
            - DateTime的有效性仅代表是否在time_t有效时间范围内转换，不会区分时间点的值来源是否为系统时间  */
        DateTime localTime() const;
        DateTime utcTime() const;

        // 根据unit计算从tp到本时间点之间的时长，溢出时会被截断到最小或最大值
        template <Unit unit>
        Rep since(const TimePoint& tp) const;

        // 在当前时间值上叠加时间段，正负均可，但结果超出合法范围时会被截断到最小或最大值
        template <Unit unit>
        TimePoint& add(Rep duration);

    private:
        DateTime tmToDateTime(const tm& timeinfo, bool is_utc) const;

        Rep timeval_ = 0;  // 以ns为单位的时间值
    };


    // 获得当前系统时间点
    TimePoint nowSystem();

    // 获取当前稳态时间点
    TimePoint nowSteady();

    // 线程休眠(秒)
    void sleep(Rep t);

    // 线程休眠(毫秒)
    void msleep(Rep t);

    // 线程休眠(微秒)
    void usleep(Rep t);

    // 线程休眠(纳秒)
    void nsleep(Rep t);


    // 只有开启时TimeCounter系列类的统计功能才生效，作用范围为当前二进制模块
    void enableGlobalTimeCounter(bool enabled);

    // 用于统计各个代码段的在一定循环次数的平均消耗时间，非线程安全
    class MeanTimeCounter final
    {
    private:
        class Guard;

    public:
        MeanTimeCounter();
        /*  构造MeanTimeCounter类
            @param count_window: 统计窗口，小于1的值会被置为1
            @param enabled: 是否开启计时功能
            @param unit: 计时单位  */
        MeanTimeCounter(Rep count_window, bool enabled = true);

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

        /*  获取最近一次统计窗口的平均时长
            @param codeblock_tag: 要统计的代码段的tag
            @param window_reached_only: 为true时仅在统计窗口达到时返回有效值，未达到时返回负数
            @return 平均消耗时间，单位由模板参数unit指定，负数代表未获取到有效值(首次统计窗口还未达到、codeblock_tag不存在等)  */
        template <Unit unit>
        Rep getMeanTimeCost(const std::string& codeblock_tag, bool window_reached_only = false);

        /*  在每个被统计的代码段达到目标统计次数后，打印其平均消耗时间并重新开始统计此段代码  
            @param print_header: 打印时的头信息(如函数名)，为空时表示无头信息  */
        template <Unit unit>
        void printOnWindowReached(std::string print_header = {});

        // 支持移动禁止拷贝
        MeanTimeCounter(MeanTimeCounter&& rvalue) = default;
        MeanTimeCounter& operator=(MeanTimeCounter&& rvalue) = default;

    private:
        class SingleCounter;

        Rep count_window_ = 1;
        bool self_enabled_ = true;
        bool final_enabled_ = true;

        // 有依赖关系
        std::list<std::string> tags_;                                        // 用于存储插入顺序
        std::unique_ptr<std::map<std::string, SingleCounter>> counter_map_;  // qnx660上一定要知道SingleCounter的内存结构才能实例化std::map<std::string, SingleCounter>
    };

    /*  统计并打印该对象从获取资源到释放资源之间的时间消耗
        - 用例：TimeCounterGuard<mtime::MSEC> guard("tag");    //获取资源
        - 用例：guard.reset("tag");    //释放旧资源，获取新资源  */
    template <Unit unit>
    class TimeCounterGuard final
    {
    public:
        // 构造一个空的对象，无资源
        TimeCounterGuard() = default;

        /*  构造对象，获取资源，统计从当前到资源释放之间的时间消耗
            @param codeblock_tag: 要计时的代码块标识符  */
        TimeCounterGuard(std::string codeblock_tag);

        // 提前释放资源并将对象置空
        void reset();
        // 释放旧资源，获取新资源
        void reset(std::string codeblock_tag);

        // 支持移动构造，禁止赋值和拷贝构造
        TimeCounterGuard(TimeCounterGuard<unit>&& obj) = default;
        TimeCounterGuard<unit>& operator=(const TimeCounterGuard<unit>& obj) = delete;
        // 释放资源
        ~TimeCounterGuard();

    private:
        void init(std::string& codeblock_tag);
        void deinit();

        bool valid_ = false;
        std::chrono::steady_clock::time_point start_t_{};
        std::string codeblock_tag_;
    };

    /*  设置从获取资源到释放资源的代码段的最短时间
        - 获取资源的时间+duration的合法范围是以纳秒为单位时Rep的最小值和最大值之间，如果超出合法范围会被截断到最小或最大值
        - 用例1：MinTimeGuard<mtime::MSEC> guard(50);    //获取资源，保证从当前到释放资源之间的代码段时间不低于50毫秒
        - 用例2：guard.reset(50);    //释放旧资源，获取新资源
        注意：在QNX660的GCC4.7.3上，实际精度为1000ns以上   */
    template <Unit unit>
    class MinTimeGuard final
    {
    public:
        // 构造空对象，无资源
        MinTimeGuard() = default;

        /*  构造对象，获取资源，确保从当前到资源释放之间的代码段时间不低于duration
            @param duration: 目标时长
            @param sleep_interval: 内部循环sleep的间隔，以unit为单位，负数时固定1ms，大于duration时置为duration
            @param quit_pred: 谓词回调，返回true时提前释放资源  */
        MinTimeGuard(Rep duration, Rep sleep_interval = -1, std::function<bool()> quit_pred = {});

        // 提前释放资源并将对象置空，释放资源时会等待剩余时间
        void reset();
        // 释放旧资源，获取新资源，释放资源时会等待剩余时间
        void reset(Rep duration, Rep sleep_interval = -1, std::function<bool()> quit_pred = {});

        // 支持移动构造，禁止赋值和拷贝构造
        MinTimeGuard(MinTimeGuard<unit>&& obj) = default;
        MinTimeGuard<unit>& operator=(const MinTimeGuard<unit>& obj) = delete;
        // 释放资源
        ~MinTimeGuard();

    private:
        void init(Rep duration, Rep sleep_interval = -1, std::function<bool()> quit_pred = {});
        void deinit();

        bool valid_ = false;
        Rep sleep_interval_ = mtime::MSEC;
        std::chrono::steady_clock::time_point target_tp_{};
        std::function<bool()> quit_pred_;
    };

}  // namespace mtime





/*--------------------------------------------内部实现--------------------------------------------*/

namespace mtime
{
    // 检查chrono::system_clock::nanoseconds / time_t的倍率关系，如果返回值小于1则代表异常
    inline Rep _checkAndCountRatioOfTimeT()
    {
        if (sizeof(Rep) < sizeof(int64_t))
        {
            mprintfW("Rep type too small to calculate ratio with time_t!\n");
            return 0;  // 类型过小不支持
        }
        if (!std::is_integral<Rep>::value || !std::is_integral<time_t>::value)
        {
            mprintfW("Rep or time_t type is not integral type, cannot calculate ratio with time_t!\n");
            return 0;  // 非整型不支持
        }
        auto ns0 = std::chrono::system_clock::time_point{std::chrono::nanoseconds(0)};
        auto ns1 = std::chrono::system_clock::time_point{std::chrono::nanoseconds(1000 * 1000 * 1000)};
        auto time_t_val0 = std::chrono::system_clock::to_time_t(ns0);
        auto time_t_val1 = std::chrono::system_clock::to_time_t(ns1);
        if (time_t_val1 <= time_t_val0)
        {
            mprintfE("Unexpected time_t difference direction, cannot calculate ratio!\n");
            return 0;
        }
        Rep ratio_a = static_cast<Rep>(1000 * 1000 * 1000 / (time_t_val1 - time_t_val0));

        ns0 = std::chrono::system_clock::time_point{std::chrono::nanoseconds(10 * 1000 * 1000 * 1000L)};
        ns1 = std::chrono::system_clock::time_point{std::chrono::nanoseconds(20 * 1000 * 1000 * 1000L)};
        time_t_val0 = std::chrono::system_clock::to_time_t(ns0);
        time_t_val1 = std::chrono::system_clock::to_time_t(ns1);
        if (time_t_val1 <= time_t_val0)
        {
            mprintfE("Unexpected time_t difference direction, cannot calculate ratio!\n");
            return 0;
        }
        Rep ratio_b = static_cast<Rep>(10 * 1000 * 1000 * 1000L / (time_t_val1 - time_t_val0));

        if (ratio_a != ratio_b || ratio_a < 1)
        {
            mprintfW("Detected unusual ratio between std::chrono::system_clock::time_point and time_t, time_t may have low resolution!\n");
            return 0;
        }

        return ratio_b;
    }


    inline std::ostream& operator<<(std::ostream& cout_obj, const DateTime& date_time)
    {
        char fmt_date[32];

        const char* fmt = nullptr;
        if (!date_time.valid)
            fmt = "%04d-%02d-%02d %02d:%02d:%02d Invalid";
        else if (date_time.isutc)
            fmt = "%04d-%02d-%02d %02d:%02d:%02d UTC";
        else if (date_time.isdst)
            fmt = "%04d-%02d-%02d %02d:%02d:%02d DST";
        else
            fmt = "%04d-%02d-%02d %02d:%02d:%02d Local";

        int n = snprintf(fmt_date, sizeof(fmt_date), fmt, date_time.year, date_time.month, date_time.mday, date_time.hour, date_time.minute, date_time.second);
        if (n < 0 || (size_t)n >= sizeof(fmt_date))
        {
            fmt_date[sizeof(fmt_date) - 1] = '\0';
            mprintfW("Unexpected snprintf failure! Please check buffer size.\n");
        }
        cout_obj << fmt_date;
        return cout_obj;
    }

    inline DateTime TimePoint::localTime() const
    {
        static Rep time_t_ratio = _checkAndCountRatioOfTimeT();
        if (time_t_ratio < 1)
        {
            mprintfW("Cannot convert TimePoint to local DateTime due to invalid time_t ratio!\n");
            return DateTime();
        }
        if (timeval_ / time_t_ratio >= std::numeric_limits<time_t>::max() || timeval_ / time_t_ratio <= std::numeric_limits<time_t>::min())
        {
            mprintfW("TimePoint value out of time_t range, cannot convert to local DateTime!\n");
            return DateTime();
        }

        std::chrono::system_clock::time_point tp{std::chrono::nanoseconds(timeval_)};
        time_t sys_time_t = std::chrono::system_clock::to_time_t(tp);
        tm buf;
#if defined(_MSC_VER)
        errno_t err = localtime_s(&buf, &sys_time_t);
        if (err != 0)
        {
            mprintfW("Got Invalid DateTime!\n");
            return DateTime();
        }
#else
        if (localtime_r(&sys_time_t, &buf) == nullptr)
        {
            mprintfW("Got Invalid DateTime!\n");
            return DateTime();
        }
#endif
        return tmToDateTime(buf, false);
    }

    inline DateTime TimePoint::utcTime() const
    {
        static Rep time_t_ratio = _checkAndCountRatioOfTimeT();
        if (time_t_ratio < 1)
        {
            mprintfW("Cannot convert TimePoint to local DateTime due to invalid time_t ratio!\n");
            return DateTime();
        }
        if (timeval_ / time_t_ratio >= std::numeric_limits<time_t>::max() || timeval_ / time_t_ratio <= std::numeric_limits<time_t>::min())
        {
            mprintfW("TimePoint value out of time_t range, cannot convert to local DateTime!\n");
            return DateTime();
        }

        std::chrono::system_clock::time_point tp{std::chrono::nanoseconds(timeval_)};
        time_t sys_time_t = std::chrono::system_clock::to_time_t(tp);
        tm buf;
#if defined(_MSC_VER)
        // Windows 特定实现
        errno_t err = gmtime_s(&buf, &sys_time_t);
        if (err != 0)
        {
            mprintfW("Got Invalid DateTime!\n");
            return DateTime();
        }
#else
        // POSIX 特定实现
        if (gmtime_r(&sys_time_t, &buf) == nullptr)
        {
            mprintfW("Got Invalid DateTime!\n");
            return DateTime();
        }
#endif
        return tmToDateTime(buf, true);
    }

    template <Unit unit>
    TimePoint& TimePoint::fromTimeValue(Rep time_value)
    {
        if (time_value < std::chrono::nanoseconds::min().count() / unit)
        {
            timeval_ = std::chrono::nanoseconds::min().count();
            mprintfW("Param time_value too small, set to min value\n");
        }
        else if (time_value > std::chrono::nanoseconds::max().count() / unit)
        {
            timeval_ = std::chrono::nanoseconds::max().count();
            mprintfW("Param time_value too large, set to max value\n");
        }
        else
            timeval_ = time_value * unit;
        return *this;
    }

    template <Unit unit>
    inline Rep TimePoint::toTimeValue() const
    {
        return timeval_ / unit;
    }

    template <Unit unit>
    inline Rep TimePoint::since(const TimePoint& tp) const
    {
        Rep duration = mbase::_safeSub(timeval_, tp.timeval_, MINE_FUNCNAME);
        return duration / unit;
    }

    template <Unit unit>
    TimePoint& TimePoint::add(Rep duration)
    {
        if (duration < std::chrono::nanoseconds::min().count() / unit)
        {
            duration = std::chrono::nanoseconds::min().count();
            mprintfW("Param duration too small, set to min supported value!\n");
        }
        else if (duration > std::chrono::nanoseconds::max().count() / unit)
        {
            duration = std::chrono::nanoseconds::max().count();
            mprintfW("Param duration too large, set to max supported value!\n");
        }
        else
            duration = duration * unit;
        timeval_ = mbase::_safeAdd(timeval_, duration, MINE_FUNCNAME);
        return *this;
    }


    inline DateTime TimePoint::tmToDateTime(const tm& timeinfo, bool is_utc) const
    {
        DateTime date_time;
        date_time.year = timeinfo.tm_year + 1900;
        date_time.month = timeinfo.tm_mon + 1;
        date_time.mday = timeinfo.tm_mday;
        date_time.hour = timeinfo.tm_hour;
        date_time.minute = timeinfo.tm_min;
        date_time.second = timeinfo.tm_sec;
        date_time.isutc = is_utc;
        if (is_utc)
            date_time.isdst = 0;
        else
            date_time.isdst = timeinfo.tm_isdst;
        date_time.valid = true;
        return date_time;
    }


    inline TimePoint nowSystem()
    {
        auto duration = std::chrono::system_clock::now().time_since_epoch();
        TimePoint systime;
        systime.fromTimeValue<mtime::NSEC>(std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count());
        return systime;
    }

    inline TimePoint nowSteady()
    {
        auto duration = std::chrono::steady_clock::now().time_since_epoch();
        TimePoint steadytime;
        steadytime.fromTimeValue<mtime::NSEC>(std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count());
        return steadytime;
    }

    // 线程休眠(秒)
    inline void sleep(Rep t)
    {
        if (t > 0)
            std::this_thread::sleep_for(std::chrono::seconds(t));
    }

    // 线程休眠(毫秒)
    inline void msleep(Rep t)
    {
        if (t > 0)
            std::this_thread::sleep_for(std::chrono::milliseconds(t));
    }

    // 线程休眠(微秒)
    inline void usleep(Rep t)
    {
        if (t > 0)
            std::this_thread::sleep_for(std::chrono::microseconds(t));
    }

    // 线程休眠(纳秒)
    inline void nsleep(Rep t)
    {
        if (t > 0)
            std::this_thread::sleep_for(std::chrono::nanoseconds(t));
    }

    inline std::atomic<bool>& _getGlobalTimeCounterEnabled()
    {
        static std::atomic<bool> g_timecounter_on(true);
        return g_timecounter_on;
    }

    inline void enableGlobalTimeCounter(bool enabled)
    {
        mtime::_getGlobalTimeCounterEnabled().store(enabled, std::memory_order_relaxed);
    }

    class MeanTimeCounter::SingleCounter final
    {
    public:
        SingleCounter() = default;

        explicit SingleCounter(Rep count_window, const char* codeblock_tag)
        {
            count_window_ = count_window;
            msg_.reserve(64);
            msg_.append("%s").append(codeblock_tag).append(" mean cost time %ss in ").append(mstr::toStr(count_window_)).append(" counts.\n");
            codeblock_tag_ = codeblock_tag;
        }

        void markStart()
        {
            start_t_ = std::chrono::steady_clock::now();
            markstart_counts_ += 1;
        }

        void markEnd()
        {
            time_cost_ += (std::chrono::steady_clock::now() - start_t_);
            markend_counts_ += 1;
            if (markend_counts_ != markstart_counts_)
            {
                mprintfW("Current times of %s: markStart=%d, markEnd=%d. Please check your code!\n", codeblock_tag_, markstart_counts_, markend_counts_);
                return;
            }
            if (markend_counts_ >= count_window_)
            {
                mean_time_cost_ = std::chrono::duration_cast<std::chrono::nanoseconds>(time_cost_).count() / markend_counts_;
                this->restart();
                can_get_ = true;
                can_print_ = true;
            }
        }

        template <Unit unit>
        Rep getMeanTimeCost(bool window_reached_only)
        {
            if (mean_time_cost_ < 0)
                return -1;
            if (window_reached_only && !can_get_)
                return -1;
            can_get_ = false;
            return mean_time_cost_ / unit;
        }

        template <Unit unit>
        void printOnWindowReached(const std::string& print_header)
        {
            if (!can_print_)
                return;
            auto mean_time_cost = mean_time_cost_ / unit;
            printf(msg_.c_str(), print_header.c_str(), mstr::toStr(mean_time_cost).c_str(), count_window_);
            can_print_ = false;
        }

    private:
        void restart()
        {
            markstart_counts_ = 0;
            markend_counts_ = 0;
            time_cost_ = std::chrono::nanoseconds(0);
        }

        Rep markstart_counts_ = 0;
        Rep markend_counts_ = 0;
        std::chrono::nanoseconds time_cost_{0};
        std::chrono::steady_clock::time_point start_t_{};
        Rep mean_time_cost_ = -1;  // 纳秒

        Rep count_window_ = 1;
        // std::string final_tag_;
        std::string msg_;
        const char* codeblock_tag_ = nullptr;
        bool can_get_ = false;
        bool can_print_ = false;
    };

    class MeanTimeCounter::Guard
    {
    public:
        Guard(Guard&& tmp) noexcept
        {
            codeblock_tag_ = std::move(tmp.codeblock_tag_);
            resource_ = tmp.resource_;
            tmp.resource_ = nullptr;
        }

        ~Guard()
        {
            if (resource_)
                resource_->markEnd(codeblock_tag_);
        }

        void reset()
        {
            if (resource_)
                resource_->markEnd(codeblock_tag_);
            resource_ = nullptr;
        }

        Guard(const Guard& tmp) = delete;
        Guard& operator=(const Guard& tmp) = delete;
        Guard& operator=(Guard&& tmp) = delete;

    private:
        Guard(MeanTimeCounter* resource, std::string& codeblock_tag)
        {
            resource->markStart(codeblock_tag);
            resource_ = resource;
            codeblock_tag_ = std::move(codeblock_tag);
        }

        MeanTimeCounter* resource_ = nullptr;
        std::string codeblock_tag_;
        friend MeanTimeCounter;
    };

    inline MeanTimeCounter::MeanTimeCounter()
    {
        count_window_ = 1;
        self_enabled_ = true;
        final_enabled_ = mtime::_getGlobalTimeCounterEnabled().load(std::memory_order_acquire);
        counter_map_.reset(new std::map<std::string, SingleCounter>);
    }

    inline MeanTimeCounter::MeanTimeCounter(Rep count_window, bool enabled)
    {
        if (count_window < 1) {
            mprintfW("Param count_window %s invalid, set to 1!\n", mstr::toStr(count_window).c_str());
            count_window = 1;
        }
        count_window_ = count_window;
        self_enabled_ = enabled;
        final_enabled_ = mtime::_getGlobalTimeCounterEnabled().load(std::memory_order_acquire) && enabled;
        counter_map_.reset(new std::map<std::string, SingleCounter>);
    }

    inline void MeanTimeCounter::markStart(const std::string& codeblock_tag)
    {
        if (!final_enabled_)
            return;
        if (counter_map_->end() == counter_map_->find(codeblock_tag))
        {
            tags_.emplace_back(codeblock_tag);
            (*counter_map_)[codeblock_tag] = MeanTimeCounter::SingleCounter(count_window_, tags_.back().c_str());
        }
        (*counter_map_)[codeblock_tag].markStart();
    }

    inline void MeanTimeCounter::markEnd(const std::string& codeblock_tag)
    {
        if (!final_enabled_)
            return;
        if (counter_map_->end() == counter_map_->find(codeblock_tag))
        {
            mprintfW("Please call \"markStart(%s)\" before \"markEnd(%s)\"!\n", codeblock_tag.c_str(), codeblock_tag.c_str());
            return;
        }
        (*counter_map_)[codeblock_tag].markEnd();
    }

    inline MeanTimeCounter::Guard MeanTimeCounter::markGuard(std::string codeblock_tag)
    {
        return MeanTimeCounter::Guard(this, codeblock_tag);
    }

    template <Unit unit>
    inline Rep MeanTimeCounter::getMeanTimeCost(const std::string& codeblock_tag, bool window_reached_only)
    {
        final_enabled_ = mtime::_getGlobalTimeCounterEnabled().load(std::memory_order_relaxed) && self_enabled_;
        if (!final_enabled_)
            return -1;
        if (counter_map_->end() == counter_map_->find(codeblock_tag))
        {
            mprintfW("No such codeblock_tag \"%s\" found in MeanTimeCounter!\n", codeblock_tag.c_str());
            return -1;
        }
        return (*counter_map_)[codeblock_tag].getMeanTimeCost<unit>(window_reached_only);
    }

    template <Unit unit>
    inline void MeanTimeCounter::printOnWindowReached(std::string print_header)
    {
        final_enabled_ = mtime::_getGlobalTimeCounterEnabled().load(std::memory_order_relaxed) && self_enabled_;
        if (!final_enabled_)
            return;
        if (!print_header.empty())
            print_header.append(": ");
        for (const std::string& tag : tags_)
        {
            (*counter_map_)[tag].printOnWindowReached<unit>(print_header);
        }
    }

    template <Unit unit>
    inline TimeCounterGuard<unit>::TimeCounterGuard(std::string codeblock_tag)
    {
        this->init(codeblock_tag);
    }

    template <Unit unit>
    inline void TimeCounterGuard<unit>::reset()
    {
        this->deinit();
    }
    template <Unit unit>
    inline void TimeCounterGuard<unit>::reset(std::string codeblock_tag)
    {
        this->deinit();
        this->init(codeblock_tag);
    }

    template <Unit unit>
    inline TimeCounterGuard<unit>::~TimeCounterGuard()
    {
        this->deinit();
    }

    template <Unit unit>
    inline void TimeCounterGuard<unit>::init(std::string& codeblock_tag)
    {
        start_t_ = std::chrono::steady_clock::now();
        codeblock_tag_ = std::move(codeblock_tag);
        valid_ = true;
    }

    template <Unit unit>
    inline void TimeCounterGuard<unit>::deinit()
    {
        if (valid_ && mtime::_getGlobalTimeCounterEnabled().load(std::memory_order_relaxed))
        {
            Rep time_cost;
            const char* msg = nullptr;
            switch (unit)
            {
            case mtime::SEC:
                time_cost = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start_t_).count();
                msg = "%s cost time %ss\n";
                break;
            case mtime::MSEC:
                time_cost = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_t_).count();
                msg = "%s cost time %sms\n";
                break;
            case mtime::USEC:
                time_cost = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start_t_).count();
                msg = "%s cost time %sus\n";
                break;
            case mtime::NSEC:
                time_cost = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - start_t_).count();
                msg = "%s cost time %sns\n";
                break;
            default:
                time_cost = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_t_).count();
                msg = "%s cost time %sms\n";
                break;
            }
            printf(msg, codeblock_tag_.c_str(), mstr::toStr(time_cost).c_str());
        }
        valid_ = false;
    }

    template <Unit unit>
    inline MinTimeGuard<unit>::MinTimeGuard(Rep duration, Rep sleep_interval, std::function<bool()> quit_pred)
    {
        this->init(duration, sleep_interval, std::move(quit_pred));
    }

    template <Unit unit>
    inline MinTimeGuard<unit>::~MinTimeGuard()
    {
        this->deinit();
    }

    template <Unit unit>
    inline void MinTimeGuard<unit>::reset()
    {
        this->deinit();
    }

    template <Unit unit>
    inline void MinTimeGuard<unit>::reset(Rep duration, Rep sleep_interval, std::function<bool()> quit_pred)
    {
        this->deinit();
        this->init(duration, sleep_interval, std::move(quit_pred));
    }

    template <Unit unit>
    inline void MinTimeGuard<unit>::init(Rep duration, Rep sleep_interval, std::function<bool()> quit_pred)
    {
        auto now_tp = std::chrono::steady_clock::now();
        auto now_timeval = std::chrono::duration_cast<std::chrono::nanoseconds>(now_tp.time_since_epoch()).count();
        if (duration < std::chrono::nanoseconds::min().count() / unit)
        {
            duration = std::chrono::nanoseconds::min().count();
            mprintfW("Param duration too small, set to min supported value\n");
        }
        else if (duration > std::chrono::nanoseconds::max().count() / unit)
        {
            duration = std::chrono::nanoseconds::max().count();
            mprintfW("Param duration too large, set to max supported value\n");
        }
        else
            duration = duration * unit;
        Rep target_tp_val = mbase::_safeAdd(now_timeval, duration, MINE_FUNCNAME);  // 防止溢出
        target_tp_ = std::chrono::steady_clock::time_point(std::chrono::nanoseconds(target_tp_val));
        if (sleep_interval <= 0)
            sleep_interval_ = mtime::MSEC;
        else if (sleep_interval > duration / unit)
            sleep_interval_ = duration;
        else
            sleep_interval_ = sleep_interval * unit;
        if (!quit_pred)
            quit_pred = []() { return false; };
        quit_pred_ = std::move(quit_pred);
        valid_ = true;
    }

    template <Unit unit>
    inline void MinTimeGuard<unit>::deinit()
    {
        if (valid_)
        {
            Rep sleep_interval_shorter = sleep_interval_ / 10 > 0 ? sleep_interval_ / 10 : 1;
            // 先睡大块时间，减少系统调用开销，再精确等待剩余时间
            while (!quit_pred_())
            {
                auto remain_duration = target_tp_ - std::chrono::steady_clock::now();
                if (remain_duration > std::chrono::nanoseconds(sleep_interval_))
                {
                    std::this_thread::sleep_for(std::chrono::nanoseconds(sleep_interval_));
                }
                else if (remain_duration > std::chrono::nanoseconds(0))
                {
                    std::this_thread::sleep_for(std::chrono::nanoseconds(sleep_interval_shorter));
                }
                else
                {
                    break;
                }
            }
        }
        valid_ = false;
        quit_pred_ = {};
        target_tp_ = std::chrono::steady_clock::time_point{};
    }

}  // namespace mtime





/*--------------------------------------------单元测试--------------------------------------------*/
#ifdef MINEUTILS_TEST_MODULES
namespace _mtimecheck
{
    inline void MeanTimeCounterTest()
    {
        mtime::MeanTimeCounter time_counter{10, true};
        {
            std::string tags[2] = {"Guard 1", "Guard 2"};
            for (int i = 0; i < 10; i++)
            {
                auto guard = time_counter.markGuard(tags[1]);
                mtime::msleep(60);
            }
            for (int i = 0; i < 10; i++)
            {
                auto guard = time_counter.markGuard(tags[0]);
                mtime::msleep(60);
            }
            printf("Author check! Expected output: \nMeanTimeCounterTest: Guard 2 mean cost time 60ms in 10 counts  \nMeanTimeCounterTest: Guard 1 mean cost time 60ms in 10 counts.\n");
            printf("Mean time cost of Guard 2: 60ms\nMean time cost of Guard 1: 60ms\n");
            printf("Mean time cost of Guard 2: -1ms\nMean time cost of Guard 1: -1ms\n");
            printf("Actual output:\n");
            time_counter.printOnWindowReached<mtime::MSEC>(__func__);
            printf("Mean time cost of Guard 2: %sms\n", mstr::toStr(time_counter.getMeanTimeCost<mtime::MSEC>("Guard 2", true)).c_str());
            printf("Mean time cost of Guard 1: %sms\n", mstr::toStr(time_counter.getMeanTimeCost<mtime::MSEC>("Guard 1", true)).c_str());
            printf("Mean time cost of Guard 2: %sms\n", mstr::toStr(time_counter.getMeanTimeCost<mtime::MSEC>("Guard 2", true)).c_str());
            printf("Mean time cost of Guard 1: %sms\n\n", mstr::toStr(time_counter.getMeanTimeCost<mtime::MSEC>("Guard 1", true)).c_str());
        }
    }

    inline void nowTest()
    {
        auto start_t = mtime::nowSystem();
        mtime::msleep(60);
        auto end_t = mtime::nowSystem();
        // mtime::SystemTime<mtime::USEC> start_t1 = start_t;
        // mtime::SystemTime<mtime::USEC> end_t1 = end_t;

        printf("Author check! Got result(60ms):%sms\n", mstr::toStr(end_t.since<mtime::MSEC>(start_t)).c_str());
        // printf("Author check! Got result:%lldus(60000us)\n", end_t1 - start_t1);
        auto start_t2 = mtime::nowSystem();
        {
            mtime::MinTimeGuard<mtime::USEC> guard(60 * 1000, 1);
            {
                guard.reset(60 * 1000, 1);
            }
        }
        auto end_t2 = mtime::nowSystem();
        printf("Author check! Got result(120ms):%sms\n", mstr::toStr(end_t2.since<mtime::MSEC>(start_t2)).c_str());

        auto now_tp = mtime::nowSystem();
        mtime::DateTime now_time = now_tp.localTime();
        mtime::DateTime now_utc_time = now_tp.utcTime();
        mtime::DateTime min_utc_time = mtime::TimePoint().fromTimeValue<mtime::USEC>(std::chrono::nanoseconds::min().count()).utcTime();
        mtime::DateTime max_utc_time = mtime::TimePoint().fromTimeValue<mtime::USEC>(std::chrono::nanoseconds::max().count()).utcTime();
        printf("Author checks next date format below!\n");
        std::cout << now_time << std::endl;
        std::cout << now_utc_time << std::endl;
        std::cout << min_utc_time << std::endl;
        std::cout << max_utc_time << std::endl;
        std::cout << mtime::DateTime() << std::endl;
    }

    inline void check()
    {
        printf("\n--------------------check mtime start--------------------\n");
        MeanTimeCounterTest();
        nowTest();
        printf("---------------------check mtime end---------------------\n\n");
    }

}  // namespace _mtimecheck
#endif
}  // namespace mineutils

#endif  // !TIME_HPP_MINEUTILS