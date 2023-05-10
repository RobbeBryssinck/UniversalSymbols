#include "DiaInterface.h"

#include <spdlog/spdlog.h>

#include <Windows.h>
#include <atlcomcli.h>
#include <dia2.h>

#include <stdexcept>
#include <memory>
#include <unordered_set>

namespace DiaInterface
{
  static CComPtr<IDiaSession> s_pSession = nullptr;
  static CComPtr<IDiaDataSource> s_pDataSource = nullptr;
  static CComPtr<IDiaSymbol> s_pGlobalScopeSymbol = nullptr;

  void Release()
  {
    if (s_pSession)
      s_pSession.Release();
    if (s_pDataSource)
      s_pDataSource.Release();

    CoUninitialize();
  }

  void InitializeDia(const char* apFileName)
  {
    CoInitialize(NULL);

    HRESULT result = CoCreateInstance(CLSID_DiaSource,
      NULL,
      CLSCTX_INPROC_SERVER,
      __uuidof(IDiaDataSource),
      (void**)&s_pDataSource);

    if (FAILED(result))
      throw std::runtime_error("CoCreateInstance failed.");

    int fileNameLength = MultiByteToWideChar(CP_UTF8, 0, apFileName, -1, NULL, 0);
    auto fileNameUnicode = std::make_unique<WCHAR[]>(fileNameLength);
    MultiByteToWideChar(CP_UTF8, 0, apFileName, -1, fileNameUnicode.get(), fileNameLength);

    if (FAILED(s_pDataSource->loadDataFromPdb(fileNameUnicode.get())))
      throw std::runtime_error("loadDataFromPdb failed.");

    if (FAILED(s_pDataSource->openSession(&s_pSession)))
      throw std::runtime_error("openSession failed.");

    if (FAILED(s_pSession->get_globalScope(&s_pGlobalScopeSymbol)))
      throw std::runtime_error("get_globalScope failed.");
  }

  std::string GetNameFromSymbol(IDiaSymbol* apSymbol)
  {
    std::string name = "";

    BSTR pwName = nullptr;
    if (apSymbol->get_name(&pwName) == S_OK)
    {
      auto nameLen = (size_t)SysStringLen(pwName) + 1;
      char* pName = new char[nameLen];
      size_t convertedCount = 0;
      wcstombs_s(&convertedCount, pName, nameLen, pwName, nameLen - 1); // -1 so the appended NULL doesn't fall outside the allocated buffer

      name = pName;

      delete[] pName;
      SysFreeString(pwName);
    }

    return name;
  }

  std::string GetBaseName(BasicType aType, size_t aSize)
  {
    switch (aType)
    {
    case BasicType::btChar:
      return "char";
    case BasicType::btWChar:
      return "wchar";
    case BasicType::btInt:
      switch (aSize)
      {
      case 1:
        return "int8_t";
      case 2:
        return "int16_t";
      case 4:
        return "int32_t";
      case 8:
        return "int64_t";
      default:
        return "int";
      }
    case BasicType::btUInt:
      switch (aSize)
      {
      case 1:
        return "uint8_t";
      case 2:
        return "uint16_t";
      case 4:
        return "uint32_t";
      case 8:
        return "uint64_t";
      default:
        return "uint";
      }
    case BasicType::btFloat:
      return "float";
    case BasicType::btBool:
      return "bool";
    case BasicType::btLong:
      return "long";
    case BasicType::btULong:
      return "unsigned long";
    case BasicType::btChar8:
      return "char8_t";
    case BasicType::btChar16:
      return "char16_t";
    case BasicType::btChar32:
      return "char32_t";
    case BasicType::btVoid:
    case BasicType::btBCD:
    case BasicType::btCurrency:
    case BasicType::btDate:
    case BasicType::btComplex:
    case BasicType::btBit:
    case BasicType::btBSTR:
    case BasicType::btHresult:
    default:
      return "void";
    }
  }

  std::string GeneratePointerName()
  {
    static size_t s_counter = 0;
    s_counter++;
    return std::format("pUnk{}", s_counter);
  }

  bool CreateTypeSymbol(USYM& aUsym, IDiaSymbol* apSymbol);

