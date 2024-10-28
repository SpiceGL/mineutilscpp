//mineutils库的OpenCV相关便利功能，基于OpenCV 3.2.0以上版本
#pragma once
#ifndef CV_HPP_MINEUTILS
#define CV_HPP_MINEUTILS

#include<algorithm>
#include<iomanip>
#include<iostream>
#include<limits.h>
#include<string>
#include<vector>
#include"opencv2/core.hpp"
#include"opencv2/highgui.hpp"
#include"opencv2/imgproc.hpp"
#include"opencv2/videoio.hpp"

#include"type.hpp"
#include"str.hpp"
#include"log.hpp"
#include"math.hpp"
#include"io.hpp"


namespace mineutils
{
    /*--------------------------------------------用户接口--------------------------------------------*/

    namespace mext
    {
        //快速设置窗口属性，返回为窗口的名字
        inline std::string setWindowCV(const std::string& win_name, cv::Size size = { -1, -1 },
            std::pair<int, int> position = { -1, -1 }, int flag = cv::WINDOW_FREERATIO);

        //快速显示图像，一步到位设置窗口和显示属性
        int quickShowCV(const std::string& win_name, cv::Mat& img,
            float wait = 1, bool close = false, cv::Size size = { -1, -1 },
            std::pair<int, int> position = { -1, -1 }, int flag = cv::WINDOW_FREERATIO);

        //快速显示视频
        void quickPlayCV(const std::string& win_name, const std::string& video_path,
            float wait = 30, cv::Size size = { -1, -1 },
            std::pair<int, int> position = { -1, -1 }, int flag = cv::WINDOW_FREERATIO);

        /*  为图像添加文字
            @param dst: cv::Mat图像
            @param label: 文字内容
            @param position: 文字左下角坐标
            @param text_color: 文字颜色
            @param word_type: 文字类型，用opencv的HersheyFonts枚举类型表示
            @param word_scale: 文字尺寸
            @param text_thickness: 文字粗细
            @param have_bg: 是否为文字添加背景
            @param bg_color: 文字背景的颜色，have_bg为true时生效   */
        void putLabelCV(cv::Mat& dst, const std::string& label, cv::Point position, cv::Scalar text_color = { 255,255,255 },
            int word_type = cv::FONT_HERSHEY_SIMPLEX, float word_scale = 1, int text_thickness = 2,
            bool have_bg = true, cv::Scalar bg_color = { 255, 0, 0 });

        /*  在图像上绘制检测框及标签
            @param dst: cv::Mat图像
            @param ltrb: 检测框坐标{left, top, right, botton}
            @param label: 文字内容
            @param bbox_color: 检测框颜色
            @param text_color: 文字颜色
            @param word_type: 文字类型，用opencv的HersheyFonts枚举类型表示
            @param word_scale: 文字尺寸
            @param bbox_thickness: 检测框粗细
            @param text_thickness: 文字粗细   */
        void putBoxCV(cv::Mat& dst, const mmath::LTRB& ltrb, const std::string& label = "",
            cv::Scalar bbox_color = { 0,255,0 }, cv::Scalar text_color = { 255,255,255 },
            int word_type = cv::FONT_HERSHEY_SIMPLEX, float word_scale = 1,
            int bbox_thickness = 3, int text_thickness = 2);

        /*  在图像上绘制检测框及标签
            @param dst: cv::Mat图像
            @param xywh: 检测框坐标{center_x, center_y, width, height}
            @param label: 文字内容
            @param bbox_color: 检测框颜色
            @param text_color: 文字颜色
            @param word_type: 文字类型，用opencv的HersheyFonts枚举类型表示XRange
            @param word_scale: 文字尺寸
            @param bbox_thickness: 检测框粗细
            @param text_thickness: 文字粗细   */
        void putBoxCV(cv::Mat& dst, mmath::XYWH xywh, const std::string& label = "",
            cv::Scalar bbox_color = { 0,255,0 }, cv::Scalar text_color = { 255,255,255 },
            int word_type = cv::FONT_HERSHEY_SIMPLEX, float word_scale = 1,
            int bbox_thickness = 3, int text_thickness = 2);

        //自定义前3个通道的值，dst通道数必须大于3，否则可能异常
        template<class T = int>
        void channelInit(cv::Mat& dst, cv::Point3_<T> channel_value = { 0, 0, 0 });


