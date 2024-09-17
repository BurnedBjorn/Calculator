#include "std_lib_facilities.h"

// struct that holds the user input. 
struct Token {
	char kind;
	double value;
	string name;
	Token(char ch) :kind(ch), value(0) { }
	Token(char ch, double val) :kind(ch), value(val) { }
	Token(char ch, string nm) :kind(ch), name(nm), value(0) { }

};

//User input management
class Token_stream {
	bool full;
	Token buffer;
	istream& ISinput;
public:
	Token_stream() :full(0), buffer(0), ISinput(cin){ }

	Token get();
	void unget(Token t) { buffer = t; full = true; }

	void ignore(char);
};

//constants
const char let = 'L';
const char quit = 'Q';
const char printres = ';';
const char number = '8';
const char name = 'a';
const char root = 'r';
const char power = 'p';
const char cst = '$';
const char help = 'h';
const string manual = "manual: \n- operations avaliable: addition(+), subtraction(-), multiplication(*),\n division(*), power(pow(n,i), square root(sqrt(n)). \n-power function: pow(n,i) n is the number or expression you want to put to a power, i is the number of power you put the number to (can be an expression), the brackets and comma are necessary\n- square root: sqrt(n) where n is any number or expression, will give you the square root of n \n-variables: to create a variable, write 'let <name of the variable> = <number or expression>'. to reassign the existing variable's value, write '<name> = <value>'.\n you can use variable like a number. \n- constants: you can add a constant. it is almost the same as the variable, but in declaration you write 'let const <name> = <value> and you can't change it. You cannot delete variables or constants, so be careful.\n-to exit the program, type 'quit' or 'exit', to print the manual again, type letter h\n\n\n";
const string prompt = "> ";
const string result = "= ";

//function that gets the user input
Token Token_stream::get()
{
	if (full) { full = false; return buffer; }
	char ch;
	cin.get(ch);
	while (ch == ' ') cin.get(ch);
	switch (ch) {
	case '(':
	case ')':
	case '+':
	case '-':
	case '*':
	case '/':
	case '%':
	case ';':
	case '=':
	case ',':
	case 'h':
		return Token(ch);
	case 'H':
		return Token(help);
	case '\n':
		return Token(printres);
	case '.':
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	{
		cin.unget();
		double val;
		cin >> val;
		return Token(number, val);
	}
	default:
		if (isalpha(ch)) {			// if character is a letter
			string s;				// create a string s
			s += ch;				// add character to string
			while (cin.get(ch) && ((isalpha(ch) || isdigit(ch) || (ch == '_')))) s += ch; // while ch is letter or digit, add to string 
			cin.unget();
			if (s == "let") return Token(let);
			if ((s == "quit")or (s=="exit")) return Token(quit);
			if (s == "sqrt") return Token(root);
			if (s == "pow") return Token(power);
			if (s == "const") return Token(cst);
			return Token(name, s);
		}
		error("Bad token");
	}
}

// Function that skips a token
void Token_stream::ignore(char c)
{
	if (full && c == buffer.kind) {
		full = false;
		return;
	}
	full = false;

	char ch;
	cin.unget();
	while (true){
		cin.get(ch);
		if (ch == c) return; 
		if (ch == '\n') return;
		continue;
	}	
	
}


// Data type for variables
struct Variable {
	string name;
	double value;
	bool isconst;
	Variable(string n, double v) :name(n), value(v), isconst(false) { }
	Variable(string n, double v, bool c) :name(n), value(v), isconst(c) { }
};

// class that handles variables
class Symbol_table {
	vector<Variable> var_table;
public:
	
	void set_value(string s, double d);
	bool is_declared(string s);
	double get_value(string s);
	void define_constant(string s, double d);
	void add_variable(string name, double d);

};

//set a value to an existing variable
void Symbol_table::set_value(string s, double d)
{

	for (int i = 0; i <= var_table.size(); ++i) {
		if ((var_table[i].name == s) && var_table[i].isconst == false) {
			var_table[i].value = d;
			return;
		}
		else if (var_table[i].isconst == true) error("trying to change a constant");
	}
	error("set: undefined name ", s);
}

//Function that just checks if the variable has been declared
bool Symbol_table::is_declared(string s)
{
	for (int i = 0; i < var_table.size(); ++i)
		if (var_table[i].name == s) return true;
	return false;
}

//Function that gets variable's name, returns it's value
double Symbol_table::get_value(string s)
{
	for (int i = 0; i < var_table.size(); ++i)
		if (var_table[i].name == s) return var_table[i].value;
	error("get: undefined name ", s);
}

//add constant to the table
void Symbol_table::define_constant(string s, double d) {
var_table.push_back(Variable(s, d, true));
}

// add variable to the table
void Symbol_table::add_variable(string name, double d) {

	var_table.push_back(Variable(name, d));
		
}


//symbol table instance
Symbol_table st;

//Token stream instance
Token_stream ts;


// expression function declaration
double expression(Token_stream& ts);

