//mineutils库的路径相关功能
#pragma once
#ifndef PATH_HPP_MINEUTILS
#define PATH_HPP_MINEUTILS

#include<fstream>
#include<iostream>
#include<set>
#include<sys/stat.h>
#include<stdio.h>
#include<stdlib.h>
#include<string>
#include<vector>

#if defined(_MSC_VER)
#include<direct.h>
#include<io.h>
#else
#include<dirent.h>
#include<unistd.h>
#endif

#include"base.hpp"


namespace mineutils
{
    /*--------------------------------------------用户接口--------------------------------------------*/

    //输入的路径应为合法格式
    namespace mpath
    {
        //将windows路径中的\\变为标准的/分隔符，去除末尾/分隔符，并将路径标准化，只能正确处理本身合法的路径
        std::string normPath(std::string path);

        //判断路径是否存在
        bool exists(std::string path);

        //分割路径字符串的文件或目录名
        std::string splitName(std::string path, bool need_extension = true);

        //获取路径字符串的后缀名
        std::string splitExt(std::string path);

        //判断路径是否为绝对路径
        bool isAbs(std::string path);

        //判断路径是否为存在的目录(将目录的软链接视为目录，但目录的快捷方式不会被视为目录)
        bool isDir(std::string path);

        //判断路径是否为存在的普通文件
        bool isFile(std::string path);

        //判断路径是否为存在的文件且拥有给定的后缀名之一
        bool isFileMatchExts(std::string path, const std::set<std::string>& file_exts);

        //实现类似python的os.path.join功能
        template<class... Strs, typename std::enable_if<mtype::ConstructibleFromEachChecker<std::string, std::string, Strs...>::value, int>::type = 0>
        std::string join(std::string path1, std::string path2, Strs... paths);

        //获取目录下的一级文件和目录
        std::vector<std::string> listDir(std::string path, bool return_path = true, const std::set<std::string>& ignore_names = {});

        //创建目录，操作完成后path目录存在则返回true
        bool makeDirs(std::string path);

        //创建文件，操作完成后path文件存在则返回true
        bool makeFile(std::string path);

        //返回路径字符串对应的父目录
        std::string parent(std::string path);

        //遍历目录下的所有文件，出错时返回空vector
        std::vector<std::string> walk(std::string path, bool return_path = true);
    }










    /*--------------------------------------------内部实现--------------------------------------------*/

    namespace mpath
    {
        bool _exists(const std::string& path);
        std::string _splitName(const std::string& path, bool need_extension);
        std::string _splitExt(const std::string& path);
        bool _isAbs(const std::string& path);
        bool _isDir(const std::string& path);
        bool _isFile(const std::string& path);
        bool _isFileMatchExts(const std::string& path, const std::set<std::string>& file_exts);
        template<class... Strs>
        inline std::string _join(const std::string& path1, const std::string& path2, const Strs&... paths);
        std::vector<std::string> _listDir(const std::string& path, bool return_path, const std::set<std::string>& ignore_names);
        bool _makeDirs(const std::string& path);
        bool _makeFile(const std::string& path);
        std::string _parent(const std::string& path);
        bool _remove(const std::string& path);
        std::vector<std::string> _walk(const std::string& path, bool return_path);

        inline std::string normPath(std::string path)
        {
            size_t pos;
            while (path.find("\\") != std::string::npos)
            {
                pos = path.find("\\");
                path.replace(pos, 1, "/");
            }
            while (path.find("//") != std::string::npos)
            {
                pos = path.find("//");
                path.replace(pos, 2, "/");
            }
            while (path.find("/./") != std::string::npos)
            {
                pos = path.find("/./");
                path.replace(pos, 3, "/");
            }
            if (path.find("./") == 0)
            {
                path.replace(0, 2, "");
            }
            if (path.empty())
                return ".";
            if (path.rfind("/.") != std::string::npos and path.rfind("/.") == path.size() - 2)
            {
                pos = path.rfind("/.");
                path.replace(pos + 1, 1, "");
            }
            if (path.rfind("/") == path.size() - 1)
            {
                if (path.find(":/") == 1 and path.size() > 4)
                    path.replace(path.size() - 1, 1, "");
                else if (path.find(":/") == std::string::npos and path.size() > 1)
                    path.replace(path.size() - 1, 1, "");
            }
            return path;
        }

