

/*
-The syntax is meant to ressemble assembly a little bit.
-A document will contain all code internally, with numbered/indexable lines like Basic, so we can jump to and fro anywhere
-No parameters passed inside parentheses.
-Parameters are setup prior to calling a function as such:
     = param1 42
     = param2 24
     () function
-Maximum of 4 parameters(although one could use global variables)
-function blocks and labels are declared with :, functions end with @, as such:
     : add_42
         param2 = 42
         + //adds params 1 and 2
     @
-functions can return up to 2 values, like
     : get2
         = return1 911
         = return2 119
     @
-Printing on standard output with one parameter:
    = param1 42
    # //output 42
-To print a character
    = param1 97
    ## //output 'a'
-names of variables and functions cannot contain whitespaces, unprintable characters or operators.
-Operators are :
    = assignment
    == eq
    != neq
    + add
    - sub
    * mult
    / div
    : label
    -> jump
    () call label
    # print
    ## printchar
    @ return
    */
    /* comments multiline
-Operators must be surrounded by whitespaces
-Results of operations naturally go in result1
-Only one operator, one instruction per line
-Data types: number
    -number: unsigned integer
-All variables are mutable, global and must be assigned a value, may it be zero
    = myvar 0
-All variables are global
-Conditions are expressed as such
    == for equality of params 1 and 2
    != for inequality of params 1 and 2
    
-True conditions will execute the following one line
-False conditions will skip the following one line

-Jumps are made as such
    -> label

-Labels are declared with colons
    : label
    
*/


/*
The lexer will read through the code and tokenize it. It will decorticate the code in pieces and assign each piece a category. The lexer also eliminates comments.
*/

#include <iostream>
#include <sstream> 
#include <vector> //for code document
#include <stack> //for call stack
#include <map> //for varmaps

using namespace std;

string code{
R"(
@
/* function label */
: printnum3times
    /* assign 3 to variable "loop" */
    = loops 3
    /* loop label */
    : loop
    
    /* print 42 instruction */
    = param1 42
    #
    
    /* print newline character instruction */
    = param1 10
    ##
    
    /* subtract 1 from loops */
    = param1 loops
    = param2 1
    -
    /* store result in loops */
    = loops return1
    
    /* loop if loops is not equal to 0 */
    = param1 loops
    = param2 0
    !=
    -> loop
@

: start
() printnum3times
: end

)"
};

string code2{
R"(
@
: fib
    = a 0
    = b 1
    = temp 0
    = loops 10
    : loop
    = param1 a
    #
    
    = param1 10
    ##
    
    = temp b
    
    = param1 a
    = param2 b
    +
    = b return1
    
    = a temp
    
    = param1 loops
    = param2 1
    -
    = loops return1
    
    = param1 loops
    = param2 0
    !=
    -> loop
@

: start
() fib
: end

)"
};


typedef unsigned long long var;

var param1, param2, param3, param4;
var return1, return2;
var programCounter = 0;
map<string,var> globals;
stack<var> callstack; //contains return document indices.
map<string,var> varmap; //contains variable registry
map<string,var> labelmap; //contains label indices registry
map<string, var> functionIndices; //indices of function blocks 


class Token{
    public:
    string type;
    string value;
    void print()const{
        cout<<"["<<type<<" "<<value <<"]"<<endl;
    }
};

//Commands

struct Command{
    virtual void operator()()=0;
    virtual ~Command()=0;
    virtual void print(){
        cout<<"Print\n";
    }
};
Command::~Command(){}

struct add:public Command{
    void operator()(){
        return1 = param1 + param2 ;
    }
    void print(){
        cout<<"Add\n";
    }
};

struct sub:public Command{
    void operator()(){
        return1 = param1 - param2 ;
    }
    void print(){
        cout<<"Sub\n";
    }
};

struct mult:public Command{
    void operator()(){
        return1 = param1 * param2 ;
    }
    void print(){
        cout<<"Mult\n";
    }
};

struct divide:public Command{
    void operator()(){
        return1 = param1 / param2 ;
    }
    void print(){
        cout<<"Div\n";
    }
};

var& getAddr(string varname){
    auto it = varmap.find(varname);
    if(it != varmap.end()){
        return (varmap[varname]);
    }else if(varname=="param1"){
        return param1;
    }else if(varname=="param2"){
        return param2;
    }else if(varname=="param3"){
        return param3;
    }else if(varname=="param4"){
        return param4;
    }else if(varname=="return1"){
        return return1;
    }else if(varname=="return2"){
        return return2;
    }
    varmap[varname]=0;
    return (varmap[varname]);
}

struct assign:public Command{
    string emitter;
    string receiver;
    assign(string a, string b){
        receiver = a;
        emitter = b;
    }
    void operator()(){
        var & aptr = getAddr(receiver);
        if(isdigit(emitter[0])){
        aptr= stoi(emitter);
        }
        else{
        var & bptr = getAddr(emitter);
        aptr = bptr;
        }
    }
    void print(){
        cout<<"Assign "<<receiver<<" "<<emitter<<endl;
    }
};

struct print:public Command{
    void operator()(){
        cout << param1 ;
    }
    
};

