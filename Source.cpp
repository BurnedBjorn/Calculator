
/*
	calculator08buggy.cpp

	Helpful comments removed.

	We have inserted 3 bugs that the compiler will catch and 3 that it won't.
*/

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
public:
	Token_stream() :full(0), buffer(0) { }

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


//function that gets the user input
Token Token_stream::get()
{
	if (full) { full = false; return buffer; }
	char ch;
	cin >> ch;
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
		return Token(ch);
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
			if (s == "quit") return Token(quit);
			if (s == "sqrt") return Token(root);
			if (s == "pow") return Token(power);
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
	while (cin >> ch)
		if (ch == c) return;
}

// Data type for variables
struct Variable {
	string name;
	double value;
	bool isconst;
	Variable(string n, double v) :name(n), value(v), isconst(false) { }
	Variable(string n, double v, bool c) :name(n), value(v), isconst(c) { }
};

class Symbol_table {
	vector<Variable> var_table;
public:
	
	void set_value(string s, double d);
	bool is_declared(string s);
	double get_value(string s);
	void define_constant(string s, double d);
	void add_variable(string name, double d);

};


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

// make a predefined variable
void Symbol_table::define_constant(string s, double d) {
	var_table.push_back(Variable(s, d, true));
}

// add variable to the table
void Symbol_table::add_variable(string name, double d) {
	var_table.push_back(Variable(name, d));
}



Symbol_table st;
//Token stream instance i guess
Token_stream ts;

// expression function declaration
double expression();



//gets the primary value to work with
double primary()
{
	Token t = ts.get();
	switch (t.kind) {
	case '(':
	{
		double d = expression();
		t = ts.get();
		if (t.kind != ')') error("'(' expected");
		return d;
	}
	case '-':
		return -primary();
	case number:
		return t.value;
	case name:
	{
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
					error("can't set value");
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
	case root:
	{
		t = ts.get();
		if (t.kind == '(') {
			double d = expression();
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
	case power:
	{
		t = ts.get();
		if (t.kind == '(')
		{
			double d = expression();
			double b = d;
			t = ts.get();
			if (t.kind != ',') error(", expected");
			double p = expression();
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
	default:
		error("primary expected");
	}
}

// Does multiplication/division to the primary if needed
double term()
{
	double left = primary();
	while (true) {
		Token t = ts.get();
		switch (t.kind) {
		case '*':
			left *= primary();
			break;
		case '/':
		{
			double d = primary();
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
double expression()
{
	double left = term();
	while (true) {
		Token t = ts.get();
		switch (t.kind) {
		case '+':
			left += term();
			break;
		case '-':
			left -= term();
			break;	
		default:
			ts.unget(t);
			return left;
		}
	}
}

//handles variable declaration
double declaration()
{
	Token t = ts.get();
	if (t.kind != 'a') error("name expected in declaration");
	string name = t.name;
	if (st.is_declared(name)) error(name, " declared twice");
	Token t2 = ts.get();
	if (t2.kind != '=') error("= missing in declaration of ", name);
	double d = expression();
	st.add_variable(name, d);
	return d;
}



//checks if the line was declaration or expression, proceeds accordingly
double statement()
{
	Token t = ts.get();
	switch (t.kind) {
	case let:
		return declaration();
		break;
	case quit:
		exit(0);
	default:
		ts.unget(t);
		return expression();
	}
}

// skips until "printres" sign
void clean_up_mess()
{
	ts.ignore(printres);
}

// symbols
const string prompt = "> ";
const string result = "= ";

// main function, runs calculacion cycle
void calculate()
{
	while (true) try {
		cout << prompt;
		Token t = ts.get();
		while (t.kind == printres) t = ts.get();
		if (t.kind == quit) return;
		ts.unget(t);
		cout << result << statement() << endl;
	}
	catch (runtime_error& e) {
		cerr << e.what() << endl;
		clean_up_mess();
	}
}

int main()
{
	try {
		//Symbol_table::define_constant("pi", 3.141592);
		calculate();
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