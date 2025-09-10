// server.cpp
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <unordered_map>

#include <grpcpp/grpcpp.h>
#include "notas.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

using gerencia_notas::GerenciadorNotas;
using gerencia_notas::Nota;
using gerencia_notas::AdicionaNotaRequest;
using gerencia_notas::StatusResponse;
using gerencia_notas::AlunoDisciplinaRequest;
using gerencia_notas::ConsultaNotaResponse;
using gerencia_notas::AlunoRequest;
using gerencia_notas::MediaResponse;

class GerenciadorNotasServiceImpl final : public GerenciadorNotas::Service {
public:
    Status AdicionarNota(ServerContext* context, const AdicionaNotaRequest* request, StatusResponse* response) override {
        std::string chave = request->ra() + "_" + request->cod_disciplina();
        std::lock_guard<std::mutex> lock(mu_);
        if (db_.count(chave)) {
            response->set_sucesso(false);
            response->set_msg("Nota já existe. Use AlterarNota.");
            return Status::OK;
        }
        Nota nota;
        nota.set_ra(request->ra());
        nota.set_cod_disciplina(request->cod_disciplina());
        nota.set_ano(request->ano());
        nota.set_semestre(request->semestre());
        nota.set_nota(request->nota());
        db_[chave] = nota;
        response->set_sucesso(true);
        response->set_msg("Nota adicionada com sucesso.");
        return Status::OK;
    }

    Status AlterarNota(ServerContext* context, const AdicionaNotaRequest* request, StatusResponse* response) override {
        std::string chave = request->ra() + "_" + request->cod_disciplina();
        std::lock_guard<std::mutex> lock(mu_);
        if (!db_.count(chave)) {
            response->set_sucesso(false);
            response->set_msg("Nota não existe. Use AdicionarNota.");
            return Status::OK;
        }
        Nota nota;
        nota.set_ra(request->ra());
        nota.set_cod_disciplina(request->cod_disciplina());
        nota.set_ano(request->ano());
        nota.set_semestre(request->semestre());
        nota.set_nota(request->nota());
        db_[chave] = nota;
        response->set_sucesso(true);
        response->set_msg("Nota alterada com sucesso.");
        return Status::OK;
    }

    Status ConsultarNota(ServerContext* context, const AlunoDisciplinaRequest* request, ConsultaNotaResponse* response) override {
        std::string chave = request->ra() + "_" + request->cod_disciplina();
        std::lock_guard<std::mutex> lock(mu_);
        if (!db_.count(chave)) {
            response->set_sucesso(false);
            response->set_msg_erro("Nota não encontrada.");
            return Status::OK;
        }
        response->set_sucesso(true);
        *response->mutable_nota() = db_[chave];
        return Status::OK;
    }

    Status CalcularMedia(ServerContext* context, const AlunoRequest* request, MediaResponse* response) override {
        std::string ra = request->ra();
        std::vector<float> notas;
        {
            std::lock_guard<std::mutex> lock(mu_);
            for (auto& kv : db_) {
                if (kv.second.ra() == ra) notas.push_back(kv.second.nota());
            }
        }
        if (notas.empty()) {
            response->set_sucesso(false);
            response->set_msg_erro("Nenhuma nota encontrada para o aluno.");
            response->set_media(0.0);
            return Status::OK;
        }
        float soma = 0;
        for (float v : notas) soma += v;
        response->set_sucesso(true);
        response->set_media(soma / notas.size());
        return Status::OK;
    }

    // Server streaming
    Status ListarNotasAluno(ServerContext* context, const AlunoRequest* request, grpc::ServerWriter<Nota>* writer) override {
        std::string ra = request->ra();
        std::lock_guard<std::mutex> lock(mu_);
        for (auto& kv : db_) {
            const Nota& n = kv.second;
            if (n.ra() == ra) {
                writer->Write(n);
                // opcional: std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        }
        return Status::OK;
    }

private:
    std::unordered_map<std::string, Nota> db_;
    std::mutex mu_;
};

void RunServer(const std::string& server_address = "0.0.0.0:50052") {
    GerenciadorNotasServiceImpl service;
    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "C++ gRPC server listening on " << server_address << std::endl;
    server->Wait();
}

int main(int argc, char** argv) {
    RunServer();
    return 0;
}
