//mineutils库的路径相关功能
#pragma once
#ifndef PATH_HPP_MINEUTILS
#define PATH_HPP_MINEUTILS

#include<cstdio>
#include<iostream>
#include<set>
#include<sys/stat.h>
#include<string>
#include<stdlib.h>
#include<vector>

#if defined(_MSC_VER)
#include<direct.h>
#include<io.h>
#else
#include<dirent.h>
#include<unistd.h>
#endif

#include"str.hpp"
#include"log.hpp"


namespace mineutils
{
    //输入的路径应为合法格式
    namespace mpath
    {
        /*--------------------------------------------用户接口--------------------------------------------*/

        //将windows路径中的\\变为标准的/分隔符，并将路径标准化
        std::string normPath(const std::string& path);

        //判断路径是否存在
        bool exists(const std::string& path);

        //从路径字符串获取文件名
        std::string splitName(const std::string& path, bool suffix = true);

        //获取输入的后缀名
        std::string extension(const std::string& path);

        //判断路径是否为绝对路径
        bool isAbs(const std::string& path);

        //判断路径是否为真实目录
        bool isDir(const std::string& path);

        //判断路径是否为真实文件
        bool isFile(const std::string& path);

        //判断路径是否为真实的图像文件
        bool isImage(const std::string& path, const std::set<std::string>& img_exts = { "png", "PNG", "jpg", "JPG", "jpeg", "JPEG" });

        //判断路径是否为真实的视频文件
        bool isVideo(const std::string& path, const std::set<std::string>& video_exts = { "avi", "AVI", "mp4", "MP4", "flv", "FLV" });

        //实现类似python的os.path.join功能
        template<class... Strs>
        std::string join(const std::string& path1, const std::string& path2, const Strs&... paths);

#if defined(_MSC_VER)
        //获取目录下的一级文件和目录
        std::vector<std::string> listDir(const std::string& path, bool return_path = true, std::set<std::string> ignore_names = {});

        //创建目录
        bool makeDirs(const std::string& path);
#else
        //获取目录下的一级文件和目录
        std::vector<std::string> listDir(const std::string& path, bool return_path = true, std::set<std::string> ignore_names = {});

        //创建目录
        bool makeDirs(const std::string& path);
#endif
        //返回路径字符串对应的父目录
        std::string parent(const std::string& path);

        //删除文件或目录
        bool remove(const std::string& path);

        //遍历目录下的所有文件，出错时返回空vector
        std::vector<std::string> walk(const std::string& path, bool return_path = true);











        /*--------------------------------------------内部实现--------------------------------------------*/

        //将windows路径中的\\变为标准的/分隔符，并将路径标准化
        inline std::string normPath(const std::string& path)
        {
            std::string pth = path;
            size_t pos;
            while (pth.find("\\") != std::string::npos)
            {
                pos = pth.find("\\");
                pth.replace(pos, 1, "/");
            }
            while (pth.find("//") != std::string::npos)
            {
                pos = pth.find("//");
                pth.replace(pos, 2, "/");
            }
            while (pth.find("/./") != std::string::npos)
            {
                pos = pth.find("/./");
                pth.replace(pos, 3, "/");
            }
            if (pth.find("./") == 0)
            {
                pth.replace(0, 2, "");
            }
            if (pth.empty())
                return ".";
            if (pth.rfind("/.") != std::string::npos and pth.rfind("/.") == pth.size() - 2)
            {
                pos = pth.rfind("/.");
                pth.replace(pos + 1, 1, "");
            }
            if (pth.rfind("/") == pth.size() - 1)
            {
                if (pth.find(":/") == 1 and pth.size() > 4)
                    pth.replace(pth.size() - 1, 1, "");
                else if (pth.find(":/") == std::string::npos and pth.size() > 1)
                    pth.replace(pth.size() - 1, 1, "");
            }
            return pth;
        }

        //判断路径是否存在
        inline bool exists(const std::string& path)
        {
            std::string pth = mpath::normPath(path);
            struct stat buffer;
            return stat(pth.c_str(), &buffer) == 0;
        }

