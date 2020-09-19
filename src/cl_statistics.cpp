//-----------------------------------------------------------------------------
//
// Skulltag Source
// Copyright (C) 2003-2007 Brad Carney
// Copyright (C) 2007-2012 Skulltag Development Team
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
// 3. Neither the name of the Skulltag Development Team nor the names of its
//    contributors may be used to endorse or promote products derived from this
//    software without specific prior written permission.
// 4. Redistributions in any form must be accompanied by information on how to
//    obtain complete source code for the software and any accompanying
//    software that uses the software. The source code must either be included
//    in the distribution or be available for no more than the cost of
//    distribution plus a nominal fee, and must be freely redistributable
//    under reasonable conditions. For an executable file, complete source
//    code means the source code for all modules it contains. It does not
//    include source code for modules or files that typically accompany the
//    major components of the operating system on which the executable file
//    runs.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// Date created:  8/19/07
//
//
// Filename: cl_statistics.cpp
//
// Description: Keeps track of the amount of data sent to and from the server,
// and possibly other related things.
//
//-----------------------------------------------------------------------------

#include "cl_statistics.h"
#include "doomtype.h"
#include "stats.h"
#include "doomdef.h"
#include "doomstat.h"

//*****************************************************************************
//	CLASSES

// [BB]
class StatTracker
{
	unsigned int _value;
	unsigned int _valueThisTick;
	unsigned int _valueThisSecond;
	unsigned int _valueLastSecond;
	unsigned int _maxValuePerSecond;
public:
	StatTracker ( )
	{
		Clear ( );
	}

	void Clear ( )
	{
		_value = 0;
		_valueThisTick = 0;
		_valueThisSecond = 0;
		_valueLastSecond = 0;
		_maxValuePerSecond = 0;
	}

	void AddToTic ( const int Value )
	{
		_valueThisTick += Value;
	}

	void TicPassed ( )
	{
		_value += _valueThisTick;
		_valueThisSecond += _valueThisTick;
		_valueThisTick = 0;
	}

	void SecondPassed ( )
	{
		_valueLastSecond = _valueThisSecond;
		_valueThisSecond = 0;
		if ( _maxValuePerSecond < _valueLastSecond )
			_maxValuePerSecond = _valueLastSecond;
	}

	unsigned int getTotalValue ( ) const
	{
		return _value;
	}

	unsigned int getValueThisTick ( ) const
	{
		return _valueThisTick;
	}

	unsigned int getValueLastSecond ( ) const
	{
		return _valueLastSecond;
	}

	unsigned int getMaxValuePerSecond ( ) const
	{
		return _maxValuePerSecond;
	}
};

//*****************************************************************************
//	VARIABLES

static	StatTracker			g_bytesSentStatTracker;
static	StatTracker			g_bytesReceivedStatTracker;
static	StatTracker			g_missingPacketsRequestedStatTracker;

//*****************************************************************************
//	FUNCTIONS

void CLIENTSTATISTICS_Construct( void )
{
	g_bytesSentStatTracker.Clear();
	g_bytesReceivedStatTracker.Clear();
	g_missingPacketsRequestedStatTracker.Clear();
}

//*****************************************************************************
//
void CLIENTSTATISTICS_Tick( void )
{
	// Add to the number of bytes sent/received this second.
	g_bytesSentStatTracker.TicPassed();
	g_bytesReceivedStatTracker.TicPassed();
	g_missingPacketsRequestedStatTracker.TicPassed();

	// Every second, update the number of bytes sent last second with the number of bytes
	// sent this second, and then reset the number of bytes sent this second.
	if (( gametic % TICRATE ) == 0 )
	{
		g_bytesSentStatTracker.SecondPassed ( );
		g_bytesReceivedStatTracker.SecondPassed ( );
		g_missingPacketsRequestedStatTracker.SecondPassed ( );
	}
}

//*****************************************************************************
//
void CLIENTSTATISTICS_AddToBytesSent( ULONG ulBytes )
{
	g_bytesSentStatTracker.AddToTic ( ulBytes );
}

//*****************************************************************************
//
void CLIENTSTATISTICS_AddToBytesReceived( ULONG ulBytes )
{
	g_bytesReceivedStatTracker.AddToTic ( ulBytes );
}

//*****************************************************************************
//
void CLIENTSTATISTICS_AddToMissingPacketsRequested( unsigned int Num )
{
	g_missingPacketsRequestedStatTracker.AddToTic ( Num );
}

//*****************************************************************************
//	STATISTICS

ADD_STAT( nettraffic )
{
	FString	Out;

	Out.Format( "In: %5d/%5d/%5d        Out: %5d/%5d/%5d        Loss: %5d/%5d", 
		static_cast<int> (g_bytesReceivedStatTracker.getValueThisTick()),
		static_cast<int> (g_bytesReceivedStatTracker.getValueLastSecond()),
		static_cast<int> (g_bytesReceivedStatTracker.getMaxValuePerSecond()),
		static_cast<int> (g_bytesSentStatTracker.getValueThisTick()),
		static_cast<int> (g_bytesSentStatTracker.getValueLastSecond()),
		static_cast<int> (g_bytesSentStatTracker.getMaxValuePerSecond()),
		static_cast<int> (g_missingPacketsRequestedStatTracker.getValueLastSecond()),
		static_cast<int> (g_missingPacketsRequestedStatTracker.getMaxValuePerSecond()) );

	return ( Out );
}
