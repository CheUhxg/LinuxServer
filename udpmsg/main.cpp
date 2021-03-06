#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MSG_BLOCKS 16
#define MSG_LENGTH 16

void client(const char* ip, const short port);
void server(const char* ip, const short port);

int main(int argc, char* argv[]) {
  if(argc != 4) {
    printf("Usage: %s [-c | -s] <ip> <port>\n", basename(argv[0]));
    return 1;
  }
  if(strcmp(argv[1], "-c") == 0) {
    client(argv[2], atoi(argv[3]));
  } else {
    server(argv[2], atoi(argv[3]));
  }
  return 0;
}

void client(const char* ip, const short port) {
  sockaddr_in addr;
  addr.sin_family = AF_INET;
  inet_pton(AF_INET, ip, &addr.sin_addr);
  addr.sin_port = htons(port);

  int sock = socket(PF_INET, SOCK_DGRAM, 0);
  assert(sock >= 0);

  char buffer[MSG_LENGTH] = "UDP message";
  msghdr msg{0};
  msg.msg_name = &addr;
  msg.msg_namelen = sizeof(addr);
  iovec msg_iov[MSG_BLOCKS];
  for(int i = 0; i < MSG_BLOCKS; ++i) {
    msg_iov[i].iov_base = buffer;
    msg_iov[i].iov_len = sizeof(buffer);
  }
  msg.msg_iov = msg_iov;
  msg.msg_iovlen = MSG_BLOCKS;
  int send_bytes = sendmsg(sock, &msg, 0);
  printf("Client send %d bytes\n", send_bytes);

  socklen_t sock_len;
  int ret = getsockname(sock, (sockaddr*)&addr, &sock_len);
  assert(ret >= 0);
  printf("Local ip: %d\nLocal port: %d\n",
         ntohl(addr.sin_addr.s_addr), ntohs(addr.sin_port));
}

void server(const char* ip, const short port) {
  sockaddr_in addr;
  addr.sin_family = AF_INET;
  inet_pton(AF_INET, ip, &addr.sin_addr);
  addr.sin_port = htons(port);

  int sock = socket(PF_INET, SOCK_DGRAM, 0);
  assert(sock >= 0);

  int ret = bind(sock, (sockaddr *) &addr, sizeof(addr));
  assert(ret >= 0);

  char buffer[MSG_LENGTH];
  msghdr msg{0};
  msg.msg_name = &addr;
  msg.msg_namelen = sizeof(addr);
  iovec msg_iov[MSG_BLOCKS];
  msg.msg_iov = msg_iov;
  msg.msg_iovlen = MSG_BLOCKS;
  for(int i = 0; i < MSG_BLOCKS; ++i) {
    msg_iov[i].iov_base = buffer;
    msg_iov[i].iov_len = sizeof(buffer);
  }
  int recv_bytes = recvmsg(sock, &msg, 0);
  printf("Server recv %d bytes:\n", recv_bytes);
  for(int i = 0; i < MSG_BLOCKS; ++i) {
    printf("%s\n", static_cast<char *>(msg.msg_iov[i].iov_base));
  }

  socklen_t sock_len;
  ret = getsockname(sock, (sockaddr *) &addr, &sock_len);
  assert(ret >= 0);
  printf("Local ip: %d\nLocal port: %d\n",
         ntohl(addr.sin_addr.s_addr), ntohs(addr.sin_port));
}
