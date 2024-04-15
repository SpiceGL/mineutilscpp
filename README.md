# mineutilscpp
一些C++的便利功能封装，用于方便自己编程，代码基于C++11标准，文本使用UTF8-SIG编码。   
所有功能都放在命名空间mineutils下，同时根据所属模块分布在次级的命名空间，如mineutils::mstr、mineutils::mtime下；基于第三方库的功能统一在次级的命名空间mineutils::mext下。

## 版本信息
当前库版本：1.1.0  
文档注释修改日期：20240410 

## 测试平台
**Windows:**  
VS2019  
**Linux:**  
gcc 9.4.0  
arm-linux-gnueabihf-gcc 8.3.0  
arm-linux-gnueabihf-gcc 9.4.0  
aarch64-linux-gnu-gcc 6.3.1  
aarch64-linux-gnu-gcc 9.4.0  
**QNX660:**  
arm-unknown-nto-qnx6.6.0eabi-gcc   
**注：** qnx660上编译时需要额外添加g++指令：-D_GLIBCXX_USE_NANOSLEEP  

## 使用方法
* 可以单独导入./mineutilshpp/modules下的模块，如：  
```
#include"mineutilshpp/modules/time.hpp"   //导入时间相关功能 
#include"mineutilshpp/modules/io.hpp"   //导入输入打印相关功能 
```   
+ 也可一键导入`mineutilshpp/__*utils__.h`，如：
```
#include"mineutilshpp/__stdutils__.h"   //一键导入基于标准库的功能封装  
#include"mineutilshpp/__cvutils__.h"   //一键导入基于标准库+OpenCV库的功能封装，需要自行添加OpenCV库
#include"mineutilshpp/__mineutils__.h"   //一键导入所有的功能封装，当前包含OpenCV和NCNN的功能，需要自行添加这两个库
```   
- 然后通过`using namespace mineutils`使用命名空间mineutils。由于所有功能都分布在次级的命名空间下，因此不用担心污染全局命名空间。  
* 最后根据模块使用其中的功能，如`mstr::toStr(123)`。 
* 在Linux上可以通过`strings xxx | grep version`命令查找应用使用的mineutils库版本。  

**注1：** io.hpp中仅声明了OpenCV和NCNN的数据类型的print相关函数，函数的实现在cv.hpp及ncnn.hpp中，因此如果使用print函数输出OpenCV或NCNN的相关数据类型，需要导入cv.hpp或ncnn.hpp。以OpenCV数据类型为例，如果未导入cv.hpp或__cvutils__.h，那么使用mio::print函数输出cv::Mat等类型时，会产生“undefined reference”、“无法解析的外部符号”类型的编译错误。   
**注2：** 
以下划线开头的函数和类不建议外部使用，这些仅用于内部功能实现，可能随时删改，且删改后只会变动PATCH版本。  

## 模块介绍
| 模块 | 功能|
|:--------:|:-------:|   
**\_\_stdutils\_\_.h** | 基于C/C++标准库实现的mineutils库部分功能。
**\_\_cvutils\_\_.h** | 基于C/C++标准库和OpenCV库实现的mineutils库部分功能。
**\_\_mineutils\_\_.h** | mineutils库的全部功能，目前依赖OpenCV和NCNN。
**base.hpp** | mineutils库的版本信息及完整实现需要的基本工具。包含于mineutils::mbase。  
**str.hpp**| std::string字符串的便捷操作，如字符串填充内容、彩色字符串等。包含于mineutils::mstr。    
**time.hpp**| 时间相关的便捷操作，如计时、休眠等。包含于mineutils::mtime。    
**type.hpp**| 类型相关操作，如判断类型异同等。包含于mineutils::mtype。    
**log.hpp**| 运行日志相关操作，目前包含生成统一格式的运行信息等。包含于mineutils::mlog。    
**file.hpp**| 文件操作，目前包含ini文件的读写等。包含于mineutils::mfile。    
**path.hpp**| 路径相关操作，如exists、listDir、join、makeDirs等便捷功能。包含于mineutils::mpath。    
**math.hpp**| 数学相关的操作，目前包含索引标准化、矩形框操作等。包含mineutils::mmath。    
**io.hpp**|  输入输出相关功能，目前包含print函数，可以自由打印STL范围内基本数据类型和容器数据类型等。包含于mineutils::mio。    
**cv.hpp**|  OpenCV相关便捷功能，如快捷显示、快捷绘制矩形框、打印cv::Mat数据等。包含于mineutils::mext和mineutils::mio。    
**ncnn.hpp**|  NCNN相关便捷功能，如快捷运行网络、打印ncnn::Mat数据等。包含于mineutils::mext和mineutils::mio。    

