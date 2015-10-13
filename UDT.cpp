#include "stdafx.h"
#include "UDT.h"
#include "DataMember.h"
#include "Function.h"
#include "useful.h"

UDT::UDT()
{
}

UDT::UDT(const std::wstring name) : Name(name)
{
}

UDT::UDT(CComPtr<IDiaSymbol> pIDiaSymbol) : Symbol(pIDiaSymbol)
{
  DWORD tag;
  checkResult(pIDiaSymbol->get_symTag(&tag));
  if(SymTagUDT != tag)
  {
    throw 0;
  }
}

UDT::UDT(const UDT& other) : Symbol(other.Symbol)
{
}

CComPtr<IDiaSymbol> UDT::sym() const
{
  return Symbol;
}

CComPtr<IDiaEnumSymbols> UDT::children(enum SymTagEnum type) const
{
  CComPtr<IDiaEnumSymbols> pEnumSymbols;
  checkResult(sym()->findChildren(type,
    NULL,nsNone,&pEnumSymbols));
  return pEnumSymbols;
}

std::wstring UDT::getName() const
{
  if(Name.empty())
    Name = ::getName(sym());
  return Name;
}

std::wstring UDT::getKind() const
{
  return ::getKind(sym());
}

bool UDT::nested() const
{
  BOOL isNested = FALSE;
  checkResult(sym()->get_nested(&isNested));
  return !!isNested;
}

UDT::Bases UDT::getBases(int depth) const
{
  UDT::Bases ret;

  CComPtr<IDiaEnumSymbols> pEnumSymbols = children(SymTagBaseClass);

  LONG count;
  checkResult(pEnumSymbols->get_Count(&count));
  for(int i = 0; i < count; ++i)
  {
    CComPtr<IDiaSymbol> pIDiaSymbol;
    checkResult(pEnumSymbols->Item(i,&pIDiaSymbol));//baseClass
    
    DWORD access;
    checkResult(pIDiaSymbol->get_access(&access));

    CComPtr<IDiaSymbol> pIDiaSymbol2;
    checkResult(pIDiaSymbol->get_type(&pIDiaSymbol2));//UDT

    UDT udt(pIDiaSymbol2);
    UDT::Bases ret_recurse = udt.getBases(depth+1);
    ret.insert(ret.end(),ret_recurse.begin(),ret_recurse.end());
    ret.push_back(std::pair<UDT,std::pair<int,int> >(udt,std::pair<int,int>(depth,access)));
  }

  return ret;
}

std::wstring UDT::getEnums() const
{
  CComPtr<IDiaEnumSymbols> pEnumSymbols = children(SymTagEnum);

  LONG count;
  checkResult(pEnumSymbols->get_Count(&count));

  std::wstring ret;
  for(int i = 0; i < count; ++i)
  {
    CComPtr<IDiaSymbol> pIDiaSymbol;
    checkResult(pEnumSymbols->Item(i,&pIDiaSymbol));//baseType

    ret += L"enum " + ::getName(pIDiaSymbol) + L" { ";

    CComPtr<IDiaEnumSymbols> pEnumConstants;
    checkResult(pIDiaSymbol->findChildren(SymTagData, //SymTagConstant
      NULL,nsNone,&pEnumConstants));

    LONG countConstants;
    checkResult(pEnumConstants->get_Count(&countConstants));
    for(int j = 0; j < countConstants; ++j)
    {
      if(j) ret += L", ";
      CComPtr<IDiaSymbol> pIDiaConstant;
      checkResult(pEnumConstants->Item(j,&pIDiaConstant));//baseType

      DWORD tagSym;
      checkResult(pIDiaConstant->get_symTag(&tagSym));

      ret += ::getName(pIDiaConstant);
    }
    ret += L" };\n";
  }
  return ret;
}

std::wstring UDT::getTypedefs() const
{
  CComPtr<IDiaEnumSymbols> pEnumSymbols = children(SymTagTypedef);

  LONG count;
  checkResult(pEnumSymbols->get_Count(&count));

  std::wstring ret;
  for(int i = 0; i < count; ++i)
  {
    CComPtr<IDiaSymbol> pIDiaSymbol;
    checkResult(pEnumSymbols->Item(i,&pIDiaSymbol));//baseType

    CComPtr<IDiaSymbol> type;
    checkResult(pIDiaSymbol->get_type(&type));
    ret += L"typedef " + expandType(type) + L" " + ::getName(pIDiaSymbol) + L";\n";
  }
  return ret;
}

UDT::DataMembers UDT::getDataMembers() const
{
  DataMembers ret;

  CComPtr<IDiaEnumSymbols> pEnumSymbols = children(SymTagData);

  LONG count;
  checkResult(pEnumSymbols->get_Count(&count));
  for(int i = 0; i < count; ++i)
  {
    CComPtr<IDiaSymbol> pIDiaSymbol;
    checkResult(pEnumSymbols->Item(i,&pIDiaSymbol));//baseType

    DataMember val(pIDiaSymbol);
    ret.push_back(val);
  }
  return ret;
}

UDT::Functions UDT::getFunctions() const
{
  Functions ret;

  CComPtr<IDiaEnumSymbols> pEnumSymbols;
  checkResult(sym()->findChildren(SymTagFunction,
    NULL,nsNone,&pEnumSymbols));

  LONG count;
  checkResult(pEnumSymbols->get_Count(&count));
  for(int i = 0; i < count; ++i)
  {
    CComPtr<IDiaSymbol> pIDiaSymbol;
    checkResult(pEnumSymbols->Item(i,&pIDiaSymbol));

    Function val(pIDiaSymbol);
    ret.push_back(val);
  }
  return ret;
}

CV_access_e UDT::getDefaultProtection() const
{
  std::wstring ret;
  DWORD kind = 0;
  checkResult(sym()->get_udtKind(&kind));

  if(UdtClass == kind)
    return CV_private;
  return CV_public;
}

UDT::Nested UDT::getNested() const
{
  Nested ret;

  CComPtr<IDiaEnumSymbols> pEnumSymbols;
  checkResult(sym()->findChildren(SymTagUDT,
    NULL,nsNone,&pEnumSymbols));

  LONG count;
  checkResult(pEnumSymbols->get_Count(&count));
  for(int i = 0; i < count; ++i)
  {
    CComPtr<IDiaSymbol> pIDiaSymbol;
    checkResult(pEnumSymbols->Item(i,&pIDiaSymbol));

    UDT udt(pIDiaSymbol);
    ret.push_back(udt);
  }
  
  return ret;
}

