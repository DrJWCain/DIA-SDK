//printing.cpp

#include "stdafx.h"
#include "printing.h"
#include "UDT.h"
#include "DataMember.h"
#include "Function.h"
#include "useful.h"


void printDepth(std::wstringstream& str, int depth)
{
  for(int i = 0; i < depth; ++i)
    str << L"  ";//two space tabs !!
}

std::wstring checkProtection(CV_access_e member,CV_access_e& current,int depth)
{
  if(current != member)
  {
    std::wstringstream str;
    printDepth(str,depth);
    switch(member)
    {
    case CV_private: str << L"private:\n"; break;
    case CV_protected: str << L"protected:\n"; break;
    case CV_public: str << L"public:\n"; break;
    }
    current = member;
    return str.str();
  }
  return L"";
}

void printAccess(std::wstringstream& str,int access)
{
  if(CV_private == access)
    str << L"private";
  else if(CV_protected == access)
    str << L"protected";
  else if(CV_public == access)
    str << L"public";
}

void printBaseList(std::wstringstream& str,const UDT::Bases& bases)
{
  bool printedColon = false;
  for(UDT::Bases::const_iterator it = bases.begin(); it != bases.end(); ++it)
  {
    const int kImmediateBase = 1;
    if(kImmediateBase == it->second.first)
    {
      if(!printedColon)
      {
        str << L" : ";
        printedColon = true;
      }
      else
      {
        str << L", ";
      }
      printAccess(str,it->second.second);
      str << L" " << it->first.getName();
    }
  }
}

void printMembers(std::wstringstream& str, const UDT::DataMembers& dataMembers,int depth,CV_access_e& protection)
{
  for(UDT::DataMembers::const_iterator it = dataMembers.begin(); it != dataMembers.end(); ++it)
  {
    str << checkProtection(it->protection(),protection,depth);
    printDepth(str,depth+1);
    if(it->isStatic())
      str << L"static ";
    if(it->pointerToFunction())
    {
      str << it->type() << L";" << std::endl;
    }
    else
    {
      str << it->type() << L" ";
      str << it->getName();
      str << L";" << std::endl;
    }
  }
}

void printFunctionReturns(std::wstringstream& str,const UDT::Functions& functions,int depth)
{
  for(UDT::Functions::const_iterator it = functions.begin(); it != functions.end(); ++it)
  {
    std::wstring name = it->getName();
    if(!name.empty())
    {
      printDepth(str,depth+1);
      str << it->returnType() << L" // " << name << L"\n";
    }
  }
}

void printFunctions(std::wstringstream& str,const UDT::Functions& functions,int depth,CV_access_e& protection)
{
  for(UDT::Functions::const_iterator it = functions.begin(); it != functions.end(); ++it)
  {
    std::wstring name = it->getName();
    if(!name.empty())
    {
      str << checkProtection(it->protection(),protection,depth);
      printDepth(str,depth+1);
      if(it->reservedName())
        str << L"// ";
      if(it->isCompilerGenerated())
        str << L"/* compiler generated */ ";
      if(it->isStatic())
        str << L"static ";
      if(it->isVirtual())
        str << L"virtual ";
      str << it->returnType() << L" ";
      str << it->callingConvention();
      str << name;
      str << L"(";
      std::list<std::wstring> args = it->getArgs();
      bool first = true;
      for(std::list<std::wstring>::iterator it2 = args.begin(); it2 != args.end(); ++it2)
      {
        if(!first)str << L", ";
        first = false;
        str << *it2;
      }
      str << L")";
      if(it->isPure())
        str << L" = 0";
      if(it->isConst())
        str << L" const";

      unsigned int rva = it->getRVA();
      if(rva)
        str << L" //" << rva;

      std::list<std::wstring> localVars = it->getLocalVars();
      if(localVars.empty())
      {
        str << L";" << std::endl;
      }
      else
      {
        str << L"\n";
        printDepth(str,depth+1);
        str << L"{\n";
        for(std::list<std::wstring>::iterator it3 = localVars.begin(); it3 != localVars.end(); ++it3)
        {
          printDepth(str,depth+2);
          str << *it3 << std::endl;
        }
        printDepth(str,depth+1);
        str << L"}\n";
      }
    }
  }
}

void printUDTReturns(std::wstringstream& str,const UDT& udt)
{
  str << udt.getKind() << L" " << udt.getName();
  str << std::endl;

  UDT::Functions functions = udt.getFunctions();
  printFunctionReturns(str,functions,2);
}

void printUDTSimple(std::wstringstream& str,const UDT& udt)
{
  str << udt.getKind() << L" " << udt.getName();
  str << std::endl;
}

void printUDT(std::wstringstream& str,const UDT& udt,int depth)
{
  printDepth(str,depth);
  //str << udt.getID() << L" " << udt.getTypeID() << L" ";
  str << udt.getKind() << L" " << udt.getName();
  UDT::Bases bases = udt.getBases();
  printBaseList(str,bases);
  str << std::endl;

  printDepth(str,depth);
  str << L"{\n";

  CV_access_e protection = udt.getDefaultProtection();

  UDT::Nested nested = udt.getNested();
  for(UDT::Nested::const_iterator it = nested.begin(); it != nested.end(); ++it)
  {
    printUDT(str,*it,depth+1);
  }

/*
  std::wstring friends = udt.getFriends();
  if(!friends.empty())
  {
    printDepth(str,depth);
    str << L"//friends " << friends << std::endl;
  }
*/

  std::wstring enums = udt.getEnums();
  if(!enums.empty())
  {
    str << enums;
  }

  std::wstring typedefs = udt.getTypedefs();
  if(!typedefs.empty())
  {
    str << typedefs;
  }

  UDT::DataMembers dataMembers = udt.getDataMembers();
  printMembers(str,dataMembers,depth,protection);

  UDT::Functions functions = udt.getFunctions();
  printFunctions(str,functions,depth,protection);

  printDepth(str,depth);
  str << L"};\n";
}