  std::optional<USYM::FieldSymbol> CreateFieldSymbol(USYM& aUsym, IDiaSymbol* apSymbol)
  {
    USYM::FieldSymbol symbol{};

    DWORD id = 0;
    if (apSymbol->get_symIndexId(&id) != S_OK)
      return std::nullopt;
    symbol.id = id;

    symbol.name = GetNameFromSymbol(apSymbol);

    LONG offset{};
    apSymbol->get_offset(&offset);
    symbol.offset = offset;

    CComPtr<IDiaSymbol> pFieldType = nullptr;
    if (apSymbol->get_type(&pFieldType) != S_OK || !pFieldType)
      return std::nullopt;

    DWORD fieldTypeId = 0;
    if (pFieldType->get_symIndexId(&fieldTypeId) != S_OK)
      return std::nullopt;

    symbol.underlyingTypeId = fieldTypeId;
    
    if (!aUsym.typeSymbols.contains(fieldTypeId))
    {
      bool createFieldSymbolResult = CreateTypeSymbol(aUsym, pFieldType);
      if (!createFieldSymbolResult)
      {
        DWORD symTag = 0;
        pFieldType->get_symTag(&symTag);
        spdlog::error("Failed to create field type symbol {}.", symTag);
        return std::nullopt;
      }
    }

    // isAnonymousUnion and unionId are not set yet on purpose, as it requires all fields to be defined first.

    return symbol;
  }

  std::optional<USYM::TypeSymbol> CreateBaseTypeSymbol(IDiaSymbol* apSymbol)
  {
    USYM::TypeSymbol symbol{};
    symbol.type = USYM::TypeSymbol::Type::kBase;

    DWORD id = 0;
    if (apSymbol->get_symIndexId(&id) != S_OK)
      return std::nullopt;
    symbol.id = id;

    DWORD baseType = 0;
    if (apSymbol->get_baseType(&baseType) != S_OK)
      return std::nullopt;

    ULONGLONG size = 0;
    if (apSymbol->get_length(&size) != S_OK)
      return std::nullopt;
    symbol.length = size;

    symbol.name = GetBaseName(static_cast<BasicType>(baseType), size);

    return symbol;
  }

  std::optional<USYM::TypeSymbol> CreateUDTSymbol(USYM& aUsym, IDiaSymbol* apSymbol)
  {
    USYM::TypeSymbol symbol{};

    DWORD udtKind = 0;
    if (apSymbol->get_udtKind(&udtKind) != S_OK)
      return std::nullopt;

    switch (udtKind)
    {
    case UdtKind::UdtStruct:
      symbol.type = USYM::TypeSymbol::Type::kStruct;
      break;
    case UdtKind::UdtClass:
      symbol.type = USYM::TypeSymbol::Type::kClass;
      break;
    case UdtKind::UdtUnion:
      symbol.type = USYM::TypeSymbol::Type::kUnion;
      break;
    case UdtKind::UdtInterface:
      symbol.type = USYM::TypeSymbol::Type::kInterface;
      break;
    default:
      symbol.type = USYM::TypeSymbol::Type::kUnknown;
    }

    DWORD id = 0;
    if (apSymbol->get_symIndexId(&id) != S_OK)
      return std::nullopt;
    symbol.id = id;

    symbol.name = GetNameFromSymbol(apSymbol);

    ULONGLONG length = 0;
    if (apSymbol->get_length(&length) != S_OK)
      return std::nullopt;
    symbol.length = length;

    CComPtr<IDiaEnumSymbols> pMemberEnum = nullptr;
    if (SUCCEEDED(apSymbol->findChildren(SymTagData, nullptr, nsNone, &pMemberEnum)))
    {
      LONG fieldCount = 0;
      pMemberEnum->get_Count(&fieldCount);
      symbol.fieldCount = fieldCount;
      symbol.fields.reserve(symbol.fieldCount);

      IDiaSymbol* rgelt = nullptr;
      ULONG pceltFetched = 0;

      while (SUCCEEDED(pMemberEnum->Next(1, &rgelt, &pceltFetched)) && (pceltFetched == 1))
      {
        CComPtr<IDiaSymbol> pField(rgelt);
        HRESULT result = 0;

        auto pFieldSymbol = CreateFieldSymbol(aUsym, pField);
        if (!pFieldSymbol)
        {
          spdlog::error("Failed to create field symbol for object {}.", symbol.name);
          // Push empty symbol to make sure the field count is still correct.
          symbol.fields.push_back(USYM::FieldSymbol());
          continue;
        }

        symbol.fields.push_back(*pFieldSymbol);
      }

      assert(symbol.fieldCount == symbol.fields.size());

      if (symbol.type != USYM::TypeSymbol::Type::kUnion)
      {
        auto count = symbol.fields.size();
        uint32_t unionId = 0;
        bool wasLastUnion = false;

        for (size_t i = 0; i < count; i++)
        {
          if (i + 1 == count)
            break;

          auto& current = symbol.fields[i];
          if (current.id == 0)
            continue;

          auto& next = symbol.fields[i + 1];
          if (next.id == 0)
            continue;

          if (current.offset == next.offset)
          {
            current.isAnonymousUnion = next.isAnonymousUnion = true;
            current.unionId = next.unionId = unionId;
            wasLastUnion = true;
          }
          else if (wasLastUnion)
          {
            unionId++;
            wasLastUnion = false;
          }
        }
      }
    }

    return symbol;
  }

