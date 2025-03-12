# mineutilscpp
## 描述
C++的便利功能封装，专注于封装逻辑复杂或代码冗长的功能，主要用于方便自己工作、精进技术和提升代码文档规范性。库采用纯头文件实现，文本使用UTF8-SIG编码，代码使用C++11标准。 
## 版本信息
当前库版本：2.2.1   
文档注释修改日期：20250312  
## 测试平台
**Windows:**  
VS2019  
**Linux:**  
x86_64-linux-gnu-g++ 9.4.0   
arm-linux-gnueabihf-g++ 8.3.0    
**QNX660:**    
arm-unknown-nto-qnx6.6.0eabi-g++ 4.7.3  
**QNX710:**  
aarch64-barebone1.1.0-g++ 8.3.0 
## 设计思路   
### 设计原则
* **类型检查**：所有模板类功能都进行严格的类型检查，保证非预期类型导致的编译错误出现在调用的入口处
+ **轻量实用**：接口的设计尽量简洁，避免功能重叠，避免过度封装，专注封装逻辑复杂或代码冗长的功能  
- **线程安全**：所有函数接口都保证线程安全；类的不同对象之间保证线程安全；但类的同一个对象的各个成员函数之间,非明确说明不保证线程安全  
* **性能优化**：在不提高接口复杂度的情况下，尽可能提升性能降低开销   
### 命名规则
* 类型统一大驼峰命名
+ 函数统一小驼峰命名
- 对象统一小写+下划线命名
* 枚举类成员统一小写+下划线命名
+ 对象式宏定义统一大写+下划线命名，以`MINE_`作为前缀
- 函数式宏统一小驼峰命名，以`m`作为前缀
### 更新规则
更新遵循大版本号删改接口，中版本号添加新功能接口，小版本号修复和优化的原则；在同一大版本内废弃接口仅标记，不删除。  
### 兼容性声明
大版本内保证接口的代码级兼容，但以下情况除外：
* **未明确指定签名的函数作为参数**：例如对于库内函数`func`，使用`std::thread(func, args...)`或`mtype::FuncChecker<decltype(func)>`的方式传递函数，这类情况由于函数签名未明确指出，可能会因为更新导致编译器无法确认指向哪个重载
+ **使用非安全的手段访问资源**：例如使用指针偏移等方式访问类和结构体的成员，不保证更新后成员的签名、类型和内存偏移不发生变动
- **依赖了类和结构体的具体size**：类和结构体的size在更新的过程中可能改变，如果代码依赖其当前具体size，可能会在更新后出现非预期的结果
* **库和可执行文件中混用不同版本mineutils且未隐藏符号**：如果库和可执行文件混用不同版本mineutils，且编译库时未隐藏使用的mineutils库内符号，就有可能因为类型的内存布局变化导致二进制不兼容

