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

        /*左上角-右下角：l、t、r、b排列的rect坐标，T只支持整型和浮点型，且不能为const、引用和volatile  */
        template<class T>
        class RectLTRB
        {
        public:
            template<class U = T, typename std::enable_if<!std::is_const<U>::value && !std::is_reference<U>::value && !std::is_volatile<U>::value && std::is_same<U, T>::value && (std::is_integral<U>::value || std::is_floating_point<U>::value), int>::type = 0>
            RectLTRB() {};

            template<class U = T, typename std::enable_if<!std::is_const<U>::value && !std::is_reference<U>::value && !std::is_volatile<U>::value && std::is_same<U, T>::value && (std::is_integral<U>::value || std::is_floating_point<U>::value), int>::type = 0>
            RectLTRB(T left_x, T top_y, T right_x, T bottom_y);

            template<class U = T, typename std::enable_if<!std::is_const<U>::value && !std::is_reference<U>::value && !std::is_volatile<U>::value && std::is_same<U, T>::value && (std::is_integral<U>::value || std::is_floating_point<U>::value), int>::type = 0>
            RectLTRB(const RectLTRB<T>& rect);

            RectLTRB<T>& operator=(const RectLTRB<T>& rect);

            bool operator==(const RectLTRB<T>& rect) const;

            T& operator[](int idx);
            const T& operator[](int idx) const;

            //将坐标转化为像素坐标，向下取整
            template<class PixT = int, typename std::enable_if<!std::is_const<PixT>::value && !std::is_reference<PixT>::value && !std::is_volatile<PixT>::value && std::is_integral<PixT>::value, int>::type = 0>
            RectLTRB<PixT> toPixel() const;
            //若自身数据为整数类型，则按像素运算，否则按连续数值运算
            RectXYWH<T> toXYWH() const;
            //若自身数据为整数类型，则按像素运算，否则按连续数值运算
            RectLTWH<T> toLTWH() const;

        private:
            std::array<T, 4> data_;
        };

        /*左上角-宽高：l、t、data_[2]、data_[3]排列的rect坐标，T只支持整型和浮点型，且不能为const、引用和volatile  */
        template<class T>
        class RectLTWH
        {
        public:
            template<class U = T, typename std::enable_if<!std::is_const<U>::value && !std::is_reference<U>::value && !std::is_volatile<U>::value && std::is_same<U, T>::value && (std::is_integral<U>::value || std::is_floating_point<U>::value), int>::type = 0>
            RectLTWH() {};

            template<class U = T, typename std::enable_if<!std::is_const<U>::value && !std::is_reference<U>::value && !std::is_volatile<U>::value && std::is_same<U, T>::value && (std::is_integral<U>::value || std::is_floating_point<U>::value), int>::type = 0>
            RectLTWH(T left_x, T top_y, T width, T height);

            template<class U = T, typename std::enable_if<!std::is_const<U>::value && !std::is_reference<U>::value && !std::is_volatile<U>::value && std::is_same<U, T>::value && (std::is_integral<U>::value || std::is_floating_point<U>::value), int>::type = 0>
            RectLTWH(const RectLTWH<T>& rect);

            RectLTWH<T>& operator=(const RectLTWH<T>& rect);

            bool operator==(const RectLTWH<T>& rect) const;

            T& operator[](int idx);
            const T& operator[](int idx) const;

            //将坐标转化为像素坐标，向下取整
            template<class PixT = int, typename std::enable_if<!std::is_const<PixT>::value && !std::is_reference<PixT>::value && !std::is_volatile<PixT>::value && std::is_integral<PixT>::value, int>::type = 0>
            RectLTWH<PixT> toPixel() const;
            //若自身数据为整数类型，则按像素运算，否则按连续数值运算
            RectLTRB<T> toLTRB() const;
            //若自身数据为整数类型，则按像素运算，否则按连续数值运算
            RectXYWH<T> toXYWH() const;

        private:
            std::array<T, 4> data_;
        };

        /*中心-宽高：data_[0]、data_[1]、data_[2]、data_[3]排列的rect坐标，T只支持整型和浮点型，且不能为const、引用和volatile  */
        template<class T>
        class RectXYWH
        {
        public:
            template<class U = T, typename std::enable_if<!std::is_const<U>::value && !std::is_reference<U>::value && !std::is_volatile<U>::value && std::is_same<U, T>::value && (std::is_integral<U>::value || std::is_floating_point<U>::value), int>::type = 0>
            RectXYWH() {};

            template<class U = T, typename std::enable_if<!std::is_const<U>::value && !std::is_reference<U>::value && !std::is_volatile<U>::value && std::is_same<U, T>::value && (std::is_integral<U>::value || std::is_floating_point<U>::value), int>::type = 0>
            RectXYWH(T center_x, T center_y, T width, T height);

            template<class U = T, typename std::enable_if<!std::is_const<U>::value && !std::is_reference<U>::value && !std::is_volatile<U>::value && std::is_same<U, T>::value && (std::is_integral<U>::value || std::is_floating_point<U>::value), int>::type = 0>
            RectXYWH(const RectXYWH<T>& rect);

            RectXYWH<T>& operator=(const RectXYWH<T>& rect);

            bool operator==(const RectXYWH<T>& rect) const;

            T& operator[](int idx);
            const T& operator[](int idx) const;

            //将坐标转化为像素坐标，向下取整
            template<class PixT = int, typename std::enable_if<!std::is_const<PixT>::value && !std::is_reference<PixT>::value && !std::is_volatile<PixT>::value && std::is_integral<PixT>::value, int>::type = 0>
            RectXYWH<PixT> toPixel() const;
            //若自身数据为整数类型，则按像素运算，否则按连续数值运算
            RectLTRB<T> toLTRB() const;
            //若自身数据为整数类型，则按像素运算，否则按连续数值运算
            RectLTWH<T> toLTWH() const;

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
        template<class U, typename std::enable_if<!std::is_const<U>::value && !std::is_reference<U>::value && !std::is_volatile<U>::value && std::is_same<U, T>::value && (std::is_integral<U>::value || std::is_floating_point<U>::value), int>::type>
        inline RectLTRB<T>::RectLTRB(T left_x, T top_y, T right_x, T bottom_y)
        {
            this->data_[0] = left_x;
            this->data_[1] = top_y;
            this->data_[2] = right_x;
            this->data_[3] = bottom_y;
        }

        template<class T>
        template<class U, typename std::enable_if<!std::is_const<U>::value && !std::is_reference<U>::value && !std::is_volatile<U>::value && std::is_same<U, T>::value && (std::is_integral<U>::value || std::is_floating_point<U>::value), int>::type>
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

        /*------------------------------LTWH-----------------------------*/

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
        template<class U, typename std::enable_if<!std::is_const<U>::value && !std::is_reference<U>::value && !std::is_volatile<U>::value && std::is_same<U, T>::value && (std::is_integral<U>::value || std::is_floating_point<U>::value), int>::type>
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

        /*------------------------------XYWH-----------------------------*/

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
        template<class U, typename std::enable_if<!std::is_const<U>::value && !std::is_reference<U>::value && !std::is_volatile<U>::value && std::is_same<U, T>::value && (std::is_integral<U>::value || std::is_floating_point<U>::value), int>::type>
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
            mmath::RectLTRB<int> rect(0, 0, 100, 200);
            bool ret1;
            ret1 = (mmath::RectLTRB<int>(0, 0, 100, 200).toLTWH() == mmath::RectLTWH<int>(0, 0, 101, 201));
            if (!ret1) mprintfE("Failed when check: mmath::RectLTRB<int>(0, 0, 100, 200).toLTWH() == mmath::RectLTWH<int>(0, 0, 101, 201)\n");

            ret1 = (mmath::RectLTWH<int>(0, 0, 100, 200).toXYWH() == mmath::RectXYWH<int>(50, 100, 100, 200));
            if (!ret1) mprintfE("Failed when check: mmath::RectLTWH<int>(0, 0, 100, 200).toXYWH() == mmath::RectXYWH<int>(50, 100, 100, 200)\n");

            ret1 = (mmath::RectLTWH<double>(0.1, 0.9, 100.5, 200.3).toPixel<int>() == mmath::RectLTWH<int>(0, 0, 100, 200));
            if (!ret1) mprintfE("Failed when check: mmath::RectLTWH<double>(0.1, 0.9, 100.5, 200.3).toPixel<int>() == mmath::RectLTWH<int>(0, 0, 100, 200)\n");


            printf("\n");
        }

        inline void check()
        {
            printf("\n--------------------check mmath start--------------------\n\n");
            RectTest();
            printf("---------------------check mmath end---------------------\n\n");
        }
    }
#endif
}

#endif // !MATH_HPP_MINEUTILS