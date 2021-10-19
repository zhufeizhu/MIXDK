#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

struct info{
    int pos;
    int rank;
    bool operator<(const struct info&){

    }
};

bool cmp(struct info& info1, struct info& info2){

}

int main(){
    int n = 0;
    vector<info> v;
    for(int i = 0; i < n; i++){
        info f;
        f.pos = i;
        cin>>f.rank;
        v.push_back(f);
    }

    sort(v.begin(),v.end(),[](struct info& info1, struct info& info2){
        return info1.pos - in
    });

    //v1<v2
}