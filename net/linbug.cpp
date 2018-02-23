//**************************************************************************************************************************************************
//*
//* linbug      - David R. Whipple 2013
//*
//* g++ -m64 -lpthread -lutil linbug.cpp -o linbug
//*
//*-------------------------------------------------------------------------------------------------------------------------------------------------
//*
//* Problem description: portions of the buffer used during a recv remain unchanged
//*
//* Observed on:
//*   RHEL 5
//*   CentOS 6.3 (tested various kernels between 2.6.32 to 3.11)
//*   CentOS 6.4
//*   Fedora 19
//*   RHELS 6.4
//*
//* Not observed on:
//*   Debian 7.1 (Linux x3550m3 3.2.0-4-amd64 #1 SMP Debian 3.2.46-1+deb7u1 x86_64 GNU/Linux)
//*   Solaris 10
//*
//* Equipment on which problem has been observed:
//*   IBM x3650    (tested CentOS 6.3)
//*   IBM x3650-M2 (tested CentOS 6.3)
//*   IBM x3550-M3 (tested all)
//*   IBM x3550-M4 (tested CentOS 6.3)
//*   SuperMicro X7DBE (tested CentOS 6.3)
//*
//*-------------------------------------------------------------------------------------------------------------------------------------------------
//*
//* reproduces problem observed in another app
//* multiple causes such as hard drive issue ("hard resetting link" seen in /var/log/messages, perhaps while app was writing to a file), forkpty(),
//*   system(), and likely others
//* observed using numerous NIC types as well as loopback (localhost)
//* perhaps Xeon specific?
//* another observed symptom is when MSG_WAITALL is used it might exhibit strange effects (gums up in the problem environments on RHELS 6.4)
//*
//**************************************************************************************************************************************************
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pty.h>

#define BUFFER_SIZE     (10 * 1024 * 1024)                                                      // 10MB
#define PORT            18000
#define NUMBER_CLIENTS  8                                                                       // can occur with 1 but generally quicker with many

//**************************************************************************************************************************************************
//*
//* SessionThread
//*
//**************************************************************************************************************************************************
void * SessionThread(void * pvParm)
{
  unsigned char * pucBuffer = NULL;
  int             iSocket   = (int) (unsigned long int) pvParm;
  printf("SessionThread(%u): Receiving packets...\n", iSocket);
  do
  {
    if (!(pucBuffer = new unsigned char[BUFFER_SIZE]))
    {
      printf("SessionThread(%u): Unable to allocate %u bytes\n", iSocket, BUFFER_SIZE);
      break;
    }
    while (1)
    {
      unsigned long int ul;
      for (ul = 0 ; ul < BUFFER_SIZE ; )
      {
        int iRC = recv(iSocket, &pucBuffer[ul], BUFFER_SIZE - ul, 0);                           // MSG_WAITALL can gum up on problem systems
        if (!iRC)
        {
          printf("SessionThread(%u): The client terminated the connection\n", iSocket);
          break;
        }
        if (iRC == -1)
        {
          printf("SessionThread(%u): recv error %d\n", iSocket, errno);
          break;
        }
        ul += iRC;
      }
      if (ul < BUFFER_SIZE)
        break;
      for (ul = 1 ; ul < BUFFER_SIZE && pucBuffer[ul] == *pucBuffer ; ul++); // yes, semi...
      if (ul < BUFFER_SIZE)
      {
        while (ul < BUFFER_SIZE)
        {
          unsigned long int ul2;
          for (ul2 = ul + 1 ; ul2 < BUFFER_SIZE && pucBuffer[ul2] == pucBuffer[ul] ; ul2++); // yes, semi...
          printf("SessionThread(%u): Offsets %lu-%lu contain 0x%02X but expected 0x%02X\n", iSocket, ul, ul2 - 1, pucBuffer[ul], *pucBuffer);
          for (ul = ul2 ; ul < BUFFER_SIZE && pucBuffer[ul] == *pucBuffer ; ul++); // yes, semi...
        }
        break;
      }
    }
  } while (0);
  shutdown(iSocket, 2);
  close(iSocket);
  iSocket = -1;
  if (pucBuffer)
  {
    delete []pucBuffer;
    pucBuffer = NULL;
  }
  exit(0);
  return 0;
}

