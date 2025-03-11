//mineutils库的数学相关功能
#pragma once
#ifndef MATH_HPP_MINEUTILS
#define MATH_HPP_MINEUTILS

#include<array>
#include<cmath>
#include<cstdlib>
#include<iostream>
#include<string>
#include<string.h>

#include"base.hpp"


namespace mineutils
{
    /*--------------------------------------------用户接口--------------------------------------------*/
    namespace mmath
    {
        template<class T>
        class RectLTRB;
        template<class T>
        class RectLTWH;
        template<class T>
        class RectXYWH;

        /*  左上角-右下角：l、t、r、b排列的rect坐标，T只支持整型和浮点型，且不能为const、引用和volatile
            - 若T为整数类型，表示数据为像素类型，为连续数值
            - 数据为像素类型时的计算规则和连续数值不一样
            - 数据为像素类型时，若L>R或T>B，Rect无效
            - 数据为连续数值时，若L>=R或T>=B，Rect无效  
            - 存在溢出时不保证结果正确  */
        template<class T>
        class RectLTRB
        {
        public:
            template<class U = T, typename std::enable_if<!std::is_const<U>::value && !std::is_reference<U>::value && !std::is_volatile<U>::value && std::is_same<U, T>::value && (std::is_integral<U>::value || std::is_floating_point<U>::value), int>::type = 0>
            RectLTRB();

            template<class U = T, typename std::enable_if<!std::is_const<U>::value && !std::is_reference<U>::value && !std::is_volatile<U>::value && std::is_same<U, T>::value && (std::is_integral<U>::value || std::is_floating_point<U>::value), int>::type = 0>
            RectLTRB(T left_x, T top_y, T right_x, T bottom_y);

            RectLTRB(const RectLTRB<T>& rect);

            RectLTRB<T>& operator=(const RectLTRB<T>& rect);

            bool operator==(const RectLTRB<T>& rect) const;

            T& operator[](int idx);
            const T& operator[](int idx) const;

            //判断当前Rect数据是否有效
            bool valid() const;
            //将坐标转化为像素坐标，向下取整
            template<class PixT = int, typename std::enable_if<!std::is_const<PixT>::value && !std::is_reference<PixT>::value && !std::is_volatile<PixT>::value && std::is_integral<PixT>::value, int>::type = 0>
            RectLTRB<PixT> toPixel() const;
            //若自身数据为整数类型，则按像素运算，否则按连续数值运算
            RectXYWH<T> toXYWH() const;
            //若自身数据为整数类型，则按像素运算，否则按连续数值运算
            RectLTWH<T> toLTWH() const;
            //裁剪自身位于另一个rect内的部分。若结果为无效代表无交集
            RectLTRB<T> clipTo(const RectLTRB<T>& other) const;
            //计算面积，注意T的范围避免溢出
            T area() const;

        private:
            std::array<T, 4> data_;
        };

        /*  左上角-宽高：l、t、w、h排列的rect坐标，T只支持整型和浮点型，且不能为const、引用和volatile
            - 若T为整数类型，表示数据为像素类型，为连续数
            - 数据为像素类型时的计算规则和连续数值不一样
            - W<=0或H<=0时，Rect无效
            - 存在溢出时不保证结果正确  */
        template<class T>
        class RectLTWH
        {
        public:
            template<class U = T, typename std::enable_if<!std::is_const<U>::value && !std::is_reference<U>::value && !std::is_volatile<U>::value && std::is_same<U, T>::value && (std::is_integral<U>::value || std::is_floating_point<U>::value), int>::type = 0>
            RectLTWH();

            template<class U = T, typename std::enable_if<!std::is_const<U>::value && !std::is_reference<U>::value && !std::is_volatile<U>::value && std::is_same<U, T>::value && (std::is_integral<U>::value || std::is_floating_point<U>::value), int>::type = 0>
            RectLTWH(T left_x, T top_y, T width, T height);

            RectLTWH(const RectLTWH<T>& rect);

