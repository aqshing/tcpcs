#pragma once
// #ifndef _MY_SQLITE_HPP_
// #define _MY_SQLITE_HPP_

#include <cstdio>
#include "sqlite3.h"
#include "debug.h"

namespace unused {
int callback(void* unused, int argc, char** argv, char** azColName) {
	for(int i=0; i<argc; i++){
	//	printf("%s = %s ", azColName[i], argv[i] ? argv[i] : "null");
		printf("%s = %s ", azColName[i], argv[i]);
	}
	printf("\n");
	return SQLITE_OK;
}
};

class CSQLITE3 {
public:
	CSQLITE3(const char* filename):m_db(nullptr) {
		auto ret = sqlite3_open(filename, &m_db);
		if (ret != SQLITE_OK) {
			COUT("Can't open database: %s\n", sqlite3_errmsg(m_db));
			return;
		}
	}

	~CSQLITE3() {
		if (m_db) {
			//TAG();
			sqlite3_close(m_db);
			m_db = nullptr;
		}
	}

	void RUNSQL(const char* sql, sqlite3_callback xCallback = nullptr, void* pArg = nullptr) {
		char* pzErrMsg = nullptr;

		auto ret = sqlite3_exec(m_db, sql, xCallback, pArg, &pzErrMsg);
		if( ret != SQLITE_OK ){
			COUT("SQL error: %s\n", pzErrMsg);
		}
		sqlite3_free(pzErrMsg);
	}

	void GETTABLE(const char* sql) {
		char* pzErrMsg = nullptr;
		char** result = nullptr;
		int row, col, index = 0;
		auto ret = sqlite3_get_table(m_db, sql, &result, &row, &col, &pzErrMsg);
		if( ret != SQLITE_OK ){
			COUT("SQL error: %s\n", pzErrMsg);
			sqlite3_free(pzErrMsg);
		}
		COUT("row = %d, col = %d\n", row, col);
		printf("%s = %s \n", result[0], result[col]);
		for (ssize_t i = 0; i < row; i++) {
			for (ssize_t j = 0; j <= col; j++) {
				printf("%s = %s ", result[j], result[index++]);
			}
			printf("\n");
		}

		sqlite3_free_table(result);
	}

private:
	sqlite3* m_db;
};
using CSQLITE = CSQLITE3;
// #endif