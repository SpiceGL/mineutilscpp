﻿//mineutils库的数学相关功能
#pragma once
#ifndef MATH_HPP_MINEUTILS
#define MATH_HPP_MINEUTILS

#include<array>
#include<cmath>
#include<iostream>
#include<string>
#include<string.h>

#include"base.hpp"
#include"str.hpp"
#include"log.hpp"


namespace mineutils
{
    /*--------------------------------------------用户接口--------------------------------------------*/
    namespace mmath
    {
        /*  将输入的Python风格索引值(负数索引)转换为非负数索引，如果出错则返回0
            @param idx：Python风格idx(包含负数索引)
            @param len：合法range长度，必须为正数
            @return 非负数索引值  */
        int normIdx(int idx, int len);


        /*  将Python风格的range(负数以及超出实际范围的range)转换为实际range(左闭右开)，如果出错则返回{0, 0}
            @param range：Python风格range
            @param len：合法range长度，必须为正数
            @return 实际range   */
        std::pair<int, int> normRange(std::pair<int, int> range, int len);

        /*  将Python风格的索引值(负数以及超出实际范围的idx转换为对应的range(左闭右开)，如果出错则返回{0, 0}
            @param idx：Python风格idx
            @param len：合法range长度，必须为正数
            @return 实际range   */
        std::pair<int, int> normRange(int idx, int len);

        //将x按a值向上对齐，注意a必须为正数才能获得正确结果
        int align(int x, int a);

        template<class T>
        class BaseBox
        {
        public:
            template<class U = T, typename std::enable_if<std::is_same<U, T>::value && (std::is_integral<U>::value || std::is_floating_point<U>::value), int>::type = 0>
            BaseBox();
            template<class U = T, typename std::enable_if<std::is_same<U, T>::value && (std::is_integral<U>::value || std::is_floating_point<U>::value), int>::type = 0>
            BaseBox(T v0, T v1, T v2, T v3);
            template<class U = T, typename std::enable_if<std::is_same<U, T>::value && (std::is_integral<U>::value || std::is_floating_point<U>::value), int>::type = 0>
            BaseBox(const BaseBox<T>& box);
            virtual ~BaseBox() {}
            T& operator[](int idx);
            const T& operator[](int idx) const;

            //已废弃
            mdeprecated(R"(Deprecated. Please replace with "std::is_integral<T>::value")") static bool belongToIntSeries();
        protected:
            T data_[4];

            template<class U>
            friend std::ostream& operator<<(std::ostream& _cout, const BaseBox<U>& bbox);
        };

        template<class T>
        struct LTRBBox;
        template<class T>
        struct LTWHBox;
        template<class T>
        struct XYWHBox;

        /*左上角-右下角：l、t、r、b排列的bbox坐标，只支持整型和浮点型  */
        template<class T>
        struct LTRBBox :public BaseBox<T>
        {
        public:
            T& left = this->data_[0];
            T& top = this->data_[1];
            T& right = this->data_[2];
            T& bottom = this->data_[3];

            T& l = left;
            T& t = top;
            T& r = right;
            T& b = bottom;

            template<class U = T, typename std::enable_if<std::is_same<U, T>::value && (std::is_integral<U>::value || std::is_floating_point<U>::value), int>::type = 0>
            LTRBBox() :BaseBox<T>() {}
            template<class U = T, typename std::enable_if<std::is_same<U, T>::value && (std::is_integral<U>::value || std::is_floating_point<U>::value), int>::type = 0>
            LTRBBox(T v0, T v1, T v2, T v3) :BaseBox<T>(v0, v1, v2, v3) {}
            template<class U = T, typename std::enable_if<std::is_same<U, T>::value && (std::is_integral<U>::value || std::is_floating_point<U>::value), int>::type = 0>
            LTRBBox(const LTRBBox<T>& ltrb) :BaseBox<T>(ltrb) {}
            LTRBBox<T>& operator=(const LTRBBox<T>& ltrb);

            //将坐标转化为像素坐标
            template<class PixT = int, typename std::enable_if<std::is_integral<PixT>::value, int>::type = 0>
            LTRBBox<PixT> toPixel() const;
            //若自身数据为整数类型，则按像素运算，否则按连续数值运算
            XYWHBox<T> toXYWH() const;
            //若自身数据为整数类型，则按像素运算，否则按连续数值运算
            LTWHBox<T> toLTWH() const;
        };

