/*
*******************************************************************************************************
Copyright (c) 2011, Christopher John Topping, Aarhus University
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided
that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list of conditions and the
following disclaimer.
Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
the following disclaimer in the documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************************************************
*/

#ifndef __ALMASSSETUP_H
#define __ALMASSSETUP_H

#define __UNIX
#define __64bit

//#define __GITVERSION

#ifdef __UNIX
typedef unsigned int uint32;
typedef unsigned long long uint64;
#else
// Do nothing for the moment - Windows way to do things is standard
typedef __int64 uint64;
typedef __int32 uint32;
#endif

#ifdef __64bit
typedef uint64 PointerInt;
#else
typedef uint32 PointerInt;
#endif

/**
\brief
A simple class defining an x,y coordinate set
*/
class APoint
{
public:
	int m_x;
	int m_y;

	APoint()
	{
		m_x = -1;
		m_y = -1;
	}
	APoint(int a_x, int a_y)
	{
		m_x = a_x;
		m_y = a_y;
	}
};



#endif