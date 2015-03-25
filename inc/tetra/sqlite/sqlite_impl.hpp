namespace tetra
{
namespace sqlite
{

template <typename Functor>
void SQLite::StepUntilDone( sqlite3_stmt* stmt, Functor fctn )
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
void SQLite::StepUntilDone( const std::string& query, Functor fctn )
{
  // the statement automatically finalizes when it goes out of scope
  auto stmt = PrepareStatement( query );

  StepUntilDone( stmt.get(), fctn );
}

inline void SQLite::ExecuteSQL( sqlite3_stmt* stmt )
{
  ExecuteSQL( stmt, ThrowIfNotDone );
}

template <typename Functor>
void SQLite::ExecuteSQL( sqlite3_stmt* stmt, Functor fctn )
{
  auto res = sqlite3_step( stmt );
  auto exit = atExit( [&]()
                      {
                        sqlite3_reset( stmt );
                      } );

  fctn( res );
}

template <class... Params>
void SQLite::BindSQL( sqlite3_stmt* stmt, Params... params )
{
  BindSQL_Impl( stmt, 1, params... );
}

template <class Param, class... Params>
void SQLite::BindSQL_Impl( sqlite3_stmt* stmt, int index, Param p,
                           Params... params )
{
  BindSQL_Impl( stmt, index, p );
  BindSQL_Impl( stmt, index + 1, params... );
}

}
} /* namespace tetra::sqlite */
