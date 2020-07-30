#include "DataManager.hpp"
using namespace std;

int main() {
    DataManager dm;
    dm.Connect();
    map<string, string> m;
    dm.Query_all(m);
    for (const auto& e : m) {
        cout << e.first << "\t" << e.second << endl;
    }

    map<string, string> m2;
    dm.Query_all(m2);
    for (const auto& e : m2) {
        cout << e.first << "\t" << e.second << endl;
    }

    bool flag = dm.Insert("11", "11.zip");
    if (flag) {
        cout << "插入成功" << endl;
    }

    flag = dm.Delete("tmp");
    if (flag) {
        cout << "删除成功" << endl;
    }

    flag = dm.Update("test", "test1", "test1.zip");
    if (flag) {
        cout << "修改成功" << endl;
    }


    dm.Close();
    return 0;
}