        /*  打印cv::Mat的值，目前只支持2D的Mat
            @param img：要打印的cv::Mat
            @param x_range：x坐标值或range，支持Python风格range，类型限制为std::pair<int, int>或int
            @param y_range：y坐标值或range，支持Python风格range，类型限制为std::pair<int, int>或int
            @param c_range：channel值或range，支持Python风格range，类型限制为std::pair<int, int>或int   */
        template<class Tx = std::pair<int, int>, class Ty = std::pair<int, int>, class Tc = std::pair<int, int>, typename std::enable_if<mtype::InTypesChecker<typename std::decay<Tx>::type, std::pair<int, int>, int>::value&& mtype::InTypesChecker<typename std::decay<Tx>::type, std::pair<int, int>, int>::value&& mtype::InTypesChecker<typename std::decay<Tx>::type, std::pair<int, int>, int>::value, int>::type = 0>
        void printMat(const cv::Mat& img, Tx x_range = { 0, INT_MAX }, Ty y_range = { 0, INT_MAX }, Tc c_range = { 0, INT_MAX });
    }







    /*--------------------------------------------内部实现--------------------------------------------*/

    namespace mext
    {
        /*
        #cv2.VideoWriter_fourcc('X', '2', '6', '4'), 该参数是较新的MPEG-4编码,产生的文件较小,文件扩展名应为.mp4
        #cv2.VideoWriter_fourcc('P', 'I', 'M', 'I'), 该参数是较旧的MPEG-1编码,文件名后缀为.avi
        #cv2.VideoWriter_fourcc('M', 'P', '4', '2'), 该参数是MPEG-2编码,产生的文件不会特别大,文件名后缀为.avi
        #cv2.VideoWriter_fourcc('D', 'I', 'V', '3'), 该参数是MPEG-3编码,产生的文件不会特别大,文件名后缀为.avi
        #cv2.VideoWriter_fourcc('D', 'I', 'V', 'X'), 该参数是MPEG-4编码,产生的文件不会特别大,文件名后缀为.avi
        #cv2.VideoWriter_fourcc('X', 'V', 'I', 'D'), 该参数是较旧的MPEG-4编码,产生的文件不会特别大,文件名后缀为.avi
        #cv2.VideoWriter_fourcc('M', 'P', '4', 'V'), 该参数也是较旧的MPEG-4编码,产生的文件不会特别大,文件扩展名应为.m4v
        #cv2.VideoWriter_fourcc('T', 'H', 'E', 'O'), 该参数是Ogg Vorbis,产生的文件相对较大,文件名后缀为.ogv
        #cv2.VideoWriter_fourcc('F', 'L', 'V', '1'), 该参数是Flash视频,产生的文件相对较大,文件名后缀为.flv
        #cv2.VideoWriter_fourcc('M', 'J', 'P', 'G'), 该参数是motion-jpeg编码,产生的文件较大,文件名后缀为.avi
        #cv2.VideoWriter_fourcc('I', '4', '2', '0'),该参数是未压缩的YUV编码,4:2:0色度子采样,这种编码广泛兼容,但会产生特别大的文件,文件扩展名应为.avi
        */

        //快速设置窗口属性，返回为窗口的名字
        inline std::string setWindowCV(const std::string& win_name, cv::Size size, std::pair<int, int> position, int flag)
        {
            cv::namedWindow(win_name, flag);
            if (size.width != -1)
                cv::resizeWindow(win_name, size.width, size.height);
            if (position.first != -1)
                cv::moveWindow(win_name, position.first, position.second);
            return win_name;
        }


        //快速显示图像，一步到位设置窗口和显示属性
        inline int quickShowCV(const std::string& win_name, cv::Mat& img, float wait, bool close, cv::Size size, std::pair<int, int> position, int flag)
        {
            cv::namedWindow(win_name, flag);
            if (size.width != -1)
                cv::resizeWindow(win_name, size.width, size.height);
            if (position.first != -1)
                cv::moveWindow(win_name, position.first, position.second);
            cv::imshow(win_name, img);
            int k = cv::waitKey(wait) & 0xff;
            if (close)
                cv::destroyWindow(win_name);
            return k;
        }

