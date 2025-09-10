// client.cpp
#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>
#include "notas.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using gerencia_notas::GerenciadorNotas;
using gerencia_notas::AdicionaNotaRequest;
using gerencia_notas::StatusResponse;
using gerencia_notas::AlunoRequest;
using gerencia_notas::Nota;

class GerenciadorNotasClient {
public:
    GerenciadorNotasClient(std::shared_ptr<Channel> channel)
        : stub_(GerenciadorNotas::NewStub(channel)) {}

    void AdicionarNota(const std::string& ra, const std::string& cod, int ano, int sem, float nota) {
        AdicionaNotaRequest req;
        req.set_ra(ra); req.set_cod_disciplina(cod); req.set_ano(ano); req.set_semestre(sem); req.set_nota(nota);
        StatusResponse resp;
        ClientContext ctx;
        Status status = stub_->AdicionarNota(&ctx, req, &resp);
        if (status.ok()) {
            std::cout << "AdicionarNota: " << resp.msg() << " (sucesso=" << resp.sucesso() << ")\n";
        } else {
            std::cout << "RPC falhou: " << status.error_message() << "\n";
        }
    }

    void ListarNotasAluno(const std::string& ra) {
        AlunoRequest req; req.set_ra(ra);
        ClientContext ctx;
        std::unique_ptr<grpc::ClientReader<Nota>> reader(stub_->ListarNotasAluno(&ctx, req));
        Nota n;
        while (reader->Read(&n)) {
            std::cout << "  -> " << n.cod_disciplina() << " | " << n.nota() << " | " << n.ano() << "/" << n.semestre() << "\n";
        }
        Status status = reader->Finish();
        if (!status.ok()) std::cout << "Stream finalizado com erro: " << status.error_message() << "\n";
    }

private:
    std::unique_ptr<GerenciadorNotas::Stub> stub_;
};

int main() {
    GerenciadorNotasClient client(grpc::CreateChannel("localhost:50052", grpc::InsecureChannelCredentials()));
    client.AdicionarNota("2021001", "CIC0001", 2024, 2, 8.5);
    client.AdicionarNota("2021001", "CIC0002", 2024, 2, 7.0);
    client.ListarNotasAluno("2021001");
    return 0;
}
