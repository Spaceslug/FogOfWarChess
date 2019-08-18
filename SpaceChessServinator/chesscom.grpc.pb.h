// Generated by the gRPC C++ plugin.
// If you make any local change, they will be lost.
// source: chesscom.proto
#ifndef GRPC_chesscom_2eproto__INCLUDED
#define GRPC_chesscom_2eproto__INCLUDED

#include "chesscom.pb.h"

#include <functional>
#include <grpcpp/impl/codegen/async_generic_service.h>
#include <grpcpp/impl/codegen/async_stream.h>
#include <grpcpp/impl/codegen/async_unary_call.h>
#include <grpcpp/impl/codegen/client_callback.h>
#include <grpcpp/impl/codegen/method_handler_impl.h>
#include <grpcpp/impl/codegen/proto_utils.h>
#include <grpcpp/impl/codegen/rpc_method.h>
#include <grpcpp/impl/codegen/server_callback.h>
#include <grpcpp/impl/codegen/service_type.h>
#include <grpcpp/impl/codegen/status.h>
#include <grpcpp/impl/codegen/stub_options.h>
#include <grpcpp/impl/codegen/sync_stream.h>

namespace grpc {
class CompletionQueue;
class Channel;
class ServerCompletionQueue;
class ServerContext;
}  // namespace grpc

namespace chesscom {

// Defines the service
class ChessCom final {
 public:
  static constexpr char const* service_full_name() {
    return "chesscom.ChessCom";
  }
  class StubInterface {
   public:
    virtual ~StubInterface() {}
    // Function invoked to send the request
    virtual ::grpc::Status sendRequest(::grpc::ClientContext* context, const ::chesscom::MathRequest& request, ::chesscom::MathReply* response) = 0;
    std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::chesscom::MathReply>> AsyncsendRequest(::grpc::ClientContext* context, const ::chesscom::MathRequest& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::chesscom::MathReply>>(AsyncsendRequestRaw(context, request, cq));
    }
    std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::chesscom::MathReply>> PrepareAsyncsendRequest(::grpc::ClientContext* context, const ::chesscom::MathRequest& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::chesscom::MathReply>>(PrepareAsyncsendRequestRaw(context, request, cq));
    }
    class experimental_async_interface {
     public:
      virtual ~experimental_async_interface() {}
      // Function invoked to send the request
      virtual void sendRequest(::grpc::ClientContext* context, const ::chesscom::MathRequest* request, ::chesscom::MathReply* response, std::function<void(::grpc::Status)>) = 0;
      virtual void sendRequest(::grpc::ClientContext* context, const ::grpc::ByteBuffer* request, ::chesscom::MathReply* response, std::function<void(::grpc::Status)>) = 0;
    };
    virtual class experimental_async_interface* experimental_async() { return nullptr; }
  private:
    virtual ::grpc::ClientAsyncResponseReaderInterface< ::chesscom::MathReply>* AsyncsendRequestRaw(::grpc::ClientContext* context, const ::chesscom::MathRequest& request, ::grpc::CompletionQueue* cq) = 0;
    virtual ::grpc::ClientAsyncResponseReaderInterface< ::chesscom::MathReply>* PrepareAsyncsendRequestRaw(::grpc::ClientContext* context, const ::chesscom::MathRequest& request, ::grpc::CompletionQueue* cq) = 0;
  };
  class Stub final : public StubInterface {
   public:
    Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel);
    ::grpc::Status sendRequest(::grpc::ClientContext* context, const ::chesscom::MathRequest& request, ::chesscom::MathReply* response) override;
    std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::chesscom::MathReply>> AsyncsendRequest(::grpc::ClientContext* context, const ::chesscom::MathRequest& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::chesscom::MathReply>>(AsyncsendRequestRaw(context, request, cq));
    }
    std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::chesscom::MathReply>> PrepareAsyncsendRequest(::grpc::ClientContext* context, const ::chesscom::MathRequest& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::chesscom::MathReply>>(PrepareAsyncsendRequestRaw(context, request, cq));
    }
    class experimental_async final :
      public StubInterface::experimental_async_interface {
     public:
      void sendRequest(::grpc::ClientContext* context, const ::chesscom::MathRequest* request, ::chesscom::MathReply* response, std::function<void(::grpc::Status)>) override;
      void sendRequest(::grpc::ClientContext* context, const ::grpc::ByteBuffer* request, ::chesscom::MathReply* response, std::function<void(::grpc::Status)>) override;
     private:
      friend class Stub;
      explicit experimental_async(Stub* stub): stub_(stub) { }
      Stub* stub() { return stub_; }
      Stub* stub_;
    };
    class experimental_async_interface* experimental_async() override { return &async_stub_; }

