#include <cassert>
#include <iostream>
#include <set>
#include <tinyxml2.h>

using namespace std;
using namespace tinyxml2;

set<string> findDescendantsOf(const set<string> &ids, string ancestor) {
  assert(ids.count(ancestor) == 1);
  ancestor += ',';
  assert(ids.count(ancestor) == 0);
  auto isDescendantOf = [&ancestor](const string &id) { return id.find(ancestor) == 0; };
  auto range = ids.equal_range(ancestor);
  assert(range.first == range.second);
  set<string> descendants;
  for (auto it = range.first; it != ids.end() && isDescendantOf(*it); ++it)
    descendants.insert(*it);
  return descendants;
}

void parse(set<string> &ids, const XMLElement *root, const string &parent = "") {
  for (auto e = root->FirstChildElement(); e; e = e->NextSiblingElement()) {
    string id = parent.empty() ? "" : parent + ",";
    id += string(e->Name()) + "=" + e->Attribute("id");
    ids.insert(id);
    parse(ids, e, id);
  }
}

int main(int c, char **v) {

  if (c != 2)
    return 1;

  XMLDocument doc;
  doc.LoadFile(v[1]);

  set<string> ids;
  auto root = doc.RootElement();
  parse(ids, root);

  for (const auto &id : ids)
    cout << id << endl;

  string ancestor;
  cin >> ancestor;

  auto descendants = findDescendantsOf(ids, ancestor);
  cout << endl;
  for (const auto &id : descendants)
    cout << id << endl;
}
