#pragma once
#ifndef __CVUTILS__H
#define __CVUTILS__H

#include"__stdutils__.h"   //基于标准库的工具
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