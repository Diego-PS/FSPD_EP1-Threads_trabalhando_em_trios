/****************************************************************************
 * trio.h - declaração das função da da abstração de sincronização de um trio
 *         para o primeiro exercício de programação da disciplina 
 *         Fundamentos de Sistemas Paralelos e Distribuídos, 2023-1
 ****************************************************************************/
#include "spend_time.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

/****************************************************************************
 * trio_t: estrutura para representar um trio com os seguintes atributos:
 * uma variável booleana 'formado' para indicar que o trio já esta formado,
 * mas não necessariamente indica que a execução já começou.
 * uma variável booleana 'em_execucao' para indicar que o trio já está
 * em processo de execução, se há alguma thread trabalhando nele.
 * um array de booleanos chamado 'no_trio', representando se uma thread 
 * foi alocada para entrar no trio ou está no trio em execucao, o valor 1
 * representa essas duas possibilidades, já o valor 0 quer dizer que a thread
 * já terminou a execução dentro do trio ou que o trio está para ser formado
 * e há um espaço vago para o grupo em questão. O índice i do array representa
 * o grupo i+1.
 * mutex 'mutex_trio' para protejer a seção crítica (acesso aos atributos do trio)
 * variável de condição 'cond_formado' para se o trio já foi formado
 * variável de condição 'cond_em_execucao'para se o trio está em execução
 * variáveis de condição no array 'cond_no_trio' que indicam que uma thread está no trio
 **************************************************************************/
typedef struct trio_t 
{
    int formado;
    int em_execucao;
    int no_trio[3];
    pthread_mutex_t* mutex_trio;
    pthread_cond_t* cond_formado;
    pthread_cond_t* cond_em_execucao;
    pthread_cond_t* cond_no_trio[3];
} 
trio_t;

/****************************************************************************
 * void init_trio(trio_t* t): inicializa o trio, de modo que os valores dos
 * atributos às variáveis de noTrio sejam 0, inicialmente o trio não foi
 * formato e não está em execução, além disso, alocamos e inicilizamos
 * o mutex e as variáveis de condição.
 **************************************************************************/
void init_trio(trio_t* t);

/****************************************************************************
 * void trio_enter(trio_t* t, int my_type): função para que uma thread
 * do grupo my_type entre no trio t.
 * Primeiro, não podemos entrar em um trio em execução, então esperamos ele
 * terminar a execução. Também não podemos entrar no trio se já houver nele
 * uma thread de mesmo grupo, então espera-se ter condição e então entra-se
 * no trio.
 * Se o trio está completo, marcamos que está formado e liberamos a respec-
 * tiva variável de condição.
 * não podemos efetivamente entrar no trio sem que ele esteja formado, então
 * espera-se essa ocorrência.
 * Então o trio foi formado e já entrará em execução, então marcamos como
 * verdadeiro a variável em_execucao.
 **************************************************************************/
void trio_enter(trio_t* t, int my_type);

/****************************************************************************
 * void trio_leave(trio_t* t, int my_type): função para realizar a saída
 * de um trio, de cara já setamos a variável no_trio correspondente como
 * falso, e então é possível liberar a variável condicional (mesmo que o trio
 * inteiro ainda não tenha terminado não há chance de outra thread entrar, pois
 * na função trio_enter checa-se também se o trio está em execução).
 * Caso as três threads tenham saído, o trio pode terminar a execução, então
 * setamos a variável correspondente para 0 e liberamos sua respectiva variável
 * de condição.
 * No final só podemos de fato sair se todas as threads do trio encerraram,
 * então esperamos até que em_execucao seja verdadeiro.
 * Finalmente, o trio foi liberado, e como ele acabou de encerrar, ainda
 * não foi formado outro, portanto setamos a variável 'formado' para falso.
 **************************************************************************/
void trio_leave(trio_t* t, int my_type);

/****************************************************************************
 * void destroy_trio(trio_t* t): Destrói os objetos de um trio_t e desaloca
 * a memória alocada pelo init_trio, esta função não foi solicitada nas 
 * especificações, mas julguei relevante adicioná-la.
 **************************************************************************/
void destroy_trio(trio_t* t);