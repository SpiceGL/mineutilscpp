//mineutils库的数学相关功能
#pragma once
#ifndef MATH_HPP_MINEUTILS
#define MATH_HPP_MINEUTILS

#include<array>
#include<cmath>
#include<iostream>
#include<string>

#include"str.hpp"
#include"log.hpp"
#include"type.hpp"


namespace mineutils
{
    namespace mmath
    {
        /*--------------------------------------------用户接口--------------------------------------------*/

        /*  将输入的Python风格索引值(负数索引)转换为非负数索引，如果超出索引范围则返回-1
            @param idx：Python风格idx(包含负数索引)
            @param len：合法range长度
            @return 非负数索引值  */
        int normIdx(int idx, int len);


        /*  将Python风格的range(负数以及超出实际范围的range)转换为实际range(左闭右开)
            @param range：Python风格range
            @param len：合法range长度
            @return 实际range   */
        std::pair<int, int> normRange(std::pair<int, int> range, int len);

        /*  将Python风格的索引值(负数以及超出实际范围的idx转换为对应的range(左闭右开)，如果idx超出索引范围则返回{-1， -1}
            @param idx：Python风格idx
            @param len：合法range长度
            @return 实际range   */
        std::pair<int, int> normRange(int idx, int len);


        template<class T>
        class BaseBox
        {
        public:
            BaseBox() {}

            BaseBox(const T& v0, const T& v1, const T& v2, const T& v3);
            BaseBox(const BaseBox<T>& box);
            virtual ~BaseBox() {}
            T& operator[](int idx);
            T operator[](int idx) const;

            static bool belongToIntSeries();
        protected:
            T data_[4];
        };

        template<class T>
        class XYWHBox;
        template<class T>
        class LTRBBox;
        template<class T>
        class LTWHBox;

        /*左上角-右下角：l、t、r、b排列的bbox坐标*/
        template<class T>
        class LTRBBox :public BaseBox<T>
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

            LTRBBox() :BaseBox<T>() {}
            LTRBBox(const T& v0, const T& v1, const T& v2, const T& v3) :BaseBox<T>(v0, v1, v2, v3) {}
            LTRBBox(const LTRBBox<T>& ltrb) :BaseBox<T>(ltrb) {}
            LTRBBox<T>& operator=(const LTRBBox<T>& ltrb);

            //将坐标转化为像素坐标
            template<class PT = int>
            LTRBBox<PT> toPixel() const;
            //若自身数据为整数类型，则按像素运算，否则按连续数值运算
            XYWHBox<T> toXYWH() const;
            //若自身数据为整数类型，则按像素运算，否则按连续数值运算
            LTWHBox<T> toLTWH() const;
        };

        /*中心-宽高：x、y、w、h排列的bbox坐标*/
        template<class T>
        class XYWHBox :public BaseBox<T>
        {
        public:
            T& x = this->data_[0];
            T& y = this->data_[1];
            T& w = this->data_[2];
            T& h = this->data_[3];

            T& width = this->w;
            T& height = this->h;

            XYWHBox() :BaseBox<T>() {}
            XYWHBox(const T& v0, const T& v1, const T& v2, const T& v3) :BaseBox<T>(v0, v1, v2, v3) {}
            XYWHBox(const XYWHBox<T>& xywh) :BaseBox<T>(xywh) {}
            XYWHBox<T>& operator=(const XYWHBox<T>& xywh);

            //将坐标转化为像素坐标
            template<class PT = int>
            XYWHBox<PT> toPixel() const;
            //若自身数据为整数类型，则按像素运算，否则按连续数值运算
            LTRBBox<T> toLTRB() const;
            //若自身数据为整数类型，则按像素运算，否则按连续数值运算
            LTWHBox<T> toLTWH() const;
        };

        /*左上角-宽高：l、t、w、h排列的bbox坐标*/
        template<class T>
        class LTWHBox :public BaseBox<T>
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

            LTWHBox() :BaseBox<T>() {}
            LTWHBox(const T& v0, const T& v1, const T& v2, const T& v3) :BaseBox<T>(v0, v1, v2, v3) {}
            LTWHBox(const LTWHBox<T>& ltwh) :BaseBox<T>(ltwh) {}
            LTWHBox<T>& operator=(const LTWHBox<T>& ltwh);

