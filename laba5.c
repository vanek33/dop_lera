#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdio.h>
#define MAX_MESSAGE_LEN 81 
#define LAST_MESSAGE 255
int main(void)
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
    int pid;

    char c[80];
    int count=0;

     /* Пытаемся получить доступ по ключу к очереди сообщений, если она существует, или создать ее, если она еще не существует, с правами доступа read & write для всех пользователей */
    if((msqid = msgget(IPC_PRIVATE, 0666 | IPC_CREAT)) < 0){
        printf("Can\'t get msqid\n");
        exit(-1);
    }

    switch(pid = fork())
    {

    case 0:  /*  ПОТОМОК  */

        while(1){
        /* В бесконечном цикле принимаем сообщения любого типа в порядке FIFO с максимальной длиной информативной части 81 символ до тех пор, пока не поступит сообщение end*/

        if((len = msgrcv(msqid, (struct msgbuf *) &mybuf, maxlen, 1, 0)) < 0){
            printf("Can\'t receive message from queue\n");
            exit(-1);
        }
        printf("Pot: message type = 1, text = %s\n", mybuf.mtext);
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

        /* Если принятое сообщение имеет содержание end, прекращаем работу. В противном случае печатаем текст принятого сообщения. */
        if (strncmp(mybuf.mtext, "end", MAX_MESSAGE_LEN) == 0) {

            mybuf.mtype = 2; // присваеваем сообщению i-ый тип
            strcpy(mybuf.mtext, "end");
            len = strlen(mybuf.mtext)+1;

            if (msgsnd(msqid, (struct msgbuf *) &mybuf, len, 0) < 0) {
            printf("Can\'t send message to queue\n");
            msgctl(msqid, IPC_RMID, (struct msqid_ds *) NULL);
            exit(-1);

            }
            exit(0);
        }
            
    }
        

    case -1: /* Cбой при вызове fork()  */
        printf("Ошибка при вызове fork() \n");
        exit(1);
    default:   /* Это ПРЕДОК   */

        gets(c);
        while(strncmp(c, "end", MAX_MESSAGE_LEN) != 0) 
        {

            mybuf.mtype = 1; 
            strcpy(mybuf.mtext, c);
            len = strlen(mybuf.mtext)+1;

            if (msgsnd(msqid, (struct msgbuf *) &mybuf, len, 0) < 0) 
            {
                printf("Can\'t send message to queue\n");
                msgctl(msqid, IPC_RMID, (struct msqid_ds *) NULL);
                exit(-1);

            }

            gets(c);

        }
        mybuf.mtype = 1; // присваеваем сообщению i-ый тип
        strcpy(mybuf.mtext, "end");
        len = strlen(mybuf.mtext)+1;

        if (msgsnd(msqid, (struct msgbuf *) &mybuf, len, 0) < 0) 
        {
            printf("Can\'t send message to queue\n");
            msgctl(msqid, IPC_RMID, (struct msqid_ds *) NULL);
            exit(-1);
        }

        while(1){
        /* В бесконечном цикле принимаем сообщения любого типа в порядке FIFO с максимальной длиной информативной части 81 символ до тех пор, пока не поступит сообщение end*/
        
        if((len = msgrcv(msqid, (struct msgbuf *) &mybuf, MAX_MESSAGE_LEN, 2, 0)) < 0){
            printf("Can\'t receive message from queue\n");
            exit(-1);
        }
        
        if (strncmp(mybuf.mtext, "end", MAX_MESSAGE_LEN) == 0) {
            msgctl(msqid, IPC_RMID, (struct msqid_ds *) NULL);
            exit(0);
        }
        printf("Rod: message type = %ld, text = %s\n", mybuf.mtype, mybuf.mtext);
    }

        exit(0);
    }
}
