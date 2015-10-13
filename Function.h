//Function

class Function
{
public:
  Function();
  Function(CComPtr<IDiaSymbol> pIDiaSymbol);
  Function(const Function& other);
  Function& operator= (const Function& other);
  bool operator== (const Function& other);
/*
  struct Predicate : public std::binary_function <Function, Function, bool>
  {
    bool operator() (const Function& lhs, const Function& rhs)
    {
      return lhs.id() < rhs.id();
    }
  };
*/
public:
  std::wstring getName() const;
  bool isVirtual() const;
  bool isPure() const;
  bool isCompilerGenerated() const;
  bool isConst() const;
  bool isStatic() const;

  bool isCtor() const;
  bool isDtor() const;
  bool reservedName() const;

  unsigned int getRVA() const;

  std::wstring returnType() const;
  std::wstring returnTypeSpecial() const;
  std::wstring callingConvention() const;

  std::list<std::wstring> getArgs() const;
  CV_access_e protection() const;

  std::list<std::wstring> getLocalVars() const;

  std::list<std::wstring> getArgsSpecial() const;

private:
  CComPtr<IDiaSymbol> sym() const;
  CComPtr<IDiaSymbol> functionType() const;
  CComPtr<IDiaSymbol> objectPointer() const;

  std::list<std::wstring> Function::_getArgs() const;

private:
  CComPtr<IDiaSymbol> Symbol;
};

