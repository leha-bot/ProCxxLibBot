#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <initializer_list>

class BotSpeakControllerInterface {
	///
public:
	virtual void speak(const std::string &s) = 0; // ответ бота
	virtual void getCommand(std::string &s) = 0; // ввод команды
};

class BotSpeakStdoutController : public BotSpeakControllerInterface {
public:
	virtual void speak(const std::string &s) override
	{
		std::cout << s;
	}
	virtual void getCommand(std::string &s) override
	{
		getline(std::cin, s);
	}	
};

// expression ::=  <literal> | <command> 
// command ::= '/'<commandname> [[' '][commandparam]]* 
// commandname ::= <literal>
// commandparam ::= <literal>
//
class AbstractExpression {
public:
	virtual bool parse(const std::string &tok) = 0;
	virtual bool action() = 0;
};

class CommandList : public AbstractExpression {
	std::vector<AbstractExpression *> commands;

protected:

	enum hook_return_value {
		hook_ret_false,
		hook_ret_true,
		hook_continue_iteration,
	};

	hook_return_value doDefaultAction(AbstractExpression &cmd)
	{
		if (cmd.action()) {
			return hook_ret_true;
		}
		return hook_continue_iteration;
	}
	
	hook_return_value doDefaultParse(AbstractExpression &cmd,
		       	const std::string &tok)
	{
		if (cmd.parse(tok)) {
			return hook_ret_true;
		}
		return hook_continue_iteration;
	}
	
	virtual hook_return_value doParse(AbstractExpression &cmd,
		       	const std::string &tok)
	{
		return doDefaultParse(cmd, tok);
	}
	
	/// @brief Synonym for doParse(). Added for overloading 
	/// in template helper method.
	/// @see doParse()
	hook_return_value doAction(AbstractExpression &cmd,
		const std::string &tok)
	{
		return doParse(cmd, tok);
	}

	virtual hook_return_value doAction(AbstractExpression &cmd)
	{
		return doDefaultAction(cmd);
	}

	template <typename... Args> bool doTemplateAction(Args &&...args)
	{
		for (auto cmd : commands) {
			hook_return_value ret = doAction(*cmd, args...);
			
			if (ret == hook_continue_iteration) {
				continue;
			} else {
				return (bool) ret;
			}
		}
		return false;
	}

public:
	CommandList(std::initializer_list<AbstractExpression*> &il)
	{
		commands.resize(il.size());
		std::copy(il.begin(), il.end(), commands.begin());
	}

	virtual bool parse(const std::string &tok) override
	{
		for (auto cmd : commands) {
			hook_return_value ret = doAction(*cmd, tok);
			
			if (ret == hook_continue_iteration) {
				continue;
			} else {
				return (bool) ret;
			}
		}
		return false;
	}

	virtual bool action() override
	{
		/*for (auto cmd : commands) {
			if(cmd->action()) {
				return true;
			}
		}
		return false;*/
		return doTemplateAction();
	}
};

class Command;
class LiteralCommand;

class ExpressionCommand : public AbstractExpression {
	CommandList cmds;
	CommandList liters;
public:
	void addCommand();
	virtual bool parse(const std::string &tok) override
	{
		if (cmds.parse(tok)) {
			return true;
		} else {
			return liters.parse(tok);
		}
	}
};

class LiteralCommand : public AbstractExpression {
	std::string literal;
public:
	LiteralCommand(const std::string &s)
		: literal(s) {}

	bool parse(const std::string &s) override
	{
		return true; //s.compare(literal, literal.size();
	}
};

class BotCommand : public AbstractExpression {
	// Command cmd;
	// const std::
};

class Command : public AbstractExpression {
	CommandList opts;
	bool optional;
public:
	bool parse(const std::string &tok) override
	{
		if (tok[0] == '/') {
			// todo: cmd literals
			return true;
		}
		return false;
	}
};

class AddBookCommand {
};

const char *COMMAND_ADDBOOK = "/addbook";

class BotContext {
	BotSpeakControllerInterface *ctrl;
	BotExpressionList commands;
public:
	BotContext(BotSpeakControllerInterface &iface)
	{
		ctrl = &iface;
	}

};

void test()
{
	Bot

int main(int argc, char *argv[])
{
	std::string cmd;
	
	cmd.reserve(40);
	std::cout << "Enter your command: " << std::endl;
	getline(std::cin, cmd);
	std::cout << "Your command: " << cmd;
	if (cmd.compare(COMMAND_ADDBOOK) == 0) {
		std::cout << "Please, set the name of book: " << std::endl;
		getline(std::cin, cmd);
		std::cout << "Okay, the book name is: " << cmd << std::endl;
	}
}
