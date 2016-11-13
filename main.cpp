#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <initializer_list>
#include <memory>
#include <sstream>
#include "BotSpeakControllerInterface.h"

class BotSpeakStdoutController : public BotSpeakControllerInterface {
public:
	virtual void speak(const std::string &s) override
	{
		std::cout << s << std::endl;
	}
	virtual void getCommand(std::string &s) override
	{
		getline(std::cin, s);
	}	
};

class BotCommandInterface {
public:
	virtual void command(const std::string &name) = 0;	
	virtual void literalCommand(const std::string &liter) = 0;	
	virtual const std::string &getDescription() = 0;
	virtual void getCommand() = 0;
};

// expression ::=  <literal> | <command> 
// command ::= '/'<commandname> [[' '][commandparam]]* 
// commandname ::= <literal>
// commandparam ::= <literal>
//
class AbstractExpression {
public:
	virtual bool parse(const std::string &tok,
			BotCommandInterface *iface) = 0;
};

class LiteralCommand : public AbstractExpression {
public:
	bool parse(const std::string &s,
		BotCommandInterface *iface) override
	{
		std::cout << "\x1b[1;31mDEBUG: literal: " << s << "\x1b[0m" << std::endl;
		iface->literalCommand(s);
		return true; //s.compare(literal, literal.size();
	}
};

class Command : public AbstractExpression {
public:
	bool parse(const std::string &tok,
		BotCommandInterface *iface) override
	{
		if (tok[0] == '/') {
			// todo: cmd literals
			iface->command(tok.substr(1));
			return true;
		}
		return false;
	}
};

class ExpressionCommand : public AbstractExpression {
	Command cmd;
	LiteralCommand liter;
public:
	virtual bool parse(const std::string &tok,
		BotCommandInterface *iface) override
	{
		if (cmd.parse(tok, iface)) {
			return true;
		} else {
			return liter.parse(tok, iface);
		}
	}
};

// bot states
// waitcmd <->waitname
class BotContext : public BotCommandInterface {
	BotSpeakControllerInterface *ctrl;
	ExpressionCommand parser;
	std::string description;
	bool requestExit;

public:
	struct BookEntry {
		std::string name;
		std::string desc;
	};
private:
	std::vector<BookEntry> books;
	
	bool addBook(const BookEntry &entry)
	{
		books.push_back(entry);
		return true;
	}

	size_t listBooks()
	{
		size_t count = 0;
		std::stringstream ss;
		ss << "All books list: " << std::endl;
		for (auto &e : books) {
			ss << ++count << ". " << e.name << ", description: " << e.desc << std::endl;
		}
		ss << "Total count: " << count;
		ctrl->speak(ss.str());
		return count;
	}
	
	void exit()
	{
		requestExit = true;
	}

	class BotCommand {
	public:
		virtual void processInput(const std::string &s, bool command = false) = 0;
	};
	
	/// @brief Incapsulates all command switch states.
	class BotStateManager {
		BotCommand *cur;
		BotContext *ctx;
		std::shared_ptr<BotCommand> states[4];
	public:
		class BotConcreteCommand : BotCommand {
			std::string name;
		};
		
		class BotUndefinedState : public BotCommand {
			BotStateManager *mgr; 
		public:
			BotUndefinedState(BotStateManager *mgr)
			{
				this->mgr = mgr;
			}

			void processInput(const std::string &s, bool command) {
				if (s.compare("start") != 0 || !command) {
					return;
				}
				mgr->setState(BotStateManager::Ready);
				mgr->getBotSpeakInterface()->speak(
					"Hi! I'm @ProCxxLibBot! I can add the book"
					" into the @ProCxxLib channel!\nType /addbook to begin!\n"
					"Type /help for help!");
			}
		};
		
		class BotReadyState : public BotCommand {
			BotStateManager *mgr; 
		public:
			BotReadyState(BotStateManager *mgr)
			{
				this->mgr = mgr;
			}

			void processInput(const std::string &s, bool command)
			{
				if (!command) {
					return;
				}

				if (s.compare("start") == 0 ||
				    s.compare("help")  == 0) {
					mgr->getBotSpeakInterface()->speak(
						"Hi again!"
						"Type /addbook to add the book into the channel!\n"
						"Type /list to show all your books.");
				} else if (s.compare("addbook") == 0) {
					mgr->getBotSpeakInterface()->speak(
							"Please, enter the name (/cancel to interrupt)");
					mgr->setState(BotStateManager::WaitForInputBook);

				} else if (s.compare("list") == 0) {
					mgr->getBotContext()->listBooks();
				} else if (s.compare("quit") == 0
					|| s.compare("exit") == 0) {
					mgr->getBotContext()->exit();
				}
			}
		};

		class BotWaitForInputBookState : public BotCommand {
			BotStateManager *mgr;
			bool descr;
			BotContext::BookEntry entry;
		public:
			BotWaitForInputBookState(BotStateManager *mgr)
			{
				descr = false;
				this->mgr = mgr;
			}

			void processInput(const std::string &s, bool command)
			{
				if(s.compare("cancel") == 0 && command) {
					mgr->getBotSpeakInterface()->speak(
							"Cancelled! Type /help for help.");
					descr = false;
					mgr->setState(Ready);
					return;
				}
				
				if (command)
					return;

				if (!descr) {
					entry.name = s;
					mgr->getBotSpeakInterface()->speak(
							"Now enter description");
				} else {
					entry.desc = s;
					mgr->getBotContext()->addBook(entry);
					mgr->getBotSpeakInterface()->speak(
							"Done! Now you can type /list"
							" to show the books or /addbook to add another.");
					mgr->setState(Ready);
				}
				descr = !descr;

			}
		};
		
		enum BotState {
			Undefined,
			Ready,
			WaitForInputBook,
		};
		
		BotStateManager(BotContext &ctx)
		{
			this->ctx = &ctx;
			states[0] = std::make_shared<BotUndefinedState>(this);
			states[1] = std::make_shared<BotReadyState>(this);
			states[2] = std::make_shared<BotWaitForInputBookState>(this);
			setState(Undefined);
		}

		BotContext *getBotContext()
		{
			return ctx;
		}

		BotSpeakControllerInterface *getBotSpeakInterface()
		{
			return ctx->getBotSpeakInterface();
		}

		void setState(BotState st)
		{
			cur = states[st].get();
		}

		BotCommand *getState()
		{
			return cur;
		}
		
	};
	BotStateManager mgr;
public:

	// ctor
	BotContext(BotSpeakControllerInterface &iface,
			const std::string &descript)
		:mgr(*this)
	{
		ctrl = &iface;
		description = descript;
		// mgr = BotStateManager(*this);
	}

	bool exitIsRequested()
	{
		return requestExit;
	}	

	void getCommand() override
	{
		std::string s;
		ctrl->getCommand(s);
		parser.parse(s, this);
	}

	void command(const std::string &name) override
	{
		mgr.getState()->processInput(name, true);
	}

	void literalCommand(const std::string &liter) override
	{
		mgr.getState()->processInput(liter, 0);
	}

	const std::string &getDescription() override
	{
		return description;
	}

	BotSpeakControllerInterface *getBotSpeakInterface()
	{
		return ctrl;
	}
};

int main(int argc, char *argv[])
{
	BotSpeakStdoutController speak;
	BotContext bot(speak, "This bot can add your book into the library.");
	speak.speak(bot.getDescription());
	while (bot.exitIsRequested() != true) {
		bot.getCommand();
	}

}
