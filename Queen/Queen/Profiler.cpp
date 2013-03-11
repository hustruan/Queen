#include "Profiler.h"
#include <cassert>

using std::chrono::duration_cast;
using std::chrono::milliseconds;

Profiler::Profiler(void)
{
}


Profiler::~Profiler(void)
{
}

void Profiler::StartTimer( const std::string& name, const std::string& desc /*= ""*/ )
{
	ProfierElement& profiler = mElements[name];
	profiler.Description = desc;
	profiler.StartTime = std::chrono::system_clock::now();
}

void Profiler::EndTimer(const std::string& name)
{
	auto profiler = mElements.find(name);
	assert(profiler != mElements.end());

	profiler->second.EndTime = std::chrono::system_clock::now();
}

long long Profiler::GetElapsedTime( const std::string& name )
{
	auto profiler = mElements.find(name);
	assert(profiler != mElements.end());

	return duration_cast<milliseconds>(profiler->second.EndTime - profiler->second.StartTime).count();
}