            RectLTWH<T>& operator=(const RectLTWH<T>& rect);

            bool operator==(const RectLTWH<T>& rect) const;

            T& operator[](int idx);
            const T& operator[](int idx) const;

            //判断当前Rect数据是否有效
            bool valid() const;
            //将坐标转化为像素坐标，向下取整
            template<class PixT = int, typename std::enable_if<!std::is_const<PixT>::value && !std::is_reference<PixT>::value && !std::is_volatile<PixT>::value && std::is_integral<PixT>::value, int>::type = 0>
            RectLTWH<PixT> toPixel() const;
            //若自身数据为整数类型，则按像素运算，否则按连续数值运算
            RectLTRB<T> toLTRB() const;
            //若自身数据为整数类型，则按像素运算，否则按连续数值运算
            RectXYWH<T> toXYWH() const;
            //裁剪自身位于另一个rect内的部分。若结果为无效代表无交集
            RectLTWH<T> clipTo(const RectLTWH<T>& other) const;
            //计算面积，注意T的范围避免溢出
            T area() const;

        private:
            std::array<T, 4> data_;
        };

        /*  中心-宽高：x、y、w、h排列的rect坐标，T只支持整型和浮点型，且不能为const、引用和volatile 
            - 若T为整数类型，表示数据为像素类型，为连续数值`
            - 数据为像素类型时的计算规则和连续数值不一样
            - W<=0或H<=0时，Rect无效
            - 存在溢出时不保证结果正确  */
        template<class T>
        class RectXYWH
        {
        public:
            template<class U = T, typename std::enable_if<!std::is_const<U>::value && !std::is_reference<U>::value && !std::is_volatile<U>::value && std::is_same<U, T>::value && (std::is_integral<U>::value || std::is_floating_point<U>::value), int>::type = 0>
            RectXYWH();

            template<class U = T, typename std::enable_if<!std::is_const<U>::value && !std::is_reference<U>::value && !std::is_volatile<U>::value && std::is_same<U, T>::value && (std::is_integral<U>::value || std::is_floating_point<U>::value), int>::type = 0>
            RectXYWH(T center_x, T center_y, T width, T height);

            RectXYWH(const RectXYWH<T>& rect);

            RectXYWH<T>& operator=(const RectXYWH<T>& rect);

            bool operator==(const RectXYWH<T>& rect) const;

            T& operator[](int idx);
            const T& operator[](int idx) const;

            //判断当前Rect数据是否有效
            bool valid() const;
            //将坐标转化为像素坐标，向下取整
            template<class PixT = int, typename std::enable_if<!std::is_const<PixT>::value && !std::is_reference<PixT>::value && !std::is_volatile<PixT>::value && std::is_integral<PixT>::value, int>::type = 0>
            RectXYWH<PixT> toPixel() const;
            //若自身数据为整数类型，则按像素运算，否则按连续数值运算
            RectLTRB<T> toLTRB() const;
            //若自身数据为整数类型，则按像素运算，否则按连续数值运算
            RectLTWH<T> toLTWH() const;
            //裁剪自身位于另一个rect内的部分。若结果为无效代表无交集
            RectXYWH<T> clipTo(const RectXYWH<T>& other) const;
            //计算面积，注意T的范围避免溢出
            T area() const;

        private:
            std::array<T, 4> data_;
        };

        //为operator<<添加对RectBase及其子类的支持
        template<class T>
        std::ostream& operator<<(std::ostream& cout_obj, const RectLTRB<T>& rect);        
        template<class T>
        std::ostream& operator<<(std::ostream& cout_obj, const RectLTWH<T>& rect);        
        template<class T>
        std::ostream& operator<<(std::ostream& cout_obj, const RectXYWH<T>& rect);
    }





    /*--------------------------------------------内部实现--------------------------------------------*/

