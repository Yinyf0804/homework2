// DLLa.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <time.h>
#include "Core.h"
using namespace std;


struct MyExpression {            //一个表达式
	string expr;                 //表达
	string expr_normalized;      //标准化表达
	string expr_answer;          //答案
	char expr_oper;              //保存操作符
};

class MyQuestion {
public:
	void GetQuestion(int operator_number, MyExpression& expression);
	//获取一个问题/表达式
protected:
	vector<char> oper_set; //运算符集合
	pair<int, int> range;  //数字的范围
	bool is_standard = true; //表达式是否符合规范
	void Clear(MyExpression& expression); //清空表达式
	void Combine(MyExpression& expression, MyExpression expression1, MyExpression expression2);
	//将两组表达式结合
	virtual bool HandleException(MyExpression& expression) = 0; //处理例外情况
	virtual string Calculate(string answer1, char oper, string answer2) = 0; //计算结果
	virtual string GetRandomNumber() = 0; //获取随机数字（的字符串形式）
};

//整数运算
class MyQuestionInInt : public MyQuestion {
public:
	MyQuestionInInt() {
		support_devision = true;
		support_exponentiation = true;
		oper_set.push_back('+');
		oper_set.push_back('-');
		oper_set.push_back('*');
		range.first = 0;
		range.second = 100;
	}
	MyQuestionInInt(vector<char> oper, pair<int, int> input_range) {
		support_devision = false;
		support_exponentiation = false;
		auto iter = find(oper.begin(), oper.end(), '/');
		if (iter != oper.end()) {
			oper.erase(iter);
			support_devision = true;
		}
		iter = find(oper.begin(), oper.end(), '^');
		if (iter != oper.end()) {
			oper.erase(iter);
			support_exponentiation = true;
		}
		oper_set = oper;
		range = input_range;
	}
private:
	bool support_devision; //是否支持除法
	bool support_exponentiation; //是否支持乘方
	bool isInRange(string a); //是否在范围内
	//以下函数继承至MyQuestion
	bool HandleException(MyExpression& expression); 
	string Calculate(string answer1, char oper, string answer2);
	string GetRandomNumber();
	string SqrtGetRandomNumber();
};

//小数运算，与上面类似
class MyQuestionInFloat : public MyQuestion {
public:
	MyQuestionInFloat() {
		oper_set.push_back('+');
		oper_set.push_back('-');
		oper_set.push_back('*');
		oper_set.push_back('/');
		range.first = 0;
		range.second = 100;
	}
	MyQuestionInFloat(vector<char> oper, pair<int, int> input_range) {
		oper_set = oper;
		range = input_range;
	}
private:
	bool HandleException(MyExpression& expression);
	string Calculate(string answer1, char oper, string answer2);
	string GetRandomNumber();
	//	double Round(double dVal, short iPlaces);
};

//分数运算
class MyQuestionInFraction : public MyQuestion {
public:
	MyQuestionInFraction() {
		oper_set.push_back('+');
		oper_set.push_back('-');
		oper_set.push_back('*');
		oper_set.push_back('/');
		range.first = 0;
		range.second = 100;
	}
	MyQuestionInFraction(vector<char> oper, pair<int, int> input_range) {
		oper_set = oper;
		range = input_range;
	}
private:
	bool HandleException(MyExpression& expression);
	string Calculate(string answer1, char oper, string answer2);
	string GetRandomNumber();
	void reduction(int a[], string& ans);
	void split(string src[], string str);
};

