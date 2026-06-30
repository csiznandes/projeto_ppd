#include <iostream>
#include <mpi.h>
#include <opencv2/opencv.hpp>
#include <vector>
#include <map>
#include "filters.h"

using namespace std;
using namespace cv;

int main(int argc, char** argv) {
    //Configura o MPI para suportar threads se necessário, ou inicialização padrão
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size < 2) {
        if (rank == 0) cout << "[ERRO] Você precisa de pelo menos 2 processos (1 coordenador e 1 trabalhador)." << endl;
        MPI_Finalize();
        return -1;
    }

    //COORDENADOR
    if (rank == 0) {
        VideoCapture video("video.mp4");
        if (!video.isOpened()) {
            cout << "[ERRO] Erro ao abrir o vídeo." << endl;
            //Avisar todos os trabalhadores para pararem imediatamente
            for (int i = 1; i < size; i++) {
                int stop = -1;
                MPI_Send(&stop, 1, MPI_INT, i, 99, MPI_COMM_WORLD);
            }
            MPI_Finalize();
            return -1;
        }

        int rows = video.get(CAP_PROP_FRAME_WIDTH); //Apenas para inicializar variáveis auxiliares
        Mat frame;
        int frame_id = 0;
        int active_workers = 0;

        int proximo_frame_para_exibir = 0;
        map<int, Mat> buffer_frames;

        cout << "[COORDENADOR] Iniciando distribuição de frames com " << size - 1 << " trabalhadores." << endl;

        //Distribuição Inicial: Envia um frame para cada trabalhador disponível
        for (int i = 1; i < size; i++) {
            if (video.read(frame)) {
                int r = frame.rows;
                int c = frame.cols;
                int t = frame.type();

                //Envia metadados (linhas, colunas, tipo, ID do frame)
                int metadata[4] = {r, c, t, frame_id};
                MPI_Send(metadata, 4, MPI_INT, i, 0, MPI_COMM_WORLD);
                //Envia os pixels do frame
                MPI_Send(frame.data, r * c * frame.elemSize(), MPI_UNSIGNED_CHAR, i, 1, MPI_COMM_WORLD);

                cout << "[COORDENADOR] Frame " << frame_id << " enviado para Trabalhador " << i << endl;
                frame_id++;
                active_workers++;
            } else {
                //Se o vídeo acabou antes de ocupar todos os trabalhadores
                int stop_signal[4] = {-1, 0, 0, 0};
                MPI_Send(stop_signal, 4, MPI_INT, i, 99, MPI_COMM_WORLD);
            }
        }

        //Processamento Dinâmico Reordenado
        while (active_workers > 0) {
            MPI_Status status;
            int metadata_recv[4];

            //Recebe os metadados do trabalhador que terminou primeiro (MPI_ANY_SOURCE)
            MPI_Recv(metadata_recv, 4, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
            int worker_que_respondeu = status.MPI_SOURCE;
            
            int r = metadata_recv[0];
            int c = metadata_recv[1];
            int t = metadata_recv[2];
            int id_recebido = metadata_recv[3];

            //Aloca espaço para receber o frame processado
            Mat frame_processado(r, c, t);
            MPI_Recv(frame_processado.data, r * c * frame_processado.elemSize(), MPI_UNSIGNED_CHAR, worker_que_respondeu, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            
            cout << "[COORDENADOR] Frame " << id_recebido << " recebido de volta do Trabalhador " << worker_que_respondeu << endl;

            //INÍCIO DO AJUSTE DE ORDENAÇÃO: Guarda o frame recebido no buffer associado ao seu ID original
            buffer_frames[id_recebido] = frame_processado;

            //Verifica em sequência se o próximo frame esperado da linha do tempo já chegou
            while (buffer_frames.find(proximo_frame_para_exibir) != buffer_frames.end()) {
                //Exibe o frame na ordem correta
                imshow("Video Processado", buffer_frames[proximo_frame_para_exibir]);
                if (waitKey(1) == 27) break; //ESC para sair

                //Apaga do mapa para liberar a memória RAM do computador
                buffer_frames.erase(proximo_frame_para_exibir);
                proximo_frame_para_exibir++;
            }
            //Envia o próximo frame para o trabalhador que acabou de ficar livre
            if (video.read(frame)) {
                int next_metadata[4] = {frame.rows, frame.cols, frame.type(), frame_id};
                MPI_Send(next_metadata, 4, MPI_INT, worker_que_respondeu, 0, MPI_COMM_WORLD);
                MPI_Send(frame.data, frame.rows * frame.cols * frame.elemSize(), MPI_UNSIGNED_CHAR, worker_que_respondeu, 1, MPI_COMM_WORLD);
                
                cout << "[COORDENADOR] Novo Frame " << frame_id << " enviado para Trabalhador " << worker_que_respondeu << endl;
                frame_id++;
            } else {
                //Acabou o vídeo para este trabalhador, manda sinal de parada
                int stop_signal[4] = {-1, 0, 0, 0};
                MPI_Send(stop_signal, 4, MPI_INT, worker_que_respondeu, 99, MPI_COMM_WORLD);
                active_workers--;
            }
        }
        
        destroyAllWindows();
        cout << "[COORDENADOR] Processamento concluído com sucesso." << endl;
    } 
    //TRABALHADOR
    else {
        while (true) {
            int metadata[4];
            MPI_Status status;

            //Recebe metadados do coordenador
            MPI_Recv(metadata, 4, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

            //Sinal de parada detectado
            if (metadata[0] == -1 || status.MPI_TAG == 99) {
                break;
            }

            int rows = metadata[0];
            int cols = metadata[1];
            int type = metadata[2];
            int frame_id = metadata[3];

            Mat frame(rows, cols, type);
            //Recebe a matriz de pixels do frame
            MPI_Recv(frame.data, rows * cols * frame.elemSize(), MPI_UNSIGNED_CHAR, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            //Executa o filtro de forma paralela
            //Alternar os filtros aqui
            sobelFilter(frame);
            //sharpenFilter(frame); 
            //gaussianFilter(frame); 

            //Devolve os metadados confirmando o ID do frame processado
            MPI_Send(metadata, 4, MPI_INT, 0, 0, MPI_COMM_WORLD);
            //Devolve o frame modificado
            MPI_Send(frame.data, rows * cols * frame.elemSize(), MPI_UNSIGNED_CHAR, 0, 1, MPI_COMM_WORLD);
        }
    }

    MPI_Finalize();
    return 0;
}
