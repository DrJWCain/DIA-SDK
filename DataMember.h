//DataMember

class DataMember
{
public:
  DataMember();
  DataMember(CComPtr<IDiaSymbol> pIDiaSymbol);
  DataMember(const DataMember& other);
  DataMember& operator= (const DataMember& other);
  bool operator== (const DataMember& other);

/*
  struct Predicate : public std::binary_function <DataMember, DataMember, bool>
  {
    bool operator() (const DataMember& lhs, const DataMember& rhs)
    {
      return lhs.id() < rhs.id();
    }
  };
*/
public:
  std::wstring getName() const;
  std::wstring type() const;
  std::wstring typeSpecial() const;

  bool isStatic() const;

  CV_access_e protection() const;
  bool pointerToFunction() const;

private:
  CComPtr<IDiaSymbol> sym() const;

private:
  CComPtr<IDiaSymbol> Symbol;
};

