#include <string>

#include <grpcpp/grpcpp.h>
#include "chesscom.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using chesscom::ChessCom;
using chesscom::MathRequest;
using chesscom::MathReply;

class ChessComClient {
    public:
        ChessComClient(std::shared_ptr<Channel> channel) : stub_(ChessCom::NewStub(channel)) {}

    int sendRequest(int a, int b) {
        MathRequest request;

        request.set_a(a);
        request.set_b(b);

        MathReply reply;

        ClientContext context;

        Status status = stub_->sendRequest(&context, request, &reply);

        if(status.ok()){
            return reply.result();
        } else {
            std::cout << status.error_code() << ": " << status.error_message() << std::endl;
            return -1;
        }
    }

    private:
        std::unique_ptr<ChessCom::Stub> stub_;
};

void Run() {
    std::string address("0.0.0.0:43326");
    ChessComClient client(
        grpc::CreateChannel(
            address, 
            grpc::InsecureChannelCredentials()
        )
    );

    int response;

    int a = 5;
    int b = 10;

    response = client.sendRequest(a, b);
    std::cout << "Answer received: " << a << " * " << b << " = " << response << std::endl;
}

int main(int argc, char* argv[]){
    Run();

    return 0;
}
