#include <all.hpp>

bool ignore(const WIN32_FIND_DATAW &data)
{
    auto name = utils::utf_convert(data.cFileName);
    if (name == "." || name == "..")
    {
        return true;
    }

    return false;
}

int main(int argc, const char **argv)
{
    auto directory = utils::get_working_directory();
    if (argc > 1)
    {
        directory = argv[1];
    }

    std::cout << "Content of " << directory << ":" << std::endl;

    std::vector<std::pair<int, std::pair<std::string, WIN32_FIND_DATAW>>> stack;
    for (const auto &child : utils::explore_directory(directory))
    {
        if (!ignore(child))
        {
            stack.push_back(std::make_pair(0, std::make_pair(directory, child)));
        }
    }

    unsigned long long bitmask = (unsigned long long)-1;
    while (!stack.empty())
    {
        const auto [level, file] = stack.back();
        const auto [path, data] = file;

        stack.pop_back();
        if (level > 63)
        {
            throw std::runtime_error("Too many levels of nesting");
        }

        bitmask |= ~((1ull << level) - 1);
        for (int i = 0; i < level; i++)
        {
            std::cout << ((bitmask & (1ull << i)) ? (char)179 : ' ') << "   "; // │
        }

        if (stack.empty() || stack.back().first < level)
        {
            std::cout << (char)192; // └
            bitmask &= ~(1ull << level);
        }
        else
        {
            std::cout << (char)195; // ├
        }
        std::cout << (char)196 << (char)196 << (char)196; // ───

        auto filename = utils::utf_convert(data.cFileName);
        std::cout << filename << '\n';

        if (
            (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
            !(data.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) &&
            !(data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) &&
            (filename != ".") &&
            (filename != ".."))
        {
            const auto new_path = utils::join(path, filename);
            for (const auto &child : utils::explore_directory(new_path))
            {
                if (!ignore(child))
                {
                    stack.push_back(std::make_pair(level + 1, std::make_pair(new_path, child)));
                }
            }
        }
    }

    return 0;
}
