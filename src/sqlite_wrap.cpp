#include "sqlite_wrap.hpp"

using namespace std;

namespace tetra
{
namespace sqlite
{

ScopeExit::ScopeExit( std::function<void()> atExit )
  : m_atExit{atExit}
{
}

ScopeExit::~ScopeExit() { m_atExit(); }

Blob::Blob( const void* ptr, size_t len )
  : m_dataPtr_{ptr}, m_dataLength_{len}
{
}

SQLiteError::SQLiteError( const string& str ) : runtime_error{str} {}

void StatementFinalizer::operator()( sqlite3_stmt* stmt )
{
  sqlite3_finalize( stmt );
}

SQLite::SQLite( const std::string& dbFilename )
{
  if ( sqlite3_open( dbFilename.c_str(), &m_context_ ) != SQLITE_OK )
  {
    auto msg = string{"Could not open database. SQLite ErrMsg: "} +
               sqlite3_errmsg( m_context_ );

    sqlite3_close( m_context_ );
    m_context_ = nullptr;
    throw SQLiteError{msg};
  }
}

SQLite::~SQLite()
{
  sqlite3_close_v2( m_context_ );
  m_context_ = nullptr;
}

bool SQLite::TableExists( const std::string& tableName )
{
  auto tableExistsQuery =
    string{"SELECT count(*) FROM sqlite_master WHERE "
           "type='table' AND name='" +
           tableName + "';"};

  auto stmt = PrepareStatement( tableExistsQuery );

  auto res = sqlite3_step( stmt.get() );
  if ( res != SQLITE_ROW )
  {
    throw SQLiteError{"Query did not return any rows"};
  }

  auto exists = sqlite3_column_int( stmt.get(), 0 );
  return exists > 0;
}

void SQLite::ExecuteSQL( const std::string& stmt )
{
  auto res = sqlite3_exec( m_context_, stmt.c_str(), nullptr, nullptr,
                           nullptr );
  if ( res != SQLITE_OK )
  {
    throw SQLiteError{string{"Error in statement: "} + stmt +
                      "\n Error: " + sqlite3_errmsg( m_context_ )};
  }
}

void SQLite::DropTable( const std::string& tableName )
{
  auto dropCmd = string{"DROP TABLE IF EXISTS "} + tableName;

  ExecuteSQL( dropCmd );
}

void SQLite::DropTables( const std::vector<std::string>& tables )
{
  for ( const auto& tableName : tables )
  {
    DropTable( tableName );
  }
}

SQLite::SafeStmt SQLite::PrepareStatement( const std::string& stmt )
{
  sqlite3_stmt* pStmt{nullptr};

  const char* pTail{nullptr};
  auto res = sqlite3_prepare_v2( m_context_, stmt.c_str(), -1, &pStmt,
                                 &pTail );

  if ( res != SQLITE_OK )
  {
    auto msg = string{"Failed to prepare statement: "} + stmt + "\n" +
               "Error: " + sqlite3_errmsg( m_context_ );
    throw SQLiteError{msg};
  }

  return SafeStmt{pStmt};
}

void SQLite::ThrowIfNotDone( int result )
{
  if ( result != SQLITE_DONE )
  {
    throw SQLiteError{"Execution of statement did not complete"};
  }
}



void SQLite::BindSQL_Impl( sqlite3_stmt* stmt, int index, int i )
{
  if ( sqlite3_bind_int( stmt, index, i ) != SQLITE_OK )
  {
    throw SQLiteError{std::string{"Failed to bind integer: "} +
                      sqlite3_errmsg( m_context_ )};
  }
}

void SQLite::BindSQL_Impl( sqlite3_stmt* stmt, int index,
                           std::string str )
{
  auto res = sqlite3_bind_text( stmt, index, str.c_str(), -1,
                                SQLITE_TRANSIENT );

  if ( res != SQLITE_OK )
  {
    throw SQLiteError{std::string{"Failed to bind Text: "} +
                      sqlite3_errmsg( m_context_ )};
  }
}

void SQLite::BindSQL_Impl( sqlite3_stmt* stmt, int index,
                           const char* str )
{
  auto res =
    sqlite3_bind_text( stmt, index, str, -1, SQLITE_TRANSIENT );
  if ( res != SQLITE_OK )
  {
    throw SQLiteError{std::string{"Failed to bind Text: "} +
                      sqlite3_errmsg( m_context_ )};
  }
}

void SQLite::BindSQL_Impl( sqlite3_stmt* stmt, int index, double d )
{
  if ( sqlite3_bind_double( stmt, index, d ) != SQLITE_OK )
  {
    throw SQLiteError{std::string{"Failed to bind double: "} +
                      sqlite3_errmsg( m_context_ )};
  }
}

void SQLite::BindSQL_Impl( sqlite3_stmt* stmt, int index, float f )
{
  auto res =
    sqlite3_bind_double( stmt, index, static_cast<double>( f ) );

  if ( res != SQLITE_OK )
  {
    throw SQLiteError{std::string{"Failed to bind float: "} +
                      sqlite3_errmsg( m_context_ )};
  }
}

void SQLite::BindSQL_Impl( sqlite3_stmt* stmt, int index,
                           std::int64_t i )
{
  if ( sqlite3_bind_int64( stmt, index, i ) != SQLITE_OK )
  {
    throw SQLiteError{std::string{"Failed to bind integer: "} +
                      sqlite3_errmsg( m_context_ )};
  }
}

void SQLite::BindSQL_Impl( sqlite3_stmt* stmt, int index, Blob blb )
{
  auto res = sqlite3_bind_blob( stmt, index, blb.m_dataPtr_,
                                static_cast<int>( blb.m_dataLength_ ),
                                SQLITE_STATIC );

  if ( res != SQLITE_OK )
  {
    throw SQLiteError{std::string{"Failed to bind blob: "} +
                      sqlite3_errmsg( m_context_ )};
  }
}

}
} /* namespace tetra::sqlite */