        inline bool exists(std::string path)
        {
            return mpath::_exists(mpath::normPath(std::move(path)));
        }
        inline bool _exists(const std::string& path)
        {
            struct ::stat buffer;
            return ::stat(path.c_str(), &buffer) == 0;
        }

        inline std::string splitName(std::string path, bool need_extension)
        {
            return mpath::_splitName(mpath::normPath(std::move(path)), need_extension);
        }
        inline std::string _splitName(const std::string& path, bool need_extension)
        {
            std::string name;
            if (need_extension)
                name = path.substr(path.rfind('/') + 1);
            else
            {
                auto start_pos = path.rfind('/') + 1;
                auto end_pos = path.rfind('.');
                if (end_pos > start_pos)
                    name = path.substr(start_pos, end_pos - start_pos);
                else name = path.substr(start_pos);
            }
            return name;
        }

        inline std::string splitExt(std::string path)
        {
            return mpath::_splitExt(mpath::normPath(std::move(path)));
        }
        inline std::string _splitExt(const std::string& path)
        {
            auto name_pos = path.rfind('/') + 1;
            auto point_pos = path.rfind('.');
            std::string ext;
            if (point_pos == std::string::npos)
                return "";
            if (point_pos >= name_pos)
                return path.substr(point_pos + 1);
            return "";
        }

        inline bool isAbs(std::string path)
        {
            return mpath::_isAbs(mpath::normPath(std::move(path)));
        }
        inline bool _isAbs(const std::string& path)
        {
            return path.find(":/") == 1 || path.substr(0, 1) == "/";
        }

        inline bool isDir(std::string path)
        {
            return mpath::_isDir(mpath::normPath(std::move(path)));
        }
        inline bool _isDir(const std::string& path)
        {
            struct ::stat buffer;
            return ::stat(path.c_str(), &buffer) == 0 && (buffer.st_mode & S_IFDIR);
        }

        inline bool isFile(std::string path)
        {
            return mpath::_isFile(mpath::normPath(std::move(path)));
        }
        inline bool _isFile(const std::string& path)
        {
            struct ::stat buffer;
            return ::stat(path.c_str(), &buffer) == 0 && (buffer.st_mode & S_IFREG);
        }
        inline bool isFileMatchExts(std::string path, const std::set<std::string>& file_exts)
        {
            return mpath::_isFileMatchExts(mpath::normPath(std::move(path)), file_exts);
        }
        inline bool _isFileMatchExts(const std::string& path, const std::set<std::string>& file_exts)
        {
            return mpath::_isFile(path) && file_exts.find(mpath::_splitExt(path)) != file_exts.end();
        }

        inline std::string _joinBranch(const std::string& path)
        {
            return path;
        }
        template<class... Strs>
        inline std::string _joinBranch(const std::string& path1, const std::string& path2, const Strs&... paths)
        {
            std::string pth2 = path1 + "/" + path2;
            return mpath::_joinBranch(pth2, paths...);
        }

        //实现类似python的os.path.join功能
        template<class... Strs, typename std::enable_if<mtype::ConstructibleFromEachChecker<std::string, std::string, Strs...>::value, int>::type>
        inline std::string join(std::string path1, std::string path2, Strs... paths)
        {
            return mpath::_join(mpath::normPath(std::move(path1)), mpath::normPath(std::move(path2)), mpath::normPath(std::move(paths))...);
        }
        template<class... Strs>
        inline std::string _join(const std::string& path1, const std::string& path2, const Strs&... paths)
        {
            return mpath::_joinBranch(path1, path2, paths...);
        }

