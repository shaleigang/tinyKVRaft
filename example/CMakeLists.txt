add_executable(rpc_server server_test.cpp)
target_link_libraries(rpc_server raft tmuduo_rpc ${PROTOBUF_LIBRARIES} tmuduo_net tmuduo_base pthread)

add_executable(rpc_client client_test.cpp)
target_link_libraries(rpc_client raft tmuduo_rpc ${PROTOBUF_LIBRARIES} tmuduo_net tmuduo_base pthread)