        //快速显示视频
        inline void quickPlayCV(const std::string& win_name, const std::string& video_path, float wait, cv::Size size, std::pair<int, int> position, int flag)
        {
            auto cap = cv::VideoCapture(video_path);
            if (not cap.isOpened())
            {
                mprintfW("Open video failed, skiped!\n");
                return;
            }
            mext::setWindowCV(win_name, size, position, flag);
            cv::Mat frame;
            while (true)
            {
                bool ret = cap.read(frame);
                if (not ret)
                {
                    cv::destroyWindow(win_name);
                    mprintfN("Video end.\n");
                    break;
                }
                cv::imshow(win_name, frame);
                int k = cv::waitKey(wait) & 0xff;
                if (k == 27)
                {
                    cv::destroyWindow(win_name);
                    mprintfN("Received stop signal, stoped the video.\n");
                    break;
                }
            }
            cap.release();
        }

        inline void putLabelCV(cv::Mat& dst, const std::string& label, cv::Point position, cv::Scalar text_color, int word_type, float word_scale, int text_thickness, bool have_bg, cv::Scalar bg_color)
        {
            if (label.size() != 0)
            {
                cv::Point& c1 = position;
                if (have_bg)
                {
                    int baseline;
                    cv::Size text_size = cv::getTextSize(label, word_type, word_scale, text_thickness, &baseline);
                    cv::Point c2 = { c1.x + text_size.width, c1.y - (int)(text_size.height * 1.2) };
                    cv::rectangle(dst, c1, c2, bg_color, -1);
                }
                cv::putText(dst, label, c1, word_type, word_scale, text_color, text_thickness, cv::LINE_AA);
            }
        }

        inline void putBoxCV(cv::Mat& dst, const mmath::LTRB& ltrb, const std::string& label, cv::Scalar bbox_color, cv::Scalar text_color, int word_type, float word_scale, int bbox_thickness, int text_thickness)
        {
            cv::Point c1 = { ltrb.left , ltrb.top };
            cv::Point c2 = { ltrb.right , ltrb.bottom };
            cv::rectangle(dst, c1, c2, bbox_color, bbox_thickness);

            c1.x -= bbox_thickness - 1;
            c1.y -= bbox_thickness - 1;

            if (label.size() > 0)
            {
                cv::Point label_pos = c1;
                int* baseline = nullptr;
                cv::Size text_size = cv::getTextSize(label, word_type, word_scale, text_thickness, baseline);
                if (label_pos.x + text_size.width > dst.cols)
                    label_pos.x = dst.cols - text_size.width;
                if (label_pos.y - text_size.height < 0)
                    label_pos.y = c1.y + text_size.height;
                mext::putLabelCV(dst, label, label_pos, text_color, word_type, word_scale, text_thickness, true, bbox_color);
            }
        }

        inline void putBoxCV(cv::Mat& dst, mmath::XYWH xywh, const std::string& label, cv::Scalar bbox_color, cv::Scalar text_color, int word_type, float word_scale, int bbox_thickness, int text_thickness)
        {
            mext::putBoxCV(dst, xywh.toLTRB(), label, bbox_color, text_color, word_type, word_scale, bbox_thickness, text_thickness);
        }

        template<class T>
        inline void channelInit(cv::Mat& dst, cv::Point3_<T> channel_value)
        {
            if (dst.empty())
                return;
            std::vector<cv::Mat> ma_mb_mc;
            cv::split(dst, ma_mb_mc);
            ma_mb_mc[0] = channel_value.x;
            ma_mb_mc[1] = channel_value.y;
            ma_mb_mc[2] = channel_value.z;
            cv::merge(ma_mb_mc, dst);
        }
        
