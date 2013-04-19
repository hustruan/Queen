#include "Profiler.h"
#include <assert.h>
#include <windows.h>

#pragma comment(lib, "glew32")

Profiler gProfiler;

Profiler::Profiler(void)
{

}


Profiler::~Profiler(void)
{
}

void Profiler::Init()
{
	mCurrFrameID = 0;

	// Init Cpu time
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);
	mFrequency = freq.QuadPart;

	LARGE_INTEGER now;
	QueryPerformanceCounter(&now);
	mInitTime = now.QuadPart;

}

uint64_t Profiler::GetTimeNs() const
{
	LARGE_INTEGER now;
	QueryPerformanceCounter(&now);
	static const uint64_t factor = 1000000000;
	return (uint64_t)( factor*(now.QuadPart-mInitTime) / mFrequency );
}


void Profiler::Shutdown()
{
	for(size_t i=0 ; i < NUM_GPU_MARKERS ; i++)
	{
		GpuMarker&	marker = mGpuThreadInfo.Markers[i];
		if(marker.StartQueryID != INVALID_QUERY)
		{
			glDeleteQueries(1, &marker.StartQueryID);
			marker.StartQueryID = INVALID_QUERY;
		}
		if(marker.EndQueryID != INVALID_QUERY)
		{
			glDeleteQueries(1, &marker.EndQueryID);
			marker.EndQueryID = INVALID_QUERY;
		}
	}
}


void Profiler::PushCpuMarker( const std::string& name )
{
	CpuThreadInfo& ti = GetOrAddCpuThreadInfo();

	CpuMarker& marker = ti.Markers[ti.CurrWriteIdx];
	//assert(marker.FrameID != mCurrFrameID && "looping: too many markers, no free slots available");

	marker.Start = GetTimeNs();
	marker.End = INVALID_TIME;
	//marker.layer = ti.nb_pushed_markers;
	marker.Name = name;
	//marker.FrameID = mCurrFrameID;

	IncrementCycle(&ti.CurrWriteIdx, NUM_CPU_MARKERS);
	ti.NumPushedMakers++;	
}

void Profiler::PopCpuMarker()
{
	CpuThreadInfo& ti = GetOrAddCpuThreadInfo();

	int32_t index = ti.CurrWriteIdx - 1;
	while(ti.Markers[index].End != INVALID_TIME)	// skip closed markers
		DecrementCycle(&index, NUM_CPU_MARKERS);

	CpuMarker& marker = ti.Markers[index];

	assert(marker.End == INVALID_TIME);
	marker.End = GetTimeNs();

	ti.NumPushedMakers--;
}

void Profiler::PushGpuMarker( const std::string& name )
{
	GpuMarker& marker = mGpuThreadInfo.Markers[mGpuThreadInfo.CurrWriteIdx];

	if (marker.StartQueryID == INVALID_QUERY)
		glGenQueries(1, &marker.StartQueryID);

	assert(marker.FrameID != mCurrFrameID && "looping: too many markers, no free slots available");

	// issue time query
	glQueryCounter(marker.StartQueryID, GL_TIMESTAMP);

	marker.Start = INVALID_TIME;
	marker.End = INVALID_TIME;
	marker.FrameID = mCurrFrameID;
	marker.Name = name;

	IncrementCycle(&mGpuThreadInfo.CurrWriteIdx, NUM_GPU_MARKERS);
	mGpuThreadInfo.NumPushedMakers++;	
}

void Profiler::PopGpuMarker()
{
	int32_t index = mGpuThreadInfo.CurrWriteIdx - 1;

	while(mGpuThreadInfo.Markers[index].End != INVALID_TIME)	// skip closed markers
		DecrementCycle(&index, NUM_GPU_MARKERS);

	GpuMarker& marker = mGpuThreadInfo.Markers[index];

	// Issue timer query
	if(marker.EndQueryID == INVALID_QUERY)
		glGenQueries(1, &marker.EndQueryID);
	glQueryCounter(marker.EndQueryID, GL_TIMESTAMP);

	mGpuThreadInfo.NumPushedMakers--;
}

void Profiler::SynchronizeFrame()
{
	mCurrFrameID++;

	mGpuThreadInfo.CurrReadIdx =  mGpuThreadInfo.NextReadIdx;
}

