#ifndef Profiler_h__
#define Profiler_h__

#include <string>
#include <unordered_map>
#include <chrono>
#include <vector>

using std::chrono::system_clock;

class Profiler
{
private:
	struct ProfierElement
	{
		std::string Description;
		system_clock::time_point StartTime;
		system_clock::time_point EndTime;
	};

public:
	Profiler(void);
	~Profiler(void);


	void StartTimer(const std::string& name, const std::string& desc = "");
	void EndTimer(const std::string& name);
	long long GetElapsedTime(const std::string& name);

protected:
	
	std::unordered_map<std::string, ProfierElement> mElements;

};


#endif // Profiler_h__
