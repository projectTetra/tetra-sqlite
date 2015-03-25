import os;
import os.path;

env = Environment();
env['CPPPATH'] = ['./inc'];

env['LIBPATH'] = ['./bin'];

env['CXX'] = 'clang++';
env['CXXFLAGS'] = [ '-std=c++11', '-ggdb' ];

buildLib = env.Library( './bin/tetraSqlite',
                        Glob( 'src/*.cpp' ) + Glob( 'src/*.c' ) );

env['LIBS'] = [ 'tetraSqlite', 'pthread', 'dl' ];

demo = env.Program( './bin/demo.out', Glob( './demo/*.cpp' ) );

Depends( demo, buildLib );

runDemo = Command( target = "runDemo"
                 , source = "./bin/demo.out"
                 , action = [ "./bin/demo.out" ]
                 );

Depends( runDemo, demo );
Default( runDemo );


