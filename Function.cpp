#include "stdafx.h"
#include "Function.h"
#include "useful.h"

Function::Function()
{
}

Function::Function(CComPtr<IDiaSymbol> pIDiaSymbol) : Symbol(pIDiaSymbol)
{
  DWORD tag;
  checkResult(pIDiaSymbol->get_symTag(&tag));
  if(SymTagFunction != tag)
  {
    throw 0;
  }
}

Function::Function(const Function& other) : Symbol(other.Symbol)
{
}

Function& Function::operator= (const Function& other)
{
  if(*this == other)
    return *this;

  Symbol = other.Symbol;
  return *this;
}

bool Function::operator== (const Function& other)
{
  return Symbol == other.Symbol;
}

CComPtr<IDiaSymbol> Function::sym() const
{
  return Symbol;
}

CComPtr<IDiaSymbol> Function::functionType() const
{
  //type is type FunctionType
  CComPtr<IDiaSymbol> type;
  checkResult(sym()->get_type(&type));
  return type;
}

CComPtr<IDiaSymbol> Function::objectPointer() const
{
  //objPointer is type const pointer (this)
  CComPtr<IDiaSymbol> objPointer;
  CComPtr<IDiaSymbol> type = functionType();
  if(!!type)
  {
    checkResult(type->get_objectPointerType(&objPointer));
  }
  return objPointer;
}

std::wstring Function::getName() const
{
  return ::getName(sym());
}

bool Function::isVirtual() const
{
  BOOL result;
  checkResult(sym()->get_virtual(&result));
  return result? true: false;
}

bool Function::isPure() const
{
  BOOL result;
  checkResult(sym()->get_pure(&result));
  return result? true: false;
}

bool Function::isCompilerGenerated() const
{
  BOOL result;
  CComPtr<IDiaSymbol> type = functionType();
  checkResult(type->get_compilerGenerated(&result));
  return result? true: false;
}

bool Function::isConst() const
{
  CComPtr<IDiaSymbol> objPointer = objectPointer();
  if(!!objPointer)
  {
    DWORD tag;
    checkResult(objPointer->get_symTag(&tag));
    if(SymTagPointerType == tag)
    {
      //if pointee is const then const function
      CComPtr<IDiaSymbol> pointee;
      checkResult(objPointer->get_type(&pointee));
      BOOL result;
      checkResult(pointee->get_constType(&result));
      return result? true: false;
    }
  }
  return false;
}

bool Function::isStatic() const
{
  CComPtr<IDiaSymbol> objPointer = objectPointer();
  if(!!objPointer)
  {
    //if there is an object pointer, this function has a this pointer
    return false;
  }
  //no this pointer means static
  return true;
}

std::wstring Function::returnTypeSpecial() const
{
  if(isCtor()) return L"/*ctor*/";
  if(isDtor()) return L"/*dtor*/";

  std::wstring ret;
  CComPtr<IDiaSymbol> type = functionType();
  if(!!type)
  {
    CComPtr<IDiaSymbol> returnType;
    checkResult(type->get_type(&returnType));
    ret += expandTypeSpecial(returnType);
  }
  return ret;
}


std::wstring Function::returnType() const
{
  if(isCtor()) return L"/*ctor*/";
  if(isDtor()) return L"/*dtor*/";

  std::wstring ret;
  CComPtr<IDiaSymbol> type = functionType();
  if(!!type)
  {
    CComPtr<IDiaSymbol> returnType;
    checkResult(type->get_type(&returnType));
    ret += expandType(returnType);
  }
  return ret;
}

std::wstring Function::callingConvention() const
{
  CComPtr<IDiaSymbol> type = functionType();
  if(!!type)
  {
    DWORD cc;
    checkResult(type->get_callingConvention(&cc));
    if(CV_CALL_NEAR_C == cc)
    {
      return L"__cdecl ";
    }
    if(CV_CALL_THISCALL == cc)
    {
      return L"";//L"/* __thiscall */ ";
    }
    if(CV_CALL_NEAR_FAST == cc)
    {
      return L"__fastcall ";
    }
    if(CV_CALL_NEAR_STD == cc)
    {
      return L"__stdcall ";
    }
  }
  return L"";
}

std::list<std::wstring> Function::getLocalVars() const
{
  std::list<std::wstring> ret;
  CComPtr<IDiaEnumSymbols> pIDiaEnumSymbols;
  checkResult(sym()->findChildren(SymTagData,//symbol data seems to return args and types used
    NULL,nsNone,&pIDiaEnumSymbols));

  if(!pIDiaEnumSymbols)return ret;

  LONG count;
  checkResult(pIDiaEnumSymbols->get_Count(&count));
  for(int i = 0; i < count; ++i)
  {
    CComPtr<IDiaSymbol> pIDiaSymbol;
    checkResult(pIDiaEnumSymbols->Item(i,&pIDiaSymbol));

    DWORD dataKind;
    checkResult(pIDiaSymbol->get_dataKind(&dataKind));

    if(DataIsLocal == dataKind || DataIsObjectPtr == dataKind)
    {
      std::wstring name = ::getName(pIDiaSymbol);
      CComPtr<IDiaSymbol> funcArg;
      checkResult(pIDiaSymbol->get_type(&funcArg));
      ret.push_back(expandType(funcArg) + L" " + name + L";");
    }
  }
  return ret;
}

