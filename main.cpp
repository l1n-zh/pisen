#include <algorithm>
#include <iostream>
#include <math.h>
#include <queue>
#include <regex>
#include <string.h>
#include <unordered_map>

using std::string;

/* TOKEN */

typedef enum {
	TK_PLUS,
	TK_MINUS,
	TK_MULTIPLY,
	TK_DIVIDE,
	TK_MODULUS,
	TK_POWER,
	TK_NUMBER,
	TK_LPAREN,
	TK_RPAREN,
	TK_EOF,
} TokenKind;

struct Token {
	operator bool() {
		return true;
	}
	TokenKind kind;
	int idx;
	std::string value;
};

/* LEXER */
std::string input;

std::unordered_map<char, TokenKind> tokenMap = {
	{'+', TK_PLUS},
	{'-', TK_MINUS},
	{'/', TK_DIVIDE},
	{'%', TK_MODULUS},
	{'(', TK_LPAREN},
	{')', TK_RPAREN},
};

void error(std::string err, int pos) {
	using namespace std;
	cout << input << endl << string(pos, ' ') + "^ " << err << endl;
	exit(0);
}

std::string match_number(std::string s) {
	using namespace std;
	regex reg("\\d+(\\.+\\d+)*");
	smatch match;
	regex_search(s, match, reg);
	return match[0];
}

std::queue<Token> tokenize(std::string s) {
	input = s;
	std::queue<Token> tokens;
	unsigned idx = 0;
	char peek;

	while (idx < input.size()) {
		peek = input[idx];

		if (isspace(peek)) {
			++idx;
			continue;
		} else {
			Token token;
			token.idx = idx;
			if (tokenMap.count(peek)) {
				token.kind = tokenMap[peek];
				tokens.push(token);
				++idx;
				continue;
			}
			if (peek == '*') {
				if (input[++idx] == '*') {
					token.kind = TK_POWER;
					tokens.push(token);
					++idx;
				} else {
					token.kind = TK_MULTIPLY;
					tokens.push(token);
				}
				continue;
			}
			if (isdigit(peek)) {
				std::string value = match_number(input.substr(idx));
				token.kind = TK_NUMBER;
				token.value = value;
				tokens.push(token);
				idx += value.size();
				continue;
			}
			error("標記解析失敗", idx);
			break;
		}
		//
		//		Token token;
		//		token.kind = TK_EOF;
		//		tokens.push(token);
	}
	return tokens;
}


string add(string a, string b) {
	int len_a = a.size() - 1, len_b = b.size() - 1;

	string result = "";
	int carry = 0;
	while (len_a >= 0 || len_b >= 0) {
		int tmp = 0;
		if (len_a >= 0)
			tmp += a[len_a--] - '0';
		if (len_b >= 0)
			tmp += b[len_b--] - '0';
		tmp += carry;

		carry = tmp / 10;
		result += '0' + tmp % 10;
	}
	if (carry > 0) {
		result += '0' + carry;
	}
	reverse(result.begin(), result.end());
	int i = 0;
	while(result[i]=='0') ++i;
	return result.substr(i);
}

string minus(string a, string b) {
	int len_a = a.size() - 1, len_b = b.size() - 1;

	string result = "";
	int carry = 0;
	while (len_a >= 0 || len_b >= 0) {
		int tmp = a[len_a--] - '0';

		if (len_b >= 0)
			tmp -= b[len_b--] - '0';

		tmp -= carry;
        if(tmp<0){
            tmp += 10;
            carry = 1;
        }else
            carry = 0;
		result += '0' + tmp % 10;
	}
	reverse(result.begin(), result.end());
	int i = 0;
	while(result[i]=='0') ++i;
	return result.substr(i);
}

string multiply(string a, string b) {
    if(a.size()<=9 && b.size()<=9) {
        if(a.size()&&b.size()) {
            return std::to_string(stoll(a)*stoll(b));
        }
        return "";
    }

    int sizeA = a.size();
    int sizeB = b.size();

    int half = std::max(sizeA, sizeB) / 2;
    string e,f,g,h,n1,n2,n3;
    if(sizeA-half < 0){
        e = "0";
        f = a;
    }else{
        e = a.substr(0,sizeA-half);
        f = a.substr(sizeA-half);
    }
    if(sizeB-half < 0){
        g = "0";
        h = b;
    }else{
        g = b.substr(0,sizeB-half);
        h = b.substr(sizeB-half);
    }

    n1 = multiply(e,g);
    n2 = multiply(f,h);
    n3 = multiply(add(e,f), add(g,h));
    n3 = minus(n3,add(n1,n2));
	return add(add(n1+string(2*half,'0'), n3+string(half,'0')), n2);
}

string divide(string a, string b) {
	return "";
}

string modulo(string a, string b) {
	return "";
}

string power(string a, string b){
    string cur = a;
    long long other = std::stoll(b);
    string result = "1";
    while (other) {
        if (other % 2)
            result = multiply(result,cur);
        other /= 2;
        cur = multiply(cur,cur);
    }
    return result;
}

/* PARSER */
std::queue<Token> tokens;

string term();
string mul();
string expr();
string pow();

string pow() {
	string a = term();
	while (!tokens.empty()) {
		Token token = tokens.front();
		if (token.kind == TK_POWER) {
			tokens.pop();
			string b = term();
			string n = a;
			a = power(a, b);
		} else
			break;
	}
	return a;
}

string term() {
	if (tokens.empty())
		error("算式未完成", input.size());
	Token token = tokens.front();
	if (token.kind == TK_LPAREN) {
		tokens.pop();
		string result = expr();
		if (tokens.front().kind == TK_RPAREN)
			tokens.pop();
		else
			error("括號不成對", token.idx);
		return result;
	}
	if (token.kind == TK_NUMBER) {
		tokens.pop();
		return token.value;
	} else
		error("不是數值", token.idx);
	return 0;
}

string mul() {
	string a = pow();
	while (!tokens.empty()) {
		Token token = tokens.front();
		if (token.kind == TK_MULTIPLY) {
			tokens.pop();
			string b = pow();
			a = multiply(a, b);
		} else if (token.kind == TK_DIVIDE) {
			tokens.pop();
			string b = pow();
			a = divide(a, b);
		} else if (token.kind == TK_MODULUS) {
			tokens.pop();
			string b = pow();
			a = modulo(a, b);
		} else
			break;
	}
	return a;
}

string expr() {
	string a = mul();
	while (!tokens.empty()) {
		Token token = tokens.front();
		if (token.kind == TK_PLUS) {
			tokens.pop();
			string b = mul();
			a = add(a, b);
		} else if (token.kind == TK_MINUS) {
			tokens.pop();
			string b = mul();
			a = minus(a, b);
		} else if (token.kind == TK_LPAREN || token.kind == TK_RPAREN)
			error("括號不成對", token.idx);
	}
	return a;
}

string parse() {
	/*
	expr  = mul ( "+" | "-" ) mul
	mul   = pow ( "*" | "/" | "% ) pow
	pow = term "**" term
	term  = num | "(" expr ")"
	*/
	return expr();
}  

/* MAIN */
int main() {
	using namespace std;
	string content;
	while (true) {
		cout << "輸入算式>> ";
		getline(cin, content);
		tokens = tokenize(content);
		cout << parse() << std::endl;
	}
	return 0;
}
