//useful.cpp

#include "stdafx.h"
#include "useful.h"
#include "UDT.h"

std::wstring getName(CComPtr<IDiaSymbol> sym)
{
  std::wstring ret;
  if(!sym)
  {
    return ret;
  }

  CComBSTR bstr;
  checkResult(sym->get_name(&bstr));

  //The BSTR may be invalid. If so do not assign to the wstring.
  if(!!bstr)
    ret = bstr;

  return ret;
}

std::wstring getKind(CComPtr<IDiaSymbol> sym)
{
  std::wstring ret;
  DWORD kind = 0;
  checkResult(sym->get_udtKind(&kind));

  switch(kind)
  {
  case UdtStruct: ret += L"struct"; break;
  case UdtClass: ret += L"class"; break;
  case UdtUnion: ret += L"union"; break;
  }
  return ret;
}

/*
enum BasicType
{
    btNoType = 0,
    btVoid = 1,
    btChar = 2,
    btWChar = 3,
    btInt = 6,
    btUInt = 7,
    btFloat = 8,
    btBCD = 9,
    btBool = 10,
    btLong = 13,
    btULong = 14,
    btCurrency = 25,
    btDate = 26,
    btVariant = 27,
    btComplex = 28,
    btBit = 29,
    btBSTR = 30,
    btHresult = 31
};
*/
static wchar_t* szBasicTypes[] = {
  L"/* no type */",
  L"void",
  L"char",
  L"wchar_t",
  L"/* unknown type */",
  L"/* unknown type */",
  L"int",
  L"unsigned int",
  L"float",
  L"/* BCD */",
  L"bool",
  L"/* unknown type */",
  L"/* unknown type */",
  L"long",
  L"unsigned long",
  L"/* unknown type */",
  L"/* unknown type */",
  L"/* unknown type */",
  L"/* unknown type */",
  L"/* unknown type */",
  L"/* unknown type */",
  L"/* unknown type */",
  L"/* unknown type */",
  L"/* unknown type */",
  L"/* unknown type */",
  L"/* Currency */",
  L"/* Date */",
  L"/* Variant */",
  L"/* Complex */",
  L"/* Bit */",
  L"/* BSTR */",
  L"HRESULT",
};

std::wstring getBasicTypeString(DWORD basicType)
{
  return szBasicTypes[basicType];
};

std::wstring removeLeading(std::wstring str)
{
  //this function is not safe for templates
  if(std::wstring::npos != str.rfind(L"<")) 
    return str;

  std::wstring::size_type pos = str.rfind(L"::");
  if(std::wstring::npos != pos)
  {
    std::wstring ret(str,pos+2,std::wstring::npos);
    return ret;
  }
  return str;
}

static wchar_t* szDataKinds[] = {
  L"Unknown",
  L"Local",
  L"Static Local",
  L"Parameter",
  L"Object Pointer",
  L"File Static",
  L"Global",
  L"Member",
  L"Static Member",
  L"Constant"
};

std::wstring dataKind(CComPtr<IDiaSymbol> sym)
{
  DWORD kind;
  checkResult(sym->get_dataKind(&kind));
  return szDataKinds[kind];
}

static wchar_t* szLocationTypes[] = {
  L"LocIsNull",
  L"LocIsStatic",
  L"LocIsTLS",
  L"LocIsRegRel",
  L"LocIsThisRel",
  L"LocIsEnregistered",
  L"LocIsBitField",
  L"LocIsSlot",
  L"LocIsIlRel",
  L"LocInMetaData",
  L"LocIsConstant",
};

std::wstring locationType(CComPtr<IDiaSymbol> sym)
{
  DWORD loc;
  checkResult(sym->get_locationType(&loc));
  return szLocationTypes[loc];
}

std::wstring toString(int val)
{
  std::wstring ret;
  std::wstringstream converter;
  converter << val;
  converter >> ret;
  return ret;
}

std::wstring expandTypeSpecial(CComPtr<IDiaSymbol> type)
{
  std::wstring ret;
  if(!type)
  {
    return ret;
  }

  DWORD tag;
  checkResult(type->get_symTag(&tag));
  if(SymTagPointerType == tag)
  {
    CComPtr<IDiaSymbol> pointee;
    checkResult(type->get_type(&pointee));
    ret += expandTypeSpecial(pointee);
  }
  else if(SymTagUDT == tag)
  {
    UDT udt(type);
    ret += udt.getName();
  }
  return ret;
}