        inline std::vector<std::string> listDir(std::string path, bool return_path, const std::set<std::string>& ignore_names)
        {
            return mpath::_listDir(mpath::normPath(std::move(path)), return_path, ignore_names);
        }
#if defined(_MSC_VER)
        inline std::vector<std::string> _listDir(const std::string& path, bool return_path, const std::set<std::string>& ignore_names)
        {
            intptr_t hFile = 0;
            struct _finddata_t fileinfo;
            std::vector<std::string> files;
            std::string p = path;

            if (!mpath::_isDir(path))
            {
                mprintfW("The input path:%s is not a folder or does not exist. Please check it!\n", path.c_str());
                return {};
            }
            if ((hFile = _findfirst(p.append("/*").c_str(), &fileinfo)) != std::string::npos)
            {
                std::string fname;
                do
                {
                    fname = fileinfo.name;
                    bool ignore = (ignore_names.find(fname) != ignore_names.end());

                    if (!ignore and fname != "." and fname != "..")
                    {
                        // 保存文件的全路径
                        if (return_path)
                            files.push_back(mpath::_join(path, fname));
                        else files.push_back(fname);
                    }
                } while (_findnext(hFile, &fileinfo) == 0);  //寻找下一个，成功返回0，否则-1
                ::_findclose(hFile);
            }
            return files;
        }
#else
        inline std::vector<std::string> _listDir(const std::string& path, bool return_path, const std::set<std::string>& ignore_names)
        {
            ::DIR* pDir;
            struct ::dirent* ptr;
            std::vector<std::string> files;

            if (!mpath::_isDir(path))
            {
                mprintfW("The input path:%s is not a folder or does not exist. Please check it!\n", path.c_str());
                return {};
            }
            pDir = ::opendir(path.c_str());
            std::string fname;
            while ((ptr = ::readdir(pDir)) != nullptr)
            {
                fname = ptr->d_name;
                bool ignore = (ignore_names.find(fname) != ignore_names.end());
                if (!ignore and fname != "." and fname != "..")
                {
                    if (return_path)
                        files.push_back(mpath::_join(path, fname));
                    else files.push_back(fname);
                }
            }
            ::closedir(pDir);
            return files;
        }

#endif
        inline bool makeDirs(std::string path)
        {
            return mpath::_makeDirs(mpath::normPath(std::move(path)));
        }
#if defined(_MSC_VER)
        inline bool _makeDirs(const std::string& path)
        {
            if (mpath::_exists(path))
                return true;
            size_t sep_pos;
            std::string pth_tree;
            if (mpath::_isAbs(path))
                sep_pos = 2;
            else sep_pos = 0;
            bool mk_res = true;
            do
            {
                sep_pos = path.find("/", sep_pos + 1);
                pth_tree = path.substr(0, sep_pos);
                if (!mpath::_isDir(pth_tree))
                    ::_mkdir(pth_tree.c_str());
            } while (sep_pos != std::string::npos);
            return mpath::_isDir(path);
        }
#else
        inline bool _makeDirs(const std::string& path)
        {
            if (mpath::_exists(path))
                return true;
            size_t sep_pos = 0;
            std::string pth_tree;
            do
            {
                sep_pos = path.find("/", sep_pos + 1);
                pth_tree = path.substr(0, sep_pos);
                if (!mpath::_isDir(pth_tree))
                    ::mkdir(pth_tree.c_str(), 0777);
            } while (sep_pos != std::string::npos);
            return mpath::_isDir(path);
        }
#endif
        inline bool makeFile(std::string path)
        {
            return mpath::_makeFile(mpath::normPath(std::move(path)));
        }

        inline bool _makeFile(const std::string& path)
        {
            if (mpath::_exists(path))
                return true;
            std::string parent_dir;
            if (parent_dir = mpath::_parent(path), !parent_dir.empty())
            {
                mpath::_makeDirs(parent_dir);
            }
            std::ofstream file(path, std::ios::binary | std::ios::out);
            file.close();
            return mpath::_isFile(path);
        }

        inline std::string parent(std::string path)
        {
            return mpath::_parent(mpath::normPath(std::move(path)));
        }
        inline std::string _parent(const std::string& path)
        {
            return path.substr(0, path.find_last_of('/'));;
        }

        inline void _removeDir(const std::string& path)
        {
            std::vector<std::string> listdir_res = mpath::_listDir(path, true, {});
            for (std::string& sub_path : listdir_res)
            {
                if (mpath::_isDir(sub_path))
                {
                    mpath::_removeDir(sub_path);
                }
                else ::remove(sub_path.c_str());
            }
#if defined(_MSC_VER)
            ::_rmdir(path.c_str());
#else
            ::rmdir(path.c_str());
#endif
        }

