cmake_minimum_required(VERSION 4.2.1)
project(SNAKE)

set(CMAKE_CXX_STANDARD 20)

find_package(SDL3 REQUIRED)
find_package(SDL3_ttf REQUIRED)
find_package(SDL3_image REQUIRED)

add_executable(main main.cpp)

target_link_libraries(main PRIVATE
   SDL3::SDL3
   SDL3_ttf::SDL3_ttf
   SDL3_image::SDL3_image
)
 