        //从路径字符串获取文件名
        inline std::string splitName(const std::string& path, bool suffix)
        {
            std::string pth = mpath::normPath(path);
            std::string name;
            if (suffix)
                name = pth.substr(pth.find_last_of('/') + 1);
            else name = pth.substr(pth.find_last_of('/') + 1, name.find_last_of("."));
            return name;
        }

        //获取输入的后缀名
        inline std::string extension(const std::string& path)
        {
            std::string pth = mpath::splitName(path, true);
            size_t ext_pos = pth.find_last_of(".");
            if (ext_pos == std::string::npos)
                return "";
            else return pth.substr(ext_pos + 1);
        }

        //判断路径是否为绝对路径
        inline bool isAbs(const std::string& path)
        {
            std::string pth = mpath::normPath(path);
            if (pth.find(":/") != std::string::npos)
                return pth.substr(1, 2) == ":/";
            else return pth.substr(0, 1) == "/";
        }

        //判断路径是否为真实目录
        inline bool isDir(const std::string& path)
        {
            std::string pth = mpath::normPath(path);
            struct stat buffer;
            if (stat(pth.c_str(), &buffer) == 0)
            {
                if (buffer.st_mode & S_IFDIR)
                    return true;
            }
            return false;
        }

        //判断路径是否为真实文件
        inline bool isFile(const std::string& path)
        {
            std::string pth = mpath::normPath(path);
            struct stat buffer;
            if (stat(pth.c_str(), &buffer) == 0)
            {
                if (buffer.st_mode & S_IFREG)
                    return true;
            }
            return false;
        }

        //判断路径是否为真实的图像文件
        inline bool isImage(const std::string& path, const std::set<std::string>& img_exts)
        {
            std::string pth = mpath::normPath(path);
            if (!mpath::isFile(pth))
                return false;
            std::string ext = mpath::extension(pth);
            if (img_exts.find(ext) == img_exts.end())
                return false;
            else return true;
        }

        //判断路径是否为真实的视频文件
        inline bool isVideo(const std::string& path, const std::set<std::string>& video_exts)
        {
            std::string pth = mpath::normPath(path);
            if (!isFile(pth))
                return false;
            std::string ext = mpath::extension(pth);
            if (video_exts.find(ext) == video_exts.end())
                return false;
            else return true;
        }

        inline std::string& _join(std::string& path)
        {
            return path;
        }

        template<class... Strs>
        inline std::string _join(const std::string& path1, const std::string& path2, const Strs&... paths)
        {
            std::string pth2 = mpath::normPath(path2);
            pth2 = path1 + "/" + pth2;
            return mpath::_join(pth2, paths...);
        }

        //实现类似python的os.path.join功能
        template<class... Strs>
        inline std::string join(const std::string& path1, const std::string& path2, const Strs&... paths)
        {
            std::string pth1 = mpath::normPath(path1);
            return mpath::_join(pth1, path2, paths...);
        }

#if defined(_MSC_VER)
        //获取目录下的一级文件和目录
        inline std::vector<std::string> listDir(const std::string& path, bool return_path, std::set<std::string> ignore_names)
        {
            std::string pth = mpath::normPath(path);
            intptr_t hFile = 0;
            struct _finddata_t fileinfo;
            std::vector<std::string> files;
            std::string p = pth;

            if (!mpath::isDir(pth))
            {
                std::cout << msgW("The input path is not a folder or does not exist. Please check it!\n");
                return {};
            }
            if ((hFile = _findfirst(p.append("/*").c_str(), &fileinfo)) != std::string::npos)
            {
                do
                {
                    std::string fname = fileinfo.name;
                    bool ignore = (ignore_names.find(fname) != ignore_names.end());

                    if (!ignore and fname != "." and fname != "..")
                    {
                        // 保存文件的全路径
                        if (return_path)
                            files.push_back(mpath::join(pth, fname));
                        else files.push_back(fname);
                    }
                } while (_findnext(hFile, &fileinfo) == 0);  //寻找下一个，成功返回0，否则-1
                _findclose(hFile);
            }
            return files;
        }