        /*左上角-宽高：l、t、w、h排列的bbox坐标，只支持整型和浮点型  */
        template<class T>
        struct LTWHBox :public BaseBox<T>
        {
        public:
            T& left = this->data_[0];
            T& top = this->data_[1];
            T& width = this->data_[2];
            T& height = this->data_[3];

            T& l = this->left;
            T& t = this->top;
            T& w = this->width;
            T& h = this->height;

            template<class U = T, typename std::enable_if<std::is_same<U, T>::value && (std::is_integral<U>::value || std::is_floating_point<U>::value), int>::type = 0>
            LTWHBox() :BaseBox<T>() {}
            template<class U = T, typename std::enable_if<std::is_same<U, T>::value && (std::is_integral<U>::value || std::is_floating_point<U>::value), int>::type = 0>
            LTWHBox(T v0, T v1, T v2, T v3) : BaseBox<T>(v0, v1, v2, v3) {}
            template<class U = T, typename std::enable_if<std::is_same<U, T>::value && (std::is_integral<U>::value || std::is_floating_point<U>::value), int>::type = 0>
            LTWHBox(const LTWHBox<T>& ltwh) : BaseBox<T>(ltwh) {}
            LTWHBox<T>& operator=(const LTWHBox<T>& ltwh);

            /*将坐标转化为像素坐标，向下取整*/
            template<class PixT = int, typename std::enable_if<std::is_integral<PixT>::value, int>::type = 0>
            LTWHBox<PixT> toPixel() const;
            //若自身数据为整数类型，则按像素运算，否则按连续数值运算
            LTRBBox<T> toLTRB() const;
            //若自身数据为整数类型，则按像素运算，否则按连续数值运算
            XYWHBox<T> toXYWH() const;
        };

        /*中心-宽高：x、y、w、h排列的bbox坐标，只支持整型和浮点型  */
        template<class T>
        struct XYWHBox :public BaseBox<T>
        {
        public:
            T& x = this->data_[0];
            T& y = this->data_[1];
            T& w = this->data_[2];
            T& h = this->data_[3];

            T& width = this->w;
            T& height = this->h;

            template<class U = T, typename std::enable_if<std::is_same<U, T>::value && (std::is_integral<U>::value || std::is_floating_point<U>::value), int>::type = 0>
            XYWHBox() :BaseBox<T>() {}
            template<class U = T, typename std::enable_if<std::is_same<U, T>::value && (std::is_integral<U>::value || std::is_floating_point<U>::value), int>::type = 0>
            XYWHBox(T v0, T v1, T v2, T v3) :BaseBox<T>(v0, v1, v2, v3) {}
            template<class U = T, typename std::enable_if<std::is_same<U, T>::value && (std::is_integral<U>::value || std::is_floating_point<U>::value), int>::type = 0>
            XYWHBox(const XYWHBox<T>& xywh) :BaseBox<T>(xywh) {}
            XYWHBox<T>& operator=(const XYWHBox<T>& xywh);

            //将坐标转化为像素坐标
            template<class PixT = int, typename std::enable_if<std::is_integral<PixT>::value, int>::type = 0>
            XYWHBox<PixT> toPixel() const;
            //若自身数据为整数类型，则按像素运算，否则按连续数值运算
            LTRBBox<T> toLTRB() const;
            //若自身数据为整数类型，则按像素运算，否则按连续数值运算
            LTWHBox<T> toLTWH() const;
        };

        //为std::cout添加对BaseBox及其子类的支持
        template<class T>
        std::ostream& operator<<(std::ostream& _cout, const BaseBox<T>& bbox);

        using LTRB = LTRBBox<int>;
        using LTRBf = LTRBBox<float>;
        using LTWH = LTWHBox<int>;
        using LTWHf = LTWHBox<float>;
        using XYWH = XYWHBox<int>;
        using XYWHf = XYWHBox<float>;
    }





    /*--------------------------------------------内部实现--------------------------------------------*/

    namespace mmath
    {
        inline int normIdx(int idx, int len)
        {
            if (len <= 0)
            {
                mprintfE("Wrong value of len:%d!\n", len);
                return 0;
            }
            int normal_idx;
            if (idx >= 0 && idx < len)
                normal_idx = idx;
            else if (idx < 0 && idx >= -len)
                normal_idx = idx + len;
            else
            {
                mprintfW("idx:%d out of index range:%d!\n", idx, len);
                return 0;
            }
            return normal_idx;
        }

