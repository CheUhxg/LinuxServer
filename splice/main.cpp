#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

int main(int argc, char* argv[]) {
  if(argc <= 2) {
    printf("usage: %s <ip> <port>\n", basename(argv[0]));
    return 1;
  }
  const char* ip = argv[1];
  int port = atoi(argv[2]);

  sockaddr_in addr = {0};
  inet_pton(AF_INET, ip, &addr.sin_addr);
  addr.sin_port = htons(port);
  addr.sin_family = AF_INET;

  int sock = socket(AF_INET, SOCK_STREAM, 0);
  assert(sock >= 0);

  int ret = bind(sock, (sockaddr*)&addr, sizeof(addr));
  assert(ret >= 0);

  ret = listen(sock, 5);
  assert(ret >= 0);

  sockaddr_in cli_addr = {0};
  socklen_t cli_addr_len = sizeof(cli_addr);
  int conn = accept(sock, (sockaddr*)&addr, &cli_addr_len);
  assert(conn >= 0);

  int pipe_fd[2];
  ret = pipe(pipe_fd);
  assert(ret >= 0);

  ret = splice(conn, NULL, pipe_fd[1], NULL, 32768, SPLICE_F_MORE | SPLICE_F_MOVE);
  assert(ret >= 0);
  ret = splice(pipe_fd[0], NULL, conn, NULL, 32768, SPLICE_F_MORE | SPLICE_F_MOVE);
  assert(ret >= 0);

  close(conn);
  close(sock);
  return 0;
}
