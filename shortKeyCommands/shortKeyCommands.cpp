#include "shortKeyCommands.hpp"

#include "config.hpp"
#include "../settings.hpp"
#include "../constructs.hpp"

#ifdef _WIN32
#include <conio.h>
#endif

#ifdef __linux__
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#endif


#include "../discordMessaging/discordMessaging.hpp"

shortKeyCommands::shortKeyCommands()
{
#ifdef __linux__
    enableRawMode();
#endif
}

void shortKeyCommands::bind(const int key, std::function<void()> action, const std::string& actionDescriptor)
{
    const auto& config = ConfigService::get();

    keyBindings_[tolower(key)] = std::move(action);
    const char c = toupper(static_cast<char>(key));

    if (config.modules.debug)
    {
        const std::string function = actionDescriptor.empty() ? "none" : actionDescriptor;
        bindingReport_ += std::string("**") + c + "** to function - **" + function + "**\\n";
    }
}

void shortKeyCommands::addTrigger(const int key, std::function<bool()> condition)
{
    triggers_.push_back({ std::move(condition), tolower(key) });
}

void shortKeyCommands::poll()
{
#ifdef _WIN32
    if (_kbhit())
    {
        const int key = tolower(_getch());
#elif __linux__
    if (kbhitLinux())
    {
        const int key = tolower(getchLinux());
#endif
        trigger(key);
    }

    for (const auto& [condition, key] : triggers_)
    {
        if (condition())
        {
            trigger(key);
        }
    }
}

void shortKeyCommands::flushBindingReport()
{
    const auto& config = ConfigService::get();

    if (!config.modules.debug || bindingReport_.empty())
        return;

    const std::string msg = "## Bound keys:\\n" + bindingReport_;

    DiscordMessenger::sendMessageToDiscord(msg, MessageType::INFO);
    bindingReport_.clear();
}

void shortKeyCommands::trigger(const int key)
{
    if (const auto it = keyBindings_.find(tolower(key)); it != keyBindings_.end())
    {
        const char c = toupper(static_cast<char>(key));
        DiscordMessenger::sendMessageToDiscord(std::string("Key **") + c + "** triggered");
        it->second();
    }
}

#ifdef __linux__
static struct termios orig_termios;

void shortKeyCommands::enableRawMode()
{
    tcgetattr(STDIN_FILENO, &orig_termios);
    termios raw = orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}

void shortKeyCommands::disableRawMode()
{
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
}

bool shortKeyCommands::kbhitLinux()
{
    timeval tv{0, 0};
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    return select(STDIN_FILENO+1, &fds, nullptr, nullptr, &tv) > 0;
}

int shortKeyCommands::getchLinux()
{
    char c;
    if (read(STDIN_FILENO, &c, 1) == 1)
        return c;
    return -1;
}
#endif