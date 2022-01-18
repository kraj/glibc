/* Verify that getcwd returns ERANGE for size 1 byte and does not underflow
   buffer when the CWD is too long and is also a mount target of /.  See bug
   #28769 or CVE-2021-3999 for more context.
   Copyright The GNU Toolchain Authors.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <support/check.h>
#include <support/temp_file.h>

#ifndef PATH_MAX
# define PATH_MAX 1024
#endif

static char *base;
#define BASENAME "tst-getcwd-smallbuff"
#define MOUNT_NAME "mpoint"
static int sockfd[2];

static void
do_cleanup (void)
{
  support_chdir_toolong_temp_directory (base);
  TEST_VERIFY_EXIT (rmdir (MOUNT_NAME) == 0);
  free (base);
}

static int
send_fd (const int sock, const int fd)
{
  struct msghdr msg;
  union
    {
      struct cmsghdr hdr;
      char buf[CMSG_SPACE (sizeof (int))];
    } cmsgbuf;
  struct cmsghdr *cmsg;
  struct iovec vec;
  char ch = 'A';
  ssize_t n;

  memset (&msg, 0, sizeof (msg));
  memset (&cmsgbuf, 0, sizeof (cmsgbuf));
  msg.msg_control = &cmsgbuf.buf;
  msg.msg_controllen = sizeof (cmsgbuf.buf);

  cmsg = CMSG_FIRSTHDR (&msg);
  cmsg->cmsg_len = CMSG_LEN (sizeof (int));
  cmsg->cmsg_level = SOL_SOCKET;
  cmsg->cmsg_type = SCM_RIGHTS;
  *(int *) CMSG_DATA (cmsg) = fd;

  vec.iov_base = &ch;
  vec.iov_len = 1;
  msg.msg_iov = &vec;
  msg.msg_iovlen = 1;

  while ((n = sendmsg (sock, &msg, 0)) == -1 && errno == EINTR);
  if (n != 1)
    return -1;
  return 0;
}

static int
recv_fd (const int sock)
{
  struct msghdr msg;
  union
    {
      struct cmsghdr hdr;
      char buf[CMSG_SPACE(sizeof(int))];
    } cmsgbuf;
  struct cmsghdr *cmsg;
  struct iovec vec;
  ssize_t n;
  char ch = '\0';
  int fd = -1;

  memset (&msg, 0, sizeof (msg));
  vec.iov_base = &ch;
  vec.iov_len = 1;
  msg.msg_iov = &vec;
  msg.msg_iovlen = 1;

  memset (&cmsgbuf, 0, sizeof (cmsgbuf));
  msg.msg_control = &cmsgbuf.buf;
  msg.msg_controllen = sizeof (cmsgbuf.buf);

  while ((n = recvmsg (sock, &msg, 0)) == -1 && errno == EINTR);
  if (n != 1 || ch != 'A')
    return -1;

  cmsg = CMSG_FIRSTHDR (&msg);
  if (cmsg == NULL)
    return -1;
  if (cmsg->cmsg_type != SCM_RIGHTS)
    return -1;
  fd = *(const int *) CMSG_DATA (cmsg);
  if (fd < 0)
    return -1;
  return fd;
}

static int
child_func (void * const arg)
{
    TEST_VERIFY_EXIT (close (sockfd[0]) == 0);
    const int sock = sockfd[1];
    char ch;

    TEST_VERIFY_EXIT (read (sock, &ch, 1) == 1);
    TEST_VERIFY_EXIT (ch == '1');

    if (mount ("/", MOUNT_NAME, NULL, MS_BIND | MS_REC, NULL))
      FAIL_EXIT1 ("mount failed: %m\n");
    const int fd = open ("mpoint",
			 O_RDONLY | O_PATH | O_DIRECTORY | O_NOFOLLOW);
    TEST_VERIFY_EXIT (fd >= 0);

    TEST_VERIFY_EXIT (send_fd (sock, fd) == 0);
    TEST_VERIFY_EXIT (close (fd) == 0);

    TEST_VERIFY_EXIT (read (sock, &ch, 1) == 1);
    TEST_VERIFY_EXIT (ch == 'a');

    TEST_VERIFY_EXIT (close (sock) == 0);
    return 0;
}

static void
update_map (char * const mapping, const char * const map_file)
{
    const size_t map_len = strlen (mapping);

    const int fd = open (map_file, O_WRONLY);
    TEST_VERIFY_EXIT (fd >= 0);
    TEST_VERIFY_EXIT (write (fd, mapping, map_len) == (ssize_t) map_len);
    TEST_VERIFY_EXIT (close(fd) == 0);
}

static void
proc_setgroups_write (const long child_pid, const char * const str)
{
    const size_t str_len = strlen(str);

    char setgroups_path[64];
    snprintf (setgroups_path, sizeof (setgroups_path),
	      "/proc/%ld/setgroups", child_pid);

    const int fd = open (setgroups_path, O_WRONLY);

    if (fd < 0)
      {
        TEST_VERIFY_EXIT (errno == ENOENT);
        return;
      }

    TEST_VERIFY_EXIT (write (fd, str, str_len) == (ssize_t) str_len);
    TEST_VERIFY_EXIT (close(fd) == 0);
}

static char child_stack[1024 * 1024];

int
do_test (void)
{
  base = support_create_and_chdir_toolong_temp_directory (BASENAME);

  TEST_VERIFY_EXIT (mkdir (MOUNT_NAME, S_IRWXU) == 0);
  atexit (do_cleanup);

  TEST_VERIFY_EXIT (socketpair(AF_UNIX, SOCK_STREAM, 0, sockfd) == 0);
  const long child_pid = clone (child_func, child_stack + sizeof(child_stack),
				CLONE_NEWUSER | CLONE_NEWNS | SIGCHLD, NULL);

  TEST_VERIFY_EXIT (child_pid > 1);
  TEST_VERIFY_EXIT (close(sockfd[1]) == 0);
  const int sock = sockfd[0];

  char map_path[64], map_buf[64];
  snprintf (map_path, sizeof (map_path), "/proc/%ld/uid_map", child_pid);
  snprintf (map_buf, sizeof (map_buf), "0 %ld 1", (long) getuid());
  update_map (map_buf, map_path);

  proc_setgroups_write (child_pid, "deny");
  snprintf (map_path, sizeof (map_path), "/proc/%ld/gid_map", child_pid);
  snprintf (map_buf, sizeof (map_buf), "0 %ld 1", (long) getgid());
  update_map (map_buf, map_path);

  TEST_VERIFY_EXIT (send (sock, "1", 1, MSG_NOSIGNAL) == 1);
  const int fd = recv_fd (sock);
  TEST_VERIFY_EXIT (fd >= 0);
  TEST_VERIFY_EXIT (fchdir(fd) == 0);

  static char buf[2 * 10 + 1];
  memset (buf, 'A', sizeof(buf));

  /* Finally, call getcwd and check if it resulted in a buffer underflow.  */
  char * cwd = getcwd (buf + sizeof(buf) / 2, 1);
  TEST_VERIFY (cwd == NULL && errno == ERANGE);

  for (int i = 0; i < sizeof (buf); i++)
    if (buf[i] != 'A')
      {
	printf ("buf[%d] = %02x\n", i, (unsigned int) buf[i]);
	support_record_failure ();
      }

  TEST_VERIFY_EXIT (send (sock, "a", 1, MSG_NOSIGNAL) == 1);
  TEST_VERIFY_EXIT (close (sock) == 0);
  TEST_VERIFY_EXIT (waitpid (child_pid, NULL, 0) == child_pid);

  return 0;
}

#define CLEANUP_HANDLER do_cleanup
#include <support/test-driver.c>
