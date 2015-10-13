//useful.h

std::wstring getName(CComPtr<IDiaSymbol> sym);
std::wstring getKind(CComPtr<IDiaSymbol> sym);

std::wstring removeLeading(std::wstring str);

std::wstring expandType(CComPtr<IDiaSymbol> type, const std::wstring& name = L"");
std::wstring getBasicTypeString(DWORD basicType);
std::wstring expandBaseType(CComPtr<IDiaSymbol> type);

void exploreSymbol(CComPtr<IDiaSymbol> symbol);

std::wstring expandTypeSpecial(CComPtr<IDiaSymbol> type);