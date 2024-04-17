/*
Uma Indústria de Alimentos de Santa Catarina chamada FoodSec S.A. possui a tarefa de escanear
alimentos por meio de câmeras e verificar se os mesmos estão corretos. Os alimentos podem passar
por uma das duas esteiras disponíveis. As duas esteiras são controladas por um por um único
computador centralizado. Esse computador recebe dados de um sensor em cada uma das esteiras que
captura a contagem de itens que são identificados como seguros. A contagem é exibida em um display
perto das esteiras (todos os displays são controlados pela mesma função, é apenas uma replicação)

A empresa está avaliando a viabilidade de implementação nos Sistemas Operacionais Windows e Linux
(qualquer distro) e também em duas formas de IPC. Com isso, você deve implementar duas versões
da solução nos dois sistemas operacionais e comparar qual é a que apresenta o melhor desempenho
quanto ao tempo de computação (tempo de processamento). A primeira solução deve usar IPC do tipo
pipe (de preferência nomeado), já a segunda solução deve ser implementada via multithread (ou multithread, como queira descrever). Logo, deve ser implementado as duas soluções para Windows e as
duas para Linux. Você poderá usar o Codespace ou Cocalc, por exemplo, para implementar a versão
em Linux. Para Windows, você pode usar máquinas virtuais também.

Os códigos disponibilizados pelo professor (de teste e exercícios) podem ser usados para o
desenvolvimento do trabalho (na verdade, é extremamente recomendado). A avaliação do tempo deve
ser entre a execução da contagem e na comunicação, sendo recomendado desconsiderar a criação de
threads, pipe ou memória compartilhada.

() revisar se atende ao que foi pedido
() revisar formatacao
() revisar funcoes
() revisar versao linux
() revisar versao windows
() fazer o tratamento de spinlock
*/

// implementação multithread
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

pthread_mutex_t exclusao_mutua = PTHREAD_MUTEX_INITIALIZER; // LOCK da seção crítica

typedef struct
{
    int quantidade;
    int peso_total;
} Esteira;

Esteira esteira1 = {0, 0};
Esteira esteira2 = {0, 0};

int itens_lidos = 0;
int atualizacoes_display = 0;
int threshold = 10;      // total de itens lidos para atualizar o display
int stop_threshold = 50; // Threshold para parar o programa

void atualiza_sensor_esteira1();
void atualiza_sensor_esteira2();
void atualiza_display();
void stop_program();

int main()
{
    pthread_t t1, t2, t3;
    struct timespec start, end;
    double tempo_execucao;

    if (pthread_mutex_init(&exclusao_mutua, NULL) != 0)
    {
        printf("\nMutex initialization failed\n");
        return 1;
    }

    clock_gettime(CLOCK_MONOTONIC, &start);

    pthread_create(&t1, NULL, (void *)atualiza_sensor_esteira1, NULL);
    pthread_create(&t2, NULL, (void *)atualiza_sensor_esteira2, NULL);
    pthread_create(&t3, NULL, (void *)atualiza_display, NULL);

    // observador que detecta quando o numero de itens passou do limite estabelecido para encerramento
    while (itens_lidos < stop_threshold)
    {
        sleep(1);
    }

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);

    clock_gettime(CLOCK_MONOTONIC, &end);
    tempo_execucao = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("\nTempo de execucao total: %.6f segundos\n", tempo_execucao);
    printf("Taxa de atualizacao do peso total: %.2f atualizacoes/segundo\n", itens_lidos / tempo_execucao);
    printf("Tempo medio de atualizacao do display: %.6f segundos\n", tempo_execucao / atualizacoes_display);

    pthread_mutex_destroy(&exclusao_mutua);

    return 0;
}

void atualiza_sensor_esteira1()
{
    double lido = 0;
    int peso = 5;
    int interval = 2; // 2 segundos
    while (itens_lidos < stop_threshold)
    {
        pthread_mutex_lock(&exclusao_mutua);

        itens_lidos++;
        esteira1.quantidade++;
        esteira1.peso_total += peso;

        pthread_mutex_unlock(&exclusao_mutua);

        sleep(interval);
        lido++;
    }
}

void atualiza_sensor_esteira2()
{
    double lido = 0;
    int peso = 2;
    int interval = 1; // 1 segundo
    while (itens_lidos < stop_threshold)
    {
        pthread_mutex_lock(&exclusao_mutua);

        itens_lidos++;
        esteira2.quantidade++;
        esteira2.peso_total += peso;

        pthread_mutex_unlock(&exclusao_mutua);

        sleep(interval);
        lido++;
    }
}

void atualiza_display()
{
    int multiplicador_quantidade = 1;
    int total_itens_lidos;
    int peso_total = 0;

    while (itens_lidos < stop_threshold)
    {
        pthread_mutex_lock(&exclusao_mutua);

        total_itens_lidos = esteira1.quantidade + esteira2.quantidade;

        if (total_itens_lidos >= (multiplicador_quantidade * threshold))
        {
            peso_total = esteira1.peso_total + esteira2.peso_total;
            multiplicador_quantidade++;
            atualizacoes_display++;
        }

        pthread_mutex_unlock(&exclusao_mutua);

        printf("E1 - Itens: %d | PesoTotal: %d\n", esteira1.quantidade, esteira1.peso_total);
        printf("E2 - Itens: %d | PesoTotal: %d\n", esteira2.quantidade, esteira2.peso_total);
        printf("T  - Itens: %d | PesoTotal: %d\n\n", total_itens_lidos, peso_total);

        sleep(1); // Espera 1 segundos
    }
}