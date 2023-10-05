#include <exception>
#include <string>

/// <summary>
/// Exceptions that occure while processing AudioHub websocket messages.
/// </summary>
class WSHException : public std::exception
{
public:
    std::string what_val;
    std::string action;
public:
    WSHException(const std::string& action, const std::string& what_val);

    char const* what() const override;
};