    namespace mmath
    {
        inline long _align(long value, long alignment, int align_mode = 1)
        {
            long rem = value % alignment;
            long quot = value / alignment;

            if (align_mode > 0)
            {
                if (rem > 0)
                    return quot * alignment + std::labs(alignment);
                else return quot * alignment;
            }
            else if (align_mode < 0)
            {
                if (rem >= 0)
                    return quot * alignment;
                else return quot * alignment - std::labs(alignment);
            }
            else
            {
                if (rem > 0)
                {
                    if (std::labs(rem) >= (std::labs(alignment) / 2))
                        return quot * alignment + std::labs(alignment);
                    else return quot * alignment;
                }
                else if (rem < 0)
                {
                    if (std::labs(rem) > (std::labs(alignment) / 2))
                        return quot * alignment - std::labs(alignment);
                    else return quot * alignment;
                }
                else return quot * alignment;
            }
        }


        /*------------------------------LTRB-----------------------------*/
        template<class T>
        template<class U, typename std::enable_if<!std::is_const<U>::value && !std::is_reference<U>::value && !std::is_volatile<U>::value&& std::is_same<U, T>::value && (std::is_integral<U>::value || std::is_floating_point<U>::value), int>::type>
        inline RectLTRB<T>::RectLTRB()
        {
            this->data_[0] = 1;
            this->data_[1] = 1;
            this->data_[2] = 0;
            this->data_[3] = 0;
        }

        template<class T>
        template<class U, typename std::enable_if<!std::is_const<U>::value && !std::is_reference<U>::value && !std::is_volatile<U>::value && std::is_same<U, T>::value && (std::is_integral<U>::value || std::is_floating_point<U>::value), int>::type>
        inline RectLTRB<T>::RectLTRB(T left_x, T top_y, T right_x, T bottom_y)
        {
            this->data_[0] = left_x;
            this->data_[1] = top_y;
            this->data_[2] = right_x;
            this->data_[3] = bottom_y;
        }

        template<class T>
        inline RectLTRB<T>::RectLTRB(const RectLTRB<T>& rect)
        {
            *this = rect;
        }

        template<class T>
        inline RectLTRB<T>& RectLTRB<T>::operator=(const RectLTRB<T>& rect)
        {
            if (this != &rect)
                this->data_ = rect.data_;           
            return *this;
        }

        template<class T>
        inline bool RectLTRB<T>::operator==(const RectLTRB<T>& rect) const
        {
            return this->data_ == rect.data_;
        }

        template<class T>
        inline T& RectLTRB<T>::operator[](int idx)
        {
            return this->data_[idx];
        }

        template<class T>
        inline const T& RectLTRB<T>::operator[](int idx) const
        {
            return this->data_[idx];
        }

        template<class T>
        inline bool RectLTRB<T>::valid() const
        {
            if (std::is_integral<T>::value)
                return (this->data_[0] <= this->data_[2]) && (this->data_[1] <= this->data_[3]);
            else return (this->data_[0] < this->data_[2]) && (this->data_[1] < this->data_[3]);
        }

        //将坐标转化为像素坐标
        template<class T> 
        template<class PixT, typename std::enable_if<!std::is_const<PixT>::value && !std::is_reference<PixT>::value && !std::is_volatile<PixT>::value && std::is_integral<PixT>::value, int>::type>
        inline RectLTRB<PixT> RectLTRB<T>::toPixel() const
        {
            return { static_cast<PixT>(std::floor(this->data_[0])), static_cast<PixT>(std::floor(this->data_[1])), static_cast<PixT>(std::floor(this->data_[2])), static_cast<PixT>(std::floor(this->data_[3])) };
        }

        template<class T>
        inline RectXYWH<T> RectLTRB<T>::toXYWH() const
        {
            if (std::is_integral<T>::value)
            {
                T w = this->data_[2] - this->data_[0] + 1;
                T h = this->data_[3] - this->data_[1] + 1;
                T x = this->data_[0] + std::ceil((w - 1) / 2.f);
                T y = this->data_[1] + std::ceil((h - 1) / 2.f);
                return { x,y,w,h };
            }
            else
            {
                T w = this->data_[2] - this->data_[0];
                T h = this->data_[3] - this->data_[1];
                T x = this->data_[0] + w / 2;
                T y = this->data_[1] + h / 2;
                return { x,y,w,h };;
            }
        }

