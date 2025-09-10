# servidor.py (esqueleto)
import grpc
from concurrent import futures
import notas_pb2
import notas_pb2_grpc

# Armazenamento em memória (simulando um banco de dados)
db_notas = {} # Ex: { "123_DISC1": Nota(...), "123_DISC2": Nota(...) }

class GerenciadorNotasServicer(notas_pb2_grpc.GerenciadorNotasServicer):
    
    def AdicionarNota(self, request, context):
        print(f"Adicionando nota para RA {request.ra} na disciplina \
            {request.cod_disciplina}")
        chave = f"{request.ra}_{request.cod_disciplina}"
        if chave in db_notas:
            return notas_pb2.StatusResponse(sucesso=False, \
                msg="Nota já existe para este aluno/disciplina. \
                    Use AlterarNota.")
        
        # Lógica para criar e armazenar o objeto Nota
        # ...
        db_notas[chave] = ...
        
        return notas_pb2.StatusResponse(sucesso=True, \
            msg="Nota adicionada com sucesso!")

    # ... Implementar os outros métodos (AlterarNota, ConsultarNota, CalcularMedia) ...

    # --- IMPLEMENTAÇÃO DO DESAFIO ---
    def ListarNotasAluno(self, request, context):
        """
        DESAFIO: Este método deve usar Server-Side Streaming.
        Ele deve buscar todas as notas associadas ao RA do aluno no 'banco de dados'
        e enviá-las uma a uma para o cliente usando a palavra-chave 'yield'.
        """
        print(f"Listando notas para o RA {request.ra} (NÃO IMPLEMENTADO)")
        
        # TODO: Implementar a lógica do desafio aqui.
        # Dica: itere pelo seu dicionário `db_notas` e 
        # use `yield` para cada nota encontrada.
        
        # Exemplo de como retornar um erro de "não implementado"
        context.set_code(grpc.StatusCode.UNIMPLEMENTED)
        context.set_details('Método não implementado!')
        return notas_pb2.Nota() # Precisa retornar um objeto do tipo
        #                         esperado, mesmo que vazio


def serve():
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    notas_pb2_grpc.add_GerenciadorNotasServicer_to_server(GerenciadorNotasServicer(),\
         server)
    server.add_insecure_port('[::]:50051')
    server.start()
    print("Servidor gRPC rodando na porta 50051.")
    server.wait_for_termination()

if __name__ == '__main__':
    serve()