        template<class DataT>
        inline void _printCVMat(const cv::Mat& img, int xstart, int xend, int ystart, int yend, bool isInt)
        {
            double neg_value, pos_value;
            cv::minMaxLoc(img(cv::Range(xstart, xend), cv::Range(ystart, yend)), &neg_value, &pos_value);
            int digits_neg = 0, digits_pos = 0;
            do
            {
                neg_value /= 10;
                digits_neg++;
            } while (neg_value <= -1);
            do
            {
                pos_value /= 10;
                digits_pos++;
            } while (pos_value >= 1);

            int digits;
            if (digits_neg >= digits_pos && digits_neg < 0)
                digits = digits_neg + 1;
            else digits = digits_pos;

            std::cout << "cv::Mat{";
            for (int y = ystart; y < yend; y++)
            {
                if (y == ystart)
                    std::cout << "[";
                else std::cout << std::string(8, ' ') << "[";
                auto* ptr = img.ptr<DataT>(y);
                for (int x = xstart; x < xend; x++)
                {
                    if (isInt)
                    {
                        std::cout << std::setw(digits) << int(ptr[x]);
                        if (x != xend - 1)
                            std::cout << " ";
                    }
                    else
                    {
                        if (digits <= 3)
                            std::cout << mstr::zfillFlt(ptr[x], digits, 4, ' ', '0');
                        else
                            std::cout << std::setiosflags(std::ios::scientific) << std::setprecision(5) << ptr[x];
                        if (x != xend - 1)
                            std::cout << " ";
                    }
                }
                if (y != yend - 1)
                    std::cout << "]\n";
                else std::cout << "]";
            }
            std::cout << "}\n";
        }

        template<class DataT, size_t CN>
        inline void _printCVMat(const cv::Mat& img, int xstart, int xend, int ystart, int yend, int cstart, int cend, bool isInt)
        {
            int digits;
            if (isInt)
            {
                double neg_value, pos_value;
                cv::minMaxLoc(img(cv::Range(ystart, yend), cv::Range(xstart, xend)), &neg_value, &pos_value);
                int digits_neg = 0, digits_pos = 0;
                do
                {
                    neg_value /= 10;
                    digits_neg++;
                } while (neg_value <= -1);
                do
                {
                    pos_value /= 10;
                    digits_pos++;
                } while (pos_value >= 1);

                if (digits_neg >= digits_pos && digits_neg < 0)
                    digits = digits_neg + 1;
                else digits = digits_pos;
            }

            std::cout << "cv::Mat{";
            for (int y = ystart; y < yend; y++)
            {
                if (y == ystart)
                    std::cout << "[";
                else std::cout << std::string(8, ' ') << "[";
                auto* ptr = img.ptr<cv::Vec<DataT, CN>>(y);
                for (int x = xstart; x < xend; x++)
                {
                    std::cout << "(";
                    for (int c = cstart; c < cend; c++)
                    {
                        if (isInt)
                        {
                            if (digits <= 11)
                                std::cout << std::setw(digits) << int(ptr[x][c]);
                            else std::cout << std::setiosflags(std::ios::scientific) 
                                << std::setprecision(4) << std::setw(11) << int(ptr[x][c]);
                            if (c != cend - 1)
                                std::cout << " ";
                        }
                        else
                        {
                            std::cout << std::setiosflags(std::ios::scientific)
                                << std::setprecision(4) << std::setw(11) << ptr[x][c];
                            if (c != cend - 1)
                                std::cout << " ";
                        }
                    }
                    if (x != xend - 1)
                        std::cout << ") ";
                    else std::cout << ")";
                }
                if (y != yend - 1)
                    std::cout << "]\n";
                else std::cout << "]";
            }
            std::cout << "}\n";
        }