## 模块介绍
### 基本信息
| 文件夹 | 模块 | 功能 |
|:--------:|:--------:|:-------:|   
-- | **\_\_stdutils\_\_.h** | 基于C/C++标准库实现的mineutils库核心功能
core | **base.hpp** | mineutils库的版本信息及基础宏定义。包含于mineutils::mbase  
core | **time.hpp** | 时间相关的便捷操作，如计时、休眠等。包含于mineutils::mtime    
core | **type.hpp** | 类型相关操作，提供可用于模板推导的类型检查功能。包含于mineutils::mtype
core | **math.hpp** | 数学相关的操作，目前包含矩形框操作等。包含于mineutils::mmath    
core | **path.hpp** | 路径相关操作，如exists、listDir、join、makeDirs等便捷功能。包含于mineutils::mpath    
core | **str.hpp** | std::string字符串的便捷操作，如转换为字符串、分割字符串等。包含于mineutils::mstr     
core | **thread.hpp** | 线程相关操作，包括线程池、自旋锁、读写锁等。包含于mineutils::mthrd
core | **file.hpp** | 文件操作，目前包含ini文件的读写等。包含于mineutils::mfile     
core | **io.hpp** |  输入输出相关功能，目前包含print函数和main函数参数解析工具等。包含于mineutils::mio    
extra | **cv.hpp** |  OpenCV3相关便捷功能，如快捷显示、快捷绘制矩形框、打印cv::Mat数据等。包含于mineutils::mext    
extra | **ncnn.hpp** |  NCNN相关便捷功能，如快捷运行模型、打印ncnn::Mat数据等。包含于mineutils::mext    
### 使用方法
* 可以单独导入./mineutilshpp下的模块，如：  
```
#include"mineutilshpp/core/time.hpp"   //导入时间相关功能 
#include"mineutilshpp/core/io.hpp"   //导入输入打印相关功能 
#include"mineutilshpp/extra/cv.hpp"   //导入OpenCV相关便利功能 
```   
+ 也可一键导入`mineutilshpp/__stdutils__.h`，如：
```
#include"mineutilshpp/__stdutils__.h"   //一键导入基于标准库的核心功能封装  
```   
- 然后通过`using namespace mineutils`使用命名空间mineutils。由于所有功能都分布在次级的命名空间下，因此不用担心污染全局命名空间  
* 最后根据模块使用其中的功能，如`mstr::toStr(123)` 
+ 版本号写在`base.hpp`里，在Linux上可以通过`strings xxx | grep version`命令查找应用使用的mineutils库版本  
### 注意事项
* 以下划线开头的函数和类不应外部使用，这些仅用于内部功能实现，随时可能删改 
+ `mineutilshpp/extra`里的功能都是基于第三方库的封装，带有一定妥协性质，不会像`mineutilshpp/core`中的代码设计严格
- QNX660的g++ 4.7.3对C++11标准支持不完善，编译时需要额外指定-D_GLIBCXX_USE_NANOSLEEP，且在该编译器上的类型检查可能导致编译错误出现在检查条件内部
* 使用QNX710上的g++ 8.3.0编译时需要指定：-std=gnu++11 
+ 可以无视unused类型警告
- 避免在全局区对象的析构函数中使用库内接口，可能导致main函数退出后的对象析构异常。

## 模块功能示例  