## 模块功能示例  

### base.hpp: 
```
...

int main()
{
    std::string version = mbase::getVersion();   //获取mineutils库版本
    
    ...
}
```   
### str.hpp:
```
...

int main()
{
    //输出红色字体
    mstr::setColorStrOn(true);   //全局开启彩色字体显示
    std::string s1 = "hello world!"
    std::cout << mstr::color(s1, mstr::Color::red) << std::endl;
    
    //填充数字
    std::string s2 = mstr::zfillInt(5, 3, '0');   //返回"005"
    
    //参数填入字符串
    std::string s3 = mstr::fstr("{} has {} billion people.", "China", "1.4");   //返回"China has 1.4 billion people."
    
    //字符串分割
    std::string s4 = " hello world! ";
    std::vector res = mstr::split(s4);   //返回vector{"hello", "world!"};
    
    ...
}
```   
### time.hpp:
```
...

int main()
{
    //普通计时
    auto start_t = mtime::now();
    ...  //do something
    auto end_t = mtine::now();
    long long cost_time = mtime::ms(end_t - start_t);
    
    //统计并打印代码段平均耗时
    mtime::MultiMeanTimeCounter time_counter(10);
    while(true)
    {
        time_counter.addStart("part1");
        ...  // do something1
        time_counter.addEnd("part1");
        
        time_counter.addStart("part2");
        ...  // do something2
        time_counter.addEnd("part2");
        
        time_counter.printAllMeanTimeCost(mtime::Unit::ms);   //每10次循环计算并打印一次平均耗时
    }
    
    //快速统计并打印代码段耗时
    {
        LocalTimeCounter local_counter("part3", mtime::Unit::ms);
        ...  // do something3
    }   //离开作用域时统计并打印耗时
    
    ...
}
```   
### type.hpp:
```
...

template<class T>
void func1(T d)
{
    if (mtype::isInTypes<T, int, char, float>())
    {
        ...  // do something1
    }   // x进入该分支
    else
    {
        ...  // do something2
    }   // y进入该分支
}

int main()
{
    //判断参数是否属于后续类型
    const int x = 1;
    double y = 1.5;
    func1(x);  
    func1(y);  

    //判断参数是否为同一类型
    int a = 0;
    const int& b = a;
    char c = '0';
    bool res3 = mtype::isSameType(a, b);   //true
    bool res4 = mtype::isSameType(a, c);   //false
    
    ...
}
```   
### log.hpp:
```
...

int main()
{
    //创建警告或错误信息，根据mstr::setColorStrOn的开启状况决定是否返回带颜色的提示符
    std::string strw = mlog::messageW("{}: Be careful!", "main");   //返回"!Warning! main: Be careful!"
    std::string stre = mlog::messageE("{}: Dangerous!", "main");   //返回"!!!Error!!! main: Dangerous!"
    
    ...
}
```   
### file.hpp:
```
...

int main()
{
    //创建.ini文件，写入内容并读取
    mfile::IniFile ini;
    ini.open("./cfg.ini");
    ini.setValue("key1", "99");
    ini.close();
    ini.open("./cfg.ini");
    std::string value1 = ini.getValue("key1");   //返回"99"
    ini.close();
    
    ...
}
```  
### path.hpp:
```
...

int main()
{
    //判断路径是否存在
    bool res1 = mpath::exists("cfg.ini");

    //判断路径是否是真实目录
    bool res2 = mpath::isDir("cfg.ini");

    //拼接路径
    std::string path1 = mpath::join("A", "B", "C");   //返回"A/B/C"

    //返回当前目录下所有文件目录名或路径
    std::vector<std::string> files1 = mpath::listDir("./", false);

    //返回当前目录及子目录下所有文件名或路径
    std::vector<std::string> files2 = mpath::walk("./", false);

    //创建、删除目录和文件
    mpath::makeDirs("A/B/C");
    mpath::remove("A");
    
    ...
}
```  
### math.hpp:
```
...

int main()
{
    //标准化索引值，Python风格转为C++风格
    int arr[5] = { 0,1,2,3,4 };
    int py_idx = -3;
    int cpp_idx = mmath::normIdx(py_idx, 5);   //返回 2

    //创建不同类型的矩形框并相互转换
    mmath::LTRB ltrb(0, 0, 100, 100);   //{left, top, right, bottom}
    mmath::XYWH xywh = ltrb.toXYWH();   //{center_x, center_y, width, height}
    mmath::LTWH ltwh = ltrb.toLTWH();   //{left, top, width, height}
    
    ...
}
```  
### io.hpp:
```
...

class MyClass {};

int main()
{
    //类似Python里的print函数，打印任意类型对象，不支持的类型将会打印<classname: address>
    int a = 10;
    float b = 10.5;
    std::string c = "ccc";
    int d[3] = { 0, 1, 2 };
    std::vector<int> vec = { 1,2,3 };
    auto cvmat = cv::Mat::zeros(3, 5, CV_8UC3);
    cv::Rect2d rect0(0, 0, 100, 200);
    MyClass mc;
    void* p = &mc;
    mio::print(a, b, c, d, vec, cvmat, rect0, mc, p);
    
    /*    打印内容如下：
    10 10.5 ccc {0 1 2} {1 2 3}
    cv::Mat{[(0 0 0) (0 0 0) (0 0 0) (0 0 0) (0 0 0)]
            [(0 0 0) (0 0 0) (0 0 0) (0 0 0) (0 0 0)]
            [(0 0 0) (0 0 0) (0 0 0) (0 0 0) (0 0 0)]}
     [100 x 200 from (0, 0)] <class MyClass: 0x0000008E111BF528> 0000008E111BF528
    */
    
    ...
}
```  
### cv.hpp:
```
...

int main()
{
    //快捷设置一个{720, 480}尺寸，左上角在{0, 0}处的窗口
    setWindowCV("win", {720, 480}, {0, 0}, cv::WINDOW_FREERATIO) 
    
    //快捷设置窗口并显示
    cv::Mat img = cv::imread("001.jpg");
    int k = mext::quickShowCV("win", img, 1, false, {720, 480}, {0, 0}, cv::WINDOW_FREERATIO);
    if (k == 27)
    {
        cv::destroyWindow("win");
        return -1;
    }
    
    //快捷绘制标签
    mext::putBoxCV(img, {10, 10, 60, 100}, "car");
    
    //输出cv::Mat 20-24列、50-51行、第0通道区域的值
    mext::printMat(img, {20, 25}, {50, 52}, {0, 1});
    
    ...
}
```  
### ncnn.hpp:
```
...

int main()
{
    //快捷加载ncnn模型
    ncnn::Net net;
    int ret = mext::loadNcnn(net, "net.param", "net.bin");
    if (ret != 0)
        return -1;
        
    //快捷运行ncnn网络推理
    cv::Mat img = cv::imread("001.jpg");
    ncnn::Mat inp = ncnn::Mat::from_pixels(img.data, ncnn::Mat::PIXEL_BGR2RGB, img.cols, img.rows);
    std::vector<ncnn::Mat> outputs = mext::quickRunNcnn(net, inp, "x", {"y1", "y2", "y3"});
    
    //输出ncnn::Mat 20-24列、50-51行、第0通道区域的值
    mext::printMat(inp, {20, 25}, {50, 52}, {0, 1});
    
    ...
}
```  