        template<class T>
        inline RectLTWH<T> RectLTRB<T>::toLTWH() const
        {
            T w = this->data_[2] - this->data_[0];
            T h = this->data_[3] - this->data_[1];
            if (std::is_integral<T>::value)
            {
                w += 1;
                h += 1;
            }
            return { this->data_[0], this->data_[1], w, h };
        }

        template<class T>
        inline RectLTRB<T> RectLTRB<T>::clipTo(const RectLTRB<T>& other) const
        {
            if (!this->valid() && !other.valid())   //如果比较的双方有一个无效，返回无效的结果
                return { 1, 1, 0, 0 };
            RectLTRB<T> tmp = *this;

            if (tmp.data_[0] > other.data_[2] || tmp.data_[2] < other.data_[0] || tmp.data_[1] > other.data_[3] || tmp.data_[3] < other.data_[1])
                return { 1, 1, 0, 0 };   //如果两者不相交，返回无效的结果
            if (tmp.data_[0] < other.data_[0])
                tmp.data_[0] = other.data_[0];
            if (tmp.data_[1] < other.data_[1])
                tmp.data_[1] = other.data_[1];
            if (tmp.data_[2] > other.data_[2])
                tmp.data_[2] = other.data_[2];
            if (tmp.data_[3] > other.data_[3])
                tmp.data_[3] = other.data_[3];
            return tmp;
        }

        template<class T>
        inline T RectLTRB<T>::area() const
        {
            if (!this->valid())
                return 0;
            if (std::is_integral<T>::value)
                return (this->data_[2] - this->data_[0] + 1) * (this->data_[3] - this->data_[1] + 1);
            else return (this->data_[2] - this->data_[0]) * (this->data_[3] - this->data_[1]);
        }

        /*------------------------------LTWH-----------------------------*/


        template<class T>
        template<class U, typename std::enable_if<!std::is_const<U>::value && !std::is_reference<U>::value && !std::is_volatile<U>::value&& std::is_same<U, T>::value && (std::is_integral<U>::value || std::is_floating_point<U>::value), int>::type>
        inline RectLTWH<T>::RectLTWH()
        {
            this->data_[0] = 1;
            this->data_[1] = 1;
            this->data_[2] = 0;
            this->data_[3] = 0;
        }

        template<class T>
        template<class U, typename std::enable_if<!std::is_const<U>::value && !std::is_reference<U>::value && !std::is_volatile<U>::value && std::is_same<U, T>::value && (std::is_integral<U>::value || std::is_floating_point<U>::value), int>::type>
        inline RectLTWH<T>::RectLTWH(T left_x, T top_y, T width, T height)
        {
            this->data_[0] = left_x;
            this->data_[1] = top_y;
            this->data_[2] = width;
            this->data_[3] = height;
        }

        template<class T>
        inline RectLTWH<T>::RectLTWH(const RectLTWH<T>& rect)
        {
            *this = rect;
        }

        template<class T>
        inline RectLTWH<T>& RectLTWH<T>::operator=(const RectLTWH<T>& rect)
        {
            if (this != &rect)
                this->data_ = rect.data_;
            return *this;
        }

        template<class T>
        inline bool RectLTWH<T>::operator==(const RectLTWH<T>& rect) const
        {
            return this->data_ == rect.data_;
        }

        template<class T>
        inline T& RectLTWH<T>::operator[](int idx)
        {
            return this->data_[idx];
        }

        template<class T>
        inline const T& RectLTWH<T>::operator[](int idx) const
        {
            return this->data_[idx];
        }

        template<class T>
        inline bool RectLTWH<T>::valid() const
        {
            return this->data_[2] > 0 && this->data_[3] > 0;
        }

