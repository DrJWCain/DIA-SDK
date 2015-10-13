

//Class for logging how long things take. Logs to stderr.
struct cDiffTimerErr
{
  cDiffTimerErr()
  {
    QueryPerformanceCounter(&start);
  }
  ~cDiffTimerErr()
  {
    LARGE_INTEGER stop, freq, diff;
    QueryPerformanceCounter(&stop);
    QueryPerformanceFrequency(&freq);
    diff.QuadPart = ((stop.QuadPart - start.QuadPart) * 1000) / freq.QuadPart;
    std::wcerr << diff.LowPart << L"ms" << std::endl;
  }
private:
  LARGE_INTEGER start;
};

struct cDiffTimerOut
{
  cDiffTimerOut()
  {
    QueryPerformanceCounter(&start);
  }
  ~cDiffTimerOut()
  {
    LARGE_INTEGER stop, freq, diff;
    QueryPerformanceCounter(&stop);
    QueryPerformanceFrequency(&freq);
    diff.QuadPart = ((stop.QuadPart - start.QuadPart) * 1000) / freq.QuadPart;
    std::wcout << diff.LowPart << L"ms" << std::endl;
  }
private:
  LARGE_INTEGER start;
};

struct cDiffTimer
{
  cDiffTimer()
  {
    QueryPerformanceCounter(&start);
  }
  int getDiff()
  {
    LARGE_INTEGER stop, freq, diff;
    QueryPerformanceCounter(&stop);
    QueryPerformanceFrequency(&freq);
    diff.QuadPart = ((stop.QuadPart - start.QuadPart) * 1000) / freq.QuadPart;
    return diff.LowPart;
  }
private:
  LARGE_INTEGER start;
};
