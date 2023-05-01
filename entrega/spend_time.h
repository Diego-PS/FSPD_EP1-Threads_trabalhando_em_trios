/****************************************************************************
 * spend_time.h - declaração da função de marcação de tempo 
 *         para o primeiro exercício de programação da disciplina 
 *         Fundamentos de Sistemas Paralelos e Distribuídos, 2023-1
 ****************************************************************************/

/****************************************************************************
 * void spend_time(int tid, int ttype, char* slot, int time_ds): para 
 * permitir simular a operação de threads que executem tarefas computacionais
 * mais complexas, essa função suspende uma thread por um espaço de tempo 
 * determinado (em décimos de segundos). 
 * Os parâmetros devem ser fornecidos por cada thread para indicar sua
 * identificação, seu tipo/grupo, um string para identificar a tarefa sendo
 * executada e quanto tempo essa tarefa deve durar.
 **************************************************************************/

void spend_time(int tid, int ttype, char* slot, int time_ds);