        inline std::pair<int, int> normRange(std::pair<int, int> range, int len)
        {
            if (len <= 0)
            {
                mprintfE("Wrong value of len:%d!\n", len);
                return { 0, 0 };
            }
            int dst_start, dst_end;
            int x1 = range.first, x2 = range.second;

            if (x1 >= 0)
                x1 < len ? dst_start = x1 : dst_start = len;
            else x1 >= -len ? dst_start = x1 + len : dst_start = 0;

            if (x2 >= 0)
                x2 < len ? dst_end = x2 : dst_end = len;
            else x2 >= -len ? dst_end = x2 + len : dst_end = 0;
            return { dst_start, dst_end };
        }

        inline std::pair<int, int> normRange(int idx, int len)
        {
            if (len <= 0)
            {
                mprintfE("Wrong value of len:%d!\n", len);
                return { 0, 0 };
            }
            int dst_start, dst_end;
            if (idx >= 0 and idx < len)
                dst_start = idx, dst_end = idx + 1;
            else if (idx < 0 and idx >= -len)
                dst_start = idx + len, dst_end = idx + len + 1;
            else
            {
                mprintfW("idx:%d out of index range:%d!\n", idx, len);
                return { 0, 0 };
            }
            return { dst_start, dst_end };
        }

        inline int align(int x, int a)
        {
            if (a <= 0)
                return x;
            int rem = x % a;
            if (rem > 0)
                return a - rem + x;
            else if (rem < 0)
                return x - rem;
            else return x;
        }


        template<class T>
        template<class U, typename std::enable_if<std::is_same<U, T>::value && (std::is_integral<U>::value || std::is_floating_point<U>::value), int>::type>
        inline BaseBox<T>::BaseBox()
        {
            memset(this->data_, 0, sizeof(T) * 4);
        }

        template<class T>
        template<class U, typename std::enable_if<std::is_same<U, T>::value && (std::is_integral<U>::value || std::is_floating_point<U>::value), int>::type>
        inline BaseBox<T>::BaseBox(T v0, T v1, T v2, T v3)
        {
            this->data_[0] = v0;
            this->data_[1] = v1;
            this->data_[2] = v2;
            this->data_[3] = v3;
        }
        template<class T>
        template<class U, typename std::enable_if<std::is_same<U, T>::value && (std::is_integral<U>::value || std::is_floating_point<U>::value), int>::type>
        inline BaseBox<T>::BaseBox(const BaseBox<T>& box)
        {
            this->data_[0] = box.data_[0];
            this->data_[1] = box.data_[1];
            this->data_[2] = box.data_[2];
            this->data_[3] = box.data_[3];
        }

        template<class T>
        inline T& BaseBox<T>::operator[](int idx)
        {
            idx = mmath::normIdx(idx, 4);
            return this->data_[idx];
        }
        template<class T>
        inline const T& BaseBox<T>::operator[](int idx) const
        {
            idx = mmath::normIdx(idx, 4);
            return this->data_[idx];
        }
        template<class T>
        bool BaseBox<T>::belongToIntSeries()
        {
            return std::is_integral<T>::value;
        }


        template<class T>
        inline LTRBBox<T>& LTRBBox<T>::operator=(const LTRBBox<T>& ltrb)
        {
            this->left = ltrb.left;
            this->top = ltrb.top;
            this->right = ltrb.right;
            this->bottom = ltrb.bottom;
            return *this;
        }

        //将坐标转化为像素坐标
        template<class T> template<class PixT, typename std::enable_if<std::is_integral<PixT>::value, int>::type>
        inline LTRBBox<PixT> LTRBBox<T>::toPixel() const
        {
            return { PixT(floor(left)), PixT(floor(top)), PixT(floor(right)), PixT(floor(bottom)) };
        }

        template<class T>
        inline XYWHBox<T> LTRBBox<T>::toXYWH() const
        {
            if (std::is_integral<T>::value)
            {

                T _w = this->right - this->left + 1;
                T _h = this->bottom - this->top + 1;
                T _x = this->left + std::ceil(double(_w - 1) / 2);
                T _y = this->top + std::ceil(double(_h - 1) / 2);
                return { _x,_y,_w,_h };
            }
            else
            {
                T _w = this->right - this->left;
                T _h = this->bottom - this->top;
                T _x = this->left + _w / 2;
                T _y = this->top + _h / 2;
                return { _x,_y,_w,_h };
            }
        }