## 版本发布日志
**v1.1.0**  
* 20240410
1. time.hpp中更改sleep系列函数的输入参数类型为long long型；
2. time.hpp添加LocalTimeController类。
3. mio::print不再对bool类型值打印true和false。
4. 现在在Linux系统上可以通过`strings xxx | grep version`命令查找版本信息

**v1.0.3**  
* 20240112
1. 灵感再次迸发！现在io.hpp中print函数可以打印任意类型的对象了，未支持的类型会打印类名+地址；
2. 随着print函数功能的升级，删去了对OpenCV中cv::Mat和cv::MatExpr之外类型的拓展，这些类型无需拓展即可打印；
3. 修复在qnx上部分模块缺少系统头文件导入的问题。

**v1.0.2**  
+ 20240111
2. 将库中的Tab制表符全部替换为4个空格；
3. Readme文档更新测试平台。  

* 20240110
1. 优化cv.hpp和ncnn.hpp中printMat函数的打印效果。

**v1.0.1**  
* 20240108
1. 灵感迸发！现在io.hpp中print函数的功能完整性不再受模块的调用顺序的影响，也不受`__*utils__.h`的混用影响；
2. 修复了mfile::IniFile类的编译问题、空cfg文件的写入异常。  

**v1.0.0**   
* 20240105
1. 重新组织了mineutils库的结构，形成正式版本；
2. 删除了原本的ColorStr::red、ColorStr::blue等彩色字体字体接口，统一更新为mstr::color接口；
3. 简化message系列函数接口；
4. 其他优化内容。