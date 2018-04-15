#pragma once
#include "stdafx.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <time.h>
using namespace std;
class __declspec(dllexport)  QuestionSetGenerator {

public:
	int ques_num;                         //题目的数量
	int oper_num;                         //一个表达式中运算符的数量
	vector<char> oper_set;               //运算符的种类
	int mode;                            //0为整数，1为小数，2为分数
	pair<int, int> input_range;          //题目数值的范围
	double Round(double dVal, short iPlaces);
	QuestionSetGenerator();
	int Setting(int question_number, int operator_number, string oper, int question_type, int lower_bound, int upper_bound);                   //读文件并更改设置
	void Generate();                     //产生题目并写文件
	bool flag;
};
