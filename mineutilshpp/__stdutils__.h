#pragma once
#ifndef __STDUTILS__H
#define __STDUTILS__H

#include"modules/base.hpp"   //mineutils库实现基础及版本信息
#include"modules/str.hpp"   //based on base.hpp
#include"modules/time.hpp"   //based on base.hpp
#include"modules/type.hpp"   //based on base.hpp
#include"modules/log.hpp"   //based on str.hpp
#include"modules/file.hpp"   //based on str.hpp & log.hpp
#include"modules/path.hpp"   //based on str.hpp & log.hpp
#include"modules/math.hpp"   //based on str.hpp, log.hpp & type.hpp
#include"modules/io.hpp"   //based on base.hpp, type.hpp, log.hpp & math.hpp, and extended by cv.hpp & ncnn.hpp


#endif


/*  设计逻辑上的层级
    TOP                cv.hpp    ncnn.hpp
                            io.hpp
               file.hpp    math.hpp    path.hpp
                           log.hpp
               str.hpp     time.hpp    type.hpp
    BASE                   base.hpp
*/