// mineutils库的数学相关功能
#pragma once
#ifndef MATH_HPP_MINEUTILS
#define MATH_HPP_MINEUTILS

#include <array>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <set>
#include <string>
#include <string.h>
#include <vector>

#include "base.hpp"


namespace mineutils
{
/*--------------------------------------------用户接口--------------------------------------------*/
namespace mmath
{
    // 矩形类
    template <class T>
    class Rect final
    {
    public:
        // 构造一个空的矩形
        template <class U = T, typename std::enable_if<std::is_same<U, T>::value && mtype::_mpriv::RectElemChecker<U>::value, int>::type = 0>
        Rect();

        // 通过两个点构造矩形，内部会自动调整坐标顺序
        template <class U = T, typename std::enable_if<std::is_same<U, T>::value && mtype::_mpriv::RectElemChecker<U>::value, int>::type = 0>
        Rect(T x1, T x2, T y1, T y2);

        Rect(const Rect<T>& rect);
        Rect<T>& operator=(const Rect<T>& rect);

        bool operator==(const Rect<T>& rect) const;
        bool operator!=(const Rect<T>& rect) const;

        // 判断矩形是否有效，起点与终点坐标相同时也算有效
        bool valid() const;

        // 获取坐标，对于无效矩形，返回的值无意义
        T xMin() const;
        T yMin() const;
        T xMax() const;
        T yMax() const;

        // 导出array形式的rect
        std::array<T, 4> toArray() const;
        // 求交集（交集只有一条线和点也算有效），无任何重叠点时返回无效矩形
        Rect<T> clipTo(const Rect<T>& other) const;

    private:
        std::array<T, 4> data_;
    };


    // 为operator<<添加对Rect的支持
    template <class T>
    std::ostream& operator<<(std::ostream& cout_obj, const Rect<T>& rect);


    
    /*  多项式拟合平面函数
        - 基于最小二乘法实现
        - 仅支持浮点数类型的模板参数FloatT
        - 模板参数不可具有const、volatile或引用修饰符  */
    template <class FloatT>
    class PolynomialFitter2D final
    {
    public:
        template <class T = FloatT, typename std::enable_if<std::is_same<T, FloatT>::value && !mtype::_mpriv::AnyCVRefChecker<T>::value && std::is_floating_point<T>::value, int>::type = 0>
        PolynomialFitter2D();

        /*  拟合操作，目前采用最小二乘法
            @param x_list: 自变量集合，不可重复，元素数量必须与y_list相等
            @param y_list: 因变量集合，可重复，元素数量必须与x_list相等
            @param degree: 多项式最高阶数
            @return 多项式系数列表，vector[i]对应第i阶的系数，失败则返回空vector  */
        const std::vector<FloatT>& fit(const std::set<FloatT>& x_list, const std::vector<FloatT>& y_list, uint8_t degree);

        // 预估自变量对应的因变量值
        FloatT evaluate(FloatT x);

    private:
        FloatT pow(FloatT x, uint8_t degree);

        std::vector<FloatT> coeffs_;
    };

}  // namespace mmath





/*--------------------------------------------内部实现--------------------------------------------*/

namespace mmath
{
    template <class T>
    template <class U, typename std::enable_if<std::is_same<U, T>::value && mtype::_mpriv::RectElemChecker<U>::value, int>::type>
    inline Rect<T>::Rect()
    {
        data_[0] = std::numeric_limits<T>::max();
        data_[1] = std::numeric_limits<T>::max();
        data_[2] = std::numeric_limits<T>::min();
        data_[3] = std::numeric_limits<T>::min();
    }

    template <class T>
    template <class U, typename std::enable_if<std::is_same<U, T>::value && mtype::_mpriv::RectElemChecker<U>::value, int>::type>
    inline Rect<T>::Rect(T x1, T y1, T x2, T y2)
    {
        if (x1 <= x2)
        {
            data_[0] = x1;
            data_[2] = x2;
        }
        else
        {
            data_[0] = x2;
            data_[2] = x1;
        }
        if (y1 <= y2)
        {
            data_[1] = y1;
            data_[3] = y2;
        }
        else
        {
            data_[1] = y2;
            data_[3] = y1;
        }
    }

    template <class T>
    inline Rect<T>::Rect(const Rect<T>& rect)
    {
        *this = rect;
    }

    template <class T>
    inline Rect<T>& Rect<T>::operator=(const Rect<T>& rect)
    {
        if (this != &rect)
            data_ = rect.data_;
        return *this;
    }

    template <class T>
    inline bool Rect<T>::operator==(const Rect<T>& rect) const
    {
        return data_ == rect.data_;
    }

    template <class T>
    inline bool Rect<T>::operator!=(const Rect<T>& rect) const
    {
        return data_ != rect.data_;
    }

    template <class T>
    inline bool Rect<T>::valid() const
    {
        return data_[0] <= data_[2] && data_[1] <= data_[3];
    }

    template <class T>
    inline T Rect<T>::xMin() const
    {
        return data_[0];
    }

    template <class T>
    inline T Rect<T>::yMin() const
    {
        return data_[1];
    }

    template <class T>
    inline T Rect<T>::xMax() const
    {
        return data_[2];
    }

    template <class T>
    inline T Rect<T>::yMax() const
    {
        return data_[3];
    }

    template <class T>
    inline std::array<T, 4> Rect<T>::toArray() const
    {
        return data_;
    }