// power functionality
double fpower(Token_stream& ts) {
	
	Token t = ts.get();
	if (t.kind == '(')
	{
		double d = expression(ts);
		double b = d;
		t = ts.get();
		if (t.kind != ',') error(", expected");
		double p = expression(ts);
		t = ts.get();
		if (t.kind == ')')
		{
			for (int i = 1; i < p; i++)
			{
				d *= b;
			}
			return(d);
		}
		else error("pow: ) expected");
	}
	else error("pow: ( expected");
}

// root functionality
double froot(Token_stream& ts) {
	Token t = ts.get();
	if (t.kind == '(') {
		double d = expression(ts);
		if (d < 0) error("can't sqrt negative number");
		t = ts.get();
		if (t.kind == ')') {
			return sqrt(d);
		}
		else {
			error("root: ) expected");
		}
	}
	else {
		error("root: ( expected");
	}
}

// variable handling functionality
double fname(Token_stream& ts) {
	Token t = ts.get();
	if (st.is_declared(t.name)) {
		string localname = t.name;
		t = ts.get();
		if (t.kind == '=') { // if next symbol is = then
			t = ts.get();
			if (t.kind == number) {
				st.set_value(localname, t.value);
				cout << "value set: ";
				return t.value;
			}
			else {
				error("name: can't set value");
			}

		}
		else {
			ts.unget(t);
			return st.get_value(localname);
		}
	}
	else
	{
		return st.get_value(t.name);
	}
}

//gets the primary value to work with
double primary(Token_stream& ts)
{
	Token t = ts.get();
	switch (t.kind) {
	case '(':
	{
		double d = expression(ts);
		t = ts.get();
		if (t.kind != ')') error("'(' expected");
		return d;
	}
	case '-':
		return -primary(ts);
	case number:
		return t.value;
	case name:
	{
		ts.unget(t);
		return(fname(ts));
	}
	case root:
	{
		return froot(ts);
	}
	case power:
	{
		return fpower(ts);
	}
	default:
		error("primary expected");
	}
}

// Does multiplication/division to the primary if needed
double term(Token_stream& ts)
{
	double left = primary(ts);
	while (true) {
		Token t = ts.get();
		switch (t.kind) {
		case '*':
			left *= primary(ts);
			break;
		case '/':
		{
			double d = primary(ts);
			if (d == 0) error("divide by zero");
			left /= d;
			break;
		}
		default:
			ts.unget(t);
			return left;
		}
	}
}

// does addition and subtraction of the terms if needed
double expression(Token_stream& ts)
{
	double left = term(ts);
	while (true) {
		Token t = ts.get();
		switch (t.kind) {
		case '+':
			left += term(ts);
			break;
		case '-':
			left -= term(ts);
			break;	
		default:
			ts.unget(t);
			return left;
		}
	}
}

//handles variable declaration
double declaration(Token_stream& ts)
{
	bool iscst = false;
	Token t = ts.get();
	if (t.kind == cst) {
		iscst = true;
		t = ts.get();
	}
	if (t.kind != 'a') error("name expected in declaration");
	string name = t.name;
	if (st.is_declared(name)) error(name, " declared twice");
	Token t2 = ts.get();
	if (t2.kind != '=') error("= missing in declaration of ", name);
	double d = expression(ts);
	if (iscst) {
		st.define_constant(name, d);
		return d;
	}
	st.add_variable(name, d);
	return d;
}

//checks if the line was declaration or expression, proceeds accordingly
double statement(Token_stream& ts)
{
	Token t = ts.get();
	switch (t.kind) {
	case let:
		return declaration(ts);
		break;
	case quit:
		exit(0);
	default:
		ts.unget(t);
		return expression(ts);
	}
}


// skips until "printres" sign
void clean_up_mess(Token_stream& ts)
{
	
	ts.ignore(printres);
	//ts.ignore('\n');
}

//outputs prompt symbol when needed
static void fprompt() {
	cin.unget();
	char ch;
	cin.get(ch);
	while (ch == printres) cin.get(ch);
	if (ch == '\n') cout << prompt;
	cin.unget();
}


// calculation cycle
void calculate(Token_stream& ts)
{
	cout << prompt;
	while (true) try {	
		
		Token t = ts.get();
		
		while (t.kind == printres) t = ts.get();
		if (t.kind == help) {
			cout << manual << '\n';
			clean_up_mess(ts);
			fprompt();
			continue;
		}
		if (t.kind == quit) return;
		
		ts.unget(t);
		cout << result << statement(ts) << endl;
		
		fprompt();
	}
	catch (runtime_error& e) {
		cerr << e.what() << endl;
		clean_up_mess(ts);
		cout << "type 'h' to print out the manual\n";
		fprompt();
		
	}
}

// main function, runs calculacion cycle
int main()
{
	try {
		cout << manual<<'\n';
		calculate(ts);
		return 0;
	}
	catch (exception& e) {
		cerr << "exception: " << e.what() << endl;
		char c;
		while (cin >> c && c != ';');
		return 1;
	}
	catch (...) {
		cerr << "exception\n";
		char c;
		while (cin >> c && c != ';');
		return 2;
	}
}