### base.hpp: 
```
...

int main()
{
    const char* version = mbase::getVersion();    //获取mineutils库版本
    
    mprintfE("do something failed!\n");    //打印错误信息
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
    long long cost_time = end_t.since<mtime::ms>(start_t);

    //打印当前时间
    std::cout << mtine::now().localTime() << std::endl;
    
    //统计并打印代码段平均耗时
    mtime::MeanTimeCounter time_counter{10, __func__, mtime::ms};
    while(true)
    {
        time_counter.markStart("part1");
        ...  // do something1
        time_counter.markEnd("part1");
        
        {
            auto guard = time_counter.markGuard("part2");
            ...  // do something2
        }
        
        time_counter.printOnTargetCount<mtime::ms>();   //每10次循环计算并打印一次平均耗时
    }
    
    //快速统计并打印代码段耗时
    {
        TimeCounterGuard<mtime::ms> guard("part3");
        ...  // do something3
    }   //离开作用域时统计并打印耗时
    
    ...
}
```   
### type.hpp:
```
...

template<class T, typename std::enable_if<mtype::InTypesChecker<T, int, char, long long>::value, int>::type = 0>
void func1(T d)   //限制T必须为int、char、long long中的一个类型
{
    do something...
}

template<class T, typename std::enable_if<!mtype::InTypesChecker<T, int, char, long long>::value, int>::type = 0>
void func1(T d)
{
    do something...
}

int main()
{
    //根据参数类型选择合适的重载
    int x = 1;
    double y = 1.5;
    func1(x);   //进入第一个重载
    func1(y);   //进入第二个重载
    
    ...
}
```   
### str.hpp:
```
...

int main()
{
    //参数填入字符串
    std::string s3 = mstr::fmtStr("{} has {} billion people.", "China", "1.4");   //返回"China has 1.4 billion people."
    
    //字符串分割
    std::string s4 = " hello world! ";
    std::vector res = mstr::split(s4);   //返回vector{"hello", "world!"};
    
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

    //创建不同类型的矩形框并相互转换
    mmath::RectLTRB<int> ltrb(0, 0, 100, 100);   //{left, top, right, bottom}
    mmath::RectXYWH<int> xywh = ltrb.toXYWH();   //{center_x, center_y, width, height}
    
    ...
}
```  
### thread.hpp:
```
...

struct Args
{
    const char* name = "Args";
};

int testFunc1(Args& args)
{
    printf("%s\n", __func__);
    args.name = __func__;
    mtime::sleep(1);
    return 0;
}

class TestFunc2
{
public:
    void testFunc2(void* args)
    {
        printf("%s\n", __func__);
        ((Args*)args)->name = __func__;
        mtime::sleep(1);
    }
};

int main()
{
    mthrd::ThreadPool thd_pool(2, 100);
    
    while(true)
    {
        Args args1;
        Args args2;
        TestFunc2 test_func2;

        auto task_future1 = thd_pool.addTask(testFunc1, std::ref(args1));
        auto task_future2 = thd_pool.addTask(&TestFunc2::testFunc2, &test_func2, &args2);

        task_future1.wait();
        task_future2.wait();
        
        ...
    } 
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
    cv::Rect2d rect0(0, 0, 100, 200);
    MyClass mc;
    void* p = &mc;
    mio::print(a, b, c, d, vec, rect0, mc, p);
    
    /*    打印内容如下：
    10 10.5 ccc {0 1 2} {1 2 3} [100 x 200 from (0, 0)] <class MyClass: 0x0000008E111BF528> 0000008E111BF528
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
    cvSetWindow("win", {720, 480}, {0, 0}, cv::WINDOW_FREERATIO) 
    
    //快捷设置窗口并显示
    cv::Mat img = cv::imread("001.jpg");
    int k = mext::cvQuickShow("win", img, 1, false, {720, 480}, {0, 0}, cv::WINDOW_FREERATIO);
    if (k == 27)
    {
        cv::destroyWindow("win");
        return -1;
    }
    
    //快捷绘制标签
    mext::cvPutBox(img, {10, 10, 60, 100}, "car");
    
    //输出cv::Mat 20-24列、50-51行、第0通道区域的值
    mext::cvPrintMat(img, {20, 25}, {50, 52}, {0, 1});
    
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
    int ret = mext::ncnnLoad(net, "net.param", "net.bin");
    if (ret != 0)
        return -1;
        
    //快捷运行ncnn网络推理
    cv::Mat img = cv::imread("001.jpg");
    ncnn::Mat inp = ncnn::Mat::from_pixels(img.data, ncnn::Mat::PIXEL_BGR2RGB, img.cols, img.rows);
    std::vector<ncnn::Mat> outputs = mext::ncnnQuickRun(net, inp, "x", {"y1", "y2", "y3"});
    
    //输出ncnn::Mat 20-24列、50-51行、第0通道区域的值
    mext::ncnnPrintMat(inp, {20, 25}, {50, 52}, {0, 1});
    
    ...
}
```  

## 版本更新日志
**v2.2.1**  
* 20250312  
1. 修复mstr的ordinalize函数部分情况编译错误的问题。

**v2.2.0**  
* 20250311  
1. mstr下添加lpad、rpad函数；
2. mstr下的toStr和format函数现在可以接收任意类型参数，并扩展了对STL容器的支持；
3. mmath的Rect系列类添加valid、clipTo和area方法；
4. 修改mio::print的实现方式并优化对C字符串指针的处理，优化mio::ArgumentParser类的打印格式。

**v2.1.2**  
* 20250304  
1. 修复mthread::Threadpool一个可能导致程序退出时异常的打印，并在README文档中添加新的使用注意。

**v2.1.1**  
* 20250301  
1. mstr::ordinalize修复对部分数字转化格式不正确的问题。

**v2.1.0**  
* 20250221  
1. mfile::IniFile添加printContents方法；
2. 废弃mpath::remove函数，因为目前不打算定义其对于通配符和符号链接的行为；
4. 修复mio::ArgumentParser打印时的flag截断问题，并微调打印格式；
3. 修复mpath::splitName无法分割后缀名的问题。

