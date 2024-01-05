//不建议在一个项目中混用__stdutils__.h、__cvutils__.h和__mineutils__.h，混用可能会导致print函数不识别cv和ncnn中的类型。
#pragma once
#ifndef __CVUTILS__H
#define __CVUTILS__H

#include"modules/base.hpp"   //mineutils库实现基础
#include"modules/str.hpp"   //no base
#include"modules/time.hpp"   //no base
#include"modules/type.hpp"   //based on base.hpp
#include"modules/log.hpp"   //based on str.hpp
#include"modules/file.hpp"   //based on str.hpp & log.hpp
#include"modules/path.hpp"   //based on str.hpp & log.hpp
#include"modules/math.hpp"   //based on str.hpp, log.hpp & type.hpp
#include"modules/cv.hpp"   //based on str.hpp & math.hpp
#include"modules/io.hpp"   //based on base.hpp, type.hpp & math.hpp, and expanded by cv.hpp & ncnn.hpp

#endif


/*	设计逻辑上的层级
	TOP					   io.hpp
					  cv.hpp   (ncnn.hpp)
			   file.hpp    math.hpp    path.hpp
						   log.hpp
			   str.hpp     time.hpp    type.hpp
	BASE				   base.hpp
*/