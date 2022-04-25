#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(int argc, char* argv[]) {
  if(argc <= 2) {
    printf("usage: %s <ip> <port>\n", basename(argv[0]));
    return 1;
  }
  sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(atoi(argv[2]));
  inet_pton(AF_INET, argv[1], &addr.sin_addr);

  int sock = socket(AF_INET, SOCK_STREAM, 0);
  assert(sock >= 0);

  int ret = bind(sock, (sockaddr*)&addr, sizeof(addr));
  assert(ret >= 0);

  ret = listen(sock, 5);
  assert(ret >= 0);

  sockaddr_in cli_addr;
  socklen_t cli_addr_len = sizeof(cli_addr);

  int conn = accept(sock, (sockaddr*)&cli_addr, &cli_addr_len);
  assert(conn >= 0);

  close(STDOUT_FILENO);   //close output file descriptor
  ret = dup(conn);        //use STDOUT_FILE as file descriptor
  assert(ret >= 0);

  printf("print to connect fd\n");
  close(conn);
  return 0;
}
