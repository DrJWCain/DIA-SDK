//UDT

class DataMember;
class Function;
class UDT
{
public:
  UDT();
  explicit UDT(const std::wstring name);
  UDT(CComPtr<IDiaSymbol> pIDiaSymbol);
  UDT(const UDT& other);

  struct Predicate : public std::binary_function <UDT, UDT, bool>
  {
    bool operator() (const UDT& lhs, const UDT& rhs)
    {
      return lhs.getName() < rhs.getName();
    }
  };

public:
  std::wstring getName() const;
  std::wstring getKind() const;

  bool nested() const;

  //The Bases type can represent the entire base class heirarchy for a type.
  //The second and third parameters represent depth and protection respectively.
  //See printBaseList in DIA3.cpp for example usage.
  typedef std::vector<std::pair<UDT,std::pair<int,int> > > Bases;
  Bases getBases(int depth = 1) const;

  typedef std::vector<DataMember> DataMembers;
  DataMembers getDataMembers() const;

  typedef std::vector<Function> Functions;
  Functions getFunctions() const;

  typedef std::vector<UDT> Nested;
  Nested getNested() const;

  std::wstring getTypedefs() const;
  std::wstring getEnums() const;
//  std::wstring getFriends() const;

  CV_access_e getDefaultProtection() const;

private:
  CComPtr<IDiaSymbol> sym() const;
  CComPtr<IDiaEnumSymbols> children(enum SymTagEnum type) const;

private:
  CComPtr<IDiaSymbol> Symbol;
  mutable std::wstring Name;
};

