namespace tetra
{
namespace sqlite
{

template <typename Functor>
void SQLite::stepUntilDone( sqlite3_stmt* stmt, Functor fctn )
{
  auto exit = atExit( [=]()
                      {
                        sqlite3_reset( stmt );
                      } );

  while ( sqlite3_step( stmt ) == SQLITE_ROW )
  {
    fctn( stmt );
  }
}

template <typename Functor>
void SQLite::stepUntilDone( const std::string& query, Functor fctn )
{
  // the statement automatically finalizes when it goes out of scope
  auto stmt = prepareStatement( query );

  stepUntilDone( stmt.get(), fctn );
}

inline void SQLite::executeSQL( sqlite3_stmt* stmt )
{
  executeSQL( stmt, throwIfNotDone );
}

template <typename Functor>
void SQLite::executeSQL( sqlite3_stmt* stmt, Functor fctn )
{
  auto res = sqlite3_step( stmt );
  auto exit = atExit( [&]()
                      {
                        sqlite3_reset( stmt );
                      } );

  fctn( res );
}

template <class... Params>
void SQLite::bindSQL( sqlite3_stmt* stmt, Params... params )
{
  bindSQL_Impl( stmt, 1, params... );
}

template <class Param, class... Params>
void SQLite::bindSQL_Impl( sqlite3_stmt* stmt, int index, Param p,
                           Params... params )
{
  bindSQL_Impl( stmt, index, p );
  bindSQL_Impl( stmt, index + 1, params... );
}

}
} /* namespace tetra::sqlite */
