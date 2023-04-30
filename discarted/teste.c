
/****************************************************************************
 * spend_time.c - implementação da função de marcação de tempo e funções auxiliares
 *         para o primeiro exercício de programação da disciplina 
 *         Fundamentos de Sistemas Paralelos e Distribuídos, 2023-1
 ****************************************************************************/

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>

int debug = 1;

long int start_ms = 0; // horário de início do programa, para controle do tempo

/****************************************************************************
 * void check_start(void): para simplificar o uso do código,
 * verifica se o horário de início da execução já foi registrado
 * (o valor de start_ms teria esse tempo em milisegundos).
 * Se start_ms é zero, o horário de início é obtido usando clock_gettime.
 **************************************************************************/
void check_start(void)
{
    if (start_ms == 0) {
        struct timespec now;
        clock_gettime(CLOCK_REALTIME,&now);
        start_ms = (1000 * now.tv_sec) + (0.000001 * now.tv_nsec);
    }
}

/****************************************************************************
 * void write_log(int tid, int ttype, char* msg): recebe como parâmetros 
 * o identificador da thread e seu tipo (ambos inteiros) e o string da
 * mensgem de log. Escreve na saída essa informação acrescida de um valor de
 * timestamp em décimos de segundos decorridos desde o início da execução.
 * 
 * ESSE FORMATO NÃO DEVE SER ALTERADO, POIS SERÁ USADO NA CORREÇÃO AUTOMÁTICA
 **************************************************************************/
void write_log(int tid, int ttype, char* msg)
{
    struct timespec now;
    long   now_ms;
    long   time_passed_ds;
    char   log_msg[1024];

    check_start();
    clock_gettime(CLOCK_REALTIME,&now);
    now_ms = (1000.0 * now.tv_sec) + (0.000001 * now.tv_nsec);
    time_passed_ds = round( ((double)(now_ms-start_ms)) / 100.0 );
    if (snprintf(log_msg,sizeof(log_msg),
                "%ld:%d:%d:%s",time_passed_ds,tid,ttype,msg)<0) {
        perror("snprintf");
        exit(1);
    }
    printf("%s\n",log_msg);
}

/****************************************************************************
 * void spend_time_ms(int tid, int ttype, char* slot, int time_ms): para 
 * permitir simular a operação de threads que executem tarefas computacionais
 * mais complexas, essa função suspende uma thread por um espaço de tempo 
 * determinado (em milissegundos). Para gerar uma mensagem de log no início
 * e no final desse intervalo, recebe como parâmetros também o identificador 
 * da thread, seu tipo e uma string que vai identificar o intervalo de tempo.
 **************************************************************************/
void spend_time(int tid, int ttype, char* slot, int time_ds)
{
    struct timespec zzz;
    char msg[1024];

    zzz.tv_sec  = time_ds/10;
    zzz.tv_nsec = (time_ds%10) * 100L * 1000L * 1000L;

    if (snprintf(msg,sizeof(msg),"%s+(%d)",slot,time_ds)<0) {
        perror("snprintf");
        exit(1);
    } 
    write_log(tid,ttype,msg);

    nanosleep(&zzz,NULL);

    if (snprintf(msg,sizeof(msg),"%s-",slot)<0) {
        perror("snprintf");
        exit(1);
    }
    write_log(tid,ttype,msg);
}

/* Teste: a função main a seguir executa três chamadas consecutivas de
 * spend_time para verificar se o comportamento é o esperado.
 * Para testar, basta descomentar a função main, executar "make spend_time"
 * e depois executar o programa resultante.
 */

/*
int main(int argc, char* argv[])
{
    spend_time(1,2,"t1",10);
    spend_time(11,22,"t2",13);
    spend_time(111,222,"t3",17);
}
*/

/* Saída esperada do programa de teste:
 *
0:1:2:t1+(10)
10:1:2:t1-
10:11:22:t2+(13)
23:11:22:t2-
23:111:222:t3+(17)
40:111:222:t3-
 *
 * O programa deve terminar 4 segundos após ser disparado.
 */


