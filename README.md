# mineutilscpp
一些C++的便利功能封装，用于方便自己编程。文本使用UTF8-SIG编码。   
所有功能都放在命名空间mineutils下，同时根据所属模块分布在次级的命名空间，如mineutils::mstr、mineutils::mtime下；基于第三方库的功能统一在次级的命名空间mineutils::mext下。

## 版本信息
当前库版本：1.0.0.0-20210105

## 使用方法
* 可以单独导入./mineutils/modules下的模块，如：  
```
#include"mineutilshpp/modules/time.hpp"  //导入时间相关功能 
```   
+ 也可一键导入`mineutilshpp/__*utils__.h`，如：
```
#include"mineutilshpp/__stdutils__.h"  //一键导入基于标准库的功能封装  
//#include"mineutilshpp/__cvutils__.h"  //一键导入基于标准库+OpenCV库的功能封装 
//#include"mineutilshpp/__mineutils__.h"  //一键导入所有的功能封装，当前包含OpenCV和NCNN的功能，需要自行添加这两个库
```   
- 然后通过`using namespace mineutils`使用命名空间mineutils。由于所有功能都分布在次级的命名空间下，因此不用担心污染全局命名空间。  
* 最后根据模块使用其中的功能，如`mstr::toStr(123)`。 

**注1**：单独导入模块时，建议最后导入io.hpp。因为io.hpp里的print函数，有部分实现依赖cv.hpp和ncnn.hpp的宏定义，提前导入io.hpp可能会导致部分功能未编译。  
**注2**：一键导入所有模块时，不建议混用`__stdutils__.h`、`__cvutils__.h`和`__mineutils__.h`，原因同上。

## 模块介绍
| 模块 | 功能|
|:--------:|:-------:|   
**base.hpp** | mineutils库的版本信息及完整实现需要的基本工具，目前内容很少。  
**str.hpp**| std::string字符串的便捷操作，如字符串填充内容、彩色字符串等。  
**time.hpp**| 时间相关的便捷操作，如计时、休眠等。  
**type.hpp**| 类型相关操作，如判断类型异同等。  
**log.hpp**| 程序的运行时形成相关信息的功能，目前包含生成统一格式的运行时message等。  
**file.hpp**| 文件操作，目前包含ini文件的读写等。  
**path.hpp**| 路径相关操作，如判断路径是否存在、listDir、join、makeDirs等便捷功能。  
**math.hpp**| 数学相关的操作，目前包含索引标准化、矩形框操作等。  
**io.hpp**|  输入输出相关功能，目前包含print函数，可以自由打印STL范围内基本数据类型和容器数据类型等。  
**cv.hpp**|  OpenCV相关便捷功能，如快捷显示、快捷绘制矩形框、打印cv::Mat数据等。  
**ncnn.hpp**|  NCNN相关便捷功能，如快捷运行网络、打印ncnn::Mat数据等。  