        /*将坐标转化为像素坐标，向下取整*/
        template<class T> template<class PixT, typename std::enable_if<!std::is_const<PixT>::value && !std::is_reference<PixT>::value && !std::is_volatile<PixT>::value && std::is_integral<PixT>::value, int>::type>
        inline RectLTWH<PixT> RectLTWH<T>::toPixel() const
        {
            return { static_cast<PixT>(std::floor(this->data_[0])), static_cast<PixT>(std::floor(this->data_[1])), static_cast<PixT>(std::floor(this->data_[2])), static_cast<PixT>(std::floor(this->data_[3])) };
        }

        template<class T>
        inline RectLTRB<T> RectLTWH<T>::toLTRB() const
        {
            T r = this->data_[0] + this->data_[2];
            T b = this->data_[1] + this->data_[3];
            if (std::is_integral<T>::value)
            {
                r -= 1;
                b -= 1;
            }
            return { this->data_[0], this->data_[1], r, b };
        }

        template<class T>
        inline RectXYWH<T> RectLTWH<T>::toXYWH() const
        {
            if (std::is_integral<T>::value)
            {
                T x = this->data_[0] + static_cast<T>(std::ceil((this->data_[2] - 1) / 2.f));
                T y = this->data_[1] + static_cast<T>(std::ceil((this->data_[3] - 1) / 2.f));
                return { x, y, this->data_[2], this->data_[3] };
            }
            else
            {
                T x = this->data_[0] + this->data_[2] / 2;
                T y = this->data_[1] + this->data_[3] / 2;
                return { x, y, this->data_[2], this->data_[3] };
            }
        }

        template<class T>
        inline RectLTWH<T> RectLTWH<T>::clipTo(const RectLTWH<T>& other) const
        {
            if (!this->valid() && !other.valid())   //如果比较的双方有一个无效，返回无效的结果
                return { 1, 1, 0, 0 };
            RectLTRB<T> tmp = this->toLTRB();
            RectLTRB<T> tmp_other = other.toLTRB();
            return tmp.clipTo(tmp_other).toLTWH();
        }

        template<class T>
        inline T RectLTWH<T>::area() const
        {
            if (!this->valid())
                return 0;
            return this->data_[2] * this->data_[3];
        }

        /*------------------------------XYWH-----------------------------*/

        template<class T>
        template<class U, typename std::enable_if<!std::is_const<U>::value && !std::is_reference<U>::value && !std::is_volatile<U>::value&& std::is_same<U, T>::value && (std::is_integral<U>::value || std::is_floating_point<U>::value), int>::type>
        inline RectXYWH<T>::RectXYWH()
        {
            this->data_[0] = 1;
            this->data_[1] = 1;
            this->data_[2] = 0;
            this->data_[3] = 0;
        }

        template<class T>
        template<class U, typename std::enable_if<!std::is_const<U>::value && !std::is_reference<U>::value && !std::is_volatile<U>::value && std::is_same<U, T>::value && (std::is_integral<U>::value || std::is_floating_point<U>::value), int>::type>
        inline RectXYWH<T>::RectXYWH(T center_x, T center_y, T width, T height)
        {
            this->data_[0] = center_x;
            this->data_[1] = center_y;
            this->data_[2] = width;
            this->data_[3] = height;
        }

        template<class T>
        inline RectXYWH<T>::RectXYWH(const RectXYWH<T>& rect)
        {
            *this = rect;
        }

        template<class T>
        inline RectXYWH<T>& RectXYWH<T>::operator=(const RectXYWH<T>& rect)
        {
            if (this != &rect)
                this->data_ = rect.data_;
            return *this;
        }

        template<class T>
        inline bool RectXYWH<T>::operator==(const RectXYWH<T>& rect) const
        {
            return this->data_ == rect.data_;
        }

        template<class T>
        inline T& RectXYWH<T>::operator[](int idx)
        {
            return this->data_[idx];
        }

        template<class T>
        inline const T& RectXYWH<T>::operator[](int idx) const
        {
            return this->data_[idx];
        }

