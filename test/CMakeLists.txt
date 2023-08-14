add_executable(spatial_test_grid grid.cpp)
target_compile_features(spatial_test_grid PUBLIC cxx_std_20)

find_package(ut REQUIRED)
target_link_libraries(spatial_test_grid PUBLIC spatial Boost::ut)