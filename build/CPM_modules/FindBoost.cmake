include("C:/Users/HP/Cpp_projects/LDLT_server/build/cmake/CPM_0.40.2.cmake")
CPMAddPackage("NAME;Boost;VERSION;1.87.0;URL;https://github.com/boostorg/boost/releases/download/boost-1.87.0/boost-1.87.0-cmake.tar.gz;OPTIONS;BOOST_ENABLE_CMAKE ON;BOOST_INCLUDE_LIBRARIES multiprecision")
set(Boost_FOUND TRUE)