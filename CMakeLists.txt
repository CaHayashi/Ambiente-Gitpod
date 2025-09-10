cmake_minimum_required(VERSION 3.15)
project(notas_grpc_cpp)

find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)

set(CMAKE_CXX_STANDARD 17)

# Arquivos .proto gerados (ou gerados em build step)
# Assumindo que você já executou protoc e gerou notas.pb.cc e notas.grpc.pb.cc
set(PROTO_SRCS notas.pb.cc notas.grpc.pb.cc)
set(PROTO_HDRS notas.pb.h notas.grpc.pb.h)

add_library(notas_proto ${PROTO_SRCS} ${PROTO_HDRS})

target_link_libraries(notas_proto PUBLIC protobuf::libprotobuf gRPC::grpc++)

# Server
add_executable(notas_server server.cpp)
target_link_libraries(notas_server PRIVATE notas_proto protobuf::libprotobuf gRPC::grpc++)

# Client
add_executable(notas_client client.cpp)
target_link_libraries(notas_client PRIVATE notas_proto protobuf::libprotobuf gRPC::grpc++)