        template<class Tx, class Ty, class Tc, typename std::enable_if<mtype::InTypesChecker<typename std::decay<Tx>::type, std::pair<int, int>, int>::value&& mtype::InTypesChecker<typename std::decay<Tx>::type, std::pair<int, int>, int>::value&& mtype::InTypesChecker<typename std::decay<Tx>::type, std::pair<int, int>, int>::value, int>::type>
        inline void printMat(const cv::Mat& img, Tx x_range, Ty y_range, Tc c_range)
        {
            /*      C1    C2    C3    C4
            CV_8U    0    8    16    24       uchar
            CV_8S    1    9    17    25       char
            CV_16U    2    10    18    26       ushort
            CV_16S    3    11    19    27       short
            CV_32S    4    12    20    28       int
            CV_32F    5    13    21    29       float
            CV_64F    6    14    22    30       double */

            using pt = std::pair<int, int>;

            pt x_norm_range = mmath::normRange(x_range, img.cols);
            pt y_norm_range = mmath::normRange(y_range, img.rows);
            pt c_norm_range = mmath::normRange(c_range, img.channels());
            int xstart = x_norm_range.first, xend = x_norm_range.second;
            int ystart = y_norm_range.first, yend = y_norm_range.second;
            int cstart = c_norm_range.first, cend = c_norm_range.second;

            if (img.channels() == 1)
            {
                if (img.depth() == CV_8U)
                    _printCVMat<uchar>(img, xstart, xend, ystart, yend, true);
                else if (img.depth() == CV_8S)
                    _printCVMat<short>(img, xstart, xend, ystart, yend, true);
                else if (img.depth() == CV_16U)
                    _printCVMat<ushort>(img, xstart, xend, ystart, yend, true);
                else if (img.depth() == CV_16S)
                    _printCVMat<short>(img, xstart, xend, ystart, yend, true);
                else if (img.depth() == CV_32S)
                    _printCVMat<int>(img, xstart, xend, ystart, yend, true);
                else if (img.depth() == CV_32F)
                    _printCVMat<float>(img, xstart, xend, ystart, yend, false);
                else if (img.depth() == CV_64F)
                    _printCVMat<double>(img, xstart, xend, ystart, yend, false);
            }
            else if (img.channels() == 2)
            {
                if (img.depth() == CV_8U)
                    _printCVMat<uchar, 2>(img, xstart, xend, ystart, yend, cstart, cend, true);
                else if (img.depth() == CV_8S)
                    _printCVMat<char, 2>(img, xstart, xend, ystart, yend, cstart, cend, true);
                else if (img.depth() == CV_16U)
                    _printCVMat<ushort, 2>(img, xstart, xend, ystart, yend, cstart, cend, true);
                else if (img.depth() == CV_16S)
                    _printCVMat<short, 2>(img, xstart, xend, ystart, yend, cstart, cend, true);
                else if (img.depth() == CV_32S)
                    _printCVMat<int, 2>(img, xstart, xend, ystart, yend, cstart, cend, true);
                else if (img.depth() == CV_32F)
                    _printCVMat<float, 2>(img, xstart, xend, ystart, yend, cstart, cend, false);
                else if (img.depth() == CV_64F)
                    _printCVMat<double, 2>(img, xstart, xend, ystart, yend, cstart, cend, false);
            }
            else if (img.channels() == 3)
            {
                if (img.depth() == CV_8U)
                    _printCVMat<uchar, 3>(img, xstart, xend, ystart, yend, cstart, cend, true);
                else if (img.depth() == CV_8S)
                    _printCVMat<char, 3>(img, xstart, xend, ystart, yend, cstart, cend, true);
                else if (img.depth() == CV_16U)
                    _printCVMat<ushort, 3>(img, xstart, xend, ystart, yend, cstart, cend, true);
                else if (img.depth() == CV_16S)
                    _printCVMat<short, 3>(img, xstart, xend, ystart, yend, cstart, cend, true);
                else if (img.depth() == CV_32S)
                    _printCVMat<int, 3>(img, xstart, xend, ystart, yend, cstart, cend, true);
                else if (img.depth() == CV_32F)
                    _printCVMat<float, 3>(img, xstart, xend, ystart, yend, cstart, cend, false);
                else if (img.depth() == CV_64F)
                    _printCVMat<double, 3>(img, xstart, xend, ystart, yend, cstart, cend, false);
            }
            else if (img.channels() == 4)
            {
                if (img.depth() == CV_8U)
                    _printCVMat<uchar, 4>(img, xstart, xend, ystart, yend, cstart, cend, true);
                else if (img.depth() == CV_8S)
                    _printCVMat<char, 4>(img, xstart, xend, ystart, yend, cstart, cend, true);
                else if (img.depth() == CV_16U)
                    _printCVMat<ushort, 4>(img, xstart, xend, ystart, yend, cstart, cend, true);
                else if (img.depth() == CV_16S)
                    _printCVMat<short, 4>(img, xstart, xend, ystart, yend, cstart, cend, true);
                else if (img.depth() == CV_32S)
                    _printCVMat<int, 4>(img, xstart, xend, ystart, yend, cstart, cend, true);
                else if (img.depth() == CV_32F)
                    _printCVMat<float, 4>(img, xstart, xend, ystart, yend, cstart, cend, false);
                else if (img.depth() == CV_64F)
                    _printCVMat<double, 4>(img, xstart, xend, ystart, yend, cstart, cend, false);
            }
            /*else if (img.channels() == 2)
            {
                if (img.depth() == CV_8U)
                    _printCVMat<cv::Vec<uchar, 2>>(img, xstart, xend, ystart, yend, cstart, cend, true);
                else if (img.depth() == CV_8S)
                    _printCVMat<cv::Vec<char, 2>>(img, xstart, xend, ystart, yend, cstart, cend, true);
                else if (img.depth() == CV_16U)
                    _printCVMat<cv::Vec<ushort, 2>>(img, xstart, xend, ystart, yend, cstart, cend, true);
                else if (img.depth() == CV_16S)
                    _printCVMat<cv::Vec<short, 2>>(img, xstart, xend, ystart, yend, cstart, cend, true);
                else if (img.depth() == CV_32S)
                    _printCVMat<cv::Vec<int, 2>>(img, xstart, xend, ystart, yend, cstart, cend, true);
                else if (img.depth() == CV_32F)
                    _printCVMat<cv::Vec<float, 2>>(img, xstart, xend, ystart, yend, cstart, cend, false);
                else if (img.depth() == CV_64F)
                    _printCVMat<cv::Vec<double, 2>>(img, xstart, xend, ystart, yend, cstart, cend, false);
            }
            else if (img.channels() == 3)
            {
                if (img.depth() == CV_8U)
                    _printCVMat<cv::Vec<uchar, 3>>(img, xstart, xend, ystart, yend, cstart, cend, true);
                else if (img.depth() == CV_8S)
                    _printCVMat<cv::Vec<char, 3>>(img, xstart, xend, ystart, yend, cstart, cend, true);
                else if (img.depth() == CV_16U)
                    _printCVMat<cv::Vec<ushort, 3>>(img, xstart, xend, ystart, yend, cstart, cend, true);
                else if (img.depth() == CV_16S)
                    _printCVMat<cv::Vec<short, 3>>(img, xstart, xend, ystart, yend, cstart, cend, true);
                else if (img.depth() == CV_32S)
                    _printCVMat<cv::Vec<int, 3>>(img, xstart, xend, ystart, yend, cstart, cend, true);
                else if (img.depth() == CV_32F)
                    _printCVMat<cv::Vec<float, 3>>(img, xstart, xend, ystart, yend, cstart, cend, false);
                else if (img.depth() == CV_64F)
                    _printCVMat<cv::Vec<double, 3>>(img, xstart, xend, ystart, yend, cstart, cend, false);
            }
            else if (img.channels() == 4)
            {
                if (img.depth() == CV_8U)
                    _printCVMat<cv::Vec<uchar, 4>>(img, xstart, xend, ystart, yend, cstart, cend, true);
                else if (img.depth() == CV_8S)
                    _printCVMat<cv::Vec<char, 4>>(img, xstart, xend, ystart, yend, cstart, cend, true);
                else if (img.depth() == CV_16U)
                    _printCVMat<cv::Vec<ushort, 4>>(img, xstart, xend, ystart, yend, cstart, cend, true);
                else if (img.depth() == CV_16S)
                    _printCVMat<cv::Vec<short, 4>>(img, xstart, xend, ystart, yend, cstart, cend, true);
                else if (img.depth() == CV_32S)
                    _printCVMat<cv::Vec<int, 4>>(img, xstart, xend, ystart, yend, cstart, cend, true);
                else if (img.depth() == CV_32F)
                    _printCVMat<cv::Vec<float, 4>>(img, xstart, xend, ystart, yend, cstart, cend, false);
                else if (img.depth() == CV_64F)
                    _printCVMat<cv::Vec<double, 4>>(img, xstart, xend, ystart, yend, cstart, cend, false);
            }*/
        }
    }

    namespace mio
    {
        //为print函数添加对cv::Mat类型的支持
        inline void _print(const cv::Mat& img)
        {
            std::cout << "\n";
            mext::printMat(img, { 0, INT_MAX }, { 0, INT_MAX }, { 0, INT_MAX });
        }

        //为print函数添加对cv::MatExpr类型的支持
        inline void _print(const cv::MatExpr& img)
        {
            std::cout << "\n";
            mext::printMat(img, { 0, INT_MAX }, { 0, INT_MAX }, { 0, INT_MAX });
        }
    }
}

#endif // !CV_HPP_MINEUTILS