        template<class T>
        inline bool RectXYWH<T>::valid() const
        {
            return this->data_[2] > 0 && this->data_[3] > 0;
        }

        //将坐标转化为像素坐标
        template<class T> template<class PixT, typename std::enable_if<!std::is_const<PixT>::value && !std::is_reference<PixT>::value && !std::is_volatile<PixT>::value && std::is_integral<PixT>::value, int>::type>
        inline RectXYWH<PixT> RectXYWH<T>::toPixel() const
        {
            return { static_cast<PixT>(std::floor(this->data_[0])), static_cast<PixT>(std::floor(this->data_[1])), static_cast<PixT>(std::floor(this->data_[2])), static_cast<PixT>(std::floor(this->data_[3])) };
        }

        template<class T>
        inline RectLTRB<T> RectXYWH<T>::toLTRB() const
        {
            if (std::is_integral<T>::value)
            {
                T l = this->data_[0] - std::ceil((this->data_[2] - 1) / 2.f);
                T t = this->data_[1] - std::ceil((this->data_[3] - 1) / 2.f);
                T r = l + this->data_[2] - 1;
                T b = t + this->data_[3] - 1;
                return { l,t,r,b };
            }
            else
            {
                T l = this->data_[0] - this->data_[2] / 2;
                T t = this->data_[1] - this->data_[3] / 2;
                T r = l + this->data_[2];
                T b = t + this->data_[3];
                return { l,t,r,b };
            }
        }

        template<class T>
        inline RectLTWH<T> RectXYWH<T>::toLTWH() const
        {
            if (std::is_integral<T>::value)
            {
                T l = this->data_[0] - std::ceil((this->data_[2] - 1) / 2.f);
                T t = this->data_[1] - std::ceil((this->data_[3] - 1) / 2.f);
                return { l, t, this->data_[2], this->data_[3] };
            }
            else
            {
                T l = this->data_[0] - this->data_[2] / 2;
                T t = this->data_[1] - this->data_[3] / 2;
                return { l, t, this->data_[2], this->data_[3] };
            }
        }

        template<class T>
        inline RectXYWH<T> RectXYWH<T>::clipTo(const RectXYWH<T>& other) const
        {
            if (!this->valid() && !other.valid())   //如果比较的双方有一个无效，返回无效的结果
                return { 1, 1, 0, 0 };
            RectLTRB<T> tmp = this->toLTRB();
            RectLTRB<T> tmp_other = other.toLTRB();
            return tmp.clipTo(tmp_other).toXYWH();
        }

        template<class T>
        inline T RectXYWH<T>::area() const
        {
            if (!this->valid())
                return 0;
            return this->data_[2] * this->data_[3];
        }

        /*------------------------------------------------------------------------------*/

        //为Rect系列对象添加对operator<< 的支持
        template<class T>
        inline std::ostream& operator<<(std::ostream& cout_obj, const RectLTRB<T>& rect)
        {
            cout_obj << "[" << rect[0] + 0 << " " << rect[1] + 0 << " "
                << rect[2] + 0 << " " << rect[3] + 0 << "]";
            return cout_obj;
        }

        template<class T>
        inline std::ostream& operator<<(std::ostream& cout_obj, const RectLTWH<T>& rect)
        {
            cout_obj << "[" << rect[0] + 0 << " " << rect[1] + 0 << " "
                << rect[2] + 0 << " " << rect[3] + 0 << "]";
            return cout_obj;
        }