            /*将坐标转化为像素坐标，向下取整*/
            template<class PT = int>
            LTWHBox<PT> toPixel() const;
            //若自身数据为整数类型，则按像素运算，否则按连续数值运算
            LTRBBox<T> toLTRB() const;
            //若自身数据为整数类型，则按像素运算，否则按连续数值运算
            XYWHBox<T> toXYWH() const;
        };

        //为std::cout添加对BaseBox及其子类的支持
        template<class T>
        std::ostream& operator<<(std::ostream& _cout, const BaseBox<T>& bbox);

        using LTRB = LTRBBox<int>;
        using LTRBf = LTRBBox<float>;
        using XYWH = XYWHBox<int>;
        using XYWHf = XYWHBox<float>;
        using LTWH = LTWHBox<int>;
        using LTWHf = LTWHBox<float>;






        /*--------------------------------------------内部实现--------------------------------------------*/

        /*  将输入的Python风格索引值(负数索引)转换为非负数索引，如果超出索引范围则返回-1
            @param idx：Python风格idx(包含负数索引)
            @param len：合法range长度
            @return 非负数索引值  */
        inline int normIdx(int idx, int len)
        {
            int normal_idx;
            if (idx >= 0 and idx < len)
                normal_idx = idx;
            else if (idx < 0 and idx >= -len)
                normal_idx = idx + len;
            else
            {
                std::cout << msgW("idx={} out of index range!\n", idx);
                return -1;
            }
            return normal_idx;
        }


        /*  将Python风格的range(负数以及超出实际范围的range)转换为实际range(左闭右开)
            @param range：Python风格range
            @param len：合法range长度
            @return 实际range   */
        inline std::pair<int, int> normRange(std::pair<int, int> range, int len)
        {
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

        /*  将Python风格的索引值(负数以及超出实际范围的idx转换为对应的range(左闭右开)，如果idx超出索引范围则返回{-1， -1}
            @param idx：Python风格idx
            @param len：合法range长度
            @return 实际range   */
        inline std::pair<int, int> normRange(int idx, int len)
        {
            int dst_start, dst_end;
            if (idx >= 0 and idx < len)
                dst_start = idx, dst_end = idx + 1;
            else if (idx < 0 and idx >= -len)
                dst_start = idx + len, dst_end = idx + len + 1;
            else
            {
                std::cout << msgW("idx={} out of index range!\n", idx);
                return { -1, -1 };
            }
            return { dst_start, dst_end };
        }

        template<class T>
        inline BaseBox<T>::BaseBox(const T& v0, const T& v1, const T& v2, const T& v3)
        {
            //static_assert(std::is_integral<T>::value || std::is_floating_point<T>::value, "Class T is not integral or floating_point!");
            data_[0] = v0;
            data_[1] = v1;
            data_[2] = v2;
            data_[3] = v3;
        }
        template<class T>
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
            idx = normIdx(idx, 4);
            return this->data_[idx];
        }
        template<class T>
        inline T BaseBox<T>::operator[](int idx) const
        {
            idx = normIdx(idx, 4);
            return this->data_[idx];
        }
        template<class T>
        bool BaseBox<T>::belongToIntSeries()
        {
            /*if (isInTypes<T, char, unsigned char, short, unsigned short,
                int, unsigned int, long, unsigned long, long long, unsigned long long>())*/
            if (std::is_integral<T>::value)
                return true;
            else return false;
        }


        template<class T>
        inline LTRBBox<T>& LTRBBox<T>::operator=(const LTRBBox<T>& ltrb)
        {
            left = ltrb.left;
            top = ltrb.top;
            right = ltrb.right;
            bottom = ltrb.bottom;
            return *this;
        }

        //将坐标转化为像素坐标
        template<class T> template<class PT>
        inline LTRBBox<PT> LTRBBox<T>::toPixel() const
        {
            return { PT(floor(left)), PT(floor(top)), PT(floor(right)), PT(floor(bottom)) };
        }

        template<class T>
        inline XYWHBox<T> LTRBBox<T>::toXYWH() const
        {
            if (BaseBox<T>::belongToIntSeries())
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
            if (BaseBox<T>::belongToIntSeries())
            {
                _w += 1;
                _h += 1;
            }
            return { this->l, this->t, _w, _h };
        }


        template<class T>
        inline XYWHBox<T>& XYWHBox<T>::operator=(const XYWHBox<T>& xywh)
        {
            x = xywh.x;
            y = xywh.y;
            w = xywh.w;
            h = xywh.h;
            return *this;
        }

        //将坐标转化为像素坐标
        template<class T> template<class PT>
        inline XYWHBox<PT> XYWHBox<T>::toPixel() const
        {
            return { PT(floor(x)), PT(floor(y)), PT(floor(w)), PT(floor(h)) };
        }

        template<class T>
        inline LTRBBox<T> XYWHBox<T>::toLTRB() const
        {
            if (BaseBox<T>::belongToIntSeries())
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
            if (BaseBox<T>::belongToIntSeries())
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
            l = ltwh.l;
            t = ltwh.t;
            w = ltwh.w;
            h = ltwh.h;
            return *this;
        }

            /*将坐标转化为像素坐标，向下取整*/
        template<class T> template<class PT>
        LTWHBox<PT>  LTWHBox<T>::toPixel() const
        {
            return { PT(floor(l)), PT(floor(t)), PT(floor(w)), PT(floor(h)) };
        }

        template<class T>
        LTRBBox<T> LTWHBox<T>::toLTRB() const
        {
            T r = this->left + this->w;
            T b = this->top + this->h;
            if (BaseBox<T>::belongToIntSeries())
            {
                r -= 1;
                b -= 1;
            }
            return { this->l, this->t, r, b };
        }

        template<class T>
        XYWHBox<T> LTWHBox<T>::toXYWH() const
        {
            if (BaseBox<T>::belongToIntSeries())
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
            std::cout << "[" << bbox[0] << " " << bbox[1] << " "
                << bbox[2] << " " << bbox[3] << "]";
            return std::cout;
        }
    }
}

#endif // !MATH_HPP_MINEUTILS