//**************************************************************************************************************************************************
//*
//* ClientThread
//*
//**************************************************************************************************************************************************
void * ClientThread(void * pvParm)
{
  int             iSocket   = -1;
  unsigned char * pucBuffer = NULL;
  do
  {
    if (!(pucBuffer = new unsigned char[BUFFER_SIZE]))
    {
      printf("ClientThread: Unable to allocate %u bytes\n", BUFFER_SIZE);
      break;
    }
    if ((iSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
      printf("ClientThread: socket error %d\n", errno);
      break;
    }
    sockaddr_in Sin;
    memset(&Sin, 0, sizeof(Sin));
    Sin.sin_family = AF_INET;
    Sin.sin_port   = htons(PORT);
    *((unsigned long int *) &Sin.sin_addr) = inet_addr("127.0.0.1");
    if (connect(iSocket, (sockaddr *) &Sin, sizeof(Sin)) == -1)
    {
      printf("ClientThread: connect error %d\n", errno);
      break;
    }
    printf("ClientThread: Sending packets...\n");
    unsigned char uc = 0;
    while (1)
    {
      memset((unsigned char *) pucBuffer, uc, BUFFER_SIZE);
      int iRC = send(iSocket, pucBuffer, BUFFER_SIZE, 0);
      if (!iRC)
      {
        printf("ClientThread: The server unexpectedly terminated the connection!\n");
        break;
      }
      if (iRC == -1)
      {
        printf("ClientThread: send error %d\n", errno);
        break;
      }
      printf("ClientThread: Sent 10MB of byte 0x%02X\n", uc++);                                 // !!!!! comment this line = no problem !!!!!
    }
  } while (0);
  if (iSocket != -1)
  {
    shutdown(iSocket, 2);
    close(iSocket);
    iSocket = -1;
  }
  if (pucBuffer)
  {
    delete []pucBuffer;
    pucBuffer = NULL;
  }
  return 0;
}

//**************************************************************************************************************************************************
//*
//* ActionThread        - cause frequent forks - either the system() loop or the forkpty() loop induces the problem
//*
//**************************************************************************************************************************************************
void * ActionThread(void * pvParm)
{
  printf("ActionThread: running...\n");
  while (1)
  {
    usleep(25000);                                                                              // 25ms
    system("dir >/dev/null");
  }
/*
  int iMaster = 0;
  int iPid    = 0;
  while (1)
  {
    usleep(125000);                                                                             // 125ms
    if (iPid > 0)
    {
      close(iPid);
      iPid = 0;
    }
    if (iMaster)
    {
      close(iMaster);
      iMaster = 0;
    }
    if ((iPid = forkpty(&iMaster, NULL, NULL, NULL)) <= 0)
    {
      printf("ActionThread: forkpty error %u\n", errno);
      break;
    }
    if (!iPid)                                                                                  // child
    {
      printf("ActionThread: child exiting\n");
      break;
    }
  }
  if (iPid > 0)
    close(iPid);
  iPid = 0;
  if (iMaster)
  {
    close(iMaster);
    iMaster = 0;
  }
*/
  return 0;
}

//**************************************************************************************************************************************************
//*
//* main
//*
//**************************************************************************************************************************************************
int main(int argc, char * * argv)
{
  int            iSocket        = -1;
  int            iSocketSession = -1;
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setstacksize(&attr, 256 * 1024);
  do
  {
    if ((iSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
      printf("socket error %d\n", errno);
      break;
    }
    sockaddr_in Sin;
    memset(&Sin, 0, sizeof(Sin));
    Sin.sin_family = AF_INET;
    Sin.sin_port   = htons(PORT);
    int iTrue = 1;
    if (setsockopt(iSocket, SOL_SOCKET, SO_REUSEADDR, (char *) &iTrue, sizeof(iTrue)) == -1)
    {
      printf("setsockopt(SO_REUSEADDR) error %d\n", errno);
      break;
    }
    if (setsockopt(iSocket, SOL_SOCKET, SO_KEEPALIVE, (char *) &iTrue, sizeof(iTrue)) == -1)
    {
      printf("setsockopt(SO_KEEPALIVE) error %d\n", errno);
      break;
    }
    Sin.sin_addr.s_addr = INADDR_ANY;
    if (bind(iSocket, (sockaddr *) &Sin, sizeof(Sin)) == -1)
    {
      printf("bind error %d\n", errno);
      break;
    }
    if (listen(iSocket, SOMAXCONN) == -1)
    {
      printf("listen error %d\n", errno);
      break;
    }
    pthread_t pThread;
    int iRC = pthread_create(&pThread, &attr, ActionThread, NULL);
    if (iRC)
    {
      printf("pthread_create error %d\n", iRC);
      break;
    }
    unsigned long int ul;
    for (ul = 0 ; ul < NUMBER_CLIENTS ; ul++)
    {
      if ((iRC = pthread_create(&pThread, &attr, ClientThread, NULL)))
      {
        printf("pthread_create error %d\n", iRC);
        break;
      }
    }
    if (ul < NUMBER_CLIENTS)
      break;
    while (1)
    {
      printf("Waiting for a connection to port %u (use CTRL+C to quit)\n", PORT);
      if ((iSocketSession = accept(iSocket, NULL, NULL)) == -1)
      {
        printf("accept error %d\n", errno);
        break;
      }
      printf("Client connected\n");
      if ((iRC = pthread_create(&pThread, &attr, SessionThread, (void *) (unsigned long int) iSocketSession)))
      {
        printf("pthread_create error %d\n", iRC);
        break;
      }
      iSocketSession = -1;                                                         // passed to the sub thread
    }
  } while (0);
  pthread_attr_destroy(&attr);
  if (iSocket != -1)
  {
    shutdown(iSocket, 2);
    close(iSocket);
    iSocket = -1;
  }
  if (iSocketSession != -1)
  {
    shutdown(iSocketSession, 2);
    close(iSocketSession);
    iSocketSession = -1;
  }
  return 0;
}