void MyQuestion::GetQuestion(int operator_number, MyExpression &expression) {
	if (operator_number == 0) {
		expression.expr_answer = GetRandomNumber();
		expression.expr = expression.expr_answer;
		expression.expr_normalized = expression.expr_answer;
		expression.expr_oper = ' ';
		return;
	}
	else if (operator_number == 1) {
		bool is_exception = HandleException(expression);
		if (is_exception) {
			return;
		}
	}
	MyExpression expression1, expression2;
	while (true)
	{
		int key = (rand() % operator_number);               //get random number key in range(0,number-1)
		Clear(expression1);
		Clear(expression2);
		GetQuestion(key, expression1);              //expression1 = getQuestion(key)
		GetQuestion(operator_number - key - 1, expression2);    //expression2 = getQuestion(num - key - 1)
		expression.expr_oper = oper_set[rand() % oper_set.size()];    //get random operator
		expression.expr_answer = Calculate(expression1.expr_answer, expression.expr_oper, expression2.expr_answer);    //calc answer
		if (is_standard) {
			break;
		}
	}
	Combine(expression, expression1, expression2);    //form question //通过operator判断是否需要加括号
}

void MyQuestion::Clear(MyExpression& expression) {
	expression.expr_oper = '\0';
	expression.expr = "";
	expression.expr_answer = "";
	expression.expr_normalized = "";
}

void MyQuestion::Combine(MyExpression& expression, MyExpression expression1, MyExpression expression2) {
	char p = expression.expr_oper;
	if (p != '^' && p != '-' && p != '/' && expression1.expr_normalized > expression2.expr_normalized)      //小的排前面
		expression1.expr_normalized.swap(expression2.expr_normalized);
	expression.expr_normalized = '(';
	expression.expr_normalized += expression1.expr_normalized;
	expression.expr_normalized += expression.expr_oper;
	expression.expr_normalized += expression2.expr_normalized;
	expression.expr_normalized += ')';
	//combine normalised question
	if (expression.expr_oper == '^') {
		if (expression1.expr_oper != ' ') {
			expression.expr += '(';
			expression.expr += expression1.expr;
			expression.expr += ')';
		}
		else {
			expression.expr += expression1.expr;
		}
		expression.expr += expression.expr_oper;

		if (expression2.expr_oper != ' ') {
			expression.expr += '(';
			expression.expr += expression2.expr;
			expression.expr += ')';
		}
		else {
			expression.expr += expression2.expr;
		}
	}

	else if ((expression.expr_oper == '*') || (expression.expr_oper == '/')) {
		if (expression1.expr_oper == '+' || expression1.expr_oper == '-') {
			expression.expr += '(';
			expression.expr += expression1.expr;
			expression.expr += ')';
		}
		else {
			expression.expr += expression1.expr;
		}
		expression.expr += expression.expr_oper;

		if (expression2.expr_oper != ' ') {
			expression.expr += '(';
			expression.expr += expression2.expr;
			expression.expr += ')';
		}
		else {
			expression.expr += expression2.expr;
		}
	}
	else {
		expression.expr += expression1.expr;
		expression.expr += expression.expr_oper;
		if (expression2.expr_oper == '+' || expression2.expr_oper == '-') {
			expression.expr += '(';
			expression.expr += expression2.expr;
			expression.expr += ')';
		}
		else {
			expression.expr += expression2.expr;
		}
	}
	//combine question/expression
}

string MyQuestionInInt::SqrtGetRandomNumber() {
	int upper = 2 * sqrt(range.second);
	int lower = (sqrt(range.first) + 1) / 2 + 1;
	int num = rand() % (upper - lower) + lower;
	return to_string(num);
}//get random number in range (sqrt(lower)/2, sqrt(upper)*2)




string MyQuestionInInt::GetRandomNumber() {
	int num = rand() % (range.second - range.first) + range.first;
	return to_string(num);
}//get random number

