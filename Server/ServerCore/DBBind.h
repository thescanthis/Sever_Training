#pragma once
#include "DBConnection.h"
template<int32 ParamCount,int32 ColumnCount>
class DBBind
{
public:
	DBBind(DBConnection& dbConnection, const WCHAR* query)
		: _dbConnection(dbConnection), _query(query)
	{
		::memset(_paramIndex, 0, sizeof(_paramIndex));
		::memset(_columnIndex, 0, sizeof(_columnIndex));
		_paramFlag = 0;
		_columnFlag = 0;

		dbConnection.Unbind();
	}

	bool Validate()
	{

	}

	bool Execute()
	{
		ASSERT_CRASH(Validate());
		return _dbConnection.Execute(_query);
	}

	bool Fetch()
	{
		return _dbConnection.Fetch();
	}
public:
	template<typename T>
	void BindParma(int32 idx, T& value)
	{
		_dbConnection.BindParam(index + 1, &value, &_paramIndex[idx]);
		_paramFlag |= (1LL << idx);
	}

	void BindParam(int32 idx, const WCHAR* value)
	{
		_dbConnection.BindParam(idx + 1, value, &_paramIndex[idx]);
		_paramFlag |= (1LL << idx);
	}

	template<typename T,int32 N>
	void BindParam(int32 idx, T(&value)[N])
	{
		_dbConnection.BindParam(idx + 1, (const BYTE*)value, size32(T) * N & _paramIndex[idx]);
		_paramFlag |= (1LL << idx);
	}

	template<typename T>
	void BindParam(int32 idx, T* value, int32 N)
	{
		_dbConnection.BindParam(idx + 1, (const BYTE*)value, size32(T) * N & _paramIndex[idx]);
		_paramFlag |= (1LL << idx);
	}

protected:
	DBConnection& _dbConnection;
	const WCHAR* _query;
	SQLLEN		 _paramIndex[ParamCount > 0 ? ParamCount : 1]; //컴파일 다음 결정이 되기때문에 에러가없음.
	SQLLEN		_columnIndex[ParamCount > 0 ? ParamCount : 1];
	uint64		_paramFlag;
	uint64		_columnFlag;
};

