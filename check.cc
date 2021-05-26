#include <gtest/gtest.h>
#include <list>
#include <poll.h>
#include <sys/socket.h>
#include <sys/un.h>

using CheckPoint = std::function<bool(const std::string &)>;
using CheckList = std::list<std::shared_ptr<CheckPoint>>;

class Fixture : public testing::Test {
public:
  static void SetUpTestCase() {
    _fd = socket(AF_LOCAL, SOCK_DGRAM, 0);
    sockaddr_un sun = {AF_LOCAL};
    strcpy(sun.sun_path, "@path");
    socklen_t len = SUN_LEN(&sun);
    sun.sun_path[0] = 0;
    if (bind(_fd, (sockaddr *)&sun, len) < 0) {
      perror("bind");
      assert(0);
    }
    std::cout << __func__ << std::endl;
  }

  static void TearDownTestCase() {
    close(_fd);
    std::cout << __func__ << std::endl;
  }

protected:
  void check(CheckList &cl) {
    pollfd fd = {_fd, POLLIN};
    while (poll(&fd, 1, 5000) > 0) {
      assert(fd.revents & POLLIN);
      char buf[32];
      auto n = recvfrom(_fd, buf, sizeof(buf), 0, nullptr, nullptr);
      assert(n > 0);
      std::string msg(buf);
      std::cout << msg << std::endl;
      for (auto it = cl.begin(); it != cl.end();) {
        if ((**it)(msg))
          it = cl.erase(it);
        else
          ++it;
      }

      if (cl.empty())
        return;
    }

    assert(!"timeout");
  }

private:
  static int _fd;
};

int Fixture::_fd;

TEST_F(Fixture, check) {
  CheckList cl;
  cl.push_back(std::make_shared<CheckPoint>([](const std::string &msg) { return msg == "3"; }));
  cl.push_back(std::make_shared<CheckPoint>([](const std::string &msg) { return msg == "4"; }));
  cl.push_back(std::make_shared<CheckPoint>([](const std::string &msg) { return msg == "1"; }));
  check(cl);
}