/****************************************************************************
 * trio.c - implementação da abstração de sincronização de um trio
 *         para o primeiro exercício de programação da disciplina 
 *         Fundamentos de Sistemas Paralelos e Distribuídos, 2023-1
 ****************************************************************************/

/****************************************************************************
 * trio_t: estrutura para representar um trio com os seguintes atributos:
 * um array noTrio com três posições, uma para cada grupo, representando
 * se existe uma thread do respectivo grupo no trio, com valor 1 para 
 * verdadeiro e 0 para falso. O index i do array representa o grupo i+1,
 * pois o array é indexado começando em 0.
 * Além disso, temos um mutex mutexTrio para protejer a seção crítica desta
 * abstração, ele será usado quando houver acesso ao atributo noTrio.
 * Ainda temos o array condNoTrio, que são três variáveis de condição para
 * permitir que uma thread possa esperar até que o seu grupo possa entrar
 * no trio, segue a mesma convenção adotada no array noTrio
 **************************************************************************/

pthread_mutex_t mutex_trio;
pthread_cond_t cond_formado;
pthread_cond_t cond_em_execucao;
pthread_cond_t cond_no_trio[3];

typedef struct trio_t 
{
    int formado;
    int em_execucao;
    int no_trio[3];
    // pthread_mutex_t* mutex_trio;
    // pthread_cond_t* cond_formado;
    // pthread_cond_t* cond_em_execucao;
    // pthread_cond_t* cond_no_trio[3];
} 
trio_t;


/****************************************************************************
 * void init_trio(trio_t* t): inicializa o trio, de modo que os valores dos
 * atributos às variáveis de noTrio sejam 0, e também iniciliza o mutex e as
 * variáveis de condição.
 **************************************************************************/
void init_trio(trio_t* t) 
{
    if (debug) printf("Entrei no init trio\n");
    pthread_mutex_init(&mutex_trio, NULL);
    if (debug) printf("iniciei o mutex_trio\n");

    for (int i = 0; i < 3; i++) {
        pthread_cond_init(&cond_no_trio[i], NULL);
    }
    if (debug) printf("iniciei o array cond_no_trio\n");

    pthread_cond_init(&cond_formado, NULL);
    if (debug) printf("iniciei o array cond_formado\n");

    pthread_cond_init(&cond_em_execucao, NULL);
    if (debug) printf("iniciei o array cond_em_execucao\n");

    for (int i = 0; i < 3; i++) {
        t->no_trio[i] = 0;
    }

    t->formado = 0;
    t->em_execucao = 0;
}

/****************************************************************************
 * void trio_enter(trio_t* t, int my_type): função para que uma thread
 * do grupo my_type entre no trio t, caso não haja uma thread do seu tipo
 * em t, ela entrará, o que corresponde a flipar o valor de noTrio 
 * correspondente à sua posição para 1, mas caso não consiga entrar,
 * pois já há uma thread do mesmo grupo no trio, deverá esperar a formação
 * de um outro trio, o que seja indicado pelo condNoTrio
 **************************************************************************/
void trio_enter(trio_t* t, int my_type)
{
    int index = my_type - 1;
    // if (debug) printf("Entrei no trio_enter\n");
    
    pthread_mutex_lock(&mutex_trio);

    // if (debug) printf("em_execucao: %d\n", t->em_execucao);
    // if (debug) printf("no_trio[%d]: %d\n", index, t->no_trio[index]);

    while (t->em_execucao) {
        pthread_cond_wait(&cond_em_execucao, &mutex_trio);
    }
    // if (debug) printf("terminou a execucao\n");

    while (t->no_trio[index]) {
        pthread_cond_wait(&cond_no_trio[index], &mutex_trio);
    }

    t->no_trio[index] = 1;
    // if (debug) printf("adicionei %d\n", my_type);

    if (t->no_trio[0] && t->no_trio[1] && t->no_trio[2]) {
        t->formado = 1;
        pthread_cond_broadcast(&cond_formado);
    } else {
        while (!t->formado) {
            pthread_cond_wait(&cond_formado, &mutex_trio);
        }
    }

    // if (debug) printf("Formei trio: %d\n", index+1);

    t->em_execucao = 1;

    pthread_mutex_unlock(&mutex_trio);
}

