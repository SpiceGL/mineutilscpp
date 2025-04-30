//mineutils库的ncnn相关功能
#pragma once
#ifndef NCNN_HPP_MINEUTILS
#define NCNN_HPP_MINEUTILS

#include<limits.h>
//基于ncnn2021-2024的版本
#include<iostream>
#include<stdlib.h>
#include<string>
#include<vector>
#include"ncnn/net.h"

#include"../core/base.hpp"
#include"../core/str.hpp"
#include"../core/math.hpp"
#include"../core/io.hpp"


namespace mineutils
{
    /*--------------------------------------------用户接口--------------------------------------------*/

    namespace mext
    {
        //从路径加载ncnn的net，正常返回0，出错返回其他值
        int ncnnLoad(ncnn::Net& out_net, const std::string& param_path, const std::string& bin_path);

        //快速运行一次ncnn的net推理
        std::vector<ncnn::Mat> ncnnQuickRun(ncnn::Net& net_in, ncnn::Mat& input, const std::string& in_name, const std::vector<std::string>& out_names);

        //输入ncnn的net路径，快速运行一次ncnn的net推理
        std::vector<ncnn::Mat> ncnnQuickRun(const std::string& param_path, const std::string& model_path, ncnn::Mat& input, const std::string& in_name, const std::vector<std::string>& out_names);

        /*  打印ncnn::Mat，只支持CHW排列的三维ncnn::Mat
            @param m：要打印的ncnn::Mat
            @param x_range：x坐标值或range，支持Python风格range，类型限制为std::pair<int, int>或int
            @param y_range：y坐标值或range，支持Python风格range，类型限制为std::pair<int, int>或int
            @param c_range：channel值或range，支持Python风格range，类型限制为std::pair<int, int>或int   */
        void ncnnPrintMat(const ncnn::Mat& m, std::pair<unsigned int, unsigned int> x_range = { 0, UINT_MAX }, std::pair<unsigned int, unsigned int> y_range = { 0, UINT_MAX }, std::pair<unsigned int, unsigned int> c_range = { 0, UINT_MAX });
    }






    /*--------------------------------------------内部实现--------------------------------------------*/

    namespace mext
    {
        //从路径加载ncnn的net，正常返回0，出错返回其他值
        inline int ncnnLoad(ncnn::Net& out_net, const std::string& param_path, const std::string& bin_path)
        {
            //ncnn::Net net;
            if (out_net.load_param(param_path.c_str()))
            {
                mprintfW("Load param file %s failed!\n", param_path.c_str());
                return -1;
            }
            if (out_net.load_model(bin_path.c_str()))
            {
                mprintfW("Load bin file %s failed!\n", bin_path.c_str());
                return -1;
            }
            return 0;
        }

        //快速运行一次ncnn的net推理
        inline std::vector<ncnn::Mat> ncnnQuickRun(ncnn::Net& net_in, ncnn::Mat& input, const std::string& in_name,
            const std::vector<std::string>& out_names)
        {

            ncnn::Extractor extractor = net_in.create_extractor();
            extractor.set_light_mode(true);
            extractor.input(in_name.c_str(), input);

            std::vector<ncnn::Mat> outs;
            ncnn::Mat out;
            for (const std::string& out_id : out_names)
            {
                extractor.extract(out_id.c_str(), out);
                outs.push_back(out);
            }
            return outs;
        }

        //输入ncnn的net路径，快速运行一次ncnn的net推理
        inline std::vector<ncnn::Mat> ncnnQuickRun(const std::string& param_path, const std::string& model_path, ncnn::Mat& input, const std::string& in_name, const std::vector<std::string>& out_names)
        {
            ncnn::Net net;
            net.load_param(param_path.c_str());
            net.load_model(model_path.c_str());
            return mext::ncnnQuickRun(net, input, in_name, out_names);
        }

        inline void ncnnPrintMat(const ncnn::Mat& m, std::pair<unsigned int, unsigned int> x_range, std::pair<unsigned int, unsigned int> y_range, std::pair<unsigned int, unsigned int> c_range)
        {
            using Range = std::pair<unsigned int, unsigned int>;
            Range x_norm_range = mbase::_normRange(x_range, m.w);
            Range y_norm_range = mbase::_normRange(y_range, m.h);
            Range c_norm_range = mbase::_normRange(c_range, m.c);
            int xstart = x_norm_range.first, xend = x_norm_range.second;
            int ystart = y_norm_range.first, yend = y_norm_range.second;
            int cstart = c_norm_range.first, cend = c_norm_range.second;

            const float* ptr = nullptr;
            int bias = ystart * m.w;
            std::cout << "ncnn::Mat{";
            for (int y = ystart; y < yend; y++)
            {
                if (y == ystart)
                    std::cout << "[";
                else std::cout << std::string(10, ' ') << "[";
                for (int x = xstart; x < xend; x++)
                {
                    std::cout << "(";
                    for (int c = cstart; c < cend; c++)
                    {
                        ptr = m.channel(c);
                        if (c != cend - 1)
                            std::cout << std::setiosflags(std::ios::scientific) << std::setprecision(4) << std::setw(11)<< ptr[x + bias] << " ";
                        else std::cout << std::setiosflags(std::ios::scientific) << std::setprecision(4) << std::setw(11) << ptr[x + bias];
                    }
                    if (x != xend - 1)
                        std::cout << ") ";
                    else std::cout << ")";
                }
                bias += m.w;
                if (y != yend - 1)
                    std::cout << "]" << std::endl;
                else std::cout << "]";
            }
            std::cout << "}\n";
        }
    }
}

#endif // !NCNN_HPP_MINEUTILS