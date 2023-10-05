#include <exception>
#include <string>

class WSHException : public std::exception
{
public:
    std::string what_val;
    std::string action;
public:
    WSHException(const std::string& action, const std::string& what_val);

    char const* what() const override;
};