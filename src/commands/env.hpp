#pragma once

#include <all.hpp>

class EnvCommand : public liteshell::BaseCommand
{
public:
    EnvCommand()
        : liteshell::BaseCommand(
              "env",
              "Display all environment variables",
              "",
              liteshell::CommandConstraint()) {}

    DWORD run(const liteshell::Context &context)
    {
        utils::Table displayer("Name", "Value");
        try
        {
            // This will throw std::runtime_error when running without a console (testing with pytest for example)
            std::size_t columns = utils::get_console_size().first;
            displayer.limits = {30, columns - 35};
        }
        catch (std::runtime_error &)
        {
            // pass
        }

        for (auto &[name, value] : context.client->get_environment()->get_values())
        {
            displayer.add_row(name, value);
        }

        std::cout << displayer.display() << std::endl;
        return 0;
    }
};