  std::optional<USYM::TypeSymbol> CreateEnumSymbol(IDiaSymbol* apSymbol)
  {
    USYM::TypeSymbol symbol{};
    symbol.type = USYM::TypeSymbol::Type::kEnum;

    DWORD id = 0;
    if (apSymbol->get_symIndexId(&id) != S_OK)
      return std::nullopt;
    symbol.id = id;

    symbol.name = GetNameFromSymbol(apSymbol);

    ULONGLONG length = 0;
    if (apSymbol->get_length(&length) != S_OK)
      return std::nullopt;
    symbol.length = length;

    return symbol;
  }

  std::optional<USYM::TypeSymbol> CreatePointerTypeSymbol(IDiaSymbol* apSymbol)
  {
    USYM::TypeSymbol symbol{};
    symbol.type = USYM::TypeSymbol::Type::kPointer;

    DWORD id = 0;
    if (apSymbol->get_symIndexId(&id) != S_OK)
      return std::nullopt;
    symbol.id = id;

    symbol.name = GeneratePointerName();

    ULONGLONG length = 0;
    if (apSymbol->get_length(&length) != S_OK)
      return std::nullopt;
    symbol.length = length;

    return symbol;
  }

  std::optional<USYM::TypeSymbol> CreateTypedefSymbol(USYM& aUsym, IDiaSymbol* apSymbol)
  {
    USYM::TypeSymbol symbol{};
    symbol.type = USYM::TypeSymbol::Type::kTypedef;

    DWORD id = 0;
    if (apSymbol->get_symIndexId(&id) != S_OK)
      return std::nullopt;
    symbol.id = id;

    symbol.name = GetNameFromSymbol(apSymbol);

    IDiaSymbol* pUnderlyingType = nullptr;
    if (apSymbol->get_type(&pUnderlyingType) != S_OK)
      return std::nullopt;

    DWORD symTag = 0;
    pUnderlyingType->get_symTag(&symTag);

    if (symTag == SymTagFunctionType)
      symbol.length = 0;
    else
    {
      auto result = CreateTypeSymbol(aUsym, pUnderlyingType);

      ULONGLONG length = 0;
      if (pUnderlyingType->get_length(&length) != S_OK)
        return std::nullopt;
      symbol.length = length;

      DWORD typedefSource = 0;
      if (pUnderlyingType->get_symIndexId(&typedefSource) != S_OK)
        return std::nullopt;
      symbol.typedefSource = typedefSource;
    }

    return symbol;
  }

  std::optional<USYM::TypeSymbol> CreateArrayTypeSymbol(USYM& aUsym, IDiaSymbol* apSymbol)
  {
    USYM::TypeSymbol symbol{};
    symbol.type = USYM::TypeSymbol::Type::kArray;

    DWORD id = 0;
    if (apSymbol->get_symIndexId(&id) != S_OK)
      return std::nullopt;
    symbol.id = id;

    symbol.name = GetNameFromSymbol(apSymbol);

    ULONGLONG length = 0;
    if (apSymbol->get_length(&length) != S_OK)
      return std::nullopt;
    symbol.length = length;

    return symbol;
  }

