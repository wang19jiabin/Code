#include <algorithm>
#include <cassert>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <unistd.h>

using namespace std;

class Socket {
public:
  Socket() { _fd = socket(AF_LOCAL, SOCK_DGRAM, 0); }

  virtual ~Socket() { close(_fd); }

  void bind(const string &path) {
    sockaddr_un sun = {AF_LOCAL};
    auto n = snprintf(sun.sun_path, sizeof(sun.sun_path), "%s", path.c_str());
    assert(n > 0 && n < sizeof(sun.sun_path));
    unlink(sun.sun_path);
    if (::bind(_fd, (sockaddr *)&sun, sizeof(sun)) < 0) {
      perror("bind");
      assert(0);
    }
  }

  void send(const string &msg, const string &to) {
    sockaddr_un sun = {AF_LOCAL};
    snprintf(sun.sun_path, sizeof(sun.sun_path), "%s", to.c_str());
    auto n =
        sendto(_fd, msg.c_str(), msg.size(), 0, (sockaddr *)&sun, sizeof(sun));
    assert(n == msg.size());
  }

  void recv(string &msg, string &from) {
    char buf[4096];
    sockaddr_un sun;
    socklen_t len = sizeof(sun);
    auto n = recvfrom(_fd, buf, sizeof(buf), 0, (sockaddr *)&sun, &len);
    assert(n > 0);
    msg.assign(buf, n);
    from = sun.sun_path;
  }

private:
  int _fd;
};

class NameServer : public Socket {
public:
  void start() {
    bind("ns");
    run();
  }

private:
  void run() {
    while (1) {
      string msg, from;
      recv(msg, from);

      stringstream ss;
      ss << msg;
      int sig;
      ss >> sig;

      switch (sig) {
      case 0:
        msg = insert(ss);
        break;
      case 1:
        msg = findPath(ss);
        break;
      case 2:
        msg = findName(ss);
        break;
      default:
        assert(0);
      }

      send(msg, from);
    }
  }

  string insert(stringstream &ss) {
    string path, name;
    ss >> path >> name;
    auto ret = _names.insert({path, name}).second;
    assert(ret);
    return "ok";
  }

  string findPath(stringstream &ss) const {
    string name;
    ss >> name;
    auto it = find_if(_names.begin(), _names.end(),
                      [&name](const auto &a) { return a.second == name; });
    assert(it != _names.end());
    return it->first;
  }

  string findName(stringstream &ss) const {
    string path;
    ss >> path;
    return _names.at(path);
  }

private:
  map<string, string> _names;
};

using Callback = function<void(size_t, stringstream &, const string &)>;

class Handler {
public:
  virtual size_t regist(const Callback &cb) = 0;
  virtual void handle(size_t, stringstream &, const string &) = 0;
};

class Server : public Handler {
public:
  size_t regist(const Callback &cb) override {
    assert(!_cb);
    _cb = cb;
    return 0;
  }

  void handle(size_t seq, stringstream &ss, const string &from) override {
    _cb(seq, ss, from);
  }

private:
  Callback _cb;
};

class Client : public Handler {
public:
  size_t regist(const Callback &cb) override {
    auto ret = _cbs.insert({_seq, cb}).second;
    assert(ret);
    return _seq++;
  }

  void handle(size_t seq, stringstream &ss, const string &from) override {
    _cbs.at(seq)(seq, ss, from);
    _cbs.erase(seq);
  }

private:
  size_t _seq = 0;
  map<size_t, Callback> _cbs;
};

class Module : public Socket {
public:
  Module(const string &name) : _name(name) {}

  void start() {
    auto path = tmpnam(nullptr);
    bind(path);
    publish(path);
    run();
  }

protected:
  void serve(int sig, const Callback &cb) {
    auto server = make_shared<Server>();
    auto seq = server->regist(cb);
    assert(seq == 0);
    auto ret = _handlers.insert({sig, server}).second;
    assert(ret);
  }

  void request(int req, const string &arg, const Callback &cb,
               const string &to) {
    auto cfm = req + 100;
    if (!_handlers[cfm])
      _handlers[cfm] = make_shared<Client>();

    auto seq = _handlers[cfm]->regist(cb);

    string msg = to_string(req) + " " + to_string(seq) + " " + arg;
    send(msg, to);
  }

  string queryPath(const string &name) {
    string msg = "1 " + name;
    send(msg, "ns");
    string from;
    recv(msg, from);
    assert(from == "ns");
    return msg;
  }

  void recv(string &msg, string &from) {
    Socket::recv(msg, from);
    log(msg, from);
  }

  void log(const string &msg, const string &from) {
    auto name = queryName(from);
    cout << _name << " <-(" << msg << ") " << name << endl;
  }

  string queryName(const string &path) {
    auto it = _cache.find(path);
    if (it != _cache.end())
      return it->second;

    string msg = "2 " + path;
    _nameClient.send(msg, "ns");
    string from;
    _nameClient.recv(msg, from);
    return msg;
  }

private:
  void publish(const string &path) {
    _nameClient.bind(tmpnam(nullptr));
    string msg = "0 " + path + " " + _name;
    _nameClient.send(msg, "ns");
    string from;
    _nameClient.recv(msg, from);
    assert(msg == "ok" && from == "ns");
  }

  void run() {
    init();

    while (1) {
      string msg, from;
      recv(msg, from);

      stringstream ss;
      ss << msg;

      int sig;
      size_t seq;
      ss >> sig >> seq;
      _handlers.at(sig)->handle(seq, ss, from);
    }
  }

  virtual void init() = 0;

private:
  string _name;
  Socket _nameClient;
  map<int, shared_ptr<Handler>> _handlers;
  map<string, string> _cache = {{"ns", "ns"}};
};

class Module0 : public Module {
public:
  Module0() : Module("m0") {}

private:
  void init() override {
    auto cb = [](size_t, stringstream &, const string &) {};
    serve(4, cb);
    auto path = queryPath("m1");
    request(3, "test", cb, path);
  }
};

class Module1 : public Module {
public:
  Module1() : Module("m1") {}

private:
  void init() override {
    auto cb = [this](size_t seq, stringstream &ss, const string &from) {
      send("103 " + to_string(seq), from);
      send("4", from);
    };

    serve(3, cb);
  }
};

void fork(const function<void()> &proc) {
  auto pid = fork();
  assert(pid != -1);
  if (pid == 0) {
    proc();
    exit(EXIT_SUCCESS);
  }
}

int main() {
  unlink("log");

  NameServer ns;
  fork(bind(&NameServer::start, &ns));

  Module0 m0;
  fork(bind(&Module0::start, &m0));

  Module1 m1;
  fork(bind(&Module1::start, &m1));

  while (wait(nullptr) != -1)
    ;

  perror("wait");
}
