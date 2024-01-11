#pragma once
#ifndef __CVUTILS__H
#define __CVUTILS__H

#include"modules/base.hpp"   //mineutils库实现基础及版本信息
#include"modules/str.hpp"   //no base
#include"modules/time.hpp"   //no base
#include"modules/type.hpp"   //based on base.hpp
#include"modules/log.hpp"   //based on str.hpp
#include"modules/file.hpp"   //based on str.hpp & log.hpp
#include"modules/path.hpp"   //based on str.hpp & log.hpp
#include"modules/math.hpp"   //based on str.hpp, log.hpp & type.hpp
#include"modules/io.hpp"   //based on base.hpp, type.hpp & math.hpp, and extended by cv.hpp & ncnn.hpp
#include"modules/cv.hpp"   //based on str.hpp, log.hpp, math.hpp & io.hpp

#endif


/*  设计逻辑上的层级
    TOP                cv.hpp    ncnn.hpp
                            io.hpp
               file.hpp    math.hpp    path.hpp
                           log.hpp
               str.hpp     time.hpp    type.hpp
    BASE                   base.hpp
*/