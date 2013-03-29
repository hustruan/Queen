#include "Timer.h"

Timer::Timer()
	: mSecondsPerCount(0.0), mDeltaTime(-1.0), mBaseTime(0),
	mPausedTime(0), mPrevTime(0), mCurrTime(0), mStopped(false)
{
	int64_t countsPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
	mSecondsPerCount = 1.0 / (double)countsPerSec;
}


float Timer::GetGameTime() const
{
	if(mStopped)
	{
		return (float)((mStopTime - mBaseTime) * mSecondsPerCount);
	}
	else
	{
		return (float)
			((mCurrTime - mBaseTime - mStopTime) * mSecondsPerCount);
	}
}


float Timer::GetDeltaTime() const
{
	return (float)mDeltaTime;
}

void Timer::Tick()
{
	if(mStopped)
	{
		mDeltaTime = 0;
		return;
	}

	// Get Time this frame
	int64_t currentTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);
	mCurrTime = currentTime;

	// Times difference between this and previous frame
	mDeltaTime = (mCurrTime - mPrevTime) * mSecondsPerCount;

	// Prepare for next frame
	mPrevTime = mCurrTime;
	
	// Force nonnegative. The DXSDK's CDXUTTimer mentions that if the
	// processor goes into a power save mode or we get shuffled to
	// another processor, then mDeltaTime can be negative.
	if(mDeltaTime < 0.0)
	{
		mDeltaTime = 0.0;
	}
}

void Timer::Reset()
{
	int64_t currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

	mBaseTime = currTime;
	mPrevTime = currTime;
	mStopTime = 0;
	mStopped = false;
}

void Timer::Stop()
{
	if(!mStopped)
	{
		int64_t currTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

		mStopTime = currTime;
		mStopped = true;
	}
}

void Timer::Start()
{
	int64_t startTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&startTime);

	if(mStopped)
	{
		mPausedTime += (startTime - mStopTime);
		mPrevTime = startTime;
		mStopTime = 0;
		mStopped = false;
	}
}
