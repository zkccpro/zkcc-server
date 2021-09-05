#include<vector>
#include<iostream>
#include<memory>
using namespace std;
int main(){
    vector<char> vec;
    vec.resize(4,2);
    size_t len=2;
    *reinterpret_cast<size_t*>(&vec[0])=len;
    //vec.push_back(222);
    //cout<<*reinterpret_cast<size_t*>(&vec[0])<<endl;

    char* tmp=new char[6]();
    *reinterpret_cast<size_t*>(tmp)=len;
    // tmp[4]=2;
    // tmp[5]=2;
    //cout<<*reinterpret_cast<size_t*>(tmp)<<endl;

    unique_ptr<char[]> arr;
    arr.reset(new char[4]);
    char* index=arr.get();
    *index='a';
    *(index+1)='a';
    index+=2;
    *index='a';
    index++;
    *index='a';
    index++;
    for(int i=0;i<4;i++){
        cout<<arr[i]<<endl;
    }
}