#ifndef Timer_h__
#define Timer_h__

#include "Prerequisite.h"

class Timer
{
public:
	Timer();

	float GetGameTime() const;		// in seconds
	float GetDeltaTime() const;		// in seconds

	void Reset();		// Call before game loop
	void Start();		// Call when unpaused
	void Stop();	    // Call when paused
	void Tick();		// Call every frame

private:
	double mSecondsPerCount;
	double mDeltaTime;

	int64_t mBaseTime;
	int64_t mPausedTime;
	int64_t mStopTime;
	int64_t mPrevTime;
	int64_t mCurrTime;

	bool mStopped;
};

#endif // Timer_h__
