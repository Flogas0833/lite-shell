#pragma once

#include "format.hpp"
#include "standard.hpp"

/* Represents the argument constraints for a command */
class ArgumentsConstraint
{
private:
    std::vector<std::string> help;
    std::vector<std::pair<unsigned, unsigned>> bounds;
    std::map<std::string, unsigned> names;
    std::set<std::set<std::string>> alias_groups;

    ArgumentsConstraint(
        const bool require_context_parsing,
        const bool arguments_checking,
        const std::pair<unsigned, unsigned> &args_bounds)
        : require_context_parsing(require_context_parsing),
          arguments_checking(arguments_checking),
          args_bounds(args_bounds)
    {
        if (require_context_parsing && arguments_checking)
        {
            if (args_bounds.first > args_bounds.second)
            {
                throw std::invalid_argument("Lower bound of positional arguments must be less than or equal to the upper bound");
            }

            if (args_bounds.first == 0)
            {
                throw std::invalid_argument("Lower bound of positional arguments must be at least 1");
            }
        }
    }

public:
    /*
    @brief Whether this command require the context to be parsed. If this value if `false`, a context passed to the command callback
    will have `.args` and `.kwargs` set to empty containers.
    */
    const bool require_context_parsing;

    /*
    @brief Whether to perform arguments checking (check positional and named arguments) on command input. If `require_context_parsing`
    is `false`, this attribute has no effect.
    */
    const bool arguments_checking;

    /*
    The lower and upper bound of the number of positional arguments. If `require_context_parsing` or `arguments_checking` is
    `false`, this attribute has no effect.
    */
    const std::pair<unsigned, unsigned> args_bounds;

    /*
    @brief Construct an `ArgumentsConstraint` object with `require_context_parsing` set to `false`.
    */
    ArgumentsConstraint() : ArgumentsConstraint(false, false, std::make_pair(0, 0)) {}

    /*
    @brief Construct an `ArgumentsConstraint` object with `require_context_parsing` set to `true` and `arguments_checking` set to
    `false`.

    @param arguments_checking Must be `false`
    */
    ArgumentsConstraint(const bool arguments_checking) : ArgumentsConstraint(true, false, std::make_pair(0, 0))
    {
        if (arguments_checking)
        {
            throw std::invalid_argument("This overload does not allow arguments checking");
        }
    }

    /*
    @brief Construct an `ArgumentsConstraint` object with `require_context_parsing` and `arguments_checking` set to `true`.

    @param args_lower The lower bound of the number of positional arguments
    @param args_upper The upper bound of the number of positional arguments
    */
    ArgumentsConstraint(const unsigned args_lower, const unsigned args_upper)
        : ArgumentsConstraint(true, true, std::make_pair(args_lower, args_upper)) {}

    /* Throw an error if this constraint has `require_context_parsing` set to `false` */
    void check_context_parsing()
    {
        if (!require_context_parsing)
        {
            throw std::invalid_argument("ArgumentsConstraint object does not support context parsing");
        }
    }

    /*
    @brief Register a named argument constraint

    @param __name The name of the argument
    @param __help The help message of the argument
    @return A pointer to the current `ArgumentsConstraint` object
    */
    ArgumentsConstraint *add_argument(
        const std::string &__name,
        const std::string &__help,
        const unsigned lower_bound,
        const unsigned upper_bound,
        const bool add_to_alias_group = true)
    {
        check_context_parsing();
        if (has_argument(__name))
        {
            throw std::invalid_argument(format("Argument %s already exists", __name.c_str()));
        }

        if (help.size() != bounds.size())
        {
            throw std::runtime_error(format("Unexpected different sizes of .help and .bounds: %d vs %d", help.size(), bounds.size()));
        }

        if (lower_bound > upper_bound)
        {
            throw std::invalid_argument(format("lower_bound = %d > upper_bound = %d", lower_bound, upper_bound));
        }

        if (add_to_alias_group)
        {
            alias_groups.insert({__name});
        }

        names[__name] = help.size();
        help.push_back(__help);
        bounds.push_back(std::make_pair(lower_bound, upper_bound));

        return this;
    }

    template <typename... Args>
    ArgumentsConstraint *add_argument(const Args &...aliases, const std::string &help, const unsigned lower_bound, const unsigned upper_bound)
    {
        check_context_parsing();
        std::set<std::string> alias_group = {aliases...};
        alias_groups.insert(alias_group);
        for (auto &alias : {aliases...})
        {
            add_argument(alias, help, lower_bound, upper_bound, false);
        }

        return this;
    }

    /*
    @brief Whether the current constraint allows a specific named argument

    @param name The argument name (e.g. "-v")
    @return `true` if the argument is allowed, `false` otherwise
    */
    bool has_argument(const std::string &name) const
    {
        return names.count(name);
    }

    std::string get_help(const std::string &name) const
    {
        auto iter = names.find(name);
        if (iter == names.end())
        {
            throw std::invalid_argument(format("Argument %s does not exist", name.c_str()));
        }

        return help[iter->second];
    }

    std::pair<unsigned, unsigned> get_bounds(const std::string &name) const
    {
        auto iter = names.find(name);
        if (iter == names.end())
        {
            throw std::invalid_argument(format("Argument %s does not exist", name.c_str()));
        }

        return bounds[iter->second];
    }

    std::set<std::set<std::string>> get_alias_groups() const
    {
        return alias_groups;
    }
};
