#include<cstdint>
const int fixed_digit = 22;
const int exponect_factor = 15;


template<typename _real>
class TrieSet
{
private:
    class TrieNode
    {
    public:
        uint32_t count;
        TrieNode * childs[10];
        TrieNode(/* args */)
        {
            count = 0;
            for(int i=0; i < 10; i++) childs[i] = NULL;
        }
        void recursiveDelete(){
            for(int i=0; i < 10; i++){
                if(childs[i] != NULL){
                    childs[i] -> recursiveDelete();
                    delete childs[i];
                    childs[i] = NULL;
                }
            }
        }
        bool clear_path(char * p, int depth){
            if(depth == 0){
                if(count){
                    count--;
                    return true;
                }
                return false;
            }

            if(childs[*p] == NULL or !childs[*p]->clear_path(p + 1, depth - 1)){
                return false;
            }
            if(childs[*p] -> count == 0){
                delete childs[*p];
                childs[*p] = NULL;
            }
            count--;
            return true;
        }
        std::vector<char> path_for_order(uint32_t order){
            uint32_t pref = 0;
            for(int i=0; i < 10; i++){
                if(childs[i] != NULL and childs[i] -> count + pref < order){
                    pref += childs[i] -> count;
                }else if(childs[i] != NULL ){
                    auto ret = childs[i]->path_for_order(order - pref);
                    ret.push_back(i);
                    return ret;
                }
            }
            std::vector<char> ret;
            return ret;
        }
    };


    TrieNode * root;

    void insert(char*);
    void erase(char*);
    void convert(_real,char*);
    
    /* data */
public:
    TrieSet(/* args */);
    ~TrieSet();
    _real find_by_order(uint32_t order);
    void insert(_real);
    void erase(_real);
};
template<typename _real> 
TrieSet<_real>::TrieSet(/* args */)
{
    root = new TrieNode();
}
template<typename _real> 
TrieSet<_real>::~TrieSet()
{
    root->recursiveDelete();
    delete root;
    root = NULL;
}
template<typename _real> 
void TrieSet<_real>::insert(char * num){
    TrieNode * tmp = root;
    for(int i = 0; i < fixed_digit; i++){

        if(tmp->childs[ num[i] ] == NULL){
            tmp->childs[ num[i] ] = new TrieNode();
        }
        tmp->count++;
        tmp = tmp->childs[ num[i] ];
    }
    tmp->count++;
}
template<typename _real> 
void TrieSet<_real>::erase(char* num){
    TrieNode * tmp = root;
    root->clear_path(num,fixed_digit);
}
template<typename _real> 
_real TrieSet<_real>::find_by_order(uint32_t order){
    if(order < 1 or order > root->count){
        return -1.0;
    }
    std::vector<char> path = root->path_for_order(order);

    _real after_decimal = 0;
    for(int i = 0; i <  exponect_factor; i++){
        after_decimal = (path[i] + after_decimal)/10.0;
    }
    _real before_decimal = 0;
    for(int i = fixed_digit - 1; i >= exponect_factor; i--){
        before_decimal = before_decimal*10 + path[i];
    }
    return before_decimal + after_decimal;
}

//0000005  900000000000000
template<typename _real> 
void TrieSet<_real>::insert(_real num){
    if(num < 0) return;
    char num_in_char[fixed_digit+1];
    convert(num,num_in_char);
    insert(num_in_char);
}
template<typename _real> 
void TrieSet<_real>::erase(_real num){
    if(num < 0) return;
    char num_in_char[fixed_digit];
    convert(num,num_in_char);
    erase(num_in_char);
}

template<typename _real> 
void TrieSet<_real>::convert(_real num, char * num_in_char){

    uint32_t integer_part = floor(num);
    _real decimal_part = num - integer_part;
    if(decimal_part < 0) decimal_part = 0;

    std::vector<char> v1;

    while(integer_part){
        v1.push_back(integer_part % 10);
        integer_part /= 10;
    }

    std::vector<char> v2;

    for(int i = 0; i < exponect_factor; i++){
        _real tmp = decimal_part * 10;
        v2.push_back(floor(tmp));
        decimal_part = tmp - floor(tmp);
    }

    int it = 0;

    for(;it < fixed_digit - v1.size() - v2.size() ;) num_in_char[it++] = 0;

    for(int i = v1.size() - 1; i >= 0; i--) {
        num_in_char[it++] = v1[i];
    }

    for(int i = 0; i < v2.size(); i++){
        num_in_char[it++] = v2[i];
    }
}

// int main(int argc, char ** argv){
//     using namespace std;
//     std::mt19937 r(time(0));

//     TrieSet<long double> tset;

//     tset.insert(5.9);
//     tset.insert(8.9);
//     tset.insert(9.3);
//     tset.insert(209.33);
//     tset.insert(0.0000085);
//     tset.insert(0.00000096);
//                 0.00000096
//     tset.insert(0.00000096);
//     tset.insert(0.002810);
//     tset.insert(3589.25);
//     tset.insert(99.36);

//     tset.erase(3589.25);
//     tset.erase(209.31);

//     cout.precision(8); cout<<fixed;
//     cout<<tset.find_by_order(1)<<endl;
//     cout<<tset.find_by_order(2)<<endl;
//     cout<<tset.find_by_order(3)<<endl;
//     cout<<tset.find_by_order(4)<<endl;
//     cout<<tset.find_by_order(5)<<endl;
//     cout<<tset.find_by_order(6)<<endl;
//     cout<<tset.find_by_order(7)<<endl;
//     cout<<tset.find_by_order(8)<<endl;
//     cout<<tset.find_by_order(9)<<endl;

//     return 0;
// }