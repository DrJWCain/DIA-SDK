
#include "stdafx.h"
#include "cDIAWrapper.h"
#include "UDT.h"
#include "printing.h"
#include "cDiffTimer.h"
#include "useful.h"
#include "Function.h"
#include "DataMember.h"

cDIAWrapper::cDIAWrapper(const std::string& fileNamePrefix) : FileNamePrefix(fileNamePrefix)
{
  FileNamePrefix += " ";
  checkResult(CoCreateInstance(_uuidof( DiaSource ),0,CLSCTX_INPROC_SERVER,
    _uuidof( IDiaDataSource ),(void**)&IDiaDataSource));
}

void cDIAWrapper::openPDB(const std::wstring& pdbName)
{
  checkResult(IDiaDataSource->loadDataFromPdb(pdbName.c_str()));
  checkResult(IDiaDataSource->openSession(&IDiaSession));
  checkResult(IDiaSession->get_globalScope(&GlobalScope));
}

std::wstring getSourceFileName(CComPtr<IDiaSymbol> sym)
{
  std::wstring ret;
  if(!sym)
  {
    return ret;
  }

  CComBSTR bstr;
  checkResult(sym->get_sourceFileName(&bstr));

  //The BSTR may be invalid. If so do not assign to the wstring.
  if(!!bstr)
    ret = bstr;

  return ret;
}

std::wstring getSourceFileName(CComPtr<IDiaSourceFile> file)
{
  std::wstring ret;
  if(!file)
  {
    return ret;
  }

  CComBSTR bstr;
  checkResult(file->get_fileName(&bstr));

  //The BSTR may be invalid. If so do not assign to the wstring.
  if(!!bstr)
    ret = bstr;

  return ret;
}

void f(CComPtr<IDiaSession> session, CComPtr<IDiaSourceFile> sourceFile)
{
  CComPtr<IDiaEnumLineNumbers> enumLineNumbers;
  checkResult(session->findLines(NULL,sourceFile,&enumLineNumbers));

  LONG count;
  checkResult(enumLineNumbers->get_Count(&count));

  std::vector<IDiaLineNumber*> IDiaLineNumberVec(count);
  ULONG num;
  checkResult(enumLineNumbers->Next(count,&IDiaLineNumberVec.front(),&num));

  for(std::vector<IDiaLineNumber*>::iterator it = IDiaLineNumberVec.begin(); it != IDiaLineNumberVec.end(); ++it)
  {
    CComPtr<IDiaLineNumber> lineNumber = *it;
    (*it)->Release();
  }
}

void cDIAWrapper::exploreSession()
{
  if(GlobalScope)
  {
    CComPtr<IDiaEnumSymbols> pDiaEnumSymbols;
    checkResult(GlobalScope->findChildren(SymTagFunction, NULL, nsCaseSensitive, &pDiaEnumSymbols));

    LONG count;
    checkResult(pDiaEnumSymbols->get_Count(&count));

    std::vector<IDiaSymbol*> IDiaSymbolVec(count);
    ULONG num;
    checkResult(pDiaEnumSymbols->Next(count,&IDiaSymbolVec.front(),&num));

    for(std::vector<IDiaSymbol*>::iterator it = IDiaSymbolVec.begin(); it != IDiaSymbolVec.end(); ++it)
    {
      CComPtr<IDiaSymbol> symbol = *it;
      std::wstring name = getName(symbol);
      printf( "%S\n", name.c_str());

      DWORD loc_type;
      checkResult(symbol->get_locationType(&loc_type));
      DWORD offset;
      checkResult(symbol->get_addressOffset(&offset));
      DWORD section;
      checkResult(symbol->get_addressSection(&section));
      ULONGLONG length;
      checkResult(symbol->get_length(&length));

      if ( section != 0 && length > 0 ) 
      {
        CComPtr< IDiaEnumLineNumbers > pLines;
        if ( SUCCEEDED( IDiaSession->findLinesByAddr( section, offset, static_cast<DWORD>( length ), &pLines ) ) ) 
        {
          CComPtr< IDiaLineNumber > pLine;
          DWORD celt;
          bool firstLine = true;
          while ( SUCCEEDED( pLines->Next( 1, &pLine, &celt ) ) && celt == 1 )
          {
            DWORD offset;
            DWORD seg;
            DWORD linenum;
//            CComPtr< IDiaSymbol > pComp;
            CComPtr< IDiaSourceFile > pSrc;
//            pLine->get_compiland( &pComp );
            pLine->get_sourceFile( &pSrc );
            pLine->get_addressSection( &seg );
            pLine->get_addressOffset( &offset );
            pLine->get_lineNumber( &linenum );
            CComPtr<IDiaSymbol> sym;
            checkResult( IDiaSession->findSymbolByAddr(section, offset, SymTagNull, &sym));
            std::wstring name = getName(sym);
            printf( "\tline %d at 0x%x:0x%x (%S, %S)\n", linenum, seg, offset, getSourceFileName(pSrc).c_str(), name.c_str() );
            pLine = NULL;

/*
            if ( firstLine ) 
            {
              // Sanity check.
              CComPtr< IDiaEnumLineNumbers > pLinesByLineNum;
              if ( SUCCEEDED( IDiaSession->findLinesByLinenum( pComp, pSrc, linenum, 0, &pLinesByLineNum ) ) ) 
              {
                CComPtr< IDiaLineNumber > pLine;
                DWORD celt;
                while ( SUCCEEDED( pLinesByLineNum->Next( 1, &pLine, &celt ) ) && celt == 1 )
                {
                  DWORD offset;
                  DWORD seg;
                  DWORD linenum;
                  pLine->get_addressSection( &seg );
                  pLine->get_addressOffset( &offset );
                  pLine->get_lineNumber( &linenum );
                  printf( "\t\tfound line %d at 0x%x:0x%x\n", linenum, seg, offset );
                  pLine = NULL;
                }
              }
              firstLine = false;
            }
*/
          }
        }
      }
      (*it)->Release();
    }
  }

  if(IDiaSession)
  {
    CComPtr<IDiaEnumSourceFiles> pEnumSourceFiles;
    IDiaSession->findFile( NULL, NULL, nsFNameExt, &pEnumSourceFiles );

    LONG count;
    checkResult(pEnumSourceFiles->get_Count(&count));

    std::vector<IDiaSourceFile*> IDiaSourceFileVec(count);
    ULONG num;
    checkResult(pEnumSourceFiles->Next(count,&IDiaSourceFileVec.front(),&num));

    for(std::vector<IDiaSourceFile*>::iterator it = IDiaSourceFileVec.begin(); it != IDiaSourceFileVec.end(); ++it)
    {
      CComPtr<IDiaSourceFile> sourceFile = *it;
      std::wstring sourceFileName = getSourceFileName(sourceFile);

//      f(IDiaSession,sourceFile);

      (*it)->Release();
    }


//    IDiaSession->findFile();
  }
}

