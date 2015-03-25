#pragma once
#ifndef TETRA_SQLITE_SQLITE
#define TETRA_SQLITE_SQLITE

#include "sqlite3.h"

#include <vector>
#include <stdexcept>
#include <memory>
#include <cstdint>

namespace tetra
{
namespace sqlite
{

template <class T>
struct ScopeExit
{
  inline ScopeExit( T fun ) : fun{fun} {};

  ~ScopeExit() { fun(); };

private:
  T fun;
};

template <class T>
ScopeExit<T> atExit( T fun )
{
  return ScopeExit<T>( fun );
};

struct Blob
{
  Blob() = default;
  Blob( const void* ptr, std::size_t len );

  const void* m_dataPtr_{nullptr};
  std::size_t m_dataLength_{0};
};

struct SQLiteError : public std::runtime_error
{
  SQLiteError( const std::string& str );
};

struct StatementFinalizer
{
  void operator()( sqlite3_stmt* stmt );
};

/**
 * A simple and light wrapper around some of sqlite3's functionality.
 **/
class SQLite
{
public:
  using SafeStmt = std::unique_ptr<sqlite3_stmt, StatementFinalizer>;

public:
  SQLite( const std::string& dbFilename );
  ~SQLite();

  bool tableExists( const std::string& tableName );
  void dropTable( const std::string& tableName );
  void dropTables( const std::vector<std::string>& tableNames );

  /**
   * Prepares the given statement for execution, this allows faster
   * satement processing as the statement only needs to be parsed
   *once.
   **/
  SafeStmt prepareStatement( const std::string& stmt );

  /**
   * Executes the given functor on the statement until there are no
   *more
   * rows left in the result.
   **/
  template <typename Functor>
  void stepUntilDone( sqlite3_stmt* stmt, Functor fctn );

  template <typename Functor>
  void stepUntilDone( const std::string& query, Functor fctn );

  /**
   * Executes the given sql statement a single time and resets its
   *virtual
   * machine, does not process the results. Throws an SQLiteError if
   * the statement is not done after being stepped once.
   **/
  void executeSQL( const std::string& stmt );
  void executeSQL( sqlite3_stmt* stmt );

  /**
   * Executes the sql statetment and then executes the functor before
   * resetting the statements's virtual machine.
   **/
  template <typename Functor>
  void executeSQL( sqlite3_stmt* stmt, Functor fctn );

  /**
   * Typesafe means of binding a value into a prepared sql statetment.
   * The first parameter after the statement corresponds to the first
   * value to be bound, the second corresponds to the second, etc...
   **/
  template <class... Params>
  void bindSQL( sqlite3_stmt* stmt, Params... params );

  static void throwIfNotDone( int result );

private:
  sqlite3* m_context_{nullptr};

private:
  void bindSQL_Impl( sqlite3_stmt* stmt, int index, int p );
  void bindSQL_Impl( sqlite3_stmt* stmt, int index, std::string p );
  void bindSQL_Impl( sqlite3_stmt* stmt, int index, const char* p );
  void bindSQL_Impl( sqlite3_stmt* stmt, int index, double p );
  void bindSQL_Impl( sqlite3_stmt* stmt, int index, float p );
  void bindSQL_Impl( sqlite3_stmt* stmt, int index, std::int64_t p );
  void bindSQL_Impl( sqlite3_stmt* stmt, int index, Blob p );

  template <class Param, class... Params>
  void bindSQL_Impl( sqlite3_stmt* stmt, int index, Param p,
                     Params... params );

}; /* class SQLite */
}
} /* namespace tetra::sqlite */

#include "sqlite_impl.hpp"
#endif
