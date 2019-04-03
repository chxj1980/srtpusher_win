// srtpusher_win.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include<string>
#include "CSender.h"

using namespace std;

int main()
{
	CSender sender;
	if (!sender.InitSender("192.168.8.117", 10000)) {
		printf("InitSender Failed!");
		return -1;
	}
	if (!sender.ConnectToServer()) {
		printf("ConnectToServer Failed!");
		return -1;
	}
	sender.StartPush(15,800000);
	//如果不阻塞，就直接析构了都
	int a;
	cin >> a;
	sender.StopPush();
}