struct printchar:public Command{
    void operator()(){
        cout << char(param1) ;
    }
    void print(){
        cout<<"Printchar\n";
    }
};

/////////////
struct call:public Command{
    string label;
    call(string s){
        label = s;
    }
    void operator()(){
        callstack.push(++programCounter);
        programCounter = functionIndices[label] - 1;
    }
    void print(){
        cout<<"Call "<<label<<"\n";
    }
};

struct ret:public Command{
    void operator()(){
        programCounter = callstack.top();
        callstack.pop();
    }
    void print(){
        cout<<"Ret\n";
    }
};

struct jump:public Command{
    string label;
    jump(string s){
        label = s;
    }
    void operator()(){
        programCounter = functionIndices[label] -1;
    }
    void print(){
        cout<<"Jump "<<label<<"\n";
    }
};
//////////////

struct eq:public Command{
    void operator()(){
        if(param1 != param2)
        ++programCounter;
    }
    void print(){
        cout<<"Eq\n";
    }
};

struct neq:public Command{
    void operator()(){
        if(param1 == param2)
        ++programCounter;
    }
    void print(){
        cout<<"Neq\n";
    }
};
//List of registered commands
vector<Command*>document;

//Interpreter functions
vector<Token> lex(string text);
void parse(const vector<Token>&tokens);
void execute();

void clearAll();
void clearDocument();
void clearVars();
void clearLabels();
void clearCallStack();

int main() {
    try{
        auto tokens = lex(code);
        parse(tokens);
    }catch(const char * err){
        cerr<<err<<endl;
    }
    execute();
    clearAll();
    programCounter = 0;
    try{
        auto tokens = lex(code2);
        parse(tokens);
    }catch(const char * err){
        cerr<<err<<endl;
    }
    execute();
    return 0;
}


















void clearAll(){
    clearVars();
    clearDocument();
    clearLabels();
    clearCallStack();
}

void clearVars(){
    varmap.clear();
}

void clearDocument(){
    for(auto & p:document){
        delete p;
    }
    document.clear();
}

void clearLabels(){
    functionIndices.clear();
}

void clearCallStack(){
    while(!callstack.empty())
    callstack.pop();
}

void execute(){
    programCounter = functionIndices["start"];
    var count=0;
    while(programCounter<document.size()&&count<400){
        (*document[programCounter])();
        ++programCounter;
        ++count;
    }
}

void parse(const vector<Token> & tokens){
    var index;
    for(index=0;index<tokens.size();++index){
        const Token t = tokens.at(index);
        if(t.type == "operator"){
            if(t.value==":"){
                Token next = tokens[++index];
                if(next.type!="id")
                throw "Label id error\n";
                functionIndices[next.value] = programCounter ;
            }else if(t.value=="="){
                Token next = tokens[++index];
                Token next2 = tokens[++index];
                if(next.type == "operator" ||
                   next.type == "number" ||
                   next2.type == "operator")
                throw "Assignment operand error\n";
                Command * c = new assign(next.value, next2.value);
                document.push_back(c);
                ++programCounter;
            }else if(t.value == "#"){
                Command * c = new print();
                document.push_back(c);
                ++programCounter;
            }else if(t.value == "##"){
                Command * c = new printchar();
                document.push_back(c);
                ++programCounter;
            }else if(t.value == "-"){
                Command * c = new sub();
                document.push_back(c);
                ++programCounter;
            }else if(t.value == "+"){
                Command * c = new add();
                document.push_back(c);
                ++programCounter;
            }else if(t.value == "*"){
                Command * c = new mult();
                document.push_back(c);
                ++programCounter;
            }else if(t.value == "/"){
                Command * c = new divide();
                document.push_back(c);
                ++programCounter;
            }else if(t.value == "=="){
                Command * c = new eq();
                document.push_back(c);
                ++programCounter;
            }else if(t.value == "!="){
                Command * c = new neq();
                document.push_back(c);
                ++programCounter;
            }else if(t.value == "->"){
                Token next = tokens[++index];
                if(next.type!="id")
                throw "Jump label error\n";
                Command * c = new jump(next.value);
                document.push_back(c);
                ++programCounter;
            }else if(t.value == "()"){
                Token next = tokens[++index];
                if(next.type!="id")
                throw "Call label error\n";
                Command * c = new call(next.value);
                document.push_back(c);
                ++programCounter;
            }else if(t.value == "@"){
                Command * c = new ret();
                document.push_back(c);
                ++programCounter;
            }else if(t.value == "/*"){
                Token next = tokens[++index];
                while(next.value!="*/")
                next = tokens[++index];
            }
            
        }else{
            throw "Line should start with operator\n";
        }
    }
}

vector<Token> lex (string text){
    stringstream ss{text};
    string sbuf{};
    vector<Token> tokens;
    while(ss>>sbuf){
        Token t;
        char first = sbuf[0];
        if(isdigit(first)){
            t.type = "number";
        }else if(isalpha(first)){
            t.type = "id";
        }else{
            t.type = "operator";
        }
        t.value = sbuf;
        tokens.push_back(t);
    }
    return tokens;
}

