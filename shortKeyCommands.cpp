#include "shortKeyCommands.hpp"
#include "settings.hpp"
#include "constructs.hpp"

#include <conio.h>

#include "discordMessaging/discordMessaging.hpp"

shortKeyCommands::shortKeyCommands()
{
}

void shortKeyCommands::poll()
{
    if (_kbhit())
    {
        const int key = tolower(_getch());

        if (const auto it = keyBindings_.find(key); it != keyBindings_.end())
        {
            const char c = toupper(static_cast<char>(key));

            DiscordMessenger::sendMessageToDiscord(std::string("Key **") + c + "** pressed");

            it->second();
        }
    }
}

void shortKeyCommands::bind(const int key, std::function<void()> action, const std::string& actionDescriptor)
{
    keyBindings_[tolower(key)] = std::move(action);
    const char c = toupper(static_cast<char>(key));

    if (SETTINGS::CONTROL::MODULE::ENABLE_DEBUG)
    {
        const std::string function = actionDescriptor.empty() ? "none" : actionDescriptor;
        bindingReport_ += std::string("**") + c + "** to function - **" + function + "**\\n";
    }
}

void shortKeyCommands::flushBindingReport()
{
    if (!SETTINGS::CONTROL::MODULE::ENABLE_DEBUG || bindingReport_.empty())
        return;

    const std::string msg = "## Bound keys:\\n" + bindingReport_;

    DiscordMessenger::sendMessageToDiscord(msg, MessageType::INFO);
    bindingReport_.clear();
}