**v2.0.1**  
* 20250211  
1. 修复某些static和thread_local局部变量，可能在main函数退出后生命周期提前结束导致的问题;
2. mtime::TimePoint类的部分方法现在正确支持const对象了。

**v2.0.0**  
* 20250206  
1. 在新的一年重新审视库内接口，删除冗余的接口和模块，保持库的轻量性；
2. 重新调整目录结构，将基于标准库实现的功能放到`core`下，将基于第三方库实现的功能放到`extra`下；
3. 重新设计和命名部分接口，在不影响便利性的前提下进一步优化性能；
4. 本次更新为非兼容更新。

**v1.16.1**  
* 20241231  
1. 修复MINE_FUNCNAME宏在VS2019上无法正确解析的问题，并极大降低MINE_FUNCNAME开销，因此mprintfN和mdprintfN宏得到相应优化。  

**v1.16.0**  
* 20241225  
1. mthrd下添加SpinLock自旋锁和ReadWriteMutex读写锁；
2. 为了与惯例保持一致，将使用RAII方式管理资源的接口统一在命名中加入Guard，为此mtime下的MeanTimeCounter::addLocal更名为MeanTimeCounter::addGuard，LocalTimeCounter更名为TimeCounterGuard，LocalTimeController更名为TimeControllerGuard，并将原命名标记为废弃。

**v1.15.0**  
* 20241219  
1. mstr下添加trim、ltrim、rtrim函数；
2. 修复被标记为废弃的printfN系列宏编译错误的问题。

**v1.14.0**  
* 20241210  
1. 添加新的mstr::toStr和mstr::fstr重载，用于为浮点数设置精度；
2. 添加一个新的mstr::zfillFlt重载，并将旧的标记为废弃；
3. 显著优化mstr下接口的整体性能，并修复mstr::toStr和基于它的接口可能以非预期的方式表示数字的问题；
4. 修复mtype::StdBindChecker对C数组参数的检查结果；
5. 优化mthrd::ThreadPool在析构时的逻辑。

**v1.13.0**  
* 20241126  
1. mtime::MeanTimeCounter现在包含mtime::MultiMeanTimeCounter的完整功能，同时将mtime::MultiMeanTimeCounter标记为废弃；
2. mthrd::ThreadPool修复在析构时可能触发异常的bug；
3. 修复mstr::fstr无法只接受1个参数的bug；
4. 尽可能将所有标记为废弃的接口都添加静态警告或运行时警告。

**v1.12.0**  
* 20241125  
1. mpath下添加isFileMatchExts函数，并将isImage和isVideo函数标记为废弃；
2. mthrd::ThreadPauser添加一个setPausePoint的新重载，并优化逻辑和性能；  
3. mthrd::ThreadPool简化实现，优化性能，添加一个新的构造函数，并将带wakeup_period_ms参数的构造函数标记为废弃；
4. mio::ArgumentParser少量优化printPresetOptions和printParsedOptions打印的格式； 
5. mtime::TimePoint将对high_resolution_clock的依赖替换为对steady_clock的依赖；   
6. 宏定义MINE_THREAD_LOCAL更名为MINE_THREAD_LOCAL_IF_HAVE，原命名废弃。  

**v1.11.2**  
* 20241112  
1. mthrd::ThreadPauser少量性能优化；
2. mthrd::TaskRetState改用std::shared_future来实现，以便多次get；  
3. mio::print消除打印函数时的警告；  
4. mio::ArgumentParser少量逻辑优化。   