  bool CreateTypeSymbol(USYM& aUsym, IDiaSymbol* apSymbol)
  {
    if (!apSymbol)
      return false;

    DWORD symTag = 0;
    HRESULT result = apSymbol->get_symTag(&symTag);
    if (result != S_OK)
      return false;

    std::optional<USYM::TypeSymbol> symbol{};

    switch (symTag)
    {
    case SymTagBaseType:
      symbol = CreateBaseTypeSymbol(apSymbol);
      break;
    case SymTagUDT:
      symbol = CreateUDTSymbol(aUsym, apSymbol);
      break;
    case SymTagEnum:
      symbol = CreateEnumSymbol(apSymbol);
      break;
    case SymTagPointerType:
      symbol = CreatePointerTypeSymbol(apSymbol);
      break;
    case SymTagTypedef:
      symbol = CreateTypedefSymbol(aUsym, apSymbol);
      break;
    case SymTagArrayType:
      symbol = CreateArrayTypeSymbol(aUsym, apSymbol);
      break;
    default:
      symbol = std::nullopt;
    }

    if (!symbol)
      return false;

    aUsym.typeSymbols[symbol->id] = *symbol;

    return true;
  }

  void BuildTypeList(USYM& aUsym, enum SymTagEnum aType)
  {
    CComPtr<IDiaEnumSymbols> pCurrentSymbol = nullptr;
    if (SUCCEEDED(s_pGlobalScopeSymbol->findChildren(aType, nullptr, nsNone, &pCurrentSymbol)))
    {
      IDiaSymbol* rgelt = nullptr;
      ULONG pceltFetched = 0;

      while (SUCCEEDED(pCurrentSymbol->Next(1, &rgelt, &pceltFetched)) && (pceltFetched == 1))
      {
        CComPtr<IDiaSymbol> pType(rgelt);

        bool result = CreateTypeSymbol(aUsym, pType);
        if (!result)
        {
          spdlog::error("Failed to create type symbol of type {}.", static_cast<int>(aType));
          continue;
        }
      }
    }
  }

  void BuildFunctionList(USYM& aUsym)
  {
    CComPtr<IDiaEnumSymbols> pCurrentSymbol = nullptr;
    if (SUCCEEDED(s_pGlobalScopeSymbol->findChildren(SymTagFunction, nullptr, nsNone, &pCurrentSymbol)))
    {
      IDiaSymbol* rgelt = nullptr;
      ULONG pceltFetched = 0;

      while (SUCCEEDED(pCurrentSymbol->Next(1, &rgelt, &pceltFetched)) && (pceltFetched == 1))
      {
        CComPtr<IDiaSymbol> pFunction(rgelt);
        HRESULT result = 0;
        
        DWORD id = 0;
        pFunction->get_symIndexId(&id);
        
        USYM::FunctionSymbol& symbol = aUsym.functionSymbols[id];
        
        symbol.id = id;

        symbol.name = GetNameFromSymbol(pFunction);

        IDiaSymbol* pFunctionType = nullptr;
        pFunction->get_type(&pFunctionType);
        if (!pFunctionType)
        {
          spdlog::warn("No function type for {}, skipping...", symbol.name);
          continue;
        }

        IDiaSymbol* pReturnType = nullptr;
        result = pFunctionType->get_type(&pReturnType);

        if (pReturnType && result == S_OK)
        {
          DWORD returnTypeId = 0;
          result = pReturnType->get_symIndexId(&returnTypeId);
          if (result == S_OK)
            symbol.returnTypeId = returnTypeId;

          if (!aUsym.typeSymbols.contains(returnTypeId))
          {
            bool result = CreateTypeSymbol(aUsym, pReturnType);
            if (!result)
            {
              DWORD symTag = 0;
              pReturnType->get_symTag(&symTag);
              spdlog::error("Failed to create return type symbol {}.", symTag);
            }
          }
        }

        DWORD argumentCount = 0;
        result = pFunctionType->get_count(&argumentCount);
        if (result == S_OK)
          symbol.argumentCount = argumentCount;

        if (symbol.argumentCount > 0)
        {
          // The 'this' pointer is not included in the FunctionArgType child list.
          IDiaSymbol* pThis = nullptr;
          result = pFunctionType->get_objectPointerType(&pThis);
          if (result == S_OK && pThis)
          {
            DWORD thisId = 0;
            pThis->get_symIndexId(&thisId);
            symbol.argumentTypeIds.push_back(thisId);

            if (!aUsym.typeSymbols.contains(thisId))
            {
              bool result = CreateTypeSymbol(aUsym, pThis);
              if (!result)
                spdlog::error("Failed to create 'this' argument symbol.");
            }
          }

          CComPtr<IDiaEnumSymbols> pEnumArgs = nullptr;
          if (SUCCEEDED(pFunctionType->findChildren(SymTagFunctionArgType, nullptr, nsNone, &pEnumArgs)))
          {
            IDiaSymbol* pCurrentArg = nullptr;
            ULONG argNum = 0;

            int position = 0;

            while (SUCCEEDED(pEnumArgs->Next(1, &pCurrentArg, &argNum)) && (argNum == 1))
            {
              CComPtr<IDiaSymbol> pCurrentArgManaged(pCurrentArg);

              IDiaSymbol* pArgumentType = nullptr;
              pCurrentArgManaged->get_type(&pArgumentType);

              // Weird edge case where some functions have a stub parameter (particularly dtors it seems).
              // Do note that variable argument functions use bNoType as the final parameter, hence the position check.
              DWORD symTag = 0;
              pArgumentType->get_symTag(&symTag);
              if (symTag == SymTagBaseType)
              {
                DWORD baseType = 0;
                pArgumentType->get_baseType(&baseType);
                if (baseType == btNoType && position == 0)
                  continue;
              }

              DWORD argTypeId = 0;
              pArgumentType->get_symIndexId(&argTypeId);
              symbol.argumentTypeIds.push_back(argTypeId);

              if (!aUsym.typeSymbols.contains(argTypeId))
              {
                bool result = CreateTypeSymbol(aUsym, pArgumentType);
                if (!result)
                  spdlog::error("Failed to create argument type symbol.");
              }

              position++;
            }
          }
        }

        assert(symbol.argumentCount == symbol.argumentTypeIds.size());

        DWORD callingConvention = 0;
        result = pFunctionType->get_callingConvention(&callingConvention);
        if (result == S_OK)
        {
          using CC = USYM::CallingConvention;
          switch (callingConvention)
          {
          case CV_call_e::CV_CALL_NEAR_C:
            symbol.callingConvention = CC::kNearC;
            break;
          case CV_call_e::CV_CALL_NEAR_FAST:
            symbol.callingConvention = CC::kNearFast;
            break;
          case CV_call_e::CV_CALL_NEAR_STD:
            symbol.callingConvention = CC::kNearStd;
            break;
          case CV_call_e::CV_CALL_NEAR_SYS:
            symbol.callingConvention = CC::kNearSys;
            break;
          case CV_call_e::CV_CALL_THISCALL:
            symbol.callingConvention = CC::kThiscall;
            break;
          case CV_call_e::CV_CALL_CLRCALL:
            symbol.callingConvention = CC::kCLRCall;
            break;
          default:
            symbol.callingConvention = CC::kUnknown;
          }
        }

        ULONGLONG virtualAddress = 0;
        result = pFunction->get_virtualAddress(&virtualAddress);
        if (result == S_OK)
          symbol.virtualAddress = virtualAddress;
      }
    }
  }

