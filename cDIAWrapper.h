//cDIAWrapper.h

class UDT;
class cDIAWrapper
{
  typedef CComPtr<IDiaDataSource> DataSource;
  typedef CComPtr<IDiaSession> Session;
  typedef CComPtr<IDiaSymbol> Symbol;
  typedef std::vector<UDT> UDTVec;

  DataSource IDiaDataSource;
  Session IDiaSession;
  Symbol GlobalScope;
  UDTVec ClassesVec;

  int getClassID(const std::wstring& name);
  std::string FileNamePrefix;
  std::vector<std::wstring> MasterClassesVec;

  int cDIAWrapper::getIndex(const std::wstring& className);
  int cDIAWrapper::getIndex(const UDT& udt);  

public:
  cDIAWrapper(const std::string& fileNamePrefix);
  void openPDB(const std::wstring& pdbName);
  void exploreSession();
  void getClasses(const wchar_t* name);
  void loadClasses();

  void printClasses();
  void printClassesSimple();

  void fileClasses();
  void fileClassStabilities();
  void fileClassReturns();
  void fileClassParams();
  void fileClassBases();
  void fileClassAttributes();
  void fileClassInner();

};