**v1.11.1**  
* 20241105  
1. mtype::StdBindChecker可以更准确地处理具有多个operator()重载的仿函数以及std::ref包装的函数和仿函数了，因此mthrd::ThreadPool::addTask得到同样的优化；  
2. mtype::StdBindChecker明确限制仿函数和成员函数均满足对象和函数的cv限定符匹配，为此mthrd::ThreadPool::addTask得到同样的限制；  
3. QNX的gcc4.7.3对部分SFINAE特性支持不全，如`std::declval<ObjT>()->*std::declval<Func>()`判断失败会直接报错等等，因此修改了部分mtype下类型检查器的实现；  
3. mtype::FuncChecker处理有多个operator()重载的仿函数时，mtype::FuncChecker<Fn>::value为false；   
5. mio::print支持打印函数和成员函数。   

**v1.11.0**  
* 20241101
1. 添加mthead::ThreadPauser，用于代替循环sleep来暂停线程；  
2. mtime::MultiMeanTimeCounter添加addLocal成员函数，使用RAII原则实现addStart和addEnd的功能；  
3. mtime内接口统一时间数字用long long类型；  
4. mfile::IniFile::close优化，未打开文件也可以close。  

**v1.10.1**  
* 20241030
1. mtype::FuncChecker修复对不可拷贝和移动的函数对象编译错误的问题；  
2. mtype::StdBindChecker修复存在的bug并添加对函数对象作为Fn的类型要求；  
3. mthead::ThreadPool::addTask相应添加对函数对象作为Fn的类型要求。  

**v1.10.0**  
* 20241028
1. mbase添加mlikely和munlikely宏，用于在GCC中使用分支预测优化； 
2. mtype添加RvalueRefMaker、EachLvalueConstructibleChecker等结构体，用于模板检查和类型处理；  
3. mtype::StdBindChecker内部添加更严格的类型检查；mthrd::addTask添加更严格的参数类型检查；  
4. MINE_DEPRECATED宏重命名为mdeprecated，保持宏命名风格一致性；mtypename宏废弃，使用mtype::getTypeName函数替换；  
5. mio::ArgumentParser类稍微修改了printPresetOptions和printParsedOptions成员函数打印的格式；  
6. mmath::BaseBox及其子类改为struct，以符合其成员变量公开的设定；  
7. mmath::BaseBox子类的toPixel函数的限制条件更改为仅限整数类型，mmath::及其子类的构造函数添加更优的类型限制；   
8. 修复在1.9.0版本中mext::printMat的兼容性被破坏的问题，现在mext::printMat有了更准确的类型限制了。  

**v1.9.0**  
* 20241012
1. mio添加ArgumentParser类，用于解析main函数参数； 
2. mio::print添加对std::initializer_list的支持，并修复部分函数确实inline的问题；     
3. mpath模块性能优化；  
4. 部分代码风格统一。 

**v1.8.0**  
* 20240926   
1. 修复mpath::splitName无法分离后缀名的bug；    
2. 添加mpath::makeFile函数用于创建文件；  
3. 优化mpath整体逻辑以减少性能开销；  
4. 修复mfile::IniFile在保存新加入的key-value对时，写入顺序与加入顺序不一致的问题；    
5. mfile::setValue模板中添加对value的类型限制；  
6. mtype添加ConstructibleFromEachChecker、FuncChecker和StdBindChecker等结构体，用于模板检查；   
7. mtype废弃函数isSameType、isInTypes，添加结构体SameTypesChecker、InTypesChecker；  
8. mbase添加mtypename宏，用于获取类型名称字符串。  

**v1.7.2**  
* 20240906   
1. 修复std的stream系列的clear()未清空缓存导致的mstr内部分函数的异常。   

**v1.7.1**  
* 20240819   
1. 修复mthrd::ThreadPool的wakeup_period_ms参数未生效的bug；  
2. 对部分接口的参数取值范围进行限制； 
3. 修改mtime::sleep系列函数的输入类型为unsigned long。      

**v1.7.0**  
* 20240815   
1. 统一函数式宏定义使用m开头，对象式宏定义使用MINE_开头，如printfE现在为mprintfE；  
2. mthrd::TaskRetState添加valid()接口；   
3. 优化mmath::BaseBox及其派生类的模板类型限制方式，标记mmath::BaseBox<T>::belongToIntSeries为废弃。  

