#include "spend_time.h"
#include "trio.h"

trio_t trio; // O trio que será utilizado, passaremos seu enderaço ao invocar trio_enter e trio_leave

typedef struct thread_t // Estrutura das nossas threads nessa implementação, com os dados que serão recebidos na entrada
{
    int tid;
    int ttype;
    int tsolo;
    int ttrio;
}
thread_t;

/****************************************************************************
 * void* executar_thread(void *thread_args): função que será passada na 
 * chamada do pthreads_create, deve receber um ponteiro vazio com os 
 * argumentos demandados, então convertemos para thread_t e extraímos os 
 * atributos relevantes, por fim executamos as tarefas da thread, tanto
 * a parte sozinha como a parte em trio
 **************************************************************************/
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

    return NULL;
}

/****************************************************************************
 * int main (): Na função principal temos que ler a entrada, e então criar
 * as pthreads, guargamos apenas as estruturas com os argumentos da entrada
 * num array 'threads_args' e então criamos as threads instanciadas no array
 * 'threads', por fim damos join nessas threads
 **************************************************************************/
int main ()
{
    init_trio(&trio);

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

    }
    for (int i = 0; i < N; i++) {
        pthread_join(threads[i], NULL);
    }

    destroy_trio(&trio);

    return 0;
}