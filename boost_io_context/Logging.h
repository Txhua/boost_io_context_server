//
// Use of this source code is governed by a BSD-style
// license that can be found in the License file.
//
// Author: Txhua
// 
// Copyright 2020, Txhua. All rights reserved.

#ifndef _IOEVENT_LOGGING_H
#define _IOEVENT_LOGGING_H

#include "Singleton.h"
#include <boost/filesystem.hpp>

namespace IOEvent
{

class Logging final :
	public Singleton<Logging>

{
public:
	Logging(const boost::filesystem::path &dir = boost::filesystem::path("./log"));
	~Logging();
public:
	void glogInitializer();
	void setLogDir(const boost::filesystem::path &dir);
private:
	boost::filesystem::path logDir_;
};

}




#endif // !_IOEVENT_LOGGING_H
