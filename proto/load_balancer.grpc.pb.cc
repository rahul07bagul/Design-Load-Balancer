// Generated by the gRPC C++ plugin.
// If you make any local change, they will be lost.
// source: proto/load_balancer.proto

#include "proto/load_balancer.pb.h"
#include "proto/load_balancer.grpc.pb.h"

#include <functional>
#include <grpcpp/support/async_stream.h>
#include <grpcpp/support/async_unary_call.h>
#include <grpcpp/impl/channel_interface.h>
#include <grpcpp/impl/client_unary_call.h>
#include <grpcpp/support/client_callback.h>
#include <grpcpp/support/message_allocator.h>
#include <grpcpp/support/method_handler.h>
#include <grpcpp/impl/rpc_service_method.h>
#include <grpcpp/support/server_callback.h>
#include <grpcpp/impl/server_callback_handlers.h>
#include <grpcpp/server_context.h>
#include <grpcpp/impl/service_type.h>
#include <grpcpp/support/sync_stream.h>
namespace loadbalancer {

static const char* LoadBalancerService_method_names[] = {
  "/loadbalancer.LoadBalancerService/HandleRequest",
};

std::unique_ptr< LoadBalancerService::Stub> LoadBalancerService::NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options) {
  (void)options;
  std::unique_ptr< LoadBalancerService::Stub> stub(new LoadBalancerService::Stub(channel, options));
  return stub;
}

LoadBalancerService::Stub::Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options)
  : channel_(channel), rpcmethod_HandleRequest_(LoadBalancerService_method_names[0], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  {}

::grpc::Status LoadBalancerService::Stub::HandleRequest(::grpc::ClientContext* context, const ::loadbalancer::Request& request, ::loadbalancer::Response* response) {
  return ::grpc::internal::BlockingUnaryCall< ::loadbalancer::Request, ::loadbalancer::Response, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_HandleRequest_, context, request, response);
}

void LoadBalancerService::Stub::async::HandleRequest(::grpc::ClientContext* context, const ::loadbalancer::Request* request, ::loadbalancer::Response* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::loadbalancer::Request, ::loadbalancer::Response, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_HandleRequest_, context, request, response, std::move(f));
}

void LoadBalancerService::Stub::async::HandleRequest(::grpc::ClientContext* context, const ::loadbalancer::Request* request, ::loadbalancer::Response* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_HandleRequest_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::loadbalancer::Response>* LoadBalancerService::Stub::PrepareAsyncHandleRequestRaw(::grpc::ClientContext* context, const ::loadbalancer::Request& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::loadbalancer::Response, ::loadbalancer::Request, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_HandleRequest_, context, request);
}

::grpc::ClientAsyncResponseReader< ::loadbalancer::Response>* LoadBalancerService::Stub::AsyncHandleRequestRaw(::grpc::ClientContext* context, const ::loadbalancer::Request& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncHandleRequestRaw(context, request, cq);
  result->StartCall();
  return result;
}

LoadBalancerService::Service::Service() {
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      LoadBalancerService_method_names[0],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< LoadBalancerService::Service, ::loadbalancer::Request, ::loadbalancer::Response, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](LoadBalancerService::Service* service,
             ::grpc::ServerContext* ctx,
             const ::loadbalancer::Request* req,
             ::loadbalancer::Response* resp) {
               return service->HandleRequest(ctx, req, resp);
             }, this)));
}

LoadBalancerService::Service::~Service() {
}

::grpc::Status LoadBalancerService::Service::HandleRequest(::grpc::ServerContext* context, const ::loadbalancer::Request* request, ::loadbalancer::Response* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}


}  // namespace loadbalancer