   private:
    std::shared_ptr< ::grpc::ChannelInterface> channel_;
    class experimental_async async_stub_{this};
    ::grpc::ClientAsyncResponseReader< ::chesscom::MathReply>* AsyncsendRequestRaw(::grpc::ClientContext* context, const ::chesscom::MathRequest& request, ::grpc::CompletionQueue* cq) override;
    ::grpc::ClientAsyncResponseReader< ::chesscom::MathReply>* PrepareAsyncsendRequestRaw(::grpc::ClientContext* context, const ::chesscom::MathRequest& request, ::grpc::CompletionQueue* cq) override;
    const ::grpc::internal::RpcMethod rpcmethod_sendRequest_;
  };
  static std::unique_ptr<Stub> NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options = ::grpc::StubOptions());

  class Service : public ::grpc::Service {
   public:
    Service();
    virtual ~Service();
    // Function invoked to send the request
    virtual ::grpc::Status sendRequest(::grpc::ServerContext* context, const ::chesscom::MathRequest* request, ::chesscom::MathReply* response);
  };
  template <class BaseClass>
  class WithAsyncMethod_sendRequest : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service *service) {}
   public:
    WithAsyncMethod_sendRequest() {
      ::grpc::Service::MarkMethodAsync(0);
    }
    ~WithAsyncMethod_sendRequest() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status sendRequest(::grpc::ServerContext* context, const ::chesscom::MathRequest* request, ::chesscom::MathReply* response) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestsendRequest(::grpc::ServerContext* context, ::chesscom::MathRequest* request, ::grpc::ServerAsyncResponseWriter< ::chesscom::MathReply>* response, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncUnary(0, context, request, response, new_call_cq, notification_cq, tag);
    }
  };
  typedef WithAsyncMethod_sendRequest<Service > AsyncService;
  template <class BaseClass>
  class ExperimentalWithCallbackMethod_sendRequest : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service *service) {}
   public:
    ExperimentalWithCallbackMethod_sendRequest() {
      ::grpc::Service::experimental().MarkMethodCallback(0,
        new ::grpc::internal::CallbackUnaryHandler< ::chesscom::MathRequest, ::chesscom::MathReply>(
          [this](::grpc::ServerContext* context,
                 const ::chesscom::MathRequest* request,
                 ::chesscom::MathReply* response,
                 ::grpc::experimental::ServerCallbackRpcController* controller) {
                   return this->sendRequest(context, request, response, controller);
                 }));
    }
    ~ExperimentalWithCallbackMethod_sendRequest() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status sendRequest(::grpc::ServerContext* context, const ::chesscom::MathRequest* request, ::chesscom::MathReply* response) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    virtual void sendRequest(::grpc::ServerContext* context, const ::chesscom::MathRequest* request, ::chesscom::MathReply* response, ::grpc::experimental::ServerCallbackRpcController* controller) { controller->Finish(::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "")); }
  };
  typedef ExperimentalWithCallbackMethod_sendRequest<Service > ExperimentalCallbackService;
  template <class BaseClass>
  class WithGenericMethod_sendRequest : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service *service) {}
   public:
    WithGenericMethod_sendRequest() {
      ::grpc::Service::MarkMethodGeneric(0);
    }
    ~WithGenericMethod_sendRequest() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status sendRequest(::grpc::ServerContext* context, const ::chesscom::MathRequest* request, ::chesscom::MathReply* response) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
  };
  template <class BaseClass>
  class WithRawMethod_sendRequest : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service *service) {}
   public:
    WithRawMethod_sendRequest() {
      ::grpc::Service::MarkMethodRaw(0);
    }
    ~WithRawMethod_sendRequest() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status sendRequest(::grpc::ServerContext* context, const ::chesscom::MathRequest* request, ::chesscom::MathReply* response) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestsendRequest(::grpc::ServerContext* context, ::grpc::ByteBuffer* request, ::grpc::ServerAsyncResponseWriter< ::grpc::ByteBuffer>* response, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncUnary(0, context, request, response, new_call_cq, notification_cq, tag);
    }
  };
  template <class BaseClass>
  class ExperimentalWithRawCallbackMethod_sendRequest : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service *service) {}
   public:
    ExperimentalWithRawCallbackMethod_sendRequest() {
      ::grpc::Service::experimental().MarkMethodRawCallback(0,
        new ::grpc::internal::CallbackUnaryHandler< ::grpc::ByteBuffer, ::grpc::ByteBuffer>(
          [this](::grpc::ServerContext* context,
                 const ::grpc::ByteBuffer* request,
                 ::grpc::ByteBuffer* response,
                 ::grpc::experimental::ServerCallbackRpcController* controller) {
                   this->sendRequest(context, request, response, controller);
                 }));
    }
    ~ExperimentalWithRawCallbackMethod_sendRequest() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status sendRequest(::grpc::ServerContext* context, const ::chesscom::MathRequest* request, ::chesscom::MathReply* response) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    virtual void sendRequest(::grpc::ServerContext* context, const ::grpc::ByteBuffer* request, ::grpc::ByteBuffer* response, ::grpc::experimental::ServerCallbackRpcController* controller) { controller->Finish(::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "")); }
  };
  template <class BaseClass>
  class WithStreamedUnaryMethod_sendRequest : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service *service) {}
   public:
    WithStreamedUnaryMethod_sendRequest() {
      ::grpc::Service::MarkMethodStreamed(0,
        new ::grpc::internal::StreamedUnaryHandler< ::chesscom::MathRequest, ::chesscom::MathReply>(std::bind(&WithStreamedUnaryMethod_sendRequest<BaseClass>::StreamedsendRequest, this, std::placeholders::_1, std::placeholders::_2)));
    }
    ~WithStreamedUnaryMethod_sendRequest() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable regular version of this method
    ::grpc::Status sendRequest(::grpc::ServerContext* context, const ::chesscom::MathRequest* request, ::chesscom::MathReply* response) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    // replace default version of method with streamed unary
    virtual ::grpc::Status StreamedsendRequest(::grpc::ServerContext* context, ::grpc::ServerUnaryStreamer< ::chesscom::MathRequest,::chesscom::MathReply>* server_unary_streamer) = 0;
  };
  typedef WithStreamedUnaryMethod_sendRequest<Service > StreamedUnaryService;
  typedef Service SplitStreamedService;
  typedef WithStreamedUnaryMethod_sendRequest<Service > StreamedService;
};

}  // namespace chesscom


#endif  // GRPC_chesscom_2eproto__INCLUDED
