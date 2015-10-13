// DIAWrapper.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "cDIAWrapper.h"
#include "UDT.h"
#include "cDiffTimer.h"

#pragma comment(lib, "diaguids.lib")

std::string getNamePrefix(const std::wstring& filename)
{
  std::string ret;
  WIN32_FILE_ATTRIBUTE_DATA attribute_data;
  memset(&attribute_data,0,sizeof(attribute_data));
  BOOL ok = GetFileAttributesExW(filename.c_str(),GetFileExInfoStandard,&attribute_data);
  if(ok)
  {
    SYSTEMTIME time;
    FileTimeToSystemTime(&(attribute_data.ftLastWriteTime),&time);
    std::stringstream str;
    str << std::setfill('0') << std::setw(4) << (int)time.wYear << std::setw(2) << (int)time.wMonth << std::setw(2) << (int)time.wDay << "-" << std::setw(2) << (int)time.wHour << std::setw(2) << (int)time.wMinute;
    ret = str.str();
  }

  return ret;
}

int wmain(int argc, wchar_t* wargv[])
{
  checkResult(CoInitialize(0));

  if(argc < 2)
  {
    exit(1);
  }
  std::wstring pdbName = wargv[1];
  if(pdbName.empty())
  {
    exit(1);
  }

  std::string fileNamePrefix = getNamePrefix(pdbName);

  try
  {
    cDiffTimerOut timer;
    std::auto_ptr<cDIAWrapper> pWrapper(new cDIAWrapper(fileNamePrefix));
    pWrapper->loadClasses();

    pWrapper->openPDB(pdbName);
//    pWrapper->exploreSession();
  
    pWrapper->getClasses(argc > 2 ? wargv[2] : 0);

    pWrapper->fileClasses();
    pWrapper->fileClassStabilities();
    pWrapper->fileClassReturns();
    pWrapper->fileClassParams();
    pWrapper->fileClassBases();
    pWrapper->fileClassAttributes();
    pWrapper->fileClassInner();
  }
  catch(HRESULT hr)
  {
    std::wcerr << L"Error:" << std::hex << hr;
    if(hr == E_PDB_NOT_FOUND)
    {
      std::wcerr << L" PDB file not found";
    }
  }

  return 0;
}