        template<class T>
        inline std::ostream& operator<<(std::ostream& cout_obj, const RectXYWH<T>& rect)
        {
            cout_obj << "[" << rect[0] + 0 << " " << rect[1] + 0 << " "
                << rect[2] + 0 << " " << rect[3] + 0 << "]";
            return cout_obj;
        }
    }





#ifdef MINEUTILS_TEST_MODULES
    namespace _mmathcheck
    {
        inline void RectTest()
        {
            bool ret0;
            ret0 = (mmath::RectLTRB<int>(0, 0, 100, 200).toLTWH() == mmath::RectLTWH<int>(0, 0, 101, 201));
            if (!ret0) mprintfE("Failed when check: mmath::RectLTRB<int>(0, 0, 100, 200).toLTWH() == mmath::RectLTWH<int>(0, 0, 101, 201)\n");

            ret0 = (mmath::RectLTWH<int>(0, 0, 100, 200).toXYWH() == mmath::RectXYWH<int>(50, 100, 100, 200));
            if (!ret0) mprintfE("Failed when check: mmath::RectLTWH<int>(0, 0, 100, 200).toXYWH() == mmath::RectXYWH<int>(50, 100, 100, 200)\n");

            ret0 = (mmath::RectLTWH<double>(0.1, 0.9, 100.5, 200.3).toPixel<int>() == mmath::RectLTWH<int>(0, 0, 100, 200));
            if (!ret0) mprintfE("Failed when check: mmath::RectLTWH<double>(0.1, 0.9, 100.5, 200.3).toPixel<int>() == mmath::RectLTWH<int>(0, 0, 100, 200)\n");

            mmath::RectLTRB<int> rect0(0, 0, 100, 200);
            mmath::RectLTRB<int> rect1(100, 200, 200, 300);
            mmath::RectLTRB<int> rect2(50, 51, 200, 300);
            ret0 = rect0.clipTo(rect1).valid();
            if (!ret0) mprintfE("Failed when check: rect0.clipTo(rect1).valid()\n");
            ret0 = rect0.clipTo(rect2) == mmath::RectLTRB<int>(50, 51, 100, 200);
            if (!ret0) mprintfE("Failed when check: rect0.clipTo(rect2) == mmath::RectLTRB<int>(50, 51, 100, 200)\n");

            mmath::RectLTWH<int> rect3(0, 0, 100, 200);
            mmath::RectLTWH<int> rect4(100, 200, 100, 100);
            mmath::RectLTWH<int> rect5(50, 51, 150, 249);
            ret0 = !rect3.clipTo(rect4).valid();
            //printf("rect3.clipTo(rect4):{%d, %d, %d, %d}\n", rect3.clipTo(rect4)[0], rect3.clipTo(rect4)[1], rect3.clipTo(rect4)[2], rect3.clipTo(rect4)[3]);
            if (!ret0) mprintfE("Failed when check: !rect3.clipTo(rect4).valid()\n");
            ret0 = rect3.clipTo(rect5) == mmath::RectLTWH<int>(50, 51, 50, 149);
            if (!ret0) mprintfE("Failed when check: rect3.clipTo(rect5) == mmath::RectLTWH<int>(50, 51, 50, 149)\n");

            mmath::RectXYWH<int> rect6(50, 50, 100, 100);
            mmath::RectXYWH<int> rect7(149, 149, 100, 100);
            mmath::RectXYWH<int> rect8(50, 51, 150, 249);
            //printf("rect6.clipTo(rect7):{%d, %d, %d, %d}\n", rect6.clipTo(rect7)[0], rect6.clipTo(rect7)[1], rect6.clipTo(rect7)[2], rect6.clipTo(rect7)[3]);
            ret0 = rect6.clipTo(rect7).valid();
            if (!ret0) mprintfE("Failed when check: rect6.clipTo(rect7).valid()\n");
            ret0 = rect6.clipTo(rect7) == mmath::RectXYWH<int>(99, 99, 1, 1);
            if (!ret0) mprintfE("Failed when check: rect6.clipTo(rect7) == mmath::RectXYWH<int>(99, 99, 1, 1)\n");
            ret0 = rect6.clipTo(rect8).valid();
            if (!ret0) mprintfE("Failed when check: rect6.clipTo(rect7).valid()\n");
        }

        inline void check()
        {
            printf("\n--------------------check mmath start--------------------\n");
            RectTest();
            printf("---------------------check mmath end---------------------\n\n");
        }
    }
#endif
}

#endif // !MATH_HPP_MINEUTILS