std::wstring expandType(CComPtr<IDiaSymbol> type, const std::wstring& name)
{
  std::wstring ret;
  if(!type)
  {
    return ret;
  }

  BOOL vol;
  checkResult(type->get_volatileType(&vol));
  if(vol)
  {
    ret += L"volatile ";
  }
  BOOL con;
  checkResult(type->get_constType(&con));
  if(con)
  {
    ret += L"const ";
  }

  DWORD tag;
  checkResult(type->get_symTag(&tag));
  if(SymTagBaseType == tag)
  {
    DWORD baseType;
    checkResult(type->get_baseType(&baseType));
    ret += getBasicTypeString(baseType);
    return ret;
  }
  if(SymTagPointerType == tag)
  {
    CComPtr<IDiaSymbol> pointee;
    checkResult(type->get_type(&pointee));

    DWORD pointeeTag;
    checkResult(pointee->get_symTag(&pointeeTag));
    if(SymTagFunctionType == pointeeTag)//function pointer!
    {
      CComPtr<IDiaSymbol> returnType;
      checkResult(pointee->get_type(&returnType));
      ret += expandType(returnType);

      ret += L" (*" + name + L")(";

      CComPtr<IDiaEnumSymbols> pIDiaEnumSymbols;
      checkResult(pointee->findChildren(SymTagFunctionArgType, NULL,
        nsNone,&pIDiaEnumSymbols));

      if(!!pIDiaEnumSymbols)
      {
        LONG count;
        checkResult(pIDiaEnumSymbols->get_Count(&count));
        for(int i = 0; i < count; ++i)
        {
          if(i)ret += L", ";
          CComPtr<IDiaSymbol> pIDiaSymbol;
          checkResult(pIDiaEnumSymbols->Item(i,&pIDiaSymbol));

          CComPtr<IDiaSymbol> funcArg;
          checkResult(pIDiaSymbol->get_type(&funcArg));
          ret += expandType(funcArg);
        }
      }
      ret += L")";
      return ret;
    }

    ret += expandType(pointee);

    BOOL ref;
    checkResult(type->get_reference(&ref));
    if(ref)
    {
      ret += L"&";
    }
    else
    {
      ret += L"*";
    }

    return ret;
  }
  else if(SymTagArrayType == tag)
  {
    CComPtr<IDiaSymbol> arrayBase;
    checkResult(type->get_type(&arrayBase));

    ULONGLONG total;
    checkResult(type->get_length(&total));

    ULONGLONG len;
    checkResult(arrayBase->get_length(&len));

    ret += expandType(arrayBase);
    ret += L"[" + toString((int)(total/len)) + L"]";//sometimes seems to be inaccurate

    return ret;
  }
  else if(SymTagUDT == tag)
  {
    UDT udt(type);
    ret = udt.getKind() + L" " + udt.getName();
    return ret;
  }
  else if(SymTagEnum == tag)
  {
    ret = L"enum " + getName(type);
    return ret;
  }
  else 
  {
    __asm int 3;//JC to do
  }

  return getName(type);
}

std::wstring expandBaseType(CComPtr<IDiaSymbol> type)
{
  std::wstring ret;
  if(!type)
  {
    return ret;
  }

  DWORD tag;
  checkResult(type->get_symTag(&tag));
  if(SymTagUDT == tag)
  {
    UDT udt(type);
    ret = udt.getKind() + L" " + udt.getName();
  }
  else if(SymTagPointerType == tag)
  {
    CComPtr<IDiaSymbol> pointee;
    checkResult(type->get_type(&pointee));

    DWORD pointeeTag;
    checkResult(pointee->get_symTag(&pointeeTag));
    if(SymTagUDT == pointeeTag)
    {
      UDT udt(pointee);
      ret = udt.getKind() + L" " + udt.getName();
    }
  }
  else if(SymTagBaseType == tag)
  {
    DWORD baseType;
    checkResult(type->get_baseType(&baseType));
    ret += getBasicTypeString(baseType);
    return ret;
  }
  else
  {
    std::wstringstream str;
    str << tag;
    str >> ret;
  }

  return ret;
}

void exploreSymbol3(CComPtr<IDiaSymbol> symbol) 
{
  std::wstring symbolName = ::getName(symbol);
  CComPtr<IDiaEnumSymbols> pIDiaEnumSymbols;
  checkResult(symbol->findChildren(SymTagNull,
    NULL,nsNone,&pIDiaEnumSymbols));

  if(!pIDiaEnumSymbols)
    return;

  LONG count;
  checkResult(pIDiaEnumSymbols->get_Count(&count));
  for(int i = 0; i < count; ++i)
  {
    CComPtr<IDiaSymbol> pIDiaSymbol;
    checkResult(pIDiaEnumSymbols->Item(i,&pIDiaSymbol));//baseType

    DWORD tagSym;
    checkResult(pIDiaSymbol->get_symTag(&tagSym));
    std::wstring name = ::getName(pIDiaSymbol);
    if(SymTagFriend == tagSym)
      __asm int 3;

  }
}

void exploreSymbol2(CComPtr<IDiaSymbol> symbol) 
{
  std::wstring symbolName = ::getName(symbol);
  CComPtr<IDiaEnumSymbols> pIDiaEnumSymbols;
  checkResult(symbol->findChildren(SymTagNull,
    NULL,nsNone,&pIDiaEnumSymbols));

  if(!pIDiaEnumSymbols)
    return;

  LONG count;
  checkResult(pIDiaEnumSymbols->get_Count(&count));
  for(int i = 0; i < count; ++i)
  {
    CComPtr<IDiaSymbol> pIDiaSymbol;
    checkResult(pIDiaEnumSymbols->Item(i,&pIDiaSymbol));//baseType

    DWORD tagSym;
    checkResult(pIDiaSymbol->get_symTag(&tagSym));
    std::wstring name = ::getName(pIDiaSymbol);
    if(SymTagFriend == tagSym)
      __asm int 3;

    exploreSymbol3(pIDiaSymbol);
  }
}

void exploreSymbol(CComPtr<IDiaSymbol> symbol) 
{
  std::wstring symbolName = ::getName(symbol);
  CComPtr<IDiaEnumSymbols> pIDiaEnumSymbols;
  checkResult(symbol->findChildren(SymTagNull,
    NULL,nsNone,&pIDiaEnumSymbols));

  if(!pIDiaEnumSymbols)
    return;

  LONG count;
  checkResult(pIDiaEnumSymbols->get_Count(&count));
  for(int i = 0; i < count; ++i)
  {
    CComPtr<IDiaSymbol> pIDiaSymbol;
    checkResult(pIDiaEnumSymbols->Item(i,&pIDiaSymbol));//baseType

    DWORD tagSym;
    checkResult(pIDiaSymbol->get_symTag(&tagSym));
    std::wstring name = ::getName(pIDiaSymbol);
    if(SymTagFriend == tagSym)
      __asm int 3;

    exploreSymbol2(pIDiaSymbol);
  }
}

