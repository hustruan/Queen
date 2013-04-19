#ifndef GPUTimer_h__
#define GPUTimer_h__

#include <string>
#include <cstdint>
#include <array>
#include <thread>
#include <mutex>
#include <GL/glew.h>

#define ENABLE_PROFILER


#define INVALID_TIME	((uint64_t)(-1))
#define INVALID_QUERY	((GLuint)0)

class Profiler;
extern Profiler gProfiler;

/**
 * A Cpu and Gpu profiler
 */
class Profiler
{
protected:

	static const size_t NUM_RECORDED_FRAMES  = 4;
	
	static const size_t MAX_GPU_MARKERS_PER_FRAME = 10;
	static const size_t	NUM_GPU_MARKERS = MAX_GPU_MARKERS_PER_FRAME * NUM_RECORDED_FRAMES;

	static const size_t	MAX_CPU_MARKERS_PER_THREAD = 20;
	static const size_t	NUM_CPU_MARKERS = MAX_CPU_MARKERS_PER_THREAD;  // Cpu only record one frame

	static const size_t	MAX_CPU_THREADS = 32;

	struct Marker 
	{
		uint64_t Start;
		uint64_t End;
		uint32_t FrameID; 
		std::string Name;

		Marker() : Start(INVALID_TIME), End(INVALID_TIME) {}
	};

	typedef Marker CpuMarker;

	struct GpuMarker : public Marker
	{
		GLuint	StartQueryID;
		GLuint	EndQueryID;
	
		GpuMarker() : Marker(), StartQueryID(INVALID_QUERY), EndQueryID(INVALID_QUERY) {}
	};
	
	struct CpuThreadInfo
	{
		std::thread::id ThreadId;
		int32_t CurrWriteIdx;
		int32_t CurrReadIdx;
		int32_t NextReadIdx;
		int32_t NumPushedMakers;
		CpuMarker Markers[MAX_CPU_MARKERS_PER_THREAD];

		void Init(const std::thread::id& id)
		{
			CurrWriteIdx = CurrReadIdx = NumPushedMakers = 0;
			ThreadId = id;
		}
	};

	struct GpuThreadInfo
	{
		int32_t CurrWriteIdx;
		int32_t CurrReadIdx;
		int32_t NextReadIdx;
		int32_t NumPushedMakers;
		GpuMarker Markers[NUM_GPU_MARKERS];

		void Init() 
		{
			CurrWriteIdx = CurrReadIdx = NumPushedMakers = 0;
		}
	};

public:
	Profiler(void);
	~Profiler(void);

	void Init();
	void Shutdown();
	void SynchronizeFrame();
	void UpdateResults();

	void PushCpuMarker(const std::string& name);
	void PopCpuMarker();

	void PushGpuMarker(const std::string& name);
	void PopGpuMarker();
	
	uint64_t GetGpuElapsedTime(const std::string& name);
	uint64_t GetCpuElapsedTime(const std::string& name);

private:
	void IncrementCycle(int32_t* val, int32_t circleSize);
	void DecrementCycle(int32_t* val, int32_t circleSize);
	CpuThreadInfo& GetOrAddCpuThreadInfo();

	uint64_t GetTimeNs() const;

private:

	int32_t mCurrFrameID;

	GpuThreadInfo mGpuThreadInfo;
	std::array<CpuThreadInfo, MAX_CPU_THREADS> mCpuThreadInfos;
	std::array<bool, MAX_CPU_THREADS> mCpuThreadInfosBook;
	std::mutex mCpuMutex;

	int64_t mFrequency;
	int64_t mInitTime;
};


// Some useful macro
#ifndef ENABLE_PROFILER

#define PROFILER_INIT()									
#define PROFILER_SHUT()									

//#define PROFILER_PUSH_CPU_MARKER(name, color)			
//#define PROFILER_POP_CPU_MARKER()						
#define PROFILER_PUSH_GPU_MARKER(name)			        
#define PROFILER_POP_GPU_MARKER()						

#define PROFILER_SYNC_FRAME()							
#define PROFILER_UPDATE_RESULTS()						

#else

#define PROFILER_INIT()									gProfiler.Init()
#define PROFILER_SHUT()									gProfiler.Shutdown()

#define PROFILER_PUSH_CPU_MARKER(name)			        gProfiler.PushCpuMarker(name)
#define PROFILER_POP_CPU_MARKER()						gProfiler.PopCpuMarker()
#define PROFILER_PUSH_GPU_MARKER(name)			        gProfiler.PushGpuMarker(name)
#define PROFILER_POP_GPU_MARKER()						gProfiler.PopGpuMarker()

#define PROFILER_SYNC_FRAME()							gProfiler.SynchronizeFrame()
#define PROFILER_UPDATE_RESULTS()						gProfiler.UpdateResults()

#endif



#endif // GPUTimer_h__