  void BuildHeader(USYM& aUsym)
  {
    aUsym.header.originalFormat = USYM::OriginalFormat::kPdb;
    
    DWORD machineType = 0;
    HRESULT result = s_pGlobalScopeSymbol->get_machineType(&machineType);
    if (result == S_OK)
    {
      switch (machineType)
      {
      case IMAGE_FILE_MACHINE_I386: // TODO: not sure about this one
        aUsym.header.architecture = USYM::Architecture::kX86;
        break;
      case IMAGE_FILE_MACHINE_AMD64:
        aUsym.header.architecture = USYM::Architecture::kX86_64;
        break;
      case IMAGE_FILE_MACHINE_ARM:
        aUsym.header.architecture = USYM::Architecture::kArm32;
        break;
      case IMAGE_FILE_MACHINE_ARM64:
        aUsym.header.architecture = USYM::Architecture::kArm64;
        break;
      default:
        aUsym.header.architecture = USYM::Architecture::kUnknown;
      }
    }
  }

  std::optional<USYM> CreateUsymFromFile(const char* apFileName)
  {
    try
    {
      InitializeDia(apFileName);
      
      USYM usym{};

      BuildHeader(usym);

      // TODO: anon structs
      // TODO: padding?
      BuildTypeList(usym, SymTagBaseType);
      BuildTypeList(usym, SymTagUDT);
      BuildTypeList(usym, SymTagEnum);
      BuildTypeList(usym, SymTagTypedef);
      BuildTypeList(usym, SymTagPointerType);

      BuildFunctionList(usym);

      Release();

      return usym;
    }
    catch (const std::exception& e)
    {
      spdlog::error("{}", e.what());
      Release();
      return std::nullopt;
    }
  }
}