    template <class T>
    inline Rect<T> Rect<T>::clipTo(const Rect<T>& other) const
    {
        if (!this->valid() || !other.valid())
            return Rect<T>();

        Rect<T> result;
        const auto& odata = other.data_;
        auto& rdata_ = result.data_;
        // 如果两个矩形没有交集，那么计算出来的坐标必然起点大于终点，会被判断为空
        rdata_[0] = (data_[0] > odata[0]) ? data_[0] : odata[0];
        rdata_[1] = (data_[1] > odata[1]) ? data_[1] : odata[1];
        rdata_[2] = (data_[2] < odata[2]) ? data_[2] : odata[2];
        rdata_[3] = (data_[3] < odata[3]) ? data_[3] : odata[3];
        return result;
    }


    // 为Rect对象添加对operator<< 的支持
    template <class T>
    inline std::ostream& operator<<(std::ostream& cout_obj, const Rect<T>& rect)
    {
        if (rect.empty())
            cout_obj << "[Empty Rect]";
        else
            cout_obj << "[" << rect[0] + 0 << " " << rect[1] + 0 << " "
                     << rect[2] + 0 << " " << rect[3] + 0 << "]";
        return cout_obj;
    }



    template <class FloatT>
    template <class T, typename std::enable_if<std::is_same<T, FloatT>::value && !mtype::_mpriv::AnyCVRefChecker<T>::value && std::is_floating_point<T>::value, int>::type>
    inline PolynomialFitter2D<FloatT>::PolynomialFitter2D()
    {
    }

    template <class FloatT>
    inline const std::vector<FloatT>& PolynomialFitter2D<FloatT>::fit(const std::set<FloatT>& x_list, const std::vector<FloatT>& y_list, uint8_t degree)
    {
        coeffs_.clear();
        if (x_list.size() != y_list.size())
        {
            mprintfE("The sizes of x_list:%d and y_list:%d do not match!\n", x_list.size(), y_list.size());
            return coeffs_;
        }

        std::vector<FloatT> coeffs;
        size_t coeffs_sz = static_cast<size_t>(degree) + 1;
        coeffs.resize(coeffs_sz);
        if (degree == 0)
        {
            coeffs[0] = std::accumulate(y_list.begin(), y_list.end(), 0.0) / y_list.size();
            coeffs_ = std::move(coeffs);
            return coeffs_;
        }

        std::vector<FloatT> x_list2(x_list.begin(), x_list.end());
        int len = x_list.size();
        int i, j, k;
        std::vector<FloatT> b(coeffs_sz, 0);
        std::vector<std::vector<FloatT>> A(coeffs_sz, std::vector<FloatT>(coeffs_sz, 0));

        for (i = 0; i < len; i++)
        {
            for (j = 0; j <= degree; j++)
            {
                b[j] += this->pow(x_list2[i], j) * y_list[i];
                for (k = 0; k <= degree; k++)
                {
                    A[j][k] += this->pow(x_list2[i], j + k);
                }
            }
        }
        // 解线性方程组，得到多项式系数
        for (i = 0; i <= degree; i++)
        {
            for (j = i + 1; j <= degree; j++)
            {
                FloatT ratio = A[j][i] / A[i][i];
                for (k = i; k <= degree; k++)
                {
                    A[j][k] -= ratio * A[i][k];
                }
                b[j] -= ratio * b[i];
            }
        }
        for (i = degree; i >= 0; i--)
        {
            FloatT sum = 0;
            for (j = i + 1; j <= degree; j++)
            {
                sum += coeffs[j] * A[i][j];
            }
            coeffs[i] = (b[i] - sum) / A[i][i];
        }
        coeffs_ = std::move(coeffs);
        return coeffs_;
    }

    template <class FloatT>
    inline FloatT PolynomialFitter2D<FloatT>::evaluate(FloatT x)
    {
        FloatT y = 0;
        if (coeffs_.empty())
        {
            mprintfW("Call fit first!\n");
            return 0.0;
        }
        uint8_t now_degree = static_cast<uint8_t>(coeffs_.size() - 1);
        for (uint8_t i = 0; i < coeffs_.size(); i++)
        {
            y += this->pow(x, i) * coeffs_[i];
        }
        return y;
    }

    template <class FloatT>
    inline FloatT PolynomialFitter2D<FloatT>::pow(FloatT x, uint8_t degree)
    {
        FloatT y = 1;
        while (degree > 0)
        {
            y *= x;
            degree--;
        }
        return y;
    }

}  // namespace mmath





/*--------------------------------------------单元测试--------------------------------------------*/
#ifdef MINEUTILS_TEST_MODULES
namespace _mmathcheck
{
    inline void PolynomialFitter2DTest()
    {
        mmath::PolynomialFitter2D<double> fitter;
        fitter.fit({0.0, 1.0, 2.0}, {0.0, 1.0, 4.0}, 2);
        double result = fitter.evaluate(1.5);
        double expected = 1.5 * 1.5;
        if (!(std::abs(result - expected) < 1e-6))
        {
            mprintfE(R"(Failed when check PolynomialFitter2DTest! result is %lf, expected is %lf)"
                     "\n",
                     result, expected);
        }
    }

    inline void check()
    {
        printf("\n--------------------check mmath start--------------------\n");
        PolynomialFitter2DTest();
        printf("---------------------check mmath end---------------------\n\n");
    }

}  // namespace _mmathcheck
#endif
}  // namespace mineutils

#endif  // !MATH_HPP_MINEUTILS