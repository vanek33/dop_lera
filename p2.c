/* Программа 2 для иллюстрации работы с очередями сообщений */ 
/* Эта программа получает доступ к очереди сообщений, и читает из нее сообщения с любым типом в порядке FIFO до тех пор, пока не получит сообщение с типом 255, которое будет служитьсигналом прекращения работы. */ 
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdio.h>
#define MAX_MESSAGE_LEN 81 
#define LAST_MESSAGE 255 /* Тип сообщения для прекращения работы */ 

int main()
{
    int msqid; /* IPC дескриптор для очереди сообщений */

    char pathname[] = "file.txt"; /* Имя файла, использующееся для генерации ключа. Файл с таким именем должен существовать в текущей директории */

    key_t key; /* IPC ключ */

    int len, maxlen; /* Реальная длина и максимальная длина информативной части сообщения */

    /* Пользовательская структура для сообщения */

    struct mymsgbuf
    {
        long mtype;
        char mtext[81];
    } mybuf;

    /* Генерируем IPC ключ из имени файла file-01.c в текущей директории и номера экземпляра очереди сообщений 0 */

    if((key = ftok(pathname,0)) < 0){
        printf("Can\'t generate key\n");
        exit(-1);
    }

    /* Пытаемся получить доступ по ключу к очереди сообщений, если она существует, или создать ее, если она еще не существует, с правами доступа read & write для всех пользователей */

    if((msqid = msgget(key, 0666 | IPC_CREAT)) < 0){
        printf("Can\'t get msqid\n");
        exit(-1);
    }

    
    char c[MAX_MESSAGE_LEN];

    maxlen = MAX_MESSAGE_LEN;


    while(1){
        /* В бесконечном цикле принимаем сообщения любого типа в порядке FIFO с максимальной длиной информативной части 81 символ до тех пор, пока не поступит сообщение end*/

        if((len = msgrcv(msqid, (struct msgbuf *) &mybuf, maxlen, 1, 0)) < 0){
            printf("Can\'t receive message from queue\n");
            exit(-1);
        }

        /* Начало обработки и запись в очередь */
        mybuf.mtype = 2;
        for(int j=0; j < strlen(mybuf.mtext); j++) {
            if (isdigit(mybuf.mtext[j]))
                mybuf.mtext[j]= 'X';
        }

        if (msgsnd(msqid, (struct msgbuf *) &mybuf, len, 0) < 0) {
        printf("Can\'t send message to queue\n");
        msgctl(msqid, IPC_RMID, (struct msqid_ds *) NULL);
        exit(-1);

        }

        /* Конец обработки */

        /* Если принятое сообщение имеет содержание end, прекращаем работу. В противном случае печатаем текст принятого сообщения. */

        if (strncmp(mybuf.mtext, "end", MAX_MESSAGE_LEN) == 0) {

            mybuf.mtype = 1; // присваеваем сообщению i-ый тип
            strcpy(mybuf.mtext, "end");
            len = strlen(mybuf.mtext)+1;

            if (msgsnd(msqid, (struct msgbuf *) &mybuf, len, 0) < 0) {
            printf("Can\'t send message to queue\n");
            msgctl(msqid, IPC_RMID, (struct msqid_ds *) NULL);
            exit(-1);

            }
            exit(0);
        }

        printf("message type = 1, text = %s\n", mybuf.mtext);
    }

    return 0; /* Исключительно для отсутствия warning'ов при компиляции. */
}