bool MyQuestionInInt::HandleException(MyExpression& expression) {
	if (!support_devision && !support_exponentiation) {
		return false;
	}
	int key;
	if (oper_set.empty()) {//if there's no operator in operset
		if (!support_devision && support_exponentiation) {
			key = 1;
		}
		else if (support_devision && !support_exponentiation) {
			key = 2;
		}
		else {
			key = (rand() % 2) + 1;
		}
	}
	else { //there is other operators in operset
		if (!support_devision && support_exponentiation) {
			key = rand() % 2;
		}
		else if (support_devision && !support_exponentiation) {
			key = (rand() % 2) * 2;
		}
		else {
			key = rand() % 3;
		}
	}
	string a1, a2, a3;
	if (key == 0) {
		return false;
	}
	else if (key == 1) { //exponentiation
		while (true) {
			a1 = GetRandomNumber();
			if (stoi(a1) <= 50) {
				break;
			}
		}

		a2 = to_string(rand() % 5);
		expression.expr_oper = '^';
	}
	else { //devision
		while (true) {
			a2 = SqrtGetRandomNumber();
			a3 = SqrtGetRandomNumber();
			a1 = Calculate(a2, '*', a3);
			if (stoi(a1) <= range.second && stoi(a1) >= range.first) {
				break;
			}
		}
		expression.expr_oper = '/';
	}
	expression.expr_answer = Calculate(a1, expression.expr_oper, a2);
	expression.expr = a1;
	expression.expr += expression.expr_oper;
	expression.expr += a2;
	expression.expr_normalized = "(";
	expression.expr_normalized += expression.expr;
	expression.expr_normalized += ")";
	return true;
}

//calculate result
string MyQuestionInInt::Calculate(string answer1, char oper, string answer2) {
	string num;
	int number, num1, num2;
	num1 = stoi(answer1);
	num2 = stoi(answer2);
	is_standard = true;
	if (oper == '+') {
		number = num1 + num2;
	}
	else if (oper == '-') {
		number = num1 - num2;
	}
	else if (oper == '*') {
		number = num1 * num2;
	}
	else if (oper == '/') {
		number = num1 / num2;
	}
	else if (oper == '^') {
		number = pow(num1, num2);
	}
	else {
		is_standard = false;
		return "-1";
	}
	if (number < 0) {
		is_standard = false;
		return "-1";
	}
	num = to_string(number);
	return num;
}




//no exception here
bool MyQuestionInFloat::HandleException(MyExpression &expression) {
	return false;
}

//calculate result
string MyQuestionInFloat::Calculate(string answer1, char oper, string answer2) {
	string num;
	double number, num1, num2;
	num1 = stod(answer1);
	num2 = stod(answer2);
	is_standard = true;
	if (oper == '+') {
		number = num1 + num2;
	}
	else if (oper == '-') {
		number = num1 - num2;
		if (number < 0) {
			is_standard = false;
		}
	}
	else if (oper == '*') {
		number = num1 * num2;
	}
	else if (oper == '/') {
		if (num2 == 0) {
			is_standard = false;
			return("-1");
		}
		number = num1 / num2;
	}
	else {
		number = -1;
	}
	/*		else if (oper == '^') {
	number = pow(num1, num2);
	}
	*/
	//	num = to_string(Round(number, 2));
	//	num = num.substr(0, num.size() - 4);
	num = to_string(number);
	return num;
}

// get random number
string MyQuestionInFloat::GetRandomNumber() {
	int random = rand() % (range.second * 10 - range.first * 10) + range.first * 10;
	double number = double(random) / 10;
	string num = to_string(number);
	return(num.substr(0, num.size() - 5));
}

//get random number
string MyQuestionInFraction::GetRandomNumber() {
	int num = rand() % (range.second - range.first) + range.first;
	return to_string(num);

}

//split string (a/b) to (a,b)
void MyQuestionInFraction::split(string src[], string str) {
	//int size = str.size();
	int i = str.find("/");
	if (i == -1) {
		src[0] = str;
		src[1] = "1";
	}
	else {
		src[0] = str.substr(0, i);
		src[1] = str.substr(i + 1);
	}
}