**v1.6.0**  
* 20240729   
1. 添加mthrd::TaskRetState<T>类，上个版本的mthrd::TaskState现在等于mthrd::TaskRetState<void>；
2. mthrd::ThreadPool::addTask现在返回mthrd::TaskRetState<T>，支持获取任务的返回值；
3. mmath::align修复对非2的幂计算错误的问题；
4. 添加mtype::StdBeginEndChecker<T>结构体，用于检查类型T是否具有类似于STL容器的begin()和end()接口；
5. mio::print现在支持所有C++11中的正式STL容器的打印了，并修复了打印不支持类型时的编译错误；
6. 修复QNX的GCC4.7.3编译器的无法编译通过的问题。

**v1.5.1**  
* 20240704  
1. 修改mstr::toOrdinal的实现方式以适配QNX项目；  
2. mfile::IniFile操作增加内部open状态判断。  

**v1.5.0**  
* 20240701  
1. 添加thread.hpp模块和线程池类型mthrd::ThreadPool类；  
2. 优化mio::print的实现方式，并修复缺少无参构造函数的类型无法正确打印的问题；  
3. 标记mbase中的BOOL_CASE_TAGS及相关类型为废弃，建议使用std::integral_constant<bool, value>代替。  

**v1.4.2**  
* 20240619  
1. 将带参数的mtype::isSameType函数标记为废弃；
2. mtime::LocalTimeCounter类的打印格式稍作调整。

**v1.4.1**  
* 20240604  
1. mpath下的函数逻辑优化；
2. 修复MINE_FUNCNAME宏在少数情况下为空的bug。

**v1.4.0**  
* 20240530
1. mstr添加toOrdinal函数；
2. mmath添加align函数；
3. mmath::BaseBox类的构造函数添加静态断言，要求模板类型必须为整数或浮点数类型；
4. log.hpp添加MINE_FUNCNAME宏，用于获取namespace::class::function形式的函数名，并相应更改printfN和msgN宏；
5. mbase添加printVersion函数。

**v1.3.0**  
* 20240513  
1. log.hpp中简化msgN、printfN、dprintfN的打印信息；
2. mstr::split函数性能优化，且可以设定最大分割次数，并添加mstr::rsplit函数；
3. mfile::IniFile使用list+map重写，优化性能；
4. mpath::isImage添加img_exts形参，mpath::isVideo添加video_exts形参；
5. 将用户接口的声明放到文件的顶部，将内部实现放在底部。

**v1.2.1**  
* 20240423  
1. 修复log.hpp中printfN、printfW、printfE等宏的打印错误。

**v1.2.0**  
* 20240422
1. base.hpp添加MINE_FUNCSIG宏用于获得函数信息，添加MINE_DEPRECATED宏用于标记废弃；
2. log.hpp添加msgW、msgE、printfW、printfE等宏用于生成警告和错误信息；
3. mlog::messageW、mlog::messageE、mlog::messageN被标记为废弃，将在2.x.y版本中删除；
4. 移动和添加dprntf、dprintfW、dprintfE等宏到log.hpp下，用于调试时使用，由编译器NDEBUG宏控制是否生效；
5. 优化mstr::toStr性能；
6. mstr::fstr函数不再打印警告信息。

**v1.1.0**  
* 20240410
1. time.hpp中更改sleep系列函数的输入参数类型为long long型；
2. time.hpp添加LocalTimeController类；
3. mio::print不再对bool类型值打印true和false；
4. 现在在Linux系统上可以通过`strings xxx | grep version`命令查找版本信息。

**v1.0.3**  
* 20240112
1. 灵感再次迸发！现在io.hpp中print函数可以打印任意类型的对象了，未支持的类型会打印类名+地址；
2. 随着print函数功能的升级，删去了对OpenCV中cv::Mat和cv::MatExpr之外类型的拓展，这些类型无需拓展即可打印；
3. 修复在qnx上部分模块缺少系统头文件导入的问题。

**v1.0.2**  
+ 20240111
1. 将库中的Tab制表符全部替换为4个空格；
2. Readme文档更新测试平台。  

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