void cDIAWrapper::getClasses(const wchar_t* name)
{
  CComPtr<IDiaEnumSymbols> pDiaEnumSymbols;
  checkResult(GlobalScope->findChildren(SymTagUDT, name, nsCaseSensitive, &pDiaEnumSymbols));

  LONG count;
  checkResult(pDiaEnumSymbols->get_Count(&count));

  std::vector<IDiaSymbol*> IDiaSymbolVec(count);
  ULONG num;
  checkResult(pDiaEnumSymbols->Next(count,&IDiaSymbolVec.front(),&num));

  std::set<std::wstring> only_add_each_udt_once;
  ClassesVec.reserve(count);
  for(std::vector<IDiaSymbol*>::iterator it = IDiaSymbolVec.begin(); it != IDiaSymbolVec.end(); ++it)
  {
    IDiaSymbol* symbol = *it;
    std::set<std::wstring>::_Pairib result = 
      only_add_each_udt_once.insert(::getName(symbol));
    if(result.second)
    {
      UDT udt(symbol);
      ClassesVec.push_back(udt);
    }
    (*it)->Release();
  }

  std::sort(ClassesVec.begin(),ClassesVec.end(),UDT::Predicate());
}

int cDIAWrapper::getClassID(const std::wstring& name)
{
  return getIndex(name);
//  UDTVec::iterator it = std::lower_bound(ClassesVec.begin(),ClassesVec.end(),UDT(name),UDT::Predicate());
//  if(it != ClassesVec.end() && it->getName() == name)
//    return (int)(it - ClassesVec.begin());
//  return -1;
}

void cDIAWrapper::loadClasses()
{
  std::wifstream file(std::string("classes.txt").c_str());
  std::wstring buf;
  while(!file.eof() && !file.fail())
  {
    std::getline(file,buf);
    MasterClassesVec.push_back(buf);
  }
}

int cDIAWrapper::getIndex(const std::wstring& name)
{
//  return getClassID(className);
  std::vector<std::wstring>::const_iterator it = std::lower_bound(MasterClassesVec.begin(), MasterClassesVec.end(), name);
  if(MasterClassesVec.end() != it && *it == name)
    return (int)(it - MasterClassesVec.begin());
  return -1;
}

int cDIAWrapper::getIndex(const UDT& udt)
{
  return getIndex(udt.getName());
}

void cDIAWrapper::fileClasses()
{
  std::wofstream file(std::string(FileNamePrefix + "classes.txt").c_str());
  for(UDTVec::iterator it = ClassesVec.begin(); it != ClassesVec.end(); ++it)
  {
    std::wstringstream str;
    const UDT& udt = *it;
    str << getIndex(udt) << L":";
    //str << (int)(it - ClassesVec.begin()) << L":";
    printUDTSimple(str,udt);
    file << str.str();
  }
}

void cDIAWrapper::fileClassStabilities()
{
  std::wofstream file(std::string(FileNamePrefix + "stabilities.txt").c_str());
  for(UDTVec::iterator it = ClassesVec.begin(); it != ClassesVec.end(); ++it)
  {
    std::wstringstream str;
    const UDT& udt = *it;
    str << getIndex(udt) << L":";
    //str << (int)(it - ClassesVec.begin()) << L":";
    std::wstring::size_type i = it->getName().find(L"std::");
    if(!i)
      str << L"1\n";
    else
      str << L"0\n";
    file << str.str();
  }
}