void Profiler::UpdateResults()
{
	int32_t displayedFrame = mCurrFrameID - int32_t(NUM_RECORDED_FRAMES-1);
	
	if(displayedFrame < 0)	// don't draw anything during the first frames
		return;


	// update Gpu makers
	{
		int32_t readIdx = mGpuThreadInfo.CurrReadIdx;

		while(mGpuThreadInfo.Markers[readIdx].FrameID == displayedFrame)
		{
			GpuMarker& marker = mGpuThreadInfo.Markers[readIdx];

			GLint available = GL_TRUE;   
			if(marker.StartQueryID == INVALID_QUERY || marker.EndQueryID == INVALID_QUERY)
				available = GL_FALSE;

			if(available)
			{
				GLint startAvailable = GL_FALSE ;
				GLint endAvailable = GL_FALSE ;
				glGetQueryObjectiv(marker.StartQueryID, GL_QUERY_RESULT_AVAILABLE, &startAvailable);
				glGetQueryObjectiv(marker.EndQueryID, GL_QUERY_RESULT_AVAILABLE, &endAvailable);
				available = (startAvailable & endAvailable);
			}
			
			if (available)
			{
				glGetQueryObjectui64v(marker.StartQueryID, GL_QUERY_RESULT, &marker.Start);
				glGetQueryObjectui64v(marker.EndQueryID, GL_QUERY_RESULT, &marker.End);
			}

			// increment
			IncrementCycle(&readIdx, NUM_GPU_MARKERS);
		}

		mGpuThreadInfo.NextReadIdx = readIdx;
	}

}

uint64_t Profiler::GetGpuElapsedTime( const std::string& name )
{
	int32_t displayedFrame = mCurrFrameID - int32_t(NUM_RECORDED_FRAMES-1);

	if (displayedFrame < 0)
		return INVALID_TIME;

	int32_t readIdx = mGpuThreadInfo.CurrReadIdx;
	while(mGpuThreadInfo.Markers[readIdx].FrameID == displayedFrame)
	{
		GpuMarker& marker = mGpuThreadInfo.Markers[readIdx];

		if (marker.Name == name)
		{
			if(marker.StartQueryID == INVALID_QUERY || marker.EndQueryID == INVALID_QUERY)
				return INVALID_TIME;

			GLint available = GL_TRUE;   
			if(available)
			{
				glGetQueryObjectiv(marker.EndQueryID, GL_QUERY_RESULT_AVAILABLE, &available);
			}

			if (available)
			{
				glGetQueryObjectui64v(marker.StartQueryID, GL_QUERY_RESULT, &marker.Start);
				glGetQueryObjectui64v(marker.EndQueryID, GL_QUERY_RESULT, &marker.End);
				return marker.End - marker.Start;
			}

			break;
		}

		// increment
		IncrementCycle(&readIdx, NUM_GPU_MARKERS);
	}


	return INVALID_TIME;
}

uint64_t Profiler::GetCpuElapsedTime( const std::string& name )
{
	return INVALID_TIME;
}

void Profiler::IncrementCycle( int32_t* pVal, int32_t circleSize )
{
	int32_t val = *pVal;
	val++;
	if(val >= circleSize)
		val = 0;
	*pVal = val;
}

void Profiler::DecrementCycle( int32_t* pVal, int32_t circleSize )
{
	int32_t val = *pVal;
	val--;
	if(val < 0)
		val = circleSize-1;
	*pVal = val;
}

Profiler::CpuThreadInfo& Profiler::GetOrAddCpuThreadInfo()
{
	std::thread::id this_id = std::this_thread::get_id();

	for(size_t i = 0; i< MAX_CPU_THREADS; ++i)
	{
		if(mCpuThreadInfosBook[i] && mCpuThreadInfos[i].ThreadId == this_id)
		{
			return mCpuThreadInfos[i];
		}
	}

	size_t freeSlot = 0;
	mCpuMutex.lock();
	while (mCpuThreadInfosBook[freeSlot]) freeSlot++;
	mCpuThreadInfosBook[freeSlot] = true;
	mCpuMutex.lock();

	mCpuThreadInfos[freeSlot].Init(this_id);

	return mCpuThreadInfos[freeSlot];
}