std::list<std::wstring> Function::getArgs() const
{
  std::list<std::wstring> ret;
  CComPtr<IDiaSymbol> type = functionType();
  if(!!type)
  {
    CComPtr<IDiaEnumSymbols> pIDiaEnumSymbols;
    checkResult(sym()->findChildren(SymTagData,//symbol data seems to return args and types used
      NULL,nsNone,&pIDiaEnumSymbols));

    if(!pIDiaEnumSymbols)return ret;

    LONG count;
    checkResult(pIDiaEnumSymbols->get_Count(&count));
    if(!count)
    {
      return _getArgs();
    }

    for(int i = 0; i < count; ++i)
    {
      CComPtr<IDiaSymbol> pIDiaSymbol;
      checkResult(pIDiaEnumSymbols->Item(i,&pIDiaSymbol));

      DWORD dataKind;
      checkResult(pIDiaSymbol->get_dataKind(&dataKind));

      if(DataIsParam == dataKind)
      {
        std::wstring name = ::getName(pIDiaSymbol);
        CComPtr<IDiaSymbol> funcArg;
        checkResult(pIDiaSymbol->get_type(&funcArg));
        ret.push_back(expandType(funcArg) + L" " + name);
      }
    }
  }
  return ret;
}

std::list<std::wstring> Function::_getArgs() const
{
  std::list<std::wstring> ret;
  CComPtr<IDiaSymbol> type = functionType();
  if(!!type)
  {
    CComPtr<IDiaEnumSymbols> pIDiaEnumSymbols;
    checkResult(type->findChildren(SymTagFunctionArgType,
      NULL,nsNone,&pIDiaEnumSymbols));

    if(!pIDiaEnumSymbols)return ret;

    LONG count;
    checkResult(pIDiaEnumSymbols->get_Count(&count));
    for(int i = 0; i < count; ++i)
    {
      CComPtr<IDiaSymbol> pIDiaSymbol;
      checkResult(pIDiaEnumSymbols->Item(i,&pIDiaSymbol));

      std::wstring name = ::getName(pIDiaSymbol);

      CComPtr<IDiaSymbol> funcArg;
      checkResult(pIDiaSymbol->get_type(&funcArg));
      ret.push_back(expandType(funcArg));
    }
  }
  return ret;
}

std::list<std::wstring> Function::getArgsSpecial() const
{
  std::list<std::wstring> ret;
  CComPtr<IDiaSymbol> type = functionType();
  if(!!type)
  {
    CComPtr<IDiaEnumSymbols> pIDiaEnumSymbols;
    checkResult(type->findChildren(SymTagFunctionArgType,
      NULL,nsNone,&pIDiaEnumSymbols));

    if(!pIDiaEnumSymbols)return ret;

    LONG count;
    checkResult(pIDiaEnumSymbols->get_Count(&count));
    for(int i = 0; i < count; ++i)
    {
      CComPtr<IDiaSymbol> pIDiaSymbol;
      checkResult(pIDiaEnumSymbols->Item(i,&pIDiaSymbol));

      std::wstring name = ::getName(pIDiaSymbol);

      CComPtr<IDiaSymbol> funcArg;
      checkResult(pIDiaSymbol->get_type(&funcArg));
      ret.push_back(expandTypeSpecial(funcArg));
    }
  }
  return ret;
}


CV_access_e Function::protection() const
{
  DWORD access;
  checkResult(sym()->get_access(&access));
  return (CV_access_e)access;
}

bool Function::isCtor() const
{
  std::wstring rawFuncName = removeLeading(getName());
  CComPtr<IDiaSymbol> type = functionType();
  if(!!type)
  {
    CComPtr<IDiaSymbol> classType;
    checkResult(type->get_classParent(&classType));

    std::wstring className = removeLeading(::getName(classType));
    return className == rawFuncName;
  }
  return false;
}

bool Function::isDtor() const
{
  std::wstring rawFuncName = removeLeading(getName());
  return (!rawFuncName.empty() && L'~' == rawFuncName[0]);
}

bool Function::reservedName() const
{
  std::wstring name = getName();
  if(name.size() >= 2)
  {
    return (name[0] == L'_' && name[1] == L'_');
  }
  return false;
}

unsigned int Function::getRVA() const
{
  unsigned int rva = 0;
  checkResult(sym()->get_relativeVirtualAddress ((DWORD*)&rva));
  return rva;
}