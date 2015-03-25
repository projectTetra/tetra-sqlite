#include <tetra/sqlite/sqlite.hpp>

#include <iostream>
#include <string>

using namespace std;
using namespace tetra;
using namespace tetra::sqlite;

void ClearDatabaseSchema( SQLite& db )
{
	db.dropTables(
	{
		"Entity_Table",
		"Family_Table",
		"FamilyMember_Table",
		"FamilyComponent_Table",
		"EntityComponent_Table"
	} );
}

void CreateDatabaseSchema( SQLite& db )
{
	db.executeSQL(
		"CREATE TABLE Entity_Table ("
			"EntityID INTEGER PRIMARY KEY AUTOINCREMENT,"
			"Test TEXT NOT NULL,"
			"Test2 INTEGER NOT NULL,"
			"Test3 BLOB"
			");"
		);

	db.executeSQL(
		"CREATE TABLE Family_Table ("
			"FamilyID INTEGER PRIMARY KEY AUTOINCREMENT,"
			"FamilyName TEXT NOT NULL"
			");"
		);

	db.executeSQL(
		"CREATE TABLE FamilyMember_Table("
			"PkID INTEGER PRIMARY KEY AUTOINCREMENT,"
			"FamilyID INTEGER NOT NULL,"
			"EntityID INTEGER NOT NULL,"
			"FOREIGN KEY (FamilyID) REFERENCES Family_Table (FamilyID),"
			"FOREIGN KEY (EntityID) REFERENCES Entity_Table (EntityID)"
			");"
		);

	db.executeSQL(
		"CREATE TABLE FamilyComponent_Table("
			"PkID INTEGER PRIMARY KEY AUTOINCREMENT,"
			"FamilyID INTEGER NOT NULL,"
			"ComponentID TEXT NOT NULL,"
			"FOREIGN KEY (FamilyID) REFERENCES Family_Table (FamilyID)"
			");"
		);

	db.executeSQL(
		"CREATE TABLE EntityComponent_Table("
			"PkID INTEGER PRIMARY KEY AUTOINCREMENT,"
			"EntityID INTEGER NOT NULL,"
			"ComponentID TEXT NOT NULL,"
			"ComponentData BLOB,"
			"FOREIGN KEY (EntityID) REFERENCES Entity_Table (EntityID)"
			");"
		);
}

int main()
{
	try
	{
		auto sqlite = SQLite { "first.db" };

		cout << sqlite.tableExists( "Entity_Table" ) << endl;
		ClearDatabaseSchema( sqlite );
		CreateDatabaseSchema( sqlite );

		auto stmt = sqlite.prepareStatement(
			"INSERT INTO Entity_Table VALUES (NULL, @Test, @Test2, @Test3);"
			);

		for (int i = 0; i < 10; i++)
		{
			sqlite.bindSQL(
				stmt.get(),
				to_string( i ) + " test",
				i,
				Blob { &i, sizeof(int) }
				);

			sqlite.executeSQL( stmt.get() );
		}

		sqlite.stepUntilDone(
			"SELECT Test, Test2, Test3 FROM Entity_Table;",
			[&]( sqlite3_stmt* select )
			{
				cout << "Test: " << sqlite3_column_text( select, 0 ) << "\t";
				cout << "Test2: " << sqlite3_column_text( select, 1 ) << "\t";
				auto* ptr = sqlite3_column_blob( select, 2 );
				auto len = sqlite3_column_bytes( select, 2 );

				cout << "Test3: " << *reinterpret_cast<const int*>(ptr);
				cout << " len { " << len << " } " << endl;
			} );

		cout << endl;
	}
	catch (SQLiteError& e)
	{
		cout << e.what() << endl;
	}

	return 0;
}
