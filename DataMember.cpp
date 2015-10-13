#include "stdafx.h"
#include "DataMember.h"
#include "useful.h"

DataMember::DataMember()
{
}

DataMember::DataMember(CComPtr<IDiaSymbol> pIDiaSymbol) : Symbol(pIDiaSymbol)
{
  DWORD tag;
  checkResult(pIDiaSymbol->get_symTag(&tag));
  if(SymTagData != tag)
  {
    throw 0;
  }
}

DataMember::DataMember(const DataMember& other) : Symbol(other.Symbol)
{
}

DataMember& DataMember::operator= (const DataMember& other)
{
  if(*this == other)
    return *this;

  Symbol = other.Symbol;
  return *this;
}

bool DataMember::operator== (const DataMember& other)
{
  return Symbol == other.Symbol;
}

CComPtr<IDiaSymbol> DataMember::sym() const
{
  return Symbol;
}

std::wstring DataMember::getName() const
{
  return ::getName(sym());
}

bool DataMember::pointerToFunction() const
{
  CComPtr<IDiaSymbol> type;
  checkResult(sym()->get_type(&type));

  DWORD tag;
  checkResult(type->get_symTag(&tag));
  if(SymTagPointerType == tag)
  {
    CComPtr<IDiaSymbol> pointee;
    checkResult(type->get_type(&pointee));

    DWORD pointeeTag;
    checkResult(pointee->get_symTag(&pointeeTag));
    return (SymTagFunctionType == pointeeTag);//function pointer!
  }
  return false;
}

std::wstring DataMember::type() const
{
  std::wstring ret;
  CComPtr<IDiaSymbol> type;
  checkResult(sym()->get_type(&type));
  if(!!type)
  {
    ret += expandType(type,getName());//name needed for function defs
  }
  return ret;
}

std::wstring DataMember::typeSpecial() const
{
  std::wstring ret;
  CComPtr<IDiaSymbol> type;
  checkResult(sym()->get_type(&type));
  if(!!type)
  {
    ret += expandTypeSpecial(type);
  }
  return ret;
}

bool DataMember::isStatic() const
{
  DWORD kind;
  checkResult(sym()->get_dataKind(&kind));
  return DataIsGlobal == kind;
}

CV_access_e DataMember::protection() const
{
  DWORD access;
  checkResult(sym()->get_access(&access));
  return (CV_access_e)access;
}
