# Projeto PPD
Integrantes: Cléo Jr. F. L. Siznandes, Kaue Müller e Luis Fernando Souza Pinto

# Processamento Distribuído de Vídeo com MPI e OpenMP

## Descrição

Este projeto foi desenvolvido para a disciplina de **Programação Paralela e Distribuída**.

O sistema realiza o processamento distribuído de vídeos utilizando duas tecnologias de paralelismo:

- **MPI (Message Passing Interface)** para distribuir os frames entre diferentes processos;
- **OpenMP** para paralelizar o processamento dos pixels dentro de cada frame.

O OpenCV foi utilizado apenas para leitura, exibição e manipulação dos vídeos, enquanto todos os filtros foram implementados manualmente, conforme solicitado no trabalho.

---

## Objetivos

- Demonstrar o uso conjunto de MPI e OpenMP;
- Distribuir dinamicamente os frames entre múltiplos processos;
- Aplicar filtros de imagem manualmente utilizando processamento paralelo;
- Reorganizar os frames processados para manter a sequência correta do vídeo.

---

## Tecnologias utilizadas

- C++
- OpenCV
- MPI (OpenMPI)
- OpenMP

## Arquitetura do sistema

O projeto utiliza uma arquitetura do tipo **Mestre-Trabalhadores**.

O coordenador:

- lê os frames do vídeo;
- distribui os frames para os trabalhadores;
- recebe os frames processados;
- reorganiza a sequência;
- exibe o vídeo resultante.

Cada trabalhador:

- recebe um frame;
- aplica um filtro utilizando OpenMP;
- devolve o frame processado ao coordenador.

---

# Funcionamento

1. O coordenador abre o vídeo.
2. Cada trabalhador recebe um frame.
3. O trabalhador aplica um filtro.
4. O frame retorna ao coordenador.
5. O coordenador envia um novo frame ao trabalhador livre.
6. Quando todos os frames forem processados, o vídeo é exibido na ordem correta.

---

# Estratégia utilizando MPI

O MPI foi utilizado para realizar a comunicação entre processos.

Cada frame é enviado juntamente com seus metadados:

- número de linhas;
- número de colunas;
- tipo da imagem;
- identificador do frame.

Os trabalhadores processam os frames independentemente.

Para melhorar o balanceamento de carga foi utilizado:

```cpp
MPI_ANY_SOURCE
```

Assim, o coordenador recebe primeiro o trabalhador que terminar o processamento, evitando que processos permaneçam ociosos.

---

# Reordenação dos frames

Como diferentes trabalhadores podem terminar em momentos distintos, os frames podem retornar fora da ordem original.

Para solucionar esse problema foi utilizado um buffer baseado em:

```cpp
map<int, Mat>
```

O identificador (`frame_id`) é utilizado como chave.

O coordenador somente exibe um frame quando seu identificador corresponde ao próximo esperado, garantindo a reconstrução correta do vídeo.

---

# Estratégia utilizando OpenMP

Cada filtro utiliza:

```cpp
#pragma omp parallel for
```

A paralelização ocorre sobre as linhas da imagem.

Cada thread processa um conjunto diferente de linhas, permitindo que vários pixels sejam processados simultaneamente.

Antes da aplicação dos filtros é criada uma cópia da imagem:

```cpp
Mat copia = frame.clone();
```

Essa abordagem evita que alterações realizadas por uma thread interfiram nos cálculos das demais.

---

# Filtros implementados

## Gaussian

Objetivo:

- Redução de ruído;
- Suavização da imagem.

Kernel utilizado:

```
1 2 1
2 4 2
1 2 1
```

---

## Sharpen

Objetivo:

- Aumentar a nitidez da imagem.

Kernel utilizado:

```
0 -1  0
-1  5 -1
0 -1  0
```

---

## Sobel

Objetivo:

- Detectar bordas.

São utilizados dois kernels:

Horizontal (Gx)

```
-1 0 1
-2 0 2
-1 0 1
```

Vertical (Gy)

```
-1 -2 -1
 0  0  0
 1  2  1
```

A intensidade da borda é calculada através da magnitude do gradiente.

---

# Logs do sistema

Durante a execução são exibidos logs como:

```
[COORDENADOR] Frame 0 enviado para Trabalhador 1

[COORDENADOR] Frame 0 recebido de volta do Trabalhador 1

[COORDENADOR] Novo Frame 5 enviado para Trabalhador 2
```

Esses logs permitem acompanhar a distribuição e o retorno dos frames.

---

# Compilação

Exemplo utilizando OpenMPI:

```bash
mpic++ main.cpp filters.cpp -o processamento \
-fopenmp `pkg-config --cflags --libs opencv4`
```

---

# Execução

Executar com quatro processos:

```bash
mpirun -np 4 ./processamento
```

Onde:

- Processo 0 → Coordenador
- Processos 1, 2 e 3 → Trabalhadores

---

# Resultados

O sistema:

- distribui dinamicamente os frames entre processos;
- processa os pixels em paralelo utilizando OpenMP;
- reorganiza os frames na sequência correta;
- exibe o vídeo processado em tempo real.

---

# Conclusão

O projeto demonstra a utilização conjunta de MPI e OpenMP para processamento distribuído de vídeos.

O MPI foi responsável pela distribuição dos frames entre processos, enquanto o OpenMP realizou o processamento paralelo dos pixels em cada trabalhador.

A utilização do buffer de reordenação garantiu que o vídeo fosse reconstruído corretamente, mesmo com diferentes tempos de processamento entre os trabalhadores.

O resultado é um sistema que atende aos requisitos da disciplina, demonstrando conceitos de paralelismo em memória distribuída (MPI) e memória compartilhada (OpenMP).
