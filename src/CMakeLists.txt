add_library(curl MODULE request.cc curl.cc)
target_link_libraries(curl xemmai CURL::libcurl)
set_property(TARGET curl PROPERTY PREFIX "")