void cDIAWrapper::fileClassReturns()
{
  std::wofstream file(std::string(FileNamePrefix + "returns.txt").c_str());
  for(UDTVec::iterator it = ClassesVec.begin(); it != ClassesVec.end(); ++it)
  {
    std::wstringstream str;

    const UDT& udt = *it;
    str << getIndex(udt) << L":";
    //str << (int)(it - ClassesVec.begin()) << L":";
    UDT::Functions functions = it->getFunctions();
    for(UDT::Functions::const_iterator it2 = functions.begin(); it2 != functions.end(); ++it2)
    {
      std::wstring name = it2->getName();
      if(!name.empty())
      {
        int id = getClassID(it2->returnTypeSpecial());
        if(id != -1)
          str << id << L" ";
      }
    }

    str << L"\n";
    
    file << str.str();
  }
}

void cDIAWrapper::fileClassParams()
{
  std::wofstream file(std::string(FileNamePrefix + "params.txt").c_str());
  for(UDTVec::iterator it = ClassesVec.begin(); it != ClassesVec.end(); ++it)
  {
    std::wstringstream str;

    const UDT& udt = *it;
    str << getIndex(udt) << L":";
    //str << (int)(it - ClassesVec.begin()) << L":";
    UDT::Functions functions = it->getFunctions();
    for(UDT::Functions::const_iterator it2 = functions.begin(); it2 != functions.end(); ++it2)
    {
      std::wstring name = it2->getName();
      if(!name.empty())
      {
        std::list<std::wstring> params = it2->getArgsSpecial();
        for(std::list<std::wstring>::iterator it3 = params.begin(); it3 != params.end(); ++it3)
        {
          if(!it3->empty())
          {
            int id = getClassID(*it3);
            if(id != -1)
              str << id << L" ";
          }
        }
      }
    }

    str << L"\n";
    
    file << str.str();
  }
}

void cDIAWrapper::fileClassBases()
{
  std::wofstream file(std::string(FileNamePrefix + "bases.txt").c_str());
  for(UDTVec::iterator it = ClassesVec.begin(); it != ClassesVec.end(); ++it)
  {
    std::wstringstream str;

    const UDT& udt = *it;
    str << getIndex(udt) << L":";
    //str << (int)(it - ClassesVec.begin()) << L":";

    UDT::Bases bases = it->getBases();
    for(UDT::Bases::const_iterator it2 = bases.begin(); it2 != bases.end(); ++it2)
    {
      const int kImmediateBase = 1;
      if(kImmediateBase == it2->second.first)
      {
        int id = getClassID(it2->first.getName());
        if(id != -1)
          str << id << L" ";
      }
    }

    str << L"\n";
    
    file << str.str();
  }
}

void cDIAWrapper::fileClassAttributes()
{
  std::wofstream file(std::string(FileNamePrefix + "attributes.txt").c_str());
  for(UDTVec::iterator it = ClassesVec.begin(); it != ClassesVec.end(); ++it)
  {
    std::wstringstream str;

    const UDT& udt = *it;
    str << getIndex(udt) << L":";
    //str << (int)(it - ClassesVec.begin()) << L":";
    UDT::DataMembers dataMembers = it->getDataMembers();
    for(UDT::DataMembers::const_iterator it2 = dataMembers.begin(); it2 != dataMembers.end(); ++it2)
    {
      int id = getClassID(it2->typeSpecial());
      if(id != -1)
        str << id << L" ";
    }

    str << L"\n";
    
    file << str.str();
  }
}

void cDIAWrapper::fileClassInner()
{
  std::wofstream file(std::string(FileNamePrefix + "inners.txt").c_str());
  for(UDTVec::iterator it = ClassesVec.begin(); it != ClassesVec.end(); ++it)
  {
    std::wstringstream str;

    const UDT& udt = *it;
    str << getIndex(udt) << L":";
    //str << (int)(it - ClassesVec.begin()) << L":";
    UDT::Nested nested = it->getNested();
    for(UDT::Nested::const_iterator it2 = nested.begin(); it2 != nested.end(); ++it2)
    {
      std::wstring scopedClass = it->getName() + L"::" + it2->getName();
      int id = getClassID(scopedClass);
      if(id != -1)
        str << id << L" ";
    }

    str << L"\n";
    
    file << str.str();
  }
}

void cDIAWrapper::printClassesSimple()
{
  for(UDTVec::iterator it = ClassesVec.begin(); it != ClassesVec.end(); ++it)
  {
    std::wstringstream str;
    printUDTSimple(str,*it);
    std::wcout << str.str();
  }
}

void cDIAWrapper::printClasses()
{
  for(UDTVec::iterator it = ClassesVec.begin(); it != ClassesVec.end(); ++it)
  {
    std::wstringstream str;
    cDiffTimer timer;
    printUDT(str,*it);
    int time = timer.getDiff();
    std::wcout << str.str();
    std::wcout << L"//" << time << L"ms\n\n";
  }
}
