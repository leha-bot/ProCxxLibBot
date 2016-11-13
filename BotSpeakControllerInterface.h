#pragma once
#include <string>

class BotSpeakControllerInterface {
	///
public:
	virtual void speak(const std::string &s) = 0; // ответ бота
	virtual void getCommand(std::string &s) = 0; // ввод команды
};