//reduction of a fraction number
void MyQuestionInFraction::reduction(int a[], string& ans) {
	if (a[0] == 0) {
		ans = "0";
		return;
	}
	if (a[1] == 0) {
		ans = "-1";
		is_standard = false;
		return;
	}
	if (a[0] < 0 || a[1] < 0) {
		ans = "-1";
		is_standard = false;
		return;
	}
	int max, min;
	if (a[0] > a[1]) {
		max = a[0];
		min = a[1];
	}
	else {
		max = a[1];
		min = a[0];
	}
	while (max % min != 0) {
		int temp = max % min;
		max = min;
		min = temp;
	}
	int up = a[0] / min, down = a[1] / min;
	if (down == 1) {
		ans = to_string(up);
	}
	else {
		ans = to_string(up);
		ans += "/";
		ans += to_string(down);
	}
}

// calculate result
string MyQuestionInFraction::Calculate(string answer1, char oper, string answer2) {
	string num, a[2], b[2];
	split(a, answer1);
	split(b, answer2);
	int answer[2];
	int a0 = stoi(a[0]), a1 = stoi(a[1]), b0 = stoi(b[0]), b1 = stoi(b[1]);
	is_standard = true;

	if (oper == '+') {
		answer[0] = a0 * b1 + a1 * b0;
		answer[1] = a1 * b1;
	}
	else if (oper == '-') {
		answer[0] = a0 * b1 - a1 * b0;
		answer[1] = a1 * b1;
	}
	else if (oper == '*') {
		answer[0] = a0 * b0;
		answer[1] = a1 * b1;
	}
	else if (oper == '/') {
		if (stoi(b[0]) == 0) {
			is_standard = false;
			return num;
		}
		answer[0] = a0 * b1;
		answer[1] = a1 * b0;
	}
	reduction(answer, num);
	return num;
}


//no exception here
bool MyQuestionInFraction::HandleException(MyExpression& expression) {
	return false;
}

QuestionSetGenerator::QuestionSetGenerator() {
	ques_num = 10;
	oper_num = 3;
	oper_set.push_back('+');
	oper_set.push_back('-');
	oper_set.push_back('*');
	oper_set.push_back('/');
	oper_set.push_back('@');
	mode = 0;
	input_range.first = 0;
	input_range.second = 100;
}

double  QuestionSetGenerator::Round(double dVal, short iPlaces) {
	double dRetval;
	double dMod = 0.0000001;
	if (dVal<0.0) dMod = -0.0000001;
	dRetval = dVal;
	dRetval += (5.0 / pow(10.0, iPlaces + 1.0));
	dRetval *= pow(10.0, iPlaces);
	dRetval = floor(dRetval + dMod);
	dRetval /= pow(10.0, iPlaces);
	return(dRetval);
}