        //创建目录
        inline bool makeDirs(const std::string& path)
        {
            if (mpath::exists(path))
                return true;
            std::string pth = mpath::normPath(path);
            size_t sep_pos;
            std::string pth_tree;
            if (mpath::isAbs(pth))
                sep_pos = 2;
            else sep_pos = 0;
            bool mk_res = true;
            do
            {
                sep_pos = pth.find("/", sep_pos + 1);
                pth_tree = pth.substr(0, sep_pos);
                if (!mpath::isDir(pth_tree))
                    _mkdir(pth_tree.c_str());
            } while (sep_pos != std::string::npos);
            return mpath::isDir(pth);
        }
#else
        //获取目录下的一级文件和目录
        inline std::vector<std::string> listDir(const std::string& path, bool return_path, std::set<std::string> ignore_names)
        {
            std::string pth = mpath::normPath(path);
            DIR* pDir;
            struct dirent* ptr;
            std::vector<std::string> files;

            if (!mpath::isDir(pth))
            {
                std::cout << msgW("The input path is not a folder or does not exist. Please check it!\n");
                return {};
            }
            pDir = opendir(pth.c_str());
            while ((ptr = readdir(pDir)) != nullptr)
            {
                std::string fname = ptr->d_name;
                bool ignore = (ignore_names.find(fname) != ignore_names.end());
                if (!ignore and fname != "." and fname != "..")
                {
                    if (return_path)
                        files.push_back(mpath::join(pth, fname));
                    else files.push_back(fname);
                }
            }
            closedir(pDir);
            return files;
        }

        //创建目录
        inline bool makeDirs(const std::string& path)
        {
            if (mpath::exists(path))
                return true;
            std::string pth = mpath::normPath(path);
            size_t sep_pos = 0;
            std::string pth_tree;
            do
            {
                sep_pos = pth.find("/", sep_pos + 1);
                pth_tree = pth.substr(0, sep_pos);
                if (!mpath::isDir(pth_tree))
                    mkdir(pth_tree.c_str(), 0777);
            } while (sep_pos != std::string::npos);
            return mpath::isDir(pth);
        }
#endif

        //返回路径字符串对应的父目录，仅分割字符串，不对..符号进行逻辑判断
        inline std::string parent(const std::string& path)
        {
            std::string pth = mpath::normPath(path);
            std::string parent = pth.substr(0, pth.find_last_of('/'));
            return parent;
        }

        inline void _removedir(const std::string& path)
        {
            std::vector<std::string> listdir_res = mpath::listDir(path, true);
            for (std::string& sub_path : listdir_res)
            {
                if (mpath::isDir(sub_path))
                {
                    mpath::_removedir(sub_path);
                }
                else ::remove(sub_path.c_str());
            }
#if defined(_MSC_VER)
            ::_rmdir(path.c_str());
#else
            ::rmdir(path.c_str());
#endif
        }

        //删除文件或目录
        inline bool remove(const std::string& path)
        {
            std::string pth = mpath::normPath(path);
            if (mpath::isDir(pth))
            {
                mpath::_removedir(pth);
            }
            else if (mpath::isFile(pth))
            {
                ::remove(pth.c_str());
            }
            return !mpath::exists(pth);
        }

        //遍历目录下的所有文件，出错时返回空vector
        inline std::vector<std::string> walk(const std::string& path, bool return_path)
        {
            std::string pth = mpath::normPath(path);
            std::vector<std::string> filenames;
            std::string filename;

            if (!mpath::isDir(pth))
            {
                std::cout << msgW("The input path is not a folder or does not exist. Please check it!\n");
                return {};
            }
            std::vector<std::string> listdir_res = mpath::listDir(pth, true);
            for (std::string& f_d_path : listdir_res)
            {
                if (mpath::isDir(f_d_path))
                {
                    std::vector<std::string> sub_filenames = mpath::walk(f_d_path, return_path);
                    filenames.insert(filenames.end(), sub_filenames.begin(), sub_filenames.end());
                }
                else return_path ? filenames.push_back(f_d_path) : filenames.push_back(mpath::splitName(f_d_path));
            }
            return filenames;
        }
    };
}
#endif // !PATH_HPP_MINEUTILS