        inline std::vector<std::string> walk(std::string path, bool return_path)
        {
            return mpath::_walk(mpath::normPath(std::move(path)), return_path);
        }
        inline std::vector<std::string> _walk(const std::string& path, bool return_path)
        {
            std::vector<std::string> filenames;

            if (!mpath::_isDir(path))
            {
                mprintfW("The input path:%s is not a folder or does not exist. Please check it!\n", path.c_str());
                return {};
            }
            std::vector<std::string> listdir_res = mpath::_listDir(path, true, {});
            for (std::string& f_d_path : listdir_res)
            {
                if (mpath::_isDir(f_d_path))
                {
                    std::vector<std::string> sub_filenames = mpath::_walk(f_d_path, return_path);
                    filenames.insert(filenames.end(), sub_filenames.begin(), sub_filenames.end());
                }
                else return_path ? filenames.push_back(f_d_path) : filenames.push_back(mpath::_splitName(f_d_path, true));
            }
            return filenames;
        }



        _mdeprecated("Deprecated! Please use the system api instead.") inline bool remove(std::string path)
        {
            return mpath::_remove(mpath::normPath(std::move(path)));
        }
        inline bool _remove(const std::string& path)
        {
            if (mpath::_isDir(path))
            {
                mpath::_removeDir(path);
            }
            else if (mpath::_isFile(path))
            {
                ::remove(path.c_str());
            }
            return !mpath::_exists(path);
        }
    }



#ifdef MINEUTILS_TEST_MODULES
    namespace _mpathcheck
    {
        inline void mpathTest()
        {
            bool ret;
            ret = mpath::normPath("./abc") == "abc";
            if (!ret) mprintfE(R"(Failed when check: mpath::normPath("./abc"):%s == "abc")""\n", mpath::normPath("./abc").c_str());
            ret = mpath::normPath("./abc/../") == "abc/..";
            if (!ret) mprintfE(R"(Failed when check: mpath::normPath("./abc/../"):%s == "abc/..")""\n", mpath::normPath("./abc/../").c_str());

            ret = mpath::splitName("./abc/file.txt", true) == "file.txt";
            if (!ret) mprintfE(R"(Failed when check: mpath::splitName("./abc/file.txt", true):%s == "file.txt")""\n", mpath::splitName("./abc/file.txt", true).c_str());
            ret = mpath::splitName("./abc/file.txt", false) == "file";
            if (!ret) mprintfE(R"(Failed when check: mpath::splitName("./abc/file.txt", false):%s == "file")""\n", mpath::splitName("./abc/file.txt", false).c_str());

            ret = mpath::splitExt("./abc/file.txt") == "txt";
            if (!ret) mprintfE(R"(Failed when check: mpath::splitExt("./abc/file.txt"):%s == "txt")""\n", mpath::splitExt("./abc/file.txt").c_str());
            ret = mpath::splitExt("txt") == "";
            if (!ret) mprintfE(R"(Failed when check: mpath::splitExt("txt"):%s == "")""\n", mpath::splitExt("txt").c_str());
            ret = mpath::splitExt("abc.aa/txt") == "";
            if (!ret) mprintfE(R"(Failed when check: mpath::splitExt("abc.aa/txt"):%s == "")""\n", mpath::splitExt("abc.aa/txt").c_str());

            ret = mpath::isAbs("/aa/bb/cc");
            if (!ret) mprintfE(R"(Failed when check: mpath::isAbs("/aa/bb/cc"):%d)""\n", mpath::isAbs("/aa/bb/cc"));

            ret = mpath::join("aa", "bb/", "cc") == "aa/bb/cc";
            if (!ret) mprintfE(R"(Failed when check: mpath::join("aa", "bb/", "cc"):%s == "aa/bb/cc";)""\n", mpath::join("aa", "bb/", "cc").c_str());

            ret = mpath::parent("./aa/bb") == "aa";
            if (!ret) mprintfE(R"(Failed when check: mpath::parent("./aa/bb"):%s == "aa";)""\n", mpath::parent("./aa/bb").c_str());

            ret = mpath::parent("./aa/../bb") == "aa/..";
            if (!ret) mprintfE(R"(Failed when check: mpath::parent("./aa/../bb"):%s == "aa/..";)""\n", mpath::parent("./aa/../bb").c_str());
        }

        inline void check()
        {

            printf("\n--------------------check mpath start--------------------\n");
            mpathTest();
            printf("---------------------check mpath end---------------------\n\n");
        }
    }
#endif
}
#endif // !PATH_HPP_MINEUTILS