/****************************************************************************
 * void trio_leave(trio_t* t, int my_type): função para realizar a saída
 * de um trio, portanto atualizamos os valores de 
 **************************************************************************/
void trio_leave(trio_t* t, int my_type)
{
    int index = my_type - 1;
    // if (debug) printf("Entrei no trio_leave: %d\n", my_type);

    pthread_mutex_lock(&mutex_trio);

    t->no_trio[index] = 0;
    pthread_cond_signal(&cond_no_trio[index]);

    if (!t->no_trio[0] && !t->no_trio[1] && !t->no_trio[2]) {
        t->em_execucao = 0;
        pthread_cond_broadcast(&cond_em_execucao);
    }

    while(t->em_execucao) {
        pthread_cond_wait(&cond_em_execucao, &mutex_trio);
    }

    t->formado = 0;

    pthread_mutex_unlock(&mutex_trio);

}

// void* executar_thread(void *arg);

// trio_t trio;

// void* executar_thread(void *arguments) 
// {
//     struct arg_struct *args = (struct arg_scruct *) arguments;
//     spend_time(args->tid, args->ttype, "S", args->tsolo);
//     trio_enter(&trio, args->ttype);
//     spend_time(args->tid, args->ttype, "T", args->ttrio);
//     trio_leave(&trio, args->ttype);
// }

// struct arg_struct {
//     int tid;
//     int ttype;
//     int tsolo;
//     int ttrio;
// };

// int main ()
// {
//     pthread_t thread1;
//     struct arg_struct args1;
//     args1.tid = 1;
//     args1.ttype = 1;
//     args1.tsolo = 10;
//     args1.t
//     pthread_t thread2;

//     pthread_create(&thread1, NULL, executar_thread, (void*) &deposit1);
//     pthread_create(&thread2, NULL, deposit, (void*) &deposit2);
// }


void* executar_thread(void *arg);

trio_t trio;

typedef struct thread_t 
{
    int tid;
    int ttype;
    int tsolo;
    int ttrio;
}
thread_t;

void* executar_thread(void *thread_args) 
{
    thread_t* thread = (struct thread_t*) thread_args;
    int tid = thread->tid;
    int ttype = thread->ttype;
    int tsolo = thread->tsolo;
    int ttrio = thread->ttrio;
    
    spend_time(tid, ttype, "S", tsolo);
    trio_enter(&trio, ttype);
    spend_time(tid, ttype, "T", ttrio);
    trio_leave(&trio, ttype);
    // pthread_exit(NULL);
}

int main ()
{
    if (debug) printf("O programa ao menos inicia\n");
    init_trio(&trio);
    if (debug) printf("inicializei o trio\n");

    int tid, ttype, tsolo, ttrio;

    thread_t threads_args[1000];
    pthread_t threads[1000];
    
    int i = 0;
    while(scanf("%d %d %d %d", &tid, &ttype, &tsolo, &ttrio) != EOF) {
        thread_t thread_arg;
        thread_arg.tid = tid;
        thread_arg.ttype = ttype;
        thread_arg.tsolo = tsolo;
        thread_arg.ttrio = ttrio;

        threads_args[i] = thread_arg;
        i++;
    }

    int N = i;



    for (int i = 0; i < N; i++) {
        pthread_create(&threads[i], NULL, executar_thread, (void *) &threads_args[i]);

        // pthread_join(threads[i], NULL);
    }
    for (int i = 0; i < N; i++) {
        pthread_join(threads[i], NULL);
    }
}