        template<class T>
        inline LTWHBox<T> LTRBBox<T>::toLTWH() const
        {
            T _w = this->right - this->left;
            T _h = this->bottom - this->top;
            if (std::is_integral<T>::value)
            {
                _w += 1;
                _h += 1;
            }
            return { this->l, this->t, _w, _h };
        }


        template<class T>
        inline XYWHBox<T>& XYWHBox<T>::operator=(const XYWHBox<T>& xywh)
        {
            this->x = xywh.x;
            this->y = xywh.y;
            this->w = xywh.w;
            this->h = xywh.h;
            return *this;
        }

        //将坐标转化为像素坐标
        template<class T> template<class PixT, typename std::enable_if<std::is_integral<PixT>::value, int>::type>
        inline XYWHBox<PixT> XYWHBox<T>::toPixel() const
        {
            return { PixT(floor(this->x)), PixT(floor(this->y)), PixT(floor(this->w)), PixT(floor(this->h)) };
        }

        template<class T>
        inline LTRBBox<T> XYWHBox<T>::toLTRB() const
        {
            if (std::is_integral<T>::value)
            {
                T _l = this->x - std::ceil(double(this->w - 1) / 2);
                T _t = this->y - std::ceil(double(this->h - 1) / 2);
                T _r = _l + this->w - 1;
                T _b = _t + this->h - 1;
                return { _l,_t,_r,_b };
            }
            else
            {
                T _l = this->x - this->w / 2;
                T _t = this->y - this->h / 2;
                T _r = _l + this->w;
                T _b = _t + this->h;
                return { _l,_t,_r,_b };
            }
        }

        template<class T>
        inline LTWHBox<T> XYWHBox<T>::toLTWH() const
        {
            if (std::is_integral<T>::value)
            {
                T _l = this->x - std::ceil(double(this->w - 1) / 2);
                T _t = this->y - std::ceil(double(this->h - 1) / 2);
                return { _l, _t, this->w, this->h };
            }
            else
            {
                T _l = this->x - this->w / 2;
                T _t = this->y - this->h / 2;
                return { _l, _t, this->w, this->h };
            }
        }

        template<class T>
        LTWHBox<T>& LTWHBox<T>::operator=(const LTWHBox<T>& ltwh)
        {
            this->l = ltwh.l;
            this->t = ltwh.t;
            this->w = ltwh.w;
            this->h = ltwh.h;
            return *this;
        }

        /*将坐标转化为像素坐标，向下取整*/
        template<class T> template<class PixT, typename std::enable_if<std::is_integral<PixT>::value, int>::type>
        LTWHBox<PixT>  LTWHBox<T>::toPixel() const
        {
            return { PixT(floor(this->l)), PixT(floor(this->t)), PixT(floor(this->w)), PixT(floor(this->h)) };
        }

        template<class T>
        LTRBBox<T> LTWHBox<T>::toLTRB() const
        {
            T r = this->left + this->w;
            T b = this->top + this->h;
            if (std::is_integral<T>::value)
            {
                r -= 1;
                b -= 1;
            }
            return { this->l, this->t, r, b };
        }

        template<class T>
        XYWHBox<T> LTWHBox<T>::toXYWH() const
        {
            if (std::is_integral<T>::value)
            {
                T _x = this->left + std::ceil(double(this->w - 1) / 2);
                T _y = this->top + std::ceil(double(this->h - 1) / 2);
                return { _x, _y, this->w, this->h };
            }
            else
            {
                T _x = this->left + this->w / 2;
                T _y = this->top + this->h / 2;
                return { _x, _y, this->w, this->h };
            }

        }

        /*------------------------------------------------------------------------------*/
        //为std::cout添加对BaseBox及其子类的支持
        template<class T>
        inline std::ostream& operator<<(std::ostream& _cout, const BaseBox<T>& bbox)
        {
            _cout << "[" << bbox[0] << " " << bbox[1] << " "
                << bbox[2] << " " << bbox[3] << "]";
            return _cout;
        }
    }
}

#endif // !MATH_HPP_MINEUTILS