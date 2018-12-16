/* Программа 1 для иллюстрации работы с очередями сообщений */ 
/* Эта программа получает доступ к очереди сообщений, отправляет в нее 5 текстовых сообщений с типом 1 и одно пустое сообщение с типом 255, которое будет служить для программы 2 сигналом прекращения работы. */ 
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdio.h>
#define LAST_MESSAGE 255 /* Тип сообщения для прекращения работы программы 2 */
#define MAX_MESSAGE_LEN 81 
int main()
{
    int msqid; /* IPC дескриптор для очереди сообщений */

    char pathname[] = "file.txt"; /* Имя файла, использующееся для генерации ключа. Файл с таким именем должен существовать в текущей директории */

    key_t key; /* IPC ключ */
    int i,len; /* Счетчик цикла и длина информативной части сообщения */

    /* Пользовательская структура для сообщения */
    struct mymsgbuf
    {
        long mtype;
        char mtext[81];
    } mybuf;

    /* Генерируем IPC ключ из имени файла в текущей директории и номера экземпляра очереди сообщений 0. */

    if((key = ftok(pathname,0)) < 0){
        printf("Can\'t generate key\n");
        exit(-1);
    }

    /* Пытаемся получить доступ по ключу к очереди сообщений, если она существует, или создать ее, если она еще не существует, с правами доступа read & write для всех пользователей */
    if((msqid = msgget(key, 0666 | IPC_CREAT)) < 0){
        printf("Can\'t get msqid\n");
        exit(-1);
    }
    
     /* Задаем количество процессов, которые будут получать сообщения */
    char c[MAX_MESSAGE_LEN];

    /* Посылаем в цикле while сообщения пока не введут end с каждым типо из количества читающих процессов в очередь сообщений, идентифицируемую msqid.*/

    gets(c);
    while(strncmp(c, "end", MAX_MESSAGE_LEN) != 0) {
        mybuf.mtype = 1; 
        strcpy(mybuf.mtext, c);
        len = strlen(mybuf.mtext)+1;

        if (msgsnd(msqid, (struct msgbuf *) &mybuf, len, 0) < 0) {
        printf("Can\'t send message to queue\n");
        msgctl(msqid, IPC_RMID, (struct msqid_ds *) NULL);
        exit(-1);
        }
        
        gets(c);

    }

    /* Отсылаем сообщение, которое скажет о конце очереди */
   
        mybuf.mtype = 1; // присваеваем сообщению i-ый тип
        strcpy(mybuf.mtext, "end");
        len = strlen(mybuf.mtext)+1;

        if (msgsnd(msqid, (struct msgbuf *) &mybuf, len, 0) < 0) {
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
        printf("message type = %ld, text = %s\n", mybuf.mtype, mybuf.mtext);
    }
        
    return 0;
}