int QuestionSetGenerator::Setting(int quesnum, int opernum, string oper, int question_type, int lower_bound, int upper_bound) {
	ques_num = 10;
	oper_num = 3;
	oper_set.push_back('+');
	oper_set.push_back('-');
	oper_set.push_back('*');
	oper_set.push_back('/');
	mode = 1;
	input_range.first = 0;
	input_range.second = 100;
	flag = true;
	if (quesnum <= 0 || opernum <= 0) {//question number and operator number should > 0
		flag = false;
		return -1;
	}
	if (question_type < 0 || question_type>2) { //question type should be 0,1 or 2
		flag = false;
		return -1;
	}
	int key[5] = { 0,0,0,0,0 };
	for (int i = 0; i < oper.size(); i++) {
		if (oper[i] == '+') {
			key[0] = 1;
		}
		else if (oper[i] == '-') {
			key[1] = 1;
		}
		else if (oper[i] == '*') {
			key[2] = 1;
		}
		else if (oper[i] == '/') {
			key[3] = 1;
		}
		else if (oper[i] == '^' && question_type == 0) {//only in mode 0 would ^ be allowed to appear
			key[4] = 1;
		}
	}
	if (key[0] == 0 && key[1] == 0 && key[2] == 0 && key[3] == 0 && key[4] == 0) { //no supporting operator is not allowed
		flag = false;
		return -1;
	}
	if (key[0] == 0 && key[1] == 0 && key[2] == 0 && key[3] == 0 && key[4] == 1 && opernum > 1 && question_type == 0) {
		flag = false;
		return -1;
	}
	//if only ^ is supported in mode 0, then operator number should be 1
	if (key[0] == 0 && key[1] == 0 && key[2] == 0 && key[3] == 1 && key[4] == 0 && opernum > 1 && question_type == 0) {
		flag = false;
		return -1;
	}
	//if only / is supported in mode 0, then operator number should be 1
	if (lower_bound >= upper_bound) { //lower bound should not be larger than upper bound
		flag = false;
		return -1;
	}
	if (lower_bound < 0) { //lower bound should > 0
		flag = false;
		return -1;
	}

	ques_num = quesnum;
	oper_num = opernum;
	mode = question_type;
	oper_set.clear();
	if (key[0] == 1) {
		oper_set.push_back('+');
	}
	if (key[1] == 1) {
		oper_set.push_back('-');
	}
	if (key[2] == 1) {
		oper_set.push_back('*');
	}
	if (key[3] == 1) {
		oper_set.push_back('/');
	}
	if (key[4] == 1) {
		oper_set.push_back('^');
	}
	input_range.first = lower_bound;
	input_range.second = upper_bound;
	return 0;
}

void QuestionSetGenerator::Generate() {
	unordered_map<string, bool> expression_set;
	if (flag == false) cout << "Sorry,you have made some stange requests" << endl;
	else {
		MyQuestion* ques;
		if (mode == 0) {
			//ques = new MyQuestionInInt();
			ques = new MyQuestionInInt(oper_set, input_range);
		}
		else if (mode == 1) {
			//ques = new MyQuestionInFloat;
			ques = new MyQuestionInFloat(oper_set, input_range);
		}
		else {
			//ques = new MyQuestionInFraction;
			ques = new MyQuestionInFraction(oper_set, input_range);
		}
		srand((unsigned)time(0));
		unsigned long key = 0;
		ofstream formula("formula.txt");
		ofstream result("result.txt");
		if (!formula.is_open())
		{
			cout << "Cannot create the file" << endl;
		}
		if (!result.is_open())
		{
			cout << "Cannot create the file" << endl;
		}

		for (int i = 0; i < ques_num; i++) {
			key++;
			if (key > 1000000) {
				formula << "Cannot generate so many questions with given condition." << endl;
				result << "Cannot generate so many questions with given condition." << endl;
			}
			MyExpression temp;
			ques->GetQuestion(oper_num, temp);             //获取一个问题
			auto iter = expression_set.find(temp.expr_normalized);
			if (iter == expression_set.end()) {            //若该问题没有重复
				expression_set[temp.expr_normalized] = true;       //存入map
				if (mode == 1)
				{
					temp.expr_answer = to_string(Round(stod(temp.expr_answer), 2));
					temp.expr_answer = temp.expr_answer.substr(0, temp.expr_answer.size() - 4);
				}
				if (mode == 2) {
					int i = temp.expr_answer.find("/");
					if (i != -1) {
						string answer;
						int num1 = stoi(temp.expr_answer.substr(0, i));
						int num2 = stoi(temp.expr_answer.substr(i + 1));
						if (num1 > num2)
						{
							answer = to_string(num1 / num2);
							answer += '+';
							answer += to_string(num1%num2);
							answer += '/';
							answer += to_string(num2);
							temp.expr_answer = answer;
						}
					}
				}
				cout << temp.expr << " = " << temp.expr_answer << endl;
					formula << temp.expr << endl;
					result << temp.expr_answer << endl;
				//write file
			}
			else {                                   //若已存在这个问题
				i--;                                //这次循环不算数
			}
		}
		formula.close();
		result